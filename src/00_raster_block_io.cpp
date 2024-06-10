#include <Rcpp.h>
#include "gdalreadwrite/gdalreadwrite.h"
#include "gdalraster/gdalraster.h"

using namespace Rcpp;


// [[Rcpp::export]]
Rcpp::List vapour_read_raster_block_cpp(CharacterVector dsource,
                                        IntegerVector offset, IntegerVector dimension, IntegerVector band,
                                        CharacterVector band_output_type, 
                                        LogicalVector unscale, LogicalVector nara) {
  return gdalreadwrite::gdal_read_block(dsource, offset, dimension, band, band_output_type, unscale, nara);
}

// [[Rcpp::export]]
Rcpp::LogicalVector vapour_write_raster_block_cpp(CharacterVector dsource, NumericVector data,
                                                  IntegerVector offset, IntegerVector dimension, IntegerVector band) {
  return gdalreadwrite::gdal_write_block(dsource, data, offset, dimension, band);
}



// [[Rcpp::export]]
Rcpp::CharacterVector vapour_create_copy_cpp(CharacterVector dsource, CharacterVector dtarget, CharacterVector driver) {
  return gdalreadwrite::gdal_create_copy(dsource, dtarget, driver);
}

// [[Rcpp::export]]
Rcpp::CharacterVector vapour_create_cpp(CharacterVector filename, CharacterVector driver,
                                        NumericVector extent, IntegerVector dimension,
                                        CharacterVector projection,
                                        IntegerVector n_bands, 
                                        CharacterVector datatype,
                                        List options_list_pairs) {
  return gdalreadwrite::gdal_create(filename, driver, extent, dimension, projection, n_bands, datatype, options_list_pairs);   
}


// [[Rcpp::export]]
Rcpp::NumericVector vapour_read_raster_value_cpp(CharacterVector dsource,
                                                 IntegerVector col, IntegerVector row,  IntegerVector band,
                                                 CharacterVector band_output_type) {
  IntegerVector dimension(2); 
  dimension[0] = 1; 
  dimension[1] = 1; 
  IntegerVector offset(2);
  IntegerVector sds0 = IntegerVector::create(0); 
  GDALDatasetH ds = gdalraster::gdalH_open_dsn(dsource[0],   sds0);  
  
  NumericVector vals(col.size()); 
  NumericVector L(1); 
  IntegerVector window(6); 
  window[2] = 1;
  window[3] = 1;
  window[4] = 1;
  window[5] = 1;
  LogicalVector tst(1);
  CharacterVector type(1); 
  type[0] = "Float64";
  CharacterVector resample(1); 
  resample[0] = "near";
  NumericVector v(1); 
  tst[0] = false; 
  std::vector<int> b = {1};
  if (band[0] < 1) stop("invalid band number"); 
  if (band[0] > ((GDALDataset *)ds)->GetRasterCount()) stop("invalid band number"); 
  GDALRasterBand * poBand = ((GDALDataset*) ds)->GetRasterBand(band[0]); 
  GDALRasterIOExtraArg psExtraArg;
  psExtraArg = gdalraster::init_resample_alg(resample); 
  CPLErr err; 
  
  for (int i = 0; i < col.size(); i++) {
    
    err = poBand->RasterIO(GF_Read, col[i], row[i], 1, 1,
                           &vals[i], 1, 1, GDT_Float64,
                           0, 0, &psExtraArg);
    
  }
  GDALClose(ds); 
  return vals; 
}


