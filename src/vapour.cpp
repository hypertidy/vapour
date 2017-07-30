#include "ogrsf_frmts.h"
#include <Rcpp.h>
using namespace Rcpp;


// copied from Edzer Pebesma, https://github.com/r-spatial/sf/blob/master/src/gdal_read.cpp
Rcpp::List allocate_attribute(OGRFeatureDefn *poFDefn, int n_features, bool int64_as_string) {

  int n = poFDefn->GetFieldCount(); //+ poFDefn->GetGeomFieldCount();
  Rcpp::List out(n);
  Rcpp::CharacterVector names(n);
  for (int i = 0; i < poFDefn->GetFieldCount(); i++) {
    OGRFieldDefn *poFieldDefn = poFDefn->GetFieldDefn(i);
    switch (poFieldDefn->GetType()) {
    case OFTInteger:
      out[i] = Rcpp::IntegerVector(n_features);
      break;
    case OFTDate: {
      Rcpp::NumericVector ret(n_features);
      ret.attr("class") = "Date";
      out[i] = ret;
    } break;
    case OFTDateTime: {
      Rcpp::NumericVector ret(n_features);
      Rcpp::CharacterVector cls(2);
      cls(0) = "POSIXct";
      cls(1) = "POSIXt";
      ret.attr("class") = cls;
      out[i] = ret;
    } break;
    case OFTInteger64: // fall through: converts Int64 -> double
      if (int64_as_string)
        out[i] = Rcpp::CharacterVector(n_features);
      else
        out[i] = Rcpp::NumericVector(n_features);
      break;
    case OFTReal:
      out[i] = Rcpp::NumericVector(n_features);
      break;
    case OFTString:
      out[i] = Rcpp::CharacterVector(n_features);
      break;
    case OFTStringList:
    case OFTRealList:
    case OFTIntegerList:
      out[i] = Rcpp::List(n_features);
      break;
    default:
      Rcpp::stop("Unrecognized field type\n"); // #nocov
    break;
    }
    names[i] = poFieldDefn->GetNameRef();
  }

  out.attr("names") = names;
  return out;
}


//' Test GDAL read
//'
//' Simple pointless function to learn the GDAL API.
//'
//' Microprocessors, databases, servers.
//' @param dsource data source name (path to file, connection string, URL)
//' @param layer integer of layer to work with, defaults to the first (0)
//' @examples
//' sfile <- system.file("shape/nc.shp", package="sf")
//' vapour(sfile)
//' pfile <- "inst/extdata/point.shp"
//' vapour(pfile)
//' @export
// [[Rcpp::export]]
List vapour(Rcpp::CharacterVector dsource, Rcpp::IntegerVector layer = 0)
{
  GDALAllRegister();
  GDALDataset       *poDS;
  poDS = (GDALDataset*) GDALOpenEx(dsource[0], GDAL_OF_VECTOR, NULL, NULL, NULL );
  if( poDS == NULL )
  {
    printf( "Open failed.\n" );
    exit( 1 );
  }
  OGRLayer  *poLayer;
  poLayer =  poDS->GetLayer(layer[0]);
  OGRFeature *poFeature;
  poLayer->ResetReading();
  //  poFeature = poLayer->GetNextFeature();
  int iField;
  int nFeature = poLayer->GetFeatureCount();
  OGRFeatureDefn *poFDefn = poLayer->GetLayerDefn();
  bool int64_as_string = true;
  Rcpp::List out = allocate_attribute(poFDefn, nFeature, int64_as_string);

  if (nFeature == 0) {
    printf("no features found");
    return(out);
  }
  int iFeature = 0;
  while( (poFeature = poLayer->GetNextFeature()) != NULL )
  {
    OGRFeatureDefn *poFDefn = poLayer->GetLayerDefn();

  int iField;
  for( iField = 0; iField < poFDefn->GetFieldCount(); iField++ )
  {

    OGRFieldDefn *poFieldDefn = poFDefn->GetFieldDefn( iField );
    if( poFieldDefn->GetType() == OFTInteger ) {
      Rcpp::IntegerVector nv;
      nv = out[iField];
      nv[iFeature] = poFeature->GetFieldAsInteger( iField );
    } else if( poFieldDefn->GetType() == OFTInteger64 )
      printf( CPL_FRMT_GIB ",", poFeature->GetFieldAsInteger64( iField ) );
    else if( poFieldDefn->GetType() == OFTReal ) {
      Rcpp::NumericVector nv;
      nv = out[iField];
      nv[iFeature] = poFeature->GetFieldAsDouble( iField );
    } else if( poFieldDefn->GetType() == OFTString ) {
      Rcpp::CharacterVector nv;
      nv = out[iField];
      nv[iFeature] = poFeature->GetFieldAsString( iField );

    }   else
      printf( "%s,", poFeature->GetFieldAsString(iField) );
  }
  iFeature = iFeature + 1;
  }
  GDALClose( poDS );
  return(out);
}




