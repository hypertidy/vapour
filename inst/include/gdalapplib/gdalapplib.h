#ifndef GDALAPPLIB_H
#define GDALAPPLIB_H
#include <Rcpp.h>
#include "gdal_priv.h"
#include "gdal_utils.h"  // for GDALTranslateOptions
#include "vrtdataset.h"
#include "cpl_string.h"
#include "ogr_spatialref.h"  // for OGRCreateCoordinateTransformation


#include "gdalwarper.h"
//#include "gdal_utils.h"  // for GDALWarpAppOptions


// 2022-05-13 taken from https://github.com/diminutive/gdalmin/tree/15f9b57304eb833511cf37967c148cc63e852e20 
// from apps/gdal_utils_priv.h
struct GDALInfoOptionsForBinary
{
  /* Filename to open. */
  char* pszFilename;
  
  /* Open options. */
  char** papszOpenOptions;
  
  /* > for reporting on a particular subdataset */
  int nSubdataset;
  
  /* Allowed input drivers. */
  char** papszAllowInputDrivers;
};


// from apps/gdalinfo_bin.cpp
/************************************************************************/
/*                         GDALInfoOptionsForBinary()                   */
/************************************************************************/

static GDALInfoOptionsForBinary *GDALInfoOptionsForBinaryNew(void)
{
  return static_cast<GDALInfoOptionsForBinary *>(
      CPLCalloc(1, sizeof(GDALInfoOptionsForBinary)));
}

// from apps/gdalinfo_bin.cpp
/************************************************************************/
/*                       GDALInfoOptionsForBinaryFree()                 */
/************************************************************************/

static void GDALInfoOptionsForBinaryFree( GDALInfoOptionsForBinary* psOptionsForBinary )
{
  if( psOptionsForBinary )
  {
    CPLFree(psOptionsForBinary->pszFilename);
    CSLDestroy(psOptionsForBinary->papszOpenOptions);
    CSLDestroy(psOptionsForBinary->papszAllowInputDrivers);
    CPLFree(psOptionsForBinary);
  }
}

