#include "ogrsf_frmts.h"
#include "ogr_api.h"
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


//' Vector attributes read
//'
//' Read layer attributes, optionally after SQL select
//' @param dsource data source name (path to file, connection string, URL)
//' @param layer integer of layer to work with, defaults to the first (0)
//' @param sql if not empty this is executed against the data source (layer will be ignored)
//' @examples
//' file <- "list_locality_postcode_meander_valley.tab"
//' mvfile <- system.file(file.path("extdata/tab", file), package="vapour")
//' vapour_read_attributes(mvfile)
//' sq <- "SELECT * FROM list_locality_postcode_meander_valley WHERE FID < 5"
//' vapour_read_attributes(mvfile, sql = sq)
//' @export
// [[Rcpp::export]]
List vapour_read_attributes(Rcpp::CharacterVector dsource,
                            Rcpp::IntegerVector layer = 0,
                            Rcpp::CharacterVector sql = "")
{
  GDALAllRegister();
  GDALDataset       *poDS;
  poDS = (GDALDataset*) GDALOpenEx(dsource[0], GDAL_OF_VECTOR, NULL, NULL, NULL );
  if( poDS == NULL )
  {
    Rcpp::stop("Open failed.\n");
  }


  OGRLayer  *poLayer;
  Rcpp::CharacterVector empty = " ";
  if (sql[0] != "") {
    poLayer =  poDS->ExecuteSQL(sql[0],
                                NULL,
                                empty[0] );

    if (poLayer == NULL) {
      Rcpp::stop("SQL execution failed.\n");
    }

  } else {
    poLayer =  poDS->GetLayer(layer[0]);
  }
  if (poLayer == NULL) {
    Rcpp::stop("Layer open failed.\n");
  }

  // poLayer =  poDS->GetLayer(layer[0]);
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
      }
      //  if( poFieldDefn->GetType() == OFTInteger64 )
      //    printf( CPL_FRMT_GIB ",", poFeature->GetFieldAsInteger64( iField ) );
      if( poFieldDefn->GetType() == OFTReal ) {
        Rcpp::NumericVector nv;
        nv = out[iField];
        nv[iFeature] = poFeature->GetFieldAsDouble( iField );
      }

      if( poFieldDefn->GetType() == OFTString ) {
        Rcpp::CharacterVector nv;
        nv = out[iField];
        nv[iFeature] = poFeature->GetFieldAsString( iField );

      }
    }
    iFeature = iFeature + 1;
  }
  GDALClose( poDS );
  return(out);
}


//'  GDAL geometry extent
//'
//' Read a GDAL geometry summary as just the native bounding box, the four
//' numbers xmin, xmax, ymin, ymax in the usual simple convention.
//'
//' @inheritParams vapour_read_attributes
//' @examples
//' file <- "list_locality_postcode_meander_valley.tab"
//' mvfile <- system.file(file.path("extdata/tab", file), package="vapour")
//' vapour_read_extent(mvfile)
//' @export
// [[Rcpp::export]]
List vapour_read_extent(Rcpp::CharacterVector dsource,
                        Rcpp::IntegerVector layer = 0,
                        Rcpp::CharacterVector sql = "")
{

  GDALAllRegister();
  GDALDataset       *poDS;
  poDS = (GDALDataset*) GDALOpenEx(dsource[0], GDAL_OF_VECTOR, NULL, NULL, NULL );
  if( poDS == NULL )
  {
    Rcpp::stop("Open failed.\n");
  }
  OGRLayer  *poLayer;

  Rcpp::CharacterVector empty = " ";
  if (sql[0] != "") {
    poLayer =  poDS->ExecuteSQL(sql[0],
                                NULL,
                                empty[0] );

    if (poLayer == NULL) {
      Rcpp::stop("SQL execution failed.\n");
    }

  } else {
    poLayer =  poDS->GetLayer(layer[0]);
  }
  if (poLayer == NULL) {
    Rcpp::stop("Layer open failed.\n");
  }

  OGRFeature *poFeature;
  poLayer->ResetReading();
  //  poFeature = poLayer->GetNextFeature();
  int iField;
  int nFeature = poLayer->GetFeatureCount();
  OGRFeatureDefn *poFDefn = poLayer->GetLayerDefn();
  Rcpp::List binary = Rcpp::List(nFeature);
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
    OGREnvelope env;
    OGR_G_GetEnvelope(poGeometry, &env);
    binary[iFeature] = Rcpp::NumericVector {env.MinX, env.MaxX, env.MinY, env.MaxY};
    //https://github.com/r-spatial/sf/blob/798068d3044a65797c52bf3b42bc4a5d83b45e9a/src/gdal.cpp#L207
    // Rcpp::RawVector raw(poGeometry->WkbSize());
    //todo we probably need better err handling see sf handle_error
    //poGeometry->exportToWkb(wkbNDR, &(raw[0]), wkbVariantIso);
    //binary[iFeature] = raw;
    OGRFeature::DestroyFeature( poFeature );
    iFeature = iFeature + 1;
  }

  GDALClose( poDS );
  return(binary);
}



