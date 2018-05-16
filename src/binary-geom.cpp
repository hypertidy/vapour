// #include "ogrsf_frmts.h"
// #include "ogr_api.h"
// #include <Rcpp.h>
// using namespace Rcpp;
//
// List read_binary(Rcpp::CharacterVector dsource,
//                               Rcpp::IntegerVector layer = 0)
// {
//   GDALAllRegister();
//   GDALDataset       *poDS;
//   poDS = (GDALDataset*) GDALOpenEx(dsource[0], GDAL_OF_VECTOR, NULL, NULL, NULL );
//   if( poDS == NULL )
//   {
//     Rcpp::stop("Open failed.\n");
//   }
//   OGRLayer  *poLayer;
//
//   Rcpp::CharacterVector empty = " ";
// poLayer =  poDS->GetLayer(layer[0]);
// if (poLayer == NULL) {
//     Rcpp::stop("Layer open failed.\n");
//   }
//   OGRFeature *poFeature;
//   poLayer->ResetReading();
//   int iField;
//
//   //OGRFeatureDefn *poFDefn = poLayer->GetLayerDefn();
//   List feature_xx(1);
//   int iFeature = 0;
//   int warncount = 0;
//
//
//   int stride = sizeof(double);
//   int n;
//
//
//  int dim;
// // while( (poFeature = poLayer->GetNextFeature()) != NULL )
// //  {
//     OGRFeatureDefn *poFDefn = poLayer->GetLayerDefn();
//     OGRGeometry *poGeometry;
//     poGeometry = poFeature->GetGeometryRef();
//     n = OGR_G_GetPointCount(poGeometry);
//     dim = OGR_G_GetCoordinateDimension(poGeometry);
//     std::vector<double> x(n);
//     std::vector<double> y(n);
//     std::vector<double> z(n);
//      OGR_G_GetPoints(poGeometry, &x, stride, &y, stride, &z, stride);
//
// //Rprintf("%i\n", n);
// //Rprintf("%i\n", dim);
//    // Rcpp::RawVector raw(poGeometry->WkbSize());
//     //poGeometry->exportToWkb(wkbNDR, &(raw[0]), wkbVariantIso);
//     //feature_xx[0] = raw;
//   //  break;
// //  }
//
//  return feature_xx;
// }