namespace gdalapplib {
using namespace Rcpp;


// GDALInfo() itself requires GDAL 2.1
//
// Note: doesn't sf do this? yes, but not vectorized in cpp (not slow though), not callable from Cpp, doesn't work with subdatasets (that is is gdalinfo_bin.cpp)
// [[Rcpp::export]]
inline CharacterVector gdalinfo_applib_cpp(CharacterVector dsn,
                              CharacterVector options) {
  
  // just always have these, then add json/cat, min_max/mm/stats/approx_stats, checksum, wkt_format, oo, if
  // CharacterVector options = CharacterVector({"-proj4","-listmdd"});
  // new arg [-json] or just concat print output
  // new args (min_max): [-mm] [-stats | -approx_stats]
  // [-hist]
  // ignore [-nogcp]
  // ignore [-nomd]
  // ignore [-norat]
  // ignore [-noct]
  // ignore [-nofl]
  // new arg [-checksum]
  // always [-proj4]
  // always [-listmdd]
  // all [-mdd domain|`all`]*
  // new arg [-wkt_format WKT1|WKT2|...]  WKT1|WKT2|WKT2_2015|WKT2_2018  (this only in GDAL 3.0)
  // new arg (sds) [-sd subdataset]
  // new arg [-oo NAME=VALUE]*
  // new arg [-if format]*                                               (this only in GDAL 3.2)
  
  
  CharacterVector out(dsn.size());
  CPLStringList argv;
  argv.AddString(dsn[0]);
  
  // not this way
  //argv = CSLAddString(argv, "gdalinfo");
  //argv = CSLAddString(argv, "sst.tif");
  //argv = CSLAddString(argv, "-stdout");
  //GDALInfoOptionsForBinary* psOptionsForBinary = GDALInfoOptionsForBinaryNew();
  //GDALInfoOptions *psOptions = GDALInfoOptionsNew(argv+1, psOptionsForBinary);
  // this way, and (I think) don't destroy or free
  //argv.AddString("-stdout");   // internally gdalinfo applies this
  // if (json[0]) argv.AddString("-json");
  // if (stats[0]) argv.AddString("-stats");
  // if (checksum[0]) argv.AddString("-checksum");
  //
  // argv.AddString(CPLSPrintf("-wkt_format %s", wkt_format[0]));
  for (int i = 0; i < options.size(); i++) {
    argv.AddString(options[i]);
    //Rprintf("%s\n", (char *)options[i]);
  }
  GDALInfoOptionsForBinary* psOptionsForBinary = GDALInfoOptionsForBinaryNew();
  // WARN: we aren't using psOptions->pszFilename
  GDALInfoOptions *psOptions = GDALInfoOptionsNew(argv.List(), psOptionsForBinary);
  
  for (int iDSN = 0; iDSN < dsn.size(); iDSN++)
  {
    
    /* -------------------------------------------------------------------- */
    /*      Open dataset.                                                   */
    /* -------------------------------------------------------------------- */
    
    GDALDatasetH hDataset
    = GDALOpenEx( dsn[iDSN],
                  //psOptionsForBinary->pszFilename
                  GDAL_OF_READONLY | GDAL_OF_RASTER | GDAL_OF_VERBOSE_ERROR,
                  psOptionsForBinary->papszAllowInputDrivers,
                  psOptionsForBinary->papszOpenOptions, nullptr );
    
    if( hDataset == nullptr )
    {
      Rprintf("gdalinfo - unable to open '%s'.\n",
              (char *) dsn[iDSN] );
      out[iDSN] = NA_STRING;
      // //gdalinfo_bin.cpp has further logic here for vsitar/vsizip to list contents
    } else {
      
      
      /* -------------------------------------------------------------------- */
      /*      Read specified subdataset if requested.                         */
      /* -------------------------------------------------------------------- */
      if ( psOptionsForBinary->nSubdataset > 0 )
      {
        char **papszSubdatasets = GDALGetMetadata( hDataset, "SUBDATASETS" );
        int nSubdatasets = CSLCount( papszSubdatasets );
        
        if ( nSubdatasets > 0 && psOptionsForBinary->nSubdataset <= nSubdatasets )
        {
          char szKeyName[1024];
          char *pszSubdatasetName;
          
          snprintf( szKeyName, sizeof(szKeyName),
                    "SUBDATASET_%d_NAME", psOptionsForBinary->nSubdataset );
          szKeyName[sizeof(szKeyName) - 1] = '\0';
          pszSubdatasetName =
            CPLStrdup( CSLFetchNameValue( papszSubdatasets, szKeyName ) );
          GDALClose( hDataset );
          hDataset = GDALOpen( pszSubdatasetName, GA_ReadOnly );
          CPLFree( pszSubdatasetName );
        } else
        {
          Rprintf(
            "gdalinfo warning: subdataset %d of %d requested. Reading the main dataset.\n",
            psOptionsForBinary->nSubdataset, nSubdatasets );
        }
      }
      
      
      char* pszGDALInfoOutput = GDALInfo( hDataset, psOptions );
      if( pszGDALInfoOutput ) {
        out[iDSN] = pszGDALInfoOutput;
      } else {
        out[iDSN] = NA_STRING;
      }
      CPLFree( pszGDALInfoOutput );
      GDALClose(hDataset);
    } // hDataset == nullptr
  } // iDSN
  
  GDALInfoOptionsForBinaryFree(psOptionsForBinary);
  GDALInfoOptionsFree( psOptions );
  //GDALDumpOpenDatasets( stderr );
  //GDALDestroyDriverManager();
  //CPLDumpSharedList( nullptr );
  return out;
}









inline List gdalwarp_applib(CharacterVector source_filename,
                                CharacterVector target_crs,
                                NumericVector target_extent,
                                IntegerVector target_dim,
                                CharacterVector target_filename,
                                IntegerVector bands,
                                CharacterVector resample,
                                LogicalVector silent,
                                CharacterVector band_output_type, 
                                CharacterVector warp_options, 
                                CharacterVector transformation_options) {
  
  
  GDALDatasetH *poSrcDS;
  poSrcDS = static_cast<GDALDatasetH *>(CPLMalloc(sizeof(GDALDatasetH) * (size_t) source_filename.size()));
  
  
  for (int i = 0; i < source_filename.size(); i++) {
     poSrcDS[i] = GDALOpen(source_filename[i], GA_ReadOnly); 
      
    // unwind everything, and stop
    if (poSrcDS[i] == nullptr) {
      if (i > 0) {
        for (int j = 0; j < i; j++) GDALClose(poSrcDS[j]);
      }
      Rprintf("input source not readable: %s\n", (char *)source_filename[i]); 
      
      CPLFree(poSrcDS);
      Rcpp::stop(""); 
    }
  }
  
  // handle warp settings and options
  char** papszArg = nullptr;
  
  papszArg = CSLAddString(papszArg, "-of");
  if (target_filename.size() < 1) {
    papszArg = CSLAddString(papszArg, "MEM");
  } else {
    papszArg = CSLAddString(papszArg, "COG");
  }
  
  // if we don't supply it don't try to set it!
  if (!target_crs[0].empty()){
    // if supplied check that it's valid
    OGRSpatialReference *oTargetSRS = nullptr;
    oTargetSRS = new OGRSpatialReference;
    OGRErr target_chk =  oTargetSRS->SetFromUserInput(target_crs[0]);
    if (oTargetSRS != nullptr) {
      delete oTargetSRS; 
    }
    if (target_chk != OGRERR_NONE) {
      Rcpp::stop("cannot initialize target projection");
    }
    const char *st = NULL;
    st = ((GDALDataset *)poSrcDS[0])->GetProjectionRef(); 
    
    if(*st == '\0')	{
      Rcpp::stop( "Transformation to this target CRS not possible from this source dataset, target CRS given: \n\n %s \n\n", 
                  (char *)  target_crs[0] );
    }

    papszArg = CSLAddString(papszArg, "-t_srs");
  papszArg = CSLAddString(papszArg, target_crs[0]);
    
 }
  
  // we always provide extent and dimension, crs is optional and just means subset/decimate
  double dfMinX = target_extent[0];
  double dfMaxX = target_extent[1];
  double dfMinY = target_extent[2];
  double dfMaxY = target_extent[3];
  
  papszArg = CSLAddString(papszArg, "-te");
  papszArg = CSLAddString(papszArg, CPLSPrintf("%.18g,", dfMinX));
  papszArg = CSLAddString(papszArg, CPLSPrintf("%.18g,", dfMinY));
  papszArg = CSLAddString(papszArg, CPLSPrintf("%.18g,", dfMaxX));
  papszArg = CSLAddString(papszArg, CPLSPrintf("%.18g,", dfMaxY));
  
  // we otherwise set a dud dimension, the user didn't set it (so they get native for the extent)
  if (target_dim.size() > 1) {
    int nXSize = target_dim[0];
    int nYSize = target_dim[1];
    papszArg = CSLAddString(papszArg, "-ts");
    papszArg = CSLAddString(papszArg, CPLSPrintf("%d", nXSize));
    papszArg = CSLAddString(papszArg, CPLSPrintf("%d", nYSize));
  }
  
  
  papszArg = CSLAddString(papszArg, "-r");
  papszArg = CSLAddString(papszArg, resample[0]);
  
  papszArg = CSLAddString(papszArg, "-multi");
  //papszArg = CSLAddString(papszArg, "-wo");
  //  papszArg = CSLAddString(papszArg, "NUM_THREADS=ALL_CPUS");  
  

  // bundle on all user-added options
  for (int wopt = 0; wopt < warp_options.length(); wopt++) {
    papszArg = CSLAddString(papszArg, "-wo");
    papszArg = CSLAddString(papszArg, warp_options[wopt]);
  }
  for (int topt = 0; topt < transformation_options.length(); topt++) {
    papszArg = CSLAddString(papszArg, "-to");
    papszArg = CSLAddString(papszArg, transformation_options[topt]);
  }
  
  auto psOptions = GDALWarpAppOptionsNew(papszArg, nullptr);
  CSLDestroy(papszArg);
  GDALWarpAppOptionsSetProgress(psOptions, NULL, NULL );
 
  GDALDatasetH hRet = GDALWarp( target_filename[0], nullptr,
                                (int)source_filename.size(), poSrcDS,
                                psOptions, nullptr);
  
  
  CPLAssert( hRet != NULL );
  GDALWarpAppOptionsFree(psOptions);
  for (int si = 0; si < source_filename.size(); si++) {
    GDALClose( (GDALDataset *)poSrcDS[si] );
  }
  CPLFree(poSrcDS);
  
  GDALClose((GDALDataset*) hRet);
  List out(0);
  return out;
}
} //  namespace gdalapplib
#endif