//' Test GDAL read
//'
//' Simple pointless function to learn the GDAL API.
//'
//' Microprocessors, databases, servers.
//' @param dsource data source name (path to file, connection string, URL)
//' @param layer integer of layer to work with, defaults to the first (0)
//' @examples
//' sfile <- system.file("shape/nc.shp", package="sf")
//' vgeom(sfile)
//' pfile <- "inst/extdata/point.shp"
//' vgeom(pfile)
//' @export
// [[Rcpp::export]]
CharacterVector to_format(Rcpp::CharacterVector dsource, Rcpp::IntegerVector layer = 0, Rcpp::CharacterVector format = "json")
{
  GDALAllRegister();
  GDALDataset       *poDS;
  poDS = (GDALDataset*) GDALOpenEx(dsource[0], GDAL_OF_VECTOR, NULL, NULL, NULL );
  if( poDS == NULL )
  {
    printf( "Open failed.\n" );
    exit( 1 );
  }
  OGRLayer  *poLayer;
  poLayer =  poDS->GetLayer(layer[0]);
  OGRFeature *poFeature;
  poLayer->ResetReading();
  //  poFeature = poLayer->GetNextFeature();
  int iField;
  int nFeature = poLayer->GetFeatureCount();
  OGRFeatureDefn *poFDefn = poLayer->GetLayerDefn();
  Rcpp::CharacterVector out = Rcpp::CharacterVector(nFeature);
  if (nFeature == 0) {
    printf("no features found");
//    return(out);
  }
  int iFeature = 0;
  //std::string istring = "";
  while( (poFeature = poLayer->GetNextFeature()) != NULL )
  {
    OGRFeatureDefn *poFDefn = poLayer->GetLayerDefn();
    OGRGeometry *poGeometry;
    poGeometry = poFeature->GetGeometryRef();
    if (format[0] == "json") {
     out[iFeature] = poGeometry->exportToJson();
    }
    if (format[0] == "gml") {
      out[iFeature] = poGeometry->exportToGML();
    }
    if (format[0] == "kml") {
      out[iFeature] = poGeometry->exportToKML();
    }
    if (format[0] == "wkt") {
      // see buffer handling for SRS here which is where
      // I got inspiration from : http://www.gdal.org/gdal_tutorial.html
      char *pszGEOM_WKT = NULL;
      // see here for the constants for the format variants
      // http://www.gdal.org/ogr__core_8h.html#a6716bd3399c31e7bc8b0fd94fd7d9ba6a7459e8d11fa69e89271771c8d0f265d8
      poGeometry->exportToWkt(&pszGEOM_WKT, wkbVariantIso );
      out[iFeature] = pszGEOM_WKT;
      CPLFree( pszGEOM_WKT );

    }
    OGRFeature::DestroyFeature( poFeature );
    iFeature = iFeature + 1;
  }

  GDALClose( poDS );
  return(out);
}
