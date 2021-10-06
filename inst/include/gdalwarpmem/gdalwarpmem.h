#ifndef GDALWARPMEM_H
#define GDALWARPMEM_H

#include <Rcpp.h>
#include "gdal_priv.h"
#include "gdalwarper.h"
#include "gdal_utils.h"  // for GDALWarpAppOptions

namespace gdalwarpmem{

using namespace Rcpp;


// this function, should live in gdallibrary and also be used by gdal_raster_io()
List get_band_values(GDALDatasetH hRet, 
                     std::vector<int> bands_to_read, 
                     CharacterVector band_output_type, 
                     double scale, double offset, 
                     int hasScale, int hasOffset, int hasNA,
                     GDALDataType src_band_type) {
  // if band_output_type is not empty, possible override:
  // complex types not supported Byte, UInt16, Int16, UInt32, Int32, Float32, Float64
  if (band_output_type[0] == "Byte")   src_band_type = GDT_Byte;
  if (band_output_type[0] == "UInt16") src_band_type = GDT_UInt16;
  if (band_output_type[0] == "Int16")  src_band_type = GDT_Int16;
  
  if (band_output_type[0] == "UInt32") src_band_type = GDT_UInt32;
  if (band_output_type[0] == "Int32") src_band_type = GDT_Int32;
  
  if (band_output_type[0] == "Float32") src_band_type = GDT_Float32;
  if (band_output_type[0] == "Float64") src_band_type = GDT_Float64;
  
  
  int sbands = (int)bands_to_read.size();
  Rcpp::List outlist(bands_to_read.size());
  
  GDALRasterBandH rasterBand; 
  for (int iband = 0; iband < sbands; iband++) {
    rasterBand = GDALGetRasterBand(hRet, bands_to_read[iband]);
    int actual_XSize = GDALGetRasterBandXSize(rasterBand);
    int actual_YSize = GDALGetRasterBandYSize(rasterBand);
    
    // if hasScale we ignore integer or byte and go with float
    if ((src_band_type == GDT_Float64) | (src_band_type == GDT_Float32) | hasScale) {
      std::vector<double> double_scanline( actual_XSize * actual_YSize );
      CPLErr err;
      err = GDALRasterIO(rasterBand,  GF_Read, 0, 0, actual_XSize, actual_YSize,
                         &double_scanline[0], actual_XSize, actual_YSize, GDT_Float64,
                         0, 0);
      if (err) Rprintf("we have a problem at RasterIO\n");
      NumericVector res(actual_XSize * actual_YSize );
      
      // consider doing at R level, at least for MEM
      double dval;
      double naflag = GDALGetRasterNoDataValue(rasterBand, &hasNA);
      
      if (hasNA && (!std::isnan(naflag))) {
        if (naflag < -3.4e+37) {
          naflag = -3.4e+37;
          
          for (size_t i=0; i< double_scanline.size(); i++) {
            if (double_scanline[i] <= naflag) {
              double_scanline[i] = NA_REAL; 
            }
          }
        } else {
          
          std::replace(double_scanline.begin(), double_scanline.end(), naflag, (double) NAN);
        }
      }
      long unsigned int isi;
      for (isi = 0; isi < (double_scanline.size()); isi++) {
        dval = double_scanline[isi];
        if (hasScale) dval = dval * scale;
        if (hasOffset) dval = dval + offset;
        res[isi] = dval;
      }
      outlist[iband] = res;
    }
    // if hasScale we assume to never use scale/offset in integer case (see block above we already dealt)
    if (!hasScale & 
        ((src_band_type == GDT_Int16) |
        (src_band_type == GDT_Int32) |
        (src_band_type == GDT_UInt16) |
        (src_band_type == GDT_UInt32)))  {
      std::vector<int32_t> integer_scanline( actual_XSize * actual_YSize );
      CPLErr err;
      err = GDALRasterIO(rasterBand,  GF_Read, 0, 0, actual_XSize, actual_YSize,
                         &integer_scanline[0], actual_XSize, actual_YSize,
                         GDT_Int32,
                         0, 0);
      if (err) Rprintf("we have a problem at RasterIO\n");
      IntegerVector res(actual_XSize * actual_YSize );
      
      // consider doing at R level, at least for MEM
      int dval;
      int naflag = GDALGetRasterNoDataValue(rasterBand, &hasNA);
      
      if (hasNA && (!std::isnan(naflag))) {
        std::replace(integer_scanline.begin(), integer_scanline.end(), naflag, (int) NAN);
        
      }
      long unsigned int isi;
      for (isi = 0; isi < (integer_scanline.size()); isi++) {
        dval = integer_scanline[isi];
        if (hasScale) dval = dval * scale;
        if (hasOffset) dval = dval + offset;
        res[isi] = dval;
      }
      outlist[iband] = res;
    }
    
    // if hasScale we assume to never use scale/offset in integer case (see block above we already dealt)
    
    if (!hasScale & (src_band_type == GDT_Byte)) {
      std::vector<uint8_t> byte_scanline( actual_XSize * actual_YSize );
      CPLErr err;
      err = GDALRasterIO(rasterBand,  GF_Read, 0, 0, actual_XSize, actual_YSize,
                         &byte_scanline[0], actual_XSize, actual_YSize,
                         GDT_Byte,
                         0, 0);
      if (err) Rprintf("we have a problem at RasterIO\n");
      RawVector res(actual_XSize * actual_YSize );
      uint8_t naflag = GDALGetRasterNoDataValue(rasterBand, &hasNA);
      
      if (hasNA && (!std::isnan(naflag))) {
        std::replace(byte_scanline.begin(), byte_scanline.end(), naflag, (uint8_t) NAN);
        
      }
      long unsigned int isi;
      for (isi = 0; isi < (byte_scanline.size()); isi++) {
        res[isi] = byte_scanline[isi];
      }
      outlist[iband] = res;
    }
    
    
    
  }
  return outlist; 
}

inline List gdal_warp_in_memory(CharacterVector source_filename,
                                CharacterVector source_WKT,
                                CharacterVector target_WKT,
                                NumericVector target_extent,
                                IntegerVector target_dim,
                                IntegerVector bands,
                                NumericVector source_extent,
                                CharacterVector resample,
                                LogicalVector silent,
                                CharacterVector band_output_type, 
                                CharacterVector warp_options, 
                                CharacterVector transformation_options, 
                                CharacterVector cutline_filename) {
  
  
  
  GDALDatasetH *poSrcDS;
  GDALAllRegister();
  poSrcDS = static_cast<GDALDatasetH *>(CPLMalloc(sizeof(GDALDatasetH) * source_filename.size()));
  
  char** papszArg = nullptr;
  int dsi0 = 0;
  for (int si = 0; si < source_filename.size(); si++) {
    poSrcDS[si] = GDALOpen((const char *) source_filename[si], GA_ReadOnly);
    
    if (poSrcDS[si] == NULL) {
      //Rprintf("cannot open %s\n", source_filename[0]);
      
      if (si > 0) {
        for (int ii = 0; ii < si; ii++) {
          GDALClose( poSrcDS[ii] );
        }
      }
      Rprintf("failed to open\n");
      Rcpp::stop("\n");
    }
    if (source_extent.length() == 1) {
      // do nothing
    } // and else also do nothing (doesn't work how I thought, only VRT can do this so wait for /vrt/)
    
    // https://github.com/OSGeo/gdal/blob/fec15b146f8a750c23c5e765cac12ed5fc9c2b85/gdal/frmts/gtiff/cogdriver.cpp#L512
    papszArg = CSLAddString(papszArg, "-of");
    papszArg = CSLAddString(papszArg, "MEM");
    
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
      const char* srcproj = nullptr;
      srcproj = GDALGetProjectionRef(poSrcDS[si]);
      if ((srcproj != NULL) && (srcproj[0] == '\0') && !target_WKT[0].empty()) {
        Rcpp::warning("no valid projection in source, specified output projection will have no effect\n (only the extent and dimension will be applied natively)\n use 'source_projection' to apply the correct details for this source");
      }
    } else {
      if (silent[0] != true) Rprintf("setting projection");
      if (si == 0) {
        // if supplied check that it's valid
        OGRSpatialReference oSourceSRS;
        OGRErr source_chk =  oSourceSRS.SetFromUserInput(source_WKT[0]);
        if (source_chk != OGRERR_NONE) Rcpp::stop("cannot initialize source projection");
        
        papszArg = CSLAddString(papszArg, "-s_srs");
        papszArg = CSLAddString(papszArg, source_WKT[0]);
      }
    }
  }//si
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
  
