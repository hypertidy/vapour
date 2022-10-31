#include <Rcpp.h>
using namespace Rcpp;
#include "gdal_priv.h"
#include "ogrsf_frmts.h"

#include "gdalraster/gdalraster.h"

// seee some examples in https://github.com/hypertidy/vapour/issues/127#issuecomment-1279661138

// [[Rcpp::export]]
SEXP gh_GDALOpenEx(CharacterVector dsn) {
  
  GDALAllRegister();
  // create pointer to an GDAL object and
  // wrap it as an external pointer
  GDALDataset       *poDS;
  poDS = (GDALDataset*) GDALOpenEx(dsn[0], GDAL_OF_READONLY, NULL, NULL, NULL);
  //poDS = (GDALDataset*) GDALOpen(dsn[0], GDAL_OF_READONLY);

  if( poDS == NULL )
  {
    Rprintf("Problem with 'dsn' input: %s\n", dsn[0]);
    Rcpp::stop("Open failed.\n");
  }
  Rcpp::XPtr<GDALDataset> ptr(poDS);
  
  return ptr;
}

// [[Rcpp::export]]
SEXP gh_GDALWarpDataset(SEXP xp,  IntegerVector dimension, NumericVector extent, CharacterVector projection) {
  
  Rcpp::XPtr<GDALDataset> ptr(xp);  // create pointer to an GDAL object and
  
  // we always provide extent and dimension, crs is optional and just means subset/decimate
  double dfMinX = extent[0];
  double dfMaxX = extent[1];
  double dfMinY = extent[2];
  double dfMaxY = extent[3];
  
  char** papszArg = nullptr;
  
  papszArg = CSLAddString(papszArg, "-of");
  papszArg = CSLAddString(papszArg, "VRT");
  
  papszArg = CSLAddString(papszArg, "-te");
  papszArg = CSLAddString(papszArg, CPLSPrintf("%.18g,", dfMinX));
  papszArg = CSLAddString(papszArg, CPLSPrintf("%.18g,", dfMinY));
  papszArg = CSLAddString(papszArg, CPLSPrintf("%.18g,", dfMaxX));
  papszArg = CSLAddString(papszArg, CPLSPrintf("%.18g,", dfMaxY));
  
  // we otherwise set a dud dimension, the user didn't set it (so they get native for the extent)
  if (dimension.size() > 1) {
    int nXSize = dimension[0];
    int nYSize = dimension[1];
    papszArg = CSLAddString(papszArg, "-ts");
    papszArg = CSLAddString(papszArg, CPLSPrintf("%d", nXSize));
    papszArg = CSLAddString(papszArg, CPLSPrintf("%d", nYSize));
  }
  
  // should check the source is ok
  if (!EQUAL(projection[0], "")) {
    papszArg = CSLAddString(papszArg, "-t_srs"); 
    papszArg = CSLAddString(papszArg, (char *) projection[0]); 
  }
  // papszArg = CSLAddString(papszArg, "-r");
  // papszArg = CSLAddString(papszArg, resample[0]);
  
  // for (int gwopt = 0; gwopt < options.length(); gwopt++) {
  //   papszArg = CSLAddString(papszArg, options[gwopt]);
  // }
  auto psOptions = GDALWarpAppOptionsNew(papszArg, nullptr);
  CSLDestroy(papszArg);
  GDALWarpAppOptionsSetProgress(psOptions, NULL, NULL );
  
  GDALDataset* poDS = (GDALDataset*)GDALWarp( "", nullptr,
                       static_cast<int>(dsn.size()), poaDS,
                       psOptions, nullptr);
  
  Rcpp::XPtr<GDALDataset> ptr(poDS);
  
  return ptr;
}
// [[Rcpp::export]]
SEXP gh_GDALWarp(CharacterVector dsn, IntegerVector sds, IntegerVector dimension, NumericVector extent, CharacterVector projection) {
  
 GDALDatasetH* poaDS = gdalraster::gdalH_open_multiple(dsn, sds); 

  // we always provide extent and dimension, crs is optional and just means subset/decimate
  double dfMinX = extent[0];
  double dfMaxX = extent[1];
  double dfMinY = extent[2];
  double dfMaxY = extent[3];
  
  char** papszArg = nullptr;
  
  papszArg = CSLAddString(papszArg, "-of");
  papszArg = CSLAddString(papszArg, "VRT");
  
  papszArg = CSLAddString(papszArg, "-te");
  papszArg = CSLAddString(papszArg, CPLSPrintf("%.18g,", dfMinX));
  papszArg = CSLAddString(papszArg, CPLSPrintf("%.18g,", dfMinY));
  papszArg = CSLAddString(papszArg, CPLSPrintf("%.18g,", dfMaxX));
  papszArg = CSLAddString(papszArg, CPLSPrintf("%.18g,", dfMaxY));
  
  // we otherwise set a dud dimension, the user didn't set it (so they get native for the extent)
  if (dimension.size() > 1) {
    int nXSize = dimension[0];
    int nYSize = dimension[1];
    papszArg = CSLAddString(papszArg, "-ts");
    papszArg = CSLAddString(papszArg, CPLSPrintf("%d", nXSize));
    papszArg = CSLAddString(papszArg, CPLSPrintf("%d", nYSize));
  }
  
  // should check the source is ok
  if (!EQUAL(projection[0], "")) {
    papszArg = CSLAddString(papszArg, "-t_srs"); 
    papszArg = CSLAddString(papszArg, (char *) projection[0]); 
  }
 // papszArg = CSLAddString(papszArg, "-r");
 // papszArg = CSLAddString(papszArg, resample[0]);
  
  // for (int gwopt = 0; gwopt < options.length(); gwopt++) {
  //   papszArg = CSLAddString(papszArg, options[gwopt]);
  // }
  auto psOptions = GDALWarpAppOptionsNew(papszArg, nullptr);
  CSLDestroy(papszArg);
  GDALWarpAppOptionsSetProgress(psOptions, NULL, NULL );
  
  GDALDataset* poDS = (GDALDataset*)GDALWarp( "", nullptr,
                                static_cast<int>(dsn.size()), poaDS,
                                psOptions, nullptr);

  Rcpp::XPtr<GDALDataset> ptr(poDS);
   
  return ptr;
}

