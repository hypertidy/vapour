#ifndef GDALWARPMEM_H
#define GDALWARPMEM_H

//#define DBL_EPSILON 2.2204460492503131e-16

#include <Rcpp.h>
#include "gdal.h"
#include "gdal_alg.h"
#include "gdal_alg_priv.h"
#include "gdal_priv.h"
#include "gdalwarper.h"
#include "gdal_utils.h"
//#include "gdal_version.h"

#include "cpl_conv.h" // for CPLMalloc()
#include "cpl_string.h" // for CSLAddString

namespace gdalwarpmem{


using namespace Rcpp;

inline List gdal_warp_in_memory(CharacterVector source_filename,
                                CharacterVector source_WKT,
                                CharacterVector target_WKT,
                                NumericVector target_extent,
                                IntegerVector target_dim,
                                IntegerVector bands,
                                NumericVector source_extent,
                                CharacterVector resample,
                                LogicalVector silent) {


  GDALAllRegister();
  GDALDatasetH poSrcDS;
  //poSrcDS = static_cast<GDALDataset *>(CPLRealloc(poSrcDS, sizeof(GDALDataset) * 1)); //source_filename.size()));
  poSrcDS = GDALOpen((const char *) source_filename[0], GA_ReadOnly);


  if (poSrcDS == NULL) {
    //Rprintf("cannot open %s\n", source_filename[0]);
    Rprintf("failed to open\n");
    Rcpp::stop("\n");
  }
  if (source_extent.length() == 1) {
    // do nothing
  } else {
    int XSize = GDALGetRasterXSize(poSrcDS);
    int YSize = GDALGetRasterYSize(poSrcDS);

    double SourceGeoTransform[6] = {0.0, 0.0, 0.0, 0.0, 0.0, 0.0};
    SourceGeoTransform[0] = source_extent[0];
    SourceGeoTransform[1] = (source_extent[1] - source_extent[0])/XSize;

    SourceGeoTransform[3] = source_extent[3];
    SourceGeoTransform[5] = -(source_extent[3] - source_extent[2])/YSize;

    GDALSetGeoTransform( poSrcDS, SourceGeoTransform );
  }
  char** papszArg = nullptr;
  // https://github.com/OSGeo/gdal/blob/fec15b146f8a750c23c5e765cac12ed5fc9c2b85/gdal/frmts/gtiff/cogdriver.cpp#L512
  papszArg = CSLAddString(papszArg, "-of");
  papszArg = CSLAddString(papszArg, "MEM");
  // papszArg = CSLAddString(papszArg, "-wo");
  // papszArg = CSLAddString(papszArg, "INIT_DEST=NODATA");

  // if we don't supply it don't try to set it!
  if (!target_WKT[0].empty()){
    // if supplied check that it's valid
    OGRSpatialReference oTargetSRS;
    OGRErr target_chk =  oTargetSRS.SetFromUserInput(target_WKT[0]);
    if (target_chk != OGRERR_NONE) Rcpp::stop("cannot initialize target projection");

    papszArg = CSLAddString(papszArg, "-t_srs");
    papszArg = CSLAddString(papszArg, target_WKT[0]);
  }
  if (source_WKT[0].empty()) {
    // TODO check source projection is valid
    // const char* srcproj = nullptr;
    // srcproj = GDALGetProjectionRef(po_SrcDS[0]);
    // if ((srcproj != NULL) && (srcproj[0] == '\0')) {
    //   GDALClose(po_SrcDS[0]);
    //   Rcpp::stop("no valid source projection in source\n");
    // }
  } else {
    if (silent[0] != true) Rprintf("setting projection");

    // if supplied check that it's valid
    OGRSpatialReference oSourceSRS;
    OGRErr source_chk =  oSourceSRS.SetFromUserInput(source_WKT[0]);
    if (source_chk != OGRERR_NONE) Rcpp::stop("cannot initialize source projection");

    papszArg = CSLAddString(papszArg, "-s_srs");
    papszArg = CSLAddString(papszArg, source_WKT[0]);

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
  int bHasNoData = FALSE;

  //  const auto poFirstBand = poSrcDS->GetRasterBand(1);
  const auto poFirstBand = GDALGetRasterBand(poSrcDS, 1);


  //  poSrcDS->GetRasterBand(1)->GetNoDataValue(&bHasNoData);
  int aerr;
  bHasNoData =  GDALGetRasterNoDataValue(poFirstBand, &aerr);


  // if( !bHasNoData && CPLTestBool(CSLFetchNameValueDef(
  //     papszOptions, "ADD_ALPHA", "YES")) )
  // {
  //   papszArg = CSLAddString(papszArg, "-dstalpha");
  // }
  papszArg = CSLAddString(papszArg, "-r");
  papszArg = CSLAddString(papszArg, resample[0]);
  papszArg = CSLAddString(papszArg, "-wo");
  papszArg = CSLAddString(papszArg, "SAMPLE_GRID=YES");

  papszArg = CSLAddString(papszArg, "-wo");
  papszArg = CSLAddString(papszArg, "SOURCE_EXTRA=64");

  //  papszArg = CSLAddString(papszArg, "-wo");
  // papszArg = CSLAddString(papszArg, "NUM_THREADS=ALL_CPUS");

  const bool bHasMask = GDALGetMaskFlags(poFirstBand);

  const int nBands = GDALGetRasterCount(poSrcDS);
  int band_length = bands.size();
  if (bands.size() == 1 && bands[0] == 0) {
    band_length = nBands;
  }

  std::vector<int> bands_to_read(band_length);
  if (bands.size() == 1 && bands[0] == 0) {
    for (int i = 0; i < nBands; i++) bands_to_read[i] = i + 1;
   // Rprintf("index bands\n");
  } else {
    for (int i = 0; i < bands.size(); i++) bands_to_read[i] = bands[i];
    // Rprintf("input bands\n");
  }
  Rcpp::List outlist(bands_to_read.size());

  auto psOptions = GDALWarpAppOptionsNew(papszArg, nullptr);
  CSLDestroy(papszArg);


 // CPLDebug("MEM", "Reprojecting source dataset: start");
  GDALWarpAppOptionsSetProgress(psOptions, NULL, NULL );
  // CPLString osTmpFile(GetTmpFilename(pszDstFilename, "warped.tif.tmp"));
  // auto hSrcDS = GDALDataset::ToHandle(&poSrcDS);


  auto hRet = GDALWarp( "", nullptr,
                        1, &poSrcDS,
                        psOptions, nullptr);

  CPLAssert( hRet != NULL );


  GDALWarpAppOptionsFree(psOptions);
  CPLDebug("MEM", "Reprojecting source dataset: end");

  //  GDALDestroyScaledProgress(pScaledProgress);

  double naflag;
  int hasNA;
  int hasScale, hasOffset;
  double scale, offset;

  CPLErr err;
  GDALRasterBandH dstBand, poBand;
  for (int iband = 0; iband < bands_to_read.size(); iband++) {
    if (bands_to_read[iband] > nBands) {
      GDALClose( hRet );
      GDALClose( poSrcDS );
      Rcpp::stop("band requested exceeds bound count: %i (source has %i band/s)", bands[iband], nBands);
    }
    //Rprintf("bands_to_read[i] %i\n", bands_to_read[iband]);
    poBand = GDALGetRasterBand(poSrcDS, bands_to_read[iband]);
    dstBand = GDALGetRasterBand(hRet, bands_to_read[iband]);
    naflag = GDALGetRasterNoDataValue(dstBand, &hasNA);


    scale = GDALGetRasterScale(poBand, &hasScale);
    offset = GDALGetRasterOffset(poBand, &hasOffset);

    int actual_XSize = GDALGetRasterBandXSize(dstBand);
    int actual_YSize = GDALGetRasterBandYSize(dstBand);
    std::vector<double> double_scanline( actual_XSize * actual_YSize );
    err = GDALRasterIO(dstBand,  GF_Read, 0, 0, actual_XSize, actual_YSize,
                       &double_scanline[0], actual_XSize, actual_YSize, GDT_Float64,
                       0, 0);
    NumericVector res(actual_XSize * actual_YSize );

    // consider doing at R level, at least for MEM
    double dval;
    if (hasNA && (!std::isnan(naflag))) {
      if (naflag < -3.4e+37) {
       naflag = -3.4e+37;

        for (size_t i=0; i< double_scanline.size(); i++) {
          if (double_scanline[i] <= naflag) {
            double_scanline[i] = NAN;
          }
        }
      } else {

        std::replace(double_scanline.begin(), double_scanline.end(), naflag, (double) NAN);
      }
    }
    for (int i = 0; i < (double_scanline.size()); i++) {
      dval = double_scanline[i];
      if (hasScale) dval = dval * scale;
      if (hasOffset) dval = dval + offset;
      res[i] = dval;
    }

    outlist[iband] = res;
  }
  GDALClose( hRet );
  GDALClose( poSrcDS );


  // CPLFree(double_scanline);

  return outlist;
}

} // namespace gdalwarpmem
#endif