  if (!cutline_filename[0].empty()) {
    papszArg = CSLAddString(papszArg, "-cutline");
    papszArg = CSLAddString(papszArg, cutline_filename[0]);
    
  }
  
  papszArg = CSLAddString(papszArg, "-r");
  papszArg = CSLAddString(papszArg, resample[0]);
  
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

  auto hRet = GDALWarp( "", nullptr,
                        source_filename.size(), poSrcDS,
                        psOptions, nullptr);
  CPLAssert( hRet != NULL );
  
  GDALWarpAppOptionsFree(psOptions);
  
  
  const int nBands = GDALGetRasterCount(hRet); 
  //Rprintf("%i source bands\n", nBands);
  //Rprintf("%i target bands\n", nOutBands);
  
  int band_length = bands.size();
  if (bands.size() == 1 && bands[0] == 0) {
    band_length = nBands;
  }
  
  std::vector<int> bands_to_read(band_length);
  if (bands.size() == 1 && bands[0] == 0) {
    for (int i = 0; i < nBands; i++) bands_to_read[i] = i + 1;
  } else {
    for (int i = 0; i < bands.size(); i++) {
      bands_to_read[i] = bands[i];
        if (bands[i] > nBands) {
        GDALClose( hRet );
        for (int si = 0; si < source_filename.size(); si++) {
          GDALClose( poSrcDS[si] );
        }
        Rcpp::stop("band requested exceeds bound count: %i (source has %i band/s)", bands[i], nBands);
      }
    }
  }
  
  int hasNA;
  int hasScale, hasOffset;
  double scale, offset;
  
  GDALRasterBandH srcBand;
  // use first band of first data source as scale/offset ref, so it's assumed to apply the same to all
  srcBand = GDALGetRasterBand(poSrcDS[0], 1);
  
  
  scale = GDALGetRasterScale(srcBand, &hasScale);
  offset = GDALGetRasterOffset(srcBand, &hasOffset);
  GDALDataType band_type =  GDALGetRasterDataType(srcBand);
  
  // now close the sources
  for (int si = 0; si < source_filename.size(); si++) {
    GDALClose( poSrcDS[si] );
  }
  
  List outlist = get_band_values(hRet, 
                       bands_to_read, 
                       band_output_type, 
                       scale, offset, 
                       hasScale, hasOffset, hasNA,
                       band_type); 
    
  GDALClose( hRet );
  return outlist;
}




} // namespace gdalwarpmem
#endif