//' Read GDAL geometry as blob
//'
//' Simple read of geometry-only as WKB format.
//'
//'
//' @inheritParams vapour_read_attributes
//' @format indicate text output format, available are "json" (default), "gml", "kml", "wkt"
//' @examples
//' file <- "list_locality_postcode_meander_valley.tab"
//' mvfile <- system.file(file.path("extdata/tab", file), package="vapour")
//' #tib <- tibble::tibble(wkb = vapour_read_geometry(mvfile)) %>%
//' #  bind_cols(read_gdal_table(mvfile))
//' pfile <- system.file("extdata/point.shp", package = "vapour")
//' vapour_read_geometry(pfile)
//' @export
// [[Rcpp::export]]
List vapour_read_geometry(Rcpp::CharacterVector dsource,
                          Rcpp::IntegerVector layer = 0,
                          Rcpp::CharacterVector sql = "")
{
  GDALAllRegister();
  GDALDataset       *poDS;
  poDS = (GDALDataset*) GDALOpenEx(dsource[0], GDAL_OF_VECTOR, NULL, NULL, NULL );
  if( poDS == NULL )
  {
    Rcpp::stop("Open failed.\n");
  }
  OGRLayer  *poLayer;

  Rcpp::CharacterVector empty = " ";
  if (sql[0] != "") {
    poLayer =  poDS->ExecuteSQL(sql[0],
                                NULL,
                                empty[0] );

    if (poLayer == NULL) {
      Rcpp::stop("SQL execution failed.\n");
    }

  } else {
    poLayer =  poDS->GetLayer(layer[0]);
  }
  if (poLayer == NULL) {
    Rcpp::stop("Layer open failed.\n");
  }


  OGRFeature *poFeature;
  poLayer->ResetReading();
  //  poFeature = poLayer->GetNextFeature();
  int iField;
  int nFeature = poLayer->GetFeatureCount();
  OGRFeatureDefn *poFDefn = poLayer->GetLayerDefn();
  Rcpp::List binary = Rcpp::List(nFeature);
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
    //https://github.com/r-spatial/sf/blob/798068d3044a65797c52bf3b42bc4a5d83b45e9a/src/gdal.cpp#L207
    Rcpp::RawVector raw(poGeometry->WkbSize());
    //todo we probably need better err handling see sf handle_error
    poGeometry->exportToWkb(wkbNDR, &(raw[0]), wkbVariantIso);
    binary[iFeature] = raw;
    OGRFeature::DestroyFeature( poFeature );
    iFeature = iFeature + 1;
  }

  GDALClose( poDS );
  return(binary);
}


//' Read GDAL geometry as text
//'
//' Simple read of geometry-only as text format.
//'
//'
//' @inheritParams vapour_read_attributes
//' @param format indicate text output format, available are "json" (default), "gml", "kml", "wkt"
//' @examples
//' file <- "list_locality_postcode_meander_valley.tab"
//' mvfile <- system.file(file.path("extdata/tab", file), package="vapour")
//' vapour_read_geometry_text(mvfile)
//' pfile <- system.file("extdata/point.shp", package = "vapour")
//' vapour_read_geometry_text(pfile)
//' @export
// [[Rcpp::export]]
CharacterVector vapour_read_geometry_text(Rcpp::CharacterVector dsource,
                                          Rcpp::IntegerVector layer = 0,
                                          Rcpp::CharacterVector sql = "",
                                          Rcpp::CharacterVector format = "json")
{
  GDALAllRegister();
  GDALDataset       *poDS;
  poDS = (GDALDataset*) GDALOpenEx(dsource[0], GDAL_OF_VECTOR, NULL, NULL, NULL );
  if( poDS == NULL )
  {
    Rcpp::stop("Open failed.\n");
  }
  OGRLayer  *poLayer;

  Rcpp::CharacterVector empty = " ";
  if (sql[0] != "") {
    poLayer =  poDS->ExecuteSQL(sql[0],
                                NULL,
                                empty[0] );

    if (poLayer == NULL) {
      Rcpp::stop("SQL execution failed.\n");
    }

  } else {
    poLayer =  poDS->GetLayer(layer[0]);
  }
  if (poLayer == NULL) {
    Rcpp::stop("Layer open failed.\n");
  }

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