inline GDALRasterIOExtraArg init_resample_alg(CharacterVector resample) {
  GDALRasterIOExtraArg psExtraArg;
  INIT_RASTERIO_EXTRA_ARG(psExtraArg);
  if (resample[0] == "average") {
    psExtraArg.eResampleAlg = GRIORA_Average;
  }
  if (resample[0] == "bilinear") {
    psExtraArg.eResampleAlg = GRIORA_Bilinear;
  }
  if (resample[0] == "cubic") {
    psExtraArg.eResampleAlg = GRIORA_Cubic;
  }
  if (resample[0] == "cubicspline") {
    psExtraArg.eResampleAlg = GRIORA_CubicSpline;
  }
  if (resample[0] == "gauss") {
    psExtraArg.eResampleAlg = GRIORA_Gauss;
  }
  if (resample[0] == "lanczos") {
    psExtraArg.eResampleAlg = GRIORA_Lanczos;
  }
  if (resample[0] == "mode") {
    psExtraArg.eResampleAlg = GRIORA_Mode;
  }
  if (resample[0] == "nearestneighbour") {
    psExtraArg.eResampleAlg = GRIORA_NearestNeighbour;
  }
  return psExtraArg;
}
// [[Rcpp::export]]
SEXP gh_GDALRasterio(SEXP xp, IntegerVector window, CharacterVector resample) {
  
  Rcpp::XPtr<GDALDataset> ptr(xp);  // create pointer to an GDAL object and

  int Xoffset;
  int Yoffset;
  int nXSize;
  int nYSize;
  
  int outXSize;
  int outYSize;
  
  int nx = ptr->GetRasterXSize(); 
  int ny = ptr->GetRasterYSize(); 
  
  if (window.size() == 6) {
    Xoffset = window[0];
    Yoffset = window[1];
    nXSize = window[2];
    nYSize = window[3];
    outXSize = window[4];
    outYSize = window[5];
  } else {
    Xoffset = 0L;
    Yoffset = 0L;
    nXSize = nx; 
    nYSize = ny;
    outXSize = nx;
    outYSize = ny; 
  }  
  
  int nBands = ptr->GetRasterCount();
  size_t n_values_out = static_cast<size_t>(outXSize * outYSize) * nBands;
 
  GDALRasterIOExtraArg psExtraArg;
  psExtraArg = init_resample_alg((char *)resample[0]);
  CPLErr err;
  
  std::vector<double> double_scanline( n_values_out);
  err = ptr->RasterIO(GF_Read, Xoffset, Yoffset, nXSize, nYSize,
         &double_scanline[0], outXSize, outYSize, GDT_Float64,
         nBands, NULL, 
         0, 0, 0, &psExtraArg);
  
  NumericVector res(outXSize * outYSize  );
  
  R_xlen_t isi;
  double dval;
  for (isi = 0; isi < (double_scanline.size()); isi++) {
    dval = double_scanline[static_cast<size_t>(isi)];
   // if (hasScale) dval = dval * scale;
  //if (hasOffset) dval = dval + offset;
    res[isi] = dval;
  }
  return res; 
}

