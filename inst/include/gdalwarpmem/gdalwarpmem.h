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
                                CharacterVector resample) {


  char** papszArg = nullptr;
  // https://github.com/OSGeo/gdal/blob/fec15b146f8a750c23c5e765cac12ed5fc9c2b85/gdal/frmts/gtiff/cogdriver.cpp#L512
  papszArg = CSLAddString(papszArg, "-of");
  papszArg = CSLAddString(papszArg, "MEM");
  papszArg = CSLAddString(papszArg, "-co");
  papszArg = CSLAddString(papszArg, "TILED=NO");
  papszArg = CSLAddString(papszArg, "-co");
  papszArg = CSLAddString(papszArg, "SPARSE_OK=NO");

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
    Rprintf("setting projection");
    papszArg = CSLAddString(papszArg, "-s_srs");
    papszArg = CSLAddString(papszArg, source_WKT[0]);

  }


  papszArg = CSLAddString(papszArg, "-te");
  papszArg = CSLAddString(papszArg, CPLSPrintf("%.18g,", dfMinX));
  papszArg = CSLAddString(papszArg, CPLSPrintf("%.18g,", dfMinY));
  papszArg = CSLAddString(papszArg, CPLSPrintf("%.18g,", dfMaxX));
  papszArg = CSLAddString(papszArg, CPLSPrintf("%.18g,", dfMaxY));
  papszArg = CSLAddString(papszArg, "-ts");
  papszArg = CSLAddString(papszArg, CPLSPrintf("%d", nXSize));
  papszArg = CSLAddString(papszArg, CPLSPrintf("%d", nYSize));
  int bHasNoData = FALSE;
  poSrcDS->GetRasterBand(1)->GetNoDataValue(&bHasNoData);
  if( !bHasNoData && CPLTestBool(CSLFetchNameValueDef(
      papszOptions, "ADD_ALPHA", "YES")) )
  {
    papszArg = CSLAddString(papszArg, "-dstalpha");
  }
  papszArg = CSLAddString(papszArg, "-r");
  papszArg = CSLAddString(papszArg, osResampling);
  papszArg = CSLAddString(papszArg, "-wo");
  papszArg = CSLAddString(papszArg, "SAMPLE_GRID=YES");
  const char* pszNumThreads = CSLFetchNameValue(papszOptions, "NUM_THREADS");
  if( pszNumThreads )
  {
    papszArg = CSLAddString(papszArg, "-wo");
    papszArg = CSLAddString(papszArg, (CPLString("NUM_THREADS=") + pszNumThreads).c_str());
  }

  const auto poFirstBand = poSrcDS->GetRasterBand(1);
  const bool bHasMask = poFirstBand->GetMaskFlags() == GMF_PER_DATASET;

  const int nBands = poSrcDS->GetRasterCount();
  const char* pszOverviews = CSLFetchNameValueDef(
    papszOptions, "OVERVIEWS", "AUTO");
  const bool bRecreateOvr = EQUAL(pszOverviews, "FORCE_USE_EXISTING") ||
    EQUAL(pszOverviews, "NONE");
  dfTotalPixelsToProcess =
    double(nXSize) * nYSize * (nBands + (bHasMask ? 1 : 0)) +
    ((bHasMask && !bRecreateOvr) ? double(nXSize) * nYSize / 3 : 0) +
    (!bRecreateOvr ? double(nXSize) * nYSize * nBands / 3: 0) +
    double(nXSize) * nYSize * (nBands + (bHasMask ? 1 : 0)) * 4. / 3;

  auto psOptions = GDALWarpAppOptionsNew(papszArg, nullptr);
  CSLDestroy(papszArg);
  if( psOptions == nullptr )
    return nullptr;

  const double dfNextPixels =
    double(nXSize) * nYSize * (nBands + (bHasMask ? 1 : 0));
  void* pScaledProgress = GDALCreateScaledProgress(
    dfCurPixels / dfTotalPixelsToProcess,
    dfNextPixels / dfTotalPixelsToProcess,
    pfnProgress, pProgressData );
  dfCurPixels = dfNextPixels;

  CPLDebug("COG", "Reprojecting source dataset: start");
  GDALWarpAppOptionsSetProgress(psOptions, GDALScaledProgress, pScaledProgress );
  CPLString osTmpFile(GetTmpFilename(pszDstFilename, "warped.tif.tmp"));
  auto hSrcDS = GDALDataset::ToHandle(poSrcDS);
  auto hRet = GDALWarp( osTmpFile, nullptr,
                        1, &hSrcDS,
                        psOptions, nullptr);
  GDALWarpAppOptionsFree(psOptions);
  CPLDebug("COG", "Reprojecting source dataset: end");

  GDALDestroyScaledProgress(pScaledProgress);

  double naflag = GDALGetRasterNoDataValue(poBand, &hasNA);
  Rprintf("%f\n", naflag);
  int hasScale, hasOffset;
  double scale, offset;
  // if (hasNA && naflag < -3.4e+37) {  // hack from terra
  //   naflag = -3.4e+37;
  // }
 //GDALFillRaster(aBand, naflag, 0);
  hDstDS = GDALWarp(NULL, hDstDS, 1, po_SrcDS, psOptions, &err_0);
 // GDALRasterBandH aBand = GDALGetRasterBand(hDstDS, band[0]);
//  GDALSetRasterNoDataValue(aBand, naflag);


  double *double_scanline;
  double_scanline = (double *) CPLMalloc(sizeof(double)*static_cast<unsigned long>(target_dim[0]) * static_cast<unsigned long>(target_dim[1]));

  CPLErr err;
  Rcpp::List outlist(band.size());

  for (int iband = 0; iband < band.size(); iband++) {
    dstBand = GDALGetRasterBand(hDstDS, band[iband]);
    naflag = GDALGetRasterNoDataValue(poBand, &hasNA);

    if (hasScale) scale = GDALGetRasterScale(poBand, &hasScale);
    if (hasOffset) offset = GDALGetRasterOffset(poBand, &hasOffset);
    Rprintf("%f\n", naflag);
    Rprintf("%f\n", scale);
    Rprintf("%f\n", offset);

    err = GDALRasterIO(dstBand,  GF_Read, 0, 0, target_dim[0], target_dim[1],
                       double_scanline, target_dim[0], target_dim[1], GDT_Float64,
                       0, 0);
    NumericVector res(target_dim[0] * target_dim[1]);

    // all this bs should be done at the R level, at least for MEM
    for (int i = 0; i < (target_dim[0] * target_dim[1]); i++) {
      double dval = double_scanline[i];
      if (dval == naflag) {
        //Rprintf("%f/n", DBL_EPSILON);
        res[i] = NA_REAL;
      } else {
        if (hasScale) dval = dval * scale;
        if (hasOffset) dval = dval + offset;

        res[i] = dval;
      }
    }
    outlist[iband] = res;
  }
  GDALClose( hDstDS );
  //  for (int i = 0; i < source_filename.size(); i++) {
  GDALClose( po_SrcDS[0] );
  //  }
  CPLFree(double_scanline);
  GDALWarpAppOptionsFree(psOptions);
  return outlist;
}

} // namespace gdalwarpmem
#endif
