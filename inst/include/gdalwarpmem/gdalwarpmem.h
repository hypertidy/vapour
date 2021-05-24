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
                                NumericVector target_geotransform,
                                IntegerVector target_dim,
                                IntegerVector band,
                                NumericVector source_geotransform,
                                CharacterVector resample,
                                LogicalVector silent) {

  Rcpp::List outlist(band.size());

  GDALAllRegister();
  GDALDatasetH poSrcDS;
  //poSrcDS = static_cast<GDALDataset *>(CPLRealloc(poSrcDS, sizeof(GDALDataset) * 1)); //source_filename.size()));
  poSrcDS = GDALOpen((const char *) source_filename[0], GA_ReadOnly);


  char** papszArg = nullptr;
  // https://github.com/OSGeo/gdal/blob/fec15b146f8a750c23c5e765cac12ed5fc9c2b85/gdal/frmts/gtiff/cogdriver.cpp#L512
  papszArg = CSLAddString(papszArg, "-of");
  papszArg = CSLAddString(papszArg, "MEM");
  // papszArg = CSLAddString(papszArg, "-wo");
  // papszArg = CSLAddString(papszArg, "INIT_DEST=NODATA");

  papszArg = CSLAddString(papszArg, "-t_srs");
  papszArg = CSLAddString(papszArg, target_WKT[0]);

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
    papszArg = CSLAddString(papszArg, "-s_srs");
    papszArg = CSLAddString(papszArg, source_WKT[0]);

  }


  double dfMinX = target_geotransform[0];
  double dfMaxY = target_geotransform[3];
  double dfMaxX = target_geotransform[0] + target_dim[0] * target_geotransform[1];
  double dfMinY = target_geotransform[3] + target_dim[1] * target_geotransform[5];

  int nXSize = target_dim[0];
  int nYSize = target_dim[1];
  papszArg = CSLAddString(papszArg, "-te");
  papszArg = CSLAddString(papszArg, CPLSPrintf("%.18g,", dfMinX));
  papszArg = CSLAddString(papszArg, CPLSPrintf("%.18g,", dfMinY));
  papszArg = CSLAddString(papszArg, CPLSPrintf("%.18g,", dfMaxX));
  papszArg = CSLAddString(papszArg, CPLSPrintf("%.18g,", dfMaxY));
  papszArg = CSLAddString(papszArg, "-ts");
  papszArg = CSLAddString(papszArg, CPLSPrintf("%d", nXSize));
  papszArg = CSLAddString(papszArg, CPLSPrintf("%d", nYSize));
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


  const bool bHasMask = GDALGetMaskFlags(poFirstBand);

  const int nBands = GDALGetRasterCount(poSrcDS);

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

  // double *double_scanline;
  // double_scanline = (double *) CPLMalloc(sizeof(double)*static_cast<unsigned long>(target_dim[0]) * static_cast<unsigned long>(target_dim[1]));
  std::vector<double> double_scanline( target_dim[0] * target_dim[1] );
  CPLErr err;
  GDALRasterBandH dstBand, poBand;

  for (int iband = 0; iband < band.size(); iband++) {
    poBand = GDALGetRasterBand(poSrcDS, band[iband]);
    dstBand = GDALGetRasterBand(hRet, band[iband]);
    naflag = GDALGetRasterNoDataValue(dstBand, &hasNA);


    scale = GDALGetRasterScale(poBand, &hasScale);
    offset = GDALGetRasterOffset(poBand, &hasOffset);


    err = GDALRasterIO(dstBand,  GF_Read, 0, 0, target_dim[0], target_dim[1],
                       &double_scanline[0], target_dim[0], target_dim[1], GDT_Float64,
                       0, 0);
    NumericVector res(target_dim[0] * target_dim[1]);

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
    for (int i = 0; i < (target_dim[0] * target_dim[1]); i++) {
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
