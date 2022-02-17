
#ifndef GDALVRTMODIFY_H
#define GDALVRTMODIFY_H
#include <Rcpp.h>
#include "gdal_priv.h"

namespace gdalvrtmodify {
using namespace Rcpp;


inline CharacterVector gdal_vrt_raster_modify(CharacterVector dsn, CharacterVector tempfile) {
  
  int   nBand = 1;
  GDALDriver *poDriver = (GDALDriver *) GDALGetDriverByName( "VRT" );
  GDALDataset *poSrcDS, *poVRTDS;
  
  
  poSrcDS = (GDALDataset *) GDALOpenShared( dsn[0], GA_ReadOnly );
  
  poVRTDS = poDriver->CreateCopy( "", poSrcDS, FALSE, NULL, NULL, NULL );
     
  char szFilterSourceXML[10000];
     
     GDALRasterBand *poBand = poVRTDS->GetRasterBand( nBand );
     sprintf( szFilterSourceXML,
              "<KernelFilteredSource>"
             "  <SourceFilename>%s</SourceFilename><SourceBand>%d</SourceBand>"
             "  <Kernel normalized='1'>"
             "    <Size>13</Size>"
             "    <Coefs>0.01111 0.04394 0.13534 0.32465 0.60653 0.8825 1.0 0.8825 0.60653 0.32465 0.13534 0.04394 0.01111</Coefs>"
             "  </Kernel>"
             "</KernelFilteredSource>",
             dsn[0], nBand );
     Rprintf("\n\n%s\n\n", szFilterSourceXML); 
     poBand->SetMetadataItem( "source_0", szFilterSourceXML, "vrt_sources" );
   
  //   //just a hack to get the VRT text via tempfile (handle in R)
     GDALDriver *potempVRTDriver = (GDALDriver *) GDALGetDriverByName( "VRT" );
     GDALDataset *potempVRTDS;
  // 
     potempVRTDS = potempVRTDriver->CreateCopy( tempfile[0], poVRTDS, FALSE, NULL, NULL, NULL );
     GDALClose((GDALDatasetH) potempVRTDS);
      GDALClose((GDALDatasetH) poSrcDS);
      
  return "a"; 
}
}
#endif