// [[Rcpp::export]]
SEXP gh_GDALGetDescription(SEXP xp) {
  
  Rcpp::XPtr<GDALDataset> ptr(xp);  // create pointer to an GDAL object and
  CharacterVector desc = Rcpp::wrap(ptr->GetDescription()); 
  
  return desc;
}
// [[Rcpp::export]]
SEXP gh_GDALGetGeoTransform(SEXP xp) {
  
  Rcpp::XPtr<GDALDataset> ptr(xp);  // create pointer to an GDAL object and

  NumericVector res(6) ;
  //std::vector<double> geotransform( 6);
  double        adfGeoTransform[6];
  //poDataset->GetGeoTransform( adfGeoTransform );
  GDALGetGeoTransform(ptr, adfGeoTransform );
  
  double dval;
  for (int isi = 0; isi < 6; isi++) {
    dval = adfGeoTransform[isi];
    res[isi] = dval;
  }
  return res; 
}


// [[Rcpp::export]]
SEXP gh_GDALGetFileList(SEXP xp) {
  
  Rcpp::XPtr<GDALDataset> ptr(xp);  // create pointer to an GDAL object and
  char **filelist = ptr->GetFileList(); 
  int fdi = 0;
  while (filelist && filelist[fdi] != NULL) {
    fdi++; // count
  }
  CharacterVector res(fdi); 
  for (int i = 0; i <- res.length(); i++) {
    res[i] = (char *) filelist[i]; 
  }
  CSLDestroy(filelist);
  return res; 
}
// [[Rcpp::export]]
SEXP gh_GDALGetRasterSize(SEXP xp) {
  
  Rcpp::XPtr<GDALDataset> ptr(xp);  // create pointer to an GDAL object and
  
  NumericVector res(2) ;
  
  res[0] = ptr->GetRasterXSize(); 
  res[1] = ptr->GetRasterYSize(); 
  
  return res; 
}
/// invoke the close method
// [[Rcpp::export]]
SEXP gh_GDALClose(SEXP xp) {
  
  // grab the object as a XPtr (smart pointer)
  // to GDALDataset
  Rcpp::XPtr<GDALDataset> ptr(xp);
  NumericVector res(1);
  res[0] = 0;
  GDALClose(ptr);
  res[0] = 1;
  // return the result to R
  return res;
}
// ExecuteSQL
// [[Rcpp::export]]
SEXP gh_ExecuteSQL(SEXP xp, CharacterVector sql, SEXP extent) {
  Rcpp::XPtr<GDALDataset> ptr(xp);
  OGRLayer  *poLayer;
  poLayer = ptr->ExecuteSQL(sql[0], NULL, NULL);
  
  Rcpp::XPtr<OGRLayer> out_ptr(poLayer);
  
  return out_ptr;
}

