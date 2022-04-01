
#ifndef GDALVRTMODIFY_H
#define GDALVRTMODIFY_H
#include <Rcpp.h>

#include "gdal_priv.h"

namespace gdalvrtmodify {
using namespace Rcpp;


// should be a VRTBuilder ultimately
inline CharacterVector gdal_raster_vrt(CharacterVector dsn, NumericVector extent, CharacterVector projection) {
  GDALAllRegister();
  GDALDriver *poDriver = (GDALDriver *) GDALGetDriverByName( "VRT" );
  GDALDataset *poSrcDS, *poVRTDS;

  poSrcDS = (GDALDataset *) GDALOpenShared( dsn[0], GA_ReadOnly );

  poVRTDS = poDriver->CreateCopy( "", poSrcDS, FALSE, NULL, NULL, NULL );

  if (projection[0].empty())  {
    poVRTDS->SetProjection(projection[0]); 
  }
  if (extent.length() == 4) {
      int nXSize = GDALGetRasterXSize(poVRTDS);
      int nYSize = GDALGetRasterYSize(poVRTDS);

    double gt[6]; 
    gt[0] = extent[0];
    gt[1] = (extent[1] - extent[0])/nXSize;
    gt[2] = 0.0;
    gt[3] = extent[3];
    gt[4] = 0.0;
    gt[5] = -(extent[3] - extent[2])/nYSize;
    
    
    poVRTDS->SetGeoTransform(gt); 
  }
  char *xml = poVRTDS->GetMetadata("xml:VRT")[0];
  CharacterVector out(1);
  out[0] = xml;
  return out;
}


// 
// inline CharacterVector gdal_vrt_raster_modify(CharacterVector dsn, CharacterVector tempfile) {
//   
//   int   nBand = 1;
//   GDALDriver *poDriver = (GDALDriver *) GDALGetDriverByName( "VRT" );
//   GDALDataset *poSrcDS, *poVRTDS;
// 
//   
//   poSrcDS = (GDALDataset *) GDALOpenShared( dsn[0], GA_ReadOnly );
//   
//   poVRTDS = poDriver->CreateCopy( "", poSrcDS, FALSE, NULL, NULL, NULL );
//      
//   char szFilterSourceXML[10000];
//      
//      GDALRasterBand *poBand = poVRTDS->GetRasterBand( nBand );
//      sprintf( szFilterSourceXML,
//               "<KernelFilteredSource>"
//              "  <SourceFilename>%s</SourceFilename><SourceBand>%d</SourceBand>"
//              "  <Kernel normalized='1'>"
//              "    <Size>13</Size>"
//              "    <Coefs>0.01111 0.04394 0.13534 0.32465 0.60653 0.8825 1.0 0.8825 0.60653 0.32465 0.13534 0.04394 0.01111</Coefs>"
//              "  </Kernel>"
//              "</KernelFilteredSource>",
//              dsn[0], nBand );
//      Rprintf("\n\n%s\n\n", szFilterSourceXML); 
//      poBand->SetMetadataItem( "source_0", szFilterSourceXML, "vrt_sources" );
//    
//   //   //just a hack to get the VRT text via tempfile (handle in R)
//      GDALDriver *potempVRTDriver = (GDALDriver *) GDALGetDriverByName( "VRT" );
//      GDALDataset *potempVRTDS;
//   // 
//      potempVRTDS = potempVRTDriver->CreateCopy( tempfile[0], poVRTDS, FALSE, NULL, NULL, NULL );
//      GDALClose((GDALDatasetH) potempVRTDS);
//       GDALClose((GDALDatasetH) poSrcDS);
//       
//   return "a"; 
// }
}
#endif
