#include "ogrsf_frmts.h"
#include "ogr_api.h"
#include <Rcpp.h>
#include "CollectorList.h"
using namespace Rcpp;

//' Layer names
//'
//' @inheritParams vapour_read_feature_what
//' @return character vector of layer names
//'
//' @examples
//' file <- "list_locality_postcode_meander_valley.tab"
//' mvfile <- system.file(file.path("extdata/tab", file), package="vapour")
//' vapour_layers(mvfile)
//' @export
// [[Rcpp::export]]
Rcpp::CharacterVector vapour_layer_names(Rcpp::CharacterVector dsource,
                            Rcpp::CharacterVector sql = "")
{

  GDALAllRegister();
  GDALDataset       *poDS;
  poDS = (GDALDataset*) GDALOpenEx(dsource[0], GDAL_OF_VECTOR, NULL, NULL, NULL );
  if( poDS == NULL )
  {
    Rcpp::stop("Open failed.\n");
  }


  int nlayer = poDS->GetLayerCount();
  Rcpp::CharacterVector lnames = Rcpp::CharacterVector(nlayer);
  OGRLayer  *poLayer;
  for (int ilayer = 0; ilayer < nlayer; ilayer++) {
    poLayer = poDS->GetLayer(ilayer);
    lnames[ilayer] = poLayer->GetName();
  }
  GDALClose(poDS);
  return(lnames);
}

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
//' att <- vapour_read_attributes(mvfile, sql = sq)
//' dsource <- "inst/extdata/tab/list_locality_postcode_meander_valley.tab"
//'
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
  // for now we do this for attributes, need to figure out the
  // ListCollector logic here ...
  if (nFeature < 0) {
    nFeature = 0;
    while( (poFeature = poLayer->GetNextFeature()) != NULL )
    {
     nFeature++;
    }
  }
  poLayer->ResetReading();
  Rcpp::List out = allocate_attribute(poFDefn, nFeature, int64_as_string);


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


//' Read GDAL feature thing 'what'
//'
//' Read GDAL geometry as blob, text, or numeric extent.
//'
//' `vapour_read_feature_what` will read a feature in various ways, as binary WKB, various text formats, a numeric
//' extent, and for each an option SQL string will be evaluated against the data source before reading.
//' The extent is the native bounding box, the four numbers xmin, xmax, ymin, ymax.
//'
//' @param dsource data source
//' @param layer layer
//' @param sql sql
//' @param what what to read, "geometry", "text", "extent"
//' @param textformat indicate text output format, available are "json" (default), "gml", "kml", "wkt"
//' @examples
//' file <- "list_locality_postcode_meander_valley.tab"
//' mvfile <- system.file(file.path("extdata/tab", file), package="vapour")
//' #tib <- tibble::tibble(wkb = vapour_read_geometry(mvfile)) %>%
//' #  bind_cols(read_gdal_table(mvfile))
//' pfile <- system.file("extdata/point.shp", package = "vapour")
//' vapour_read_geometry(pfile)
//'
//' file <- "list_locality_postcode_meander_valley.tab"
//' mvfile <- system.file(file.path("extdata/tab", file), package="vapour")
//' vapour_read_geometry_text(mvfile)
//' pfile <- system.file("extdata/point.shp", package = "vapour")
//' vapour_read_geometry_text(pfile)
//'
//' file <- "list_locality_postcode_meander_valley.tab"
//' mvfile <- system.file(file.path("extdata/tab", file), package="vapour")
//' vapour_read_extent(mvfile)
//' @export
// [[Rcpp::export]]
List vapour_read_feature_what(Rcpp::CharacterVector dsource,
                            Rcpp::IntegerVector layer = 0,
                            Rcpp::CharacterVector sql = "",
                            Rcpp::CharacterVector what = "geometry",
                            Rcpp::CharacterVector textformat = "json")
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
  int iField;

  //OGRFeatureDefn *poFDefn = poLayer->GetLayerDefn();
  CollectorList feature_xx;
  int iFeature = 0;
  int warncount = 0;
  while( (poFeature = poLayer->GetNextFeature()) != NULL )
  {


    OGRFeatureDefn *poFDefn = poLayer->GetLayerDefn();
    OGRGeometry *poGeometry;
    poGeometry = poFeature->GetGeometryRef();
    if (poGeometry == NULL) {
      warncount++;
      feature_xx.push_back(R_NilValue);
      if (warncount == 1) Rcpp::warning("at least one geometry is NULL, perhaps the 'sql' argument excludes the native geometry?\n(use 'SELECT * FROM ..') ");
    } else {
    // GEOMETRY
    // geometry native binary
    // text     various text forms
    // extent   simple bbox
    if (what[0] == "geometry") {
      //https://github.com/r-spatial/sf/blob/798068d3044a65797c52bf3b42bc4a5d83b45e9a/src/gdal.cpp#L207
      Rcpp::RawVector raw(poGeometry->WkbSize());

      //todo we probably need better err handling see sf handle_error
      poGeometry->exportToWkb(wkbNDR, &(raw[0]), wkbVariantIso);
      feature_xx.push_back(raw);
      iFeature = iFeature + 1;
    }
    if (what[0] == "text") {
      Rcpp::CharacterVector txt(1);
      if (textformat[0] == "json") {
        txt[0] = poGeometry->exportToJson();
      }
      if (textformat[0] == "gml") {
        txt[0] = poGeometry->exportToGML();
      }
      if (textformat[0] == "kml") {
        txt[0] = poGeometry->exportToKML();
      }
      if (textformat[0] == "wkt") {
        //     // see buffer handling for SRS here which is where
        //     // I got inspiration from : http://www.gdal.org/gdal_tutorial.html
        char *pszGEOM_WKT = NULL;
        //     // see here for the constants for the format variants
        //     // http://www.gdal.org/ogr__core_8h.html#a6716bd3399c31e7bc8b0fd94fd7d9ba6a7459e8d11fa69e89271771c8d0f265d8
        poGeometry->exportToWkt(&pszGEOM_WKT, wkbVariantIso );
        txt[0] = pszGEOM_WKT;
        CPLFree( pszGEOM_WKT );

      }
      feature_xx.push_back(txt);
      iFeature = iFeature + 1;

    }
    if (what[0] == "extent") {
      OGREnvelope env;
      OGR_G_GetEnvelope(poGeometry, &env);
      Rcpp::NumericVector extent = NumericVector::create(env.MinX, env.MaxX, env.MinY, env.MaxY);
      feature_xx.push_back(extent);
      iFeature = iFeature + 1;

    }
    }
    OGRFeature::DestroyFeature( poFeature );

  }

  GDALClose( poDS );
  return(feature_xx.vector());
}