// [[Rcpp::export]]
SEXP gh_GetLayer(SEXP xp, IntegerVector layer) {
  // this, or harmless emssage from XPtr below?
  // if (R_ExternalPtrAddr(xp) == NULL)  {
  //   Rcpp::stop("dataset pointer is nil");
  // }
  
  Rcpp::XPtr<GDALDataset> ptr(xp);
  
  OGRLayer  *poLayer;
  poLayer = ptr->GetLayer(layer[0]);
  poLayer->ResetReading();
  Rcpp::XPtr<OGRLayer> out_ptr(poLayer);
  return out_ptr;
}
// [[Rcpp::export]]
SEXP gh_GetNextFeature(SEXP xp) {
  XPtr<OGRLayer> lyr(xp);
  OGRFeature *poFeature;
  
  //lyr->ResetReading();
  poFeature = lyr->GetNextFeature();
  // double* pdfProgressPct; 
  // GDALProgressFunc pfnProgress; 
  // void* pProgressData; 
  // poFeature = GDALDataset::GetNextFeature(lyr, pdfProgressPct, pfnProgress, pProgressData); 
  // //double nFeature = (double)
  
  //  poFeature = ptr->GetNextFeature();
  XPtr<OGRFeature> out_ptr(poFeature);
  return out_ptr;
}
// [[Rcpp::export]]
SEXP gh_DestroyFeature(SEXP xp) {
  XPtr<OGRFeature> feature(xp);
  
  OGRFeature::DestroyFeature(feature);
  return Rcpp::wrap(1);
}
// [[Rcpp::export]]
SEXP gh_getGeometryRef(SEXP xp) {
  XPtr<OGRFeature> feature(xp);

  OGRGeometry *poGeometry;
  poGeometry = feature->GetGeometryRef();
  XPtr<OGRGeometry> out_ptr(poGeometry);
  return out_ptr;
}
// [[Rcpp::export]]
SEXP gh_getPoints_preview(SEXP xp) {
  XPtr<OGRGeometry> poGeometry(xp); 
  OGRGeometryH hR  = OGR_G_GetGeometryRef(poGeometry, 0); 
  
  int nn = 4; 
  int stride = sizeof(double), n = OGR_G_GetPointCount(hR);
  if (n < nn) nn = n; 
  SEXP x, y, z;
  x = PROTECT(Rf_allocVector(REALSXP, n));
  y = PROTECT(Rf_allocVector(REALSXP, n));
  z = PROTECT(Rf_allocVector(REALSXP, n));
  OGR_G_GetPoints(hR, REAL(x), stride, REAL(y), stride, REAL(z), stride);
  SEXP vec = PROTECT(Rf_allocVector(VECSXP, 3));
  SET_VECTOR_ELT(vec, 0, x);
  SET_VECTOR_ELT(vec, 1, y);
  SET_VECTOR_ELT(vec, 2, z);
  
  UNPROTECT(4); 
  return vec; 
}


// [[Rcpp::export]]
SEXP gh_exportToWkb(SEXP xp) {
  XPtr<OGRGeometry> geom(xp);
  Rcpp::RawVector raw(geom->WkbSize());
  geom->exportToWkb(wkbNDR, &(raw[0]), wkbVariantIso);
  return Rcpp::wrap(raw);
}
// surely no point to a hook like this, we just want a src/ wrap to do what gdal_list_drivers() gets
// [[Rcpp::export]]
SEXP gh_GetGDALDriverManager()  {
  GDALDriverManager *gdm  = GetGDALDriverManager();
  Rcpp::XPtr<GDALDriverManager> ptr(gdm);
  return ptr;
}

// but I'm practicing the art ...
// [[Rcpp::export]]
SEXP gh_GetDriverCount(SEXP xp)  {
  
  Rcpp::XPtr<GDALDriverManager> ptr(xp);
  int n = ptr->GetDriverCount();
  return Rcpp::wrap(n);
}