// [[Rcpp::export]]
Rcpp::List blocks_cpp1(CharacterVector dsource, IntegerVector iblock, LogicalVector read) {                                                 

  //https://gdal.org/doxygen/classGDALRasterBand.html#a09e1d83971ddff0b43deffd54ef25eef
  IntegerVector sds0 = IntegerVector::create(0); 
  GDALDatasetH ds = gdalraster::gdalH_open_dsn(dsource[0],   sds0);  
  GDALRasterBand * poBand = ((GDALDataset*) ds)->GetRasterBand(1); 
  
  if (! (poBand->GetRasterDataType() == GDT_Float32 )) Rcpp::stop("\n");
  
  int nXBlockSize, nYBlockSize;
  poBand->GetBlockSize( &nXBlockSize, &nYBlockSize );
  
  List out(1);
  int cnt = 0; 
  //GByte *pabyData = (GByte *) CPLMalloc(nXBlockSize * nYBlockSize);
//  std::vector<float> faBlockData( static_cast<size_t>( nXBlockSize * nYBlockSize ));
  float *faBlockData = (float *) CPLMalloc(nXBlockSize * nYBlockSize); 
  int iXBlock = 0;
  int iYBlock = 0; 
  int        nXValid, nYValid;
  // Compute the portion of the block that is valid
  // for partial edge blocks.
  poBand->GetActualBlockSize(iXBlock, iYBlock, &nXValid, &nYValid); 
  NumericVector float_data(nXValid *  nYValid); 
  CPLErr err = poBand->ReadBlock( iXBlock, iYBlock, faBlockData );
  if (!(err == OGRERR_NONE)) {
    GDALClose(ds); 
    CPLFree(faBlockData); 
    Rcpp::stop("could not read block\n"); 
    
  }
  for( int iY = 0; iY < nYValid; iY++ )
  {
    for( int iX = 0; iX < nXValid; iX++ )
    {
      float_data[cnt] = (double)faBlockData[iX + iY * nXBlockSize];
      cnt++; 
  }
  }
   float_data.attr("actual_block_size") =       IntegerVector::create(nXValid, nYValid); 
  out[0] = float_data;
  GDALClose(ds); 
  CPLFree(faBlockData); 
  return out; 
}





// [[Rcpp::export]]
Rcpp::List blocks_cpp(CharacterVector dsource, IntegerVector iblock, LogicalVector read) {                                                 
  
  
  //https://gdal.org/doxygen/classGDALRasterBand.html#a09e1d83971ddff0b43deffd54ef25eef
  IntegerVector sds0 = IntegerVector::create(0); 
  GDALDatasetH ds = gdalraster::gdalH_open_dsn(dsource[0],   sds0);  
  GDALRasterBand * poBand = ((GDALDataset*) ds)->GetRasterBand(1); 
  
  int nXBlockSize, nYBlockSize;
  poBand->GetBlockSize( &nXBlockSize, &nYBlockSize );
  
  
 
  int nXBlocks = (poBand->GetXSize() + nXBlockSize - 1) / nXBlockSize;
  int nYBlocks = (poBand->GetYSize() + nYBlockSize - 1) / nYBlockSize;
  
  List out(nXBlocks * nYBlocks);
  int cnt = 0; 
  
 // GByte *pabyData = (GByte *) CPLMalloc(nXBlockSize * nYBlockSize);
  
  for( int iYBlock = 0; iYBlock < nYBlocks; iYBlock++ )
  {
    for( int iXBlock = 0; iXBlock < nXBlocks; iXBlock++ )
    {
      int        nXValid, nYValid;
      // Compute the portion of the block that is valid
      // for partial edge blocks.
      poBand->GetActualBlockSize(iXBlock, iYBlock, &nXValid, &nYValid); 
      LogicalVector dummy_vec(0); 
      if (read[0]) {
        RawVector raw_bytes(nXValid *  nYValid); 
        Rprintf("%i\n", iYBlock); 
        
       // poBand->ReadBlock( iXBlock, iYBlock, pabyData );
        // for (int ibyte = 0; ibyte < raw_bytes.size(); ibyte++) {
        //   raw_bytes[ibyte] = pabyData[ibyte];
        // }
        raw_bytes.attr("actual_block_size") =       IntegerVector::create(nXValid, nYValid); 
        out[cnt] = raw_bytes;
       // CPLFree(pabyData); 
      } else {
        dummy_vec.attr("actual_block_size") =     IntegerVector::create(nXValid, nYValid); 
        out[cnt] = dummy_vec;
      }
      cnt++; 
    }
    
  }
  GDALClose(ds); 
//CPLFree(pabyData); 
  return out; 
}
