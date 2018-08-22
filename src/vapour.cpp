#include <limits>

#include "ogrsf_frmts.h"
#include "ogr_api.h"
#include <Rcpp.h>
#include "CollectorList.h"

using namespace Rcpp;

constexpr int MAX_INT =  std::numeric_limits<int>::max ();


// [[Rcpp::export]]
Rcpp::CharacterVector vapour_layer_names_cpp(Rcpp::CharacterVector dsource,
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



// [[Rcpp::export]]
List vapour_read_attributes_cpp(Rcpp::CharacterVector dsource,
                            Rcpp::IntegerVector layer = 0,
                            Rcpp::CharacterVector sql = "",
                            Rcpp::IntegerVector limit_n = 0)
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
  long long nFeature_long = poLayer->GetFeatureCount();
  if (nFeature_long > static_cast<long long>(MAX_INT))
      Rcpp::stop("Number of features exceeds maximal number able to be read");
  //int nFeature = poLayer->GetFeatureCount();
  int nFeature = static_cast<int>(nFeature_long);
  if (limit_n[0] > 0) {
    if (limit_n[0] < nFeature) {
      nFeature = limit_n[0];
    }
  }
  OGRFeatureDefn *poFDefn = poLayer->GetLayerDefn();
  bool int64_as_string = true;
  // do not think this code ever runs, or needs to MDS 2018-08-21
  // if (nFeature < 0) {
  //   nFeature = 0;
  //   while( (poFeature = poLayer->GetNextFeature()) != NULL )
  //   {
  //    nFeature++;
  //   }
  // }
  // poLayer->ResetReading();
  Rcpp::List out = allocate_attribute(poFDefn, nFeature, int64_as_string);
  int iFeature = 0;
  while( (poFeature = poLayer->GetNextFeature()) != NULL && iFeature < nFeature)
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
    OGRFeature::DestroyFeature( poFeature );

  }
  GDALClose( poDS );
  return(out);
}



// [[Rcpp::export]]
List vapour_read_geometry_cpp(Rcpp::CharacterVector dsource,
                            Rcpp::IntegerVector layer = 0,
                            Rcpp::CharacterVector sql = "",
                            Rcpp::CharacterVector what = "geometry",
                            Rcpp::CharacterVector textformat = "json",
                            Rcpp::IntegerVector limit_n = 0)
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

  //OGRFeatureDefn *poFDefn = poLayer->GetLayerDefn();
  CollectorList feature_xx;
  int iFeature = 0;
  int warncount = 0;
  while( (poFeature = poLayer->GetNextFeature()) != NULL )
  {


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
      // if geometry is empty, set the envelope to undefined (otherwise all 0s)
      double minx, maxx, miny, maxy;
      if (poGeometry->IsEmpty()) {

        minx = NA_REAL;
        maxx = NA_REAL;
        miny = NA_REAL;
        maxy = NA_REAL;

      } else {
        minx = env.MinX;
        maxx = env.MaxX;
        miny = env.MinY;
        maxy = env.MaxY;
      }

      Rcpp::NumericVector extent = NumericVector::create(minx, maxx, miny, maxy);
      feature_xx.push_back(extent);
      iFeature = iFeature + 1;

    }
    }
    OGRFeature::DestroyFeature( poFeature );

    if (limit_n[0] > 0 && iFeature >= limit_n[0]) {
      break;  // short-circuit for limit_n
    }
  }

  GDALClose( poDS );
  return(feature_xx.vector());
}



// [[Rcpp::export]]
List vapour_projection_info_cpp(Rcpp::CharacterVector dsource,
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
  OGRSpatialReference *SRS =  poLayer->GetSpatialRef();

  char *proj;  // this gets cleaned up lower in the SRS==NULL else
  List info_out(6);
  CharacterVector outproj(1);
  CharacterVector outnames(6);
  outnames[0] = "Proj4";
  outnames[1] = "MICoordSys";
  outnames[2] = "PrettyWkt";
  outnames[3] = "Wkt";
  outnames[4] = "EPSG";
  outnames[5] = "XML";
  info_out.attr("names") = outnames;

  if (SRS == NULL) {
    //Rcpp::warning("null");
    // do nothing, or warn
    // e.g. .shp with no .prj
  } else {
    Rcpp::warning("not null");
    // SRS is not NULL, so explore validation
    OGRErr err = SRS->Validate();
    SRS->exportToProj4(&proj);
    outproj[0] = proj;
    info_out[0] = Rcpp::clone(outproj);

    SRS->exportToMICoordSys(&proj);
    outproj[0] = proj;
    info_out[1] = Rcpp::clone(outproj);

    SRS->exportToPrettyWkt(&proj, false);
    outproj[0] = proj;
    info_out[2] = Rcpp::clone(outproj);

    SRS->exportToWkt(&proj);
    outproj[0] = proj;
    info_out[3] = Rcpp::clone(outproj);

    int epsg = SRS->GetEPSGGeogCS();
    info_out[4] = epsg;

    SRS->exportToXML(&proj);
    outproj[0] = proj;
    info_out[5] = Rcpp::clone(outproj);

    CPLFree(proj);
  }

  return info_out;
}


// [[Rcpp::export]]
List vapour_read_names_cpp(Rcpp::CharacterVector dsource,
                              Rcpp::IntegerVector layer = 0,
                              Rcpp::CharacterVector sql = "",
                              Rcpp::IntegerVector limit_n = 0)
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

  //OGRFeatureDefn *poFDefn = poLayer->GetLayerDefn();
  CollectorList feature_xx;
  int iFeature = 0;
  int warncount = 0;
  long long aFID;
  Rcpp::NumericVector rFID(1);
  while( (poFeature = poLayer->GetNextFeature()) != NULL )
  {

    iFeature++;
    aFID = poFeature->GetFID();
    rFID[0] = aFID;
    feature_xx.push_back(Rcpp::clone(rFID));
    if (limit_n[0] > 0 && iFeature >= limit_n[0]) {
      break;  // short-circuit for limit_n
    }
  }
  OGRFeature::DestroyFeature( poFeature );
  GDALClose( poDS );
  return(feature_xx.vector());
}

