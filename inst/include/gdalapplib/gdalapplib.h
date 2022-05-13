#ifndef GDALAPPLIB_H
#define GDALAPPLIB_H
#include <Rcpp.h>
#include "gdal_priv.h"
#include "gdal_utils.h"  // for GDALTranslateOptions
#include "vrtdataset.h"
#include "cpl_string.h"



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

} //  namespace gdalapplib
#endif
