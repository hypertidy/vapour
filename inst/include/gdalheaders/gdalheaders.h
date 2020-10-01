#ifndef GDALHEADERS_H
#define GDALHEADERS_H
#include <Rcpp.h>
#include "ogrsf_frmts.h"
//#include "ogr_api.h"
#include "gdal_priv.h"
#include "CollectorList.h"
// #include "ogr_spatialref.h" // for OGRSpatialReference
// #include "cpl_conv.h" // for CPLFree()

namespace gdalheaders {
using namespace Rcpp;

constexpr int MAX_INT =  std::numeric_limits<int>::max ();

inline void gdal_register_all() {
  GDALAllRegister();
}
inline void ogr_register_all() {
  OGRRegisterAll();
}
inline void ogr_cleanup_all() {
  OGRCleanupAll();
}
inline void osr_cleanup() {
  OSRCleanup();
}

inline CharacterVector gdal_layer_has_geometry(OGRLayer *poLayer) {
  CharacterVector out(1);
  poLayer->ResetReading();

  //OGRFeature *poFeature = poLayer->GetNextFeature();
  OGRFeatureDefn *poFDefn = poLayer->GetLayerDefn();
  OGRGeomFieldDefn *poGFDefn = poFDefn->GetGeomFieldDefn(0);
  const char *geom_name =    poGFDefn->GetNameRef();
  out[0] = geom_name;
  //OGRFeature::DestroyFeature( poFeature );
  return out;
}
// this force function takes cheap count, checks, then more expensive, checks,
// then iterates and resets reading
inline double force_layer_feature_count(OGRLayer *poLayer) {
  double out;
  out = (double)poLayer->GetFeatureCount(false);
  if (out == -1) {
    out = (double)poLayer->GetFeatureCount(true);
  }
  if (out == -1) {
    out = 0;
   poLayer->ResetReading();
   while(poLayer->GetNextFeature() != NULL) {
     out++;
   }
   poLayer->ResetReading();
  }
  return out;
}
inline CharacterVector gdal_version()
{
  Rcpp::CharacterVector out(1);

  out[0] = GDALVersionInfo("--version");
  return out;
}


inline OGRLayer *gdal_layer(GDALDataset *poDS, IntegerVector layer, CharacterVector sql, NumericVector ex) {
  OGRLayer  *poLayer;
  OGRPolygon poly;
  OGRLinearRing ring;

  if (ex.length() == 4) {
    ring.addPoint(ex[0], ex[2]); //xmin, ymin
    ring.addPoint(ex[0], ex[3]); //xmin, ymax
    ring.addPoint(ex[1], ex[3]); //xmax, ymax
    ring.addPoint(ex[1], ex[2]); //xmax, ymin
    ring.closeRings();
    poly.addRing(&ring);
  }

  if (sql[0] != "") {
    if (ex.length() == 4) {
      poLayer =  poDS->ExecuteSQL(sql[0],
                                  &poly,
                                  NULL );
    } else {
      poLayer =  poDS->ExecuteSQL(sql[0],
                                  NULL,
                                  NULL );
    }

    if (poLayer == NULL) {
      Rcpp::stop("SQL execution failed.\n");
    }

  } else {
    int nlayer = poDS->GetLayerCount();
    if (layer[0] >= nlayer) {
      Rprintf("layer count: %i\n", nlayer);
      Rprintf("layer index: %i\n", layer[0]);
      Rcpp::stop("layer index exceeds layer count");
    }
    poLayer =  poDS->GetLayer(layer[0]);
  }
  if (poLayer == NULL) {
    Rcpp::stop("Layer open failed.\n");
  }
  return poLayer;
}

inline List gdal_list_drivers()
{
  int n = GetGDALDriverManager()->GetDriverCount();
  Rcpp::CharacterVector sname(n);
  Rcpp::CharacterVector lname(n);
  Rcpp::LogicalVector isvector(n);
  Rcpp::LogicalVector israster(n);
  Rcpp::LogicalVector iscopy(n);
  Rcpp::LogicalVector iscreate(n);
  Rcpp::LogicalVector isvirt(n);
  for (int idriver = 0; idriver < n; idriver++) {
    GDALDriver *dr = GetGDALDriverManager()->GetDriver(idriver);
    sname(idriver) = GDALGetDriverShortName(dr);
    lname(idriver) = GDALGetDriverLongName(dr);
    isvector(idriver) = (dr->GetMetadataItem(GDAL_DCAP_VECTOR) != NULL);
    israster(idriver) = (dr->GetMetadataItem(GDAL_DCAP_RASTER) != NULL);
    iscopy(idriver) = (dr->GetMetadataItem(GDAL_DCAP_CREATECOPY) != NULL);
    iscreate(idriver) = (dr->GetMetadataItem(GDAL_DCAP_CREATE) != NULL);
    isvirt(idriver) = (dr->GetMetadataItem(GDAL_DCAP_VIRTUALIO) != NULL);
  }
  Rcpp::List out = Rcpp::List::create(Rcpp::Named("driver") = sname,
                                      Rcpp::Named("name") = lname,
                                      Rcpp::Named("vector") = isvector,
                                      Rcpp::Named("raster") = israster,
                                      Rcpp::Named("create") = iscreate,
                                      Rcpp::Named("copy") = iscopy,
                                      Rcpp::Named("virtual") = isvirt);
  return out;
}

// allocate_fields_list: stolen from allocate_out_list
// allocate_out_list: this is the non-geometry part of sf allocate_out_list by Edzer Pebsema
// https://github.com/r-spatial/sf/blob/cc7fba3c5a34ec1c545a4ab82369f33f47c3745f/src/gdal_read.cpp#L12-L65
// originally used in vapour at allocate_attributes
// to update this copy the source of allocate_fields_list from sf, delete the geom stuff at the end
// (between  names[i] = poFieldDefn->GetNameRef();
// and out.attr("names") = names;
// and remove the GetGeomFieldCount from the sum of n fields
inline Rcpp::List allocate_fields_list(OGRFeatureDefn *poFDefn, int n_features, bool int64_as_string,
                                       Rcpp::CharacterVector fid_column) {

  if (fid_column.size() > 1)
    Rcpp::stop("FID column name should be a length 1 character vector"); // #nocov

  // modified MDS
  //int n = poFDefn->GetFieldCount() + poFDefn->GetGeomFieldCount() + fid_column.size();
  int n = poFDefn->GetFieldCount() + fid_column.size();

  Rcpp::List out(n);
  Rcpp::CharacterVector names(n);
  for (int i = 0; i < poFDefn->GetFieldCount(); i++) {
    OGRFieldDefn *poFieldDefn = poFDefn->GetFieldDefn(i);
    switch (poFieldDefn->GetType()) {
    case OFTInteger: {
      if (poFieldDefn->GetSubType() == OFSTBoolean)
        out[i] = Rcpp::LogicalVector(n_features);
      else
        out[i] = Rcpp::IntegerVector(n_features);
    }
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
    case OFTStringList:
    case OFTRealList:
    case OFTIntegerList:
    case OFTInteger64List:
      out[i] = Rcpp::List(n_features);
      break;
    case OFTString:
    default:
      out[i] = Rcpp::CharacterVector(n_features);
      break;
    }
    names[i] = poFieldDefn->GetNameRef();
  }


  out.attr("names") = names;
  return out;
}



inline List gdal_read_fields(CharacterVector dsn,
                      IntegerVector layer,
                      CharacterVector sql,
                      IntegerVector limit_n,
                      IntegerVector skip_n,
                      NumericVector ex,
                      CharacterVector fid_column_name)
{
  GDALDataset       *poDS;
  poDS = (GDALDataset*) GDALOpenEx(dsn[0], GDAL_OF_VECTOR, NULL, NULL, NULL );
  if( poDS == NULL )
  {
    Rcpp::stop("Open failed.\n");
  }
  OGRLayer *poLayer = gdal_layer(poDS, layer, sql = sql, ex =  ex);

  OGRFeature *poFeature;

  double  nFeature = force_layer_feature_count(poLayer);

  if (nFeature > MAX_INT) {
    Rcpp::warning("Number of features exceeds maximal number able to be read");
   nFeature = MAX_INT;
  }
  // this is poorly laid out but works, check twice to avoid
  // over allocating as per #60
  if (limit_n[0] > 0) {
    if (limit_n[0] < nFeature) {
      nFeature = nFeature - skip_n[0];
      if (limit_n[0] < nFeature) {
        nFeature = limit_n[0];
      }
    }
  }

  if (nFeature < 1) {
    if (skip_n[0] > 0) {
      Rcpp::stop("no features to be read (is 'skip_n' set too high?");
    }
    Rcpp::stop("no features to be read");
  }

  OGRFeatureDefn *poFDefn = poLayer->GetLayerDefn();
  bool int64_as_string = false;
  List out = allocate_fields_list(poFDefn, nFeature, int64_as_string, fid_column_name);
  int iFeature = 0;  // always increment iFeature, it is position through the loop
  int lFeature = 0; // keep a count of the features we actually send out
  while((poFeature = poLayer->GetNextFeature()) != NULL)
  {

    if (lFeature >= nFeature) {
      break;
    }
    OGRFeatureDefn *poFDefn = poLayer->GetLayerDefn();
    // only increment lFeature if we actually keep this one
    if (iFeature >= skip_n[0]) {  // we are at skip_n

      int iField;
      for( iField = 0; iField < poFDefn->GetFieldCount(); iField++ )
      {
        OGRFieldDefn *poFieldDefn = poFDefn->GetFieldDefn( iField );
        if( poFieldDefn->GetType() == OFTInteger   ) {
          IntegerVector nv;
          nv = out[iField];
          nv[lFeature] = poFeature->GetFieldAsInteger( iField );
        }

        if( poFieldDefn->GetType() == OFTReal || poFieldDefn->GetType() == OFTInteger64) {
          NumericVector nv;
          nv = out[iField];
          nv[lFeature] = poFeature->GetFieldAsDouble( iField );
        }

        if( poFieldDefn->GetType() == OFTString || poFieldDefn->GetType() == OFTDate || poFieldDefn->GetType() == OFTTime || poFieldDefn->GetType() == OFTDateTime) {
          CharacterVector nv;
          nv = out[iField];
          nv[lFeature] = poFeature->GetFieldAsString( iField );

        }
      }
      // so we start counting
      lFeature = lFeature + 1;
    }
    // always increment iFeature, it's position through the loop
    iFeature = iFeature + 1;
    OGRFeature::DestroyFeature( poFeature );
  }
  // clean up if SQL was used https://www.gdal.org/classGDALDataset.html#ab2c2b105b8f76a279e6a53b9b4a182e0
  if (sql[0] != "") {
    poDS->ReleaseResultSet(poLayer);
  }
  GDALClose( poDS );
  if (lFeature < 1) {
    if (skip_n[0] > 0) {
      Rcpp::stop("no features to be read (is 'skip_n' set too high?");
    }
    Rcpp::stop("no features to be read");
  }
  return(out);
}


inline DoubleVector gdal_feature_count(CharacterVector dsn,
                                       IntegerVector layer, CharacterVector sql, NumericVector ex) {
  GDALDataset       *poDS;
  poDS = (GDALDataset*) GDALOpenEx(dsn[0], GDAL_OF_VECTOR, NULL, NULL, NULL );
  if( poDS == NULL )
  {
    Rcpp::stop("Open failed.\n");
  }

  OGRLayer *poLayer = gdal_layer(poDS, layer, sql = sql, ex =  ex);

  poLayer->ResetReading();
  double nFeature = force_layer_feature_count(poLayer);

  // clean up if SQL was used https://www.gdal.org/classGDALDataset.html#ab2c2b105b8f76a279e6a53b9b4a182e0
  if (sql[0] != "") {
    poDS->ReleaseResultSet(poLayer);
  }
  GDALClose( poDS );

  DoubleVector out(1);
  out[0] = nFeature;
  return(out);
}

inline CharacterVector gdal_driver(CharacterVector dsn)
{

  GDALDataset       *poDS;
  poDS = (GDALDataset*) GDALOpenEx(dsn[0], GA_ReadOnly, NULL, NULL, NULL );
  if( poDS == NULL )
  {
    Rcpp::stop("Open failed.\n");
  }
  CharacterVector dname(1);
  dname[0] = poDS->GetDriverName();
  GDALClose(poDS);
  return(dname);
} // gdal_driver

inline CharacterVector gdal_layer_names(CharacterVector dsn)
{


  // remove sql 2020-05-31
  GDALDataset       *poDS;
  poDS = (GDALDataset*) GDALOpenEx(dsn[0], GDAL_OF_VECTOR, NULL, NULL, NULL );
  if( poDS == NULL )
  {
    Rcpp::stop("Open failed.\n");
  }
  OGRLayer  *poLayer;  //gdal_layer_names
  int nlayer = poDS->GetLayerCount();
  CharacterVector lnames = CharacterVector(nlayer);
  for (int ilayer = 0; ilayer < nlayer; ilayer++) {
    poLayer = poDS->GetLayer(ilayer);
    lnames[ilayer] = poLayer->GetName();
  }
  GDALClose(poDS);
  return(lnames);
} // gdal_layer_names




inline List gdal_read_geometry(CharacterVector dsn,
                                     IntegerVector layer,
                                     CharacterVector sql,
                                     CharacterVector what,
                                     CharacterVector textformat,
                                     IntegerVector limit_n,
                                     IntegerVector skip_n,
                                     NumericVector ex)
{
  GDALDataset       *poDS;
  poDS = (GDALDataset*) GDALOpenEx(dsn[0], GDAL_OF_VECTOR, NULL, NULL, NULL );
  if( poDS == NULL )
  {
    Rcpp::stop("Open failed.\n");
  }
  OGRLayer *poLayer = gdal_layer(poDS, layer, sql = sql, ex =  ex);

  OGRFeature *poFeature;
  poLayer->ResetReading();

  //OGRFeatureDefn *poFDefn = poLayer->GetLayerDefn();
  CollectorList feature_xx;
  int iFeature = 0;
  int lFeature = 0;
  int   nFeature = force_layer_feature_count(poLayer);

  if (nFeature > MAX_INT) {
    nFeature = MAX_INT;
    Rcpp::warning("Number of features exceeds maximal number able to be read");
  }



  if (limit_n[0] > 0) {
    if (limit_n[0] < nFeature) {
      nFeature = limit_n[0];
    }
  }
  if (nFeature < 1) {
    Rcpp::stop("no features to be read");
  }


  int warncount = 0;
  while( (poFeature = poLayer->GetNextFeature()) != NULL )
  {

    if (iFeature >= skip_n[0]) {  // we are at skip_n

      OGRGeometry *poGeometry;
      poGeometry = poFeature->GetGeometryRef();
      if (poGeometry == NULL) {
        warncount++;
        feature_xx.push_back(R_NilValue);
        //if (warncount == 1) Rcpp::warning("at least one geometry is NULL, perhaps the 'sql' argument excludes the native geometry?\n(use 'SELECT * FROM ..') ");
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

        }
        if (what[0] == "text") {
          CharacterVector txt(1);
          if (textformat[0] == "json") {
            char *export_txt = NULL;
            export_txt = poGeometry->exportToJson();
            txt[0] = export_txt;
            CPLFree(export_txt);
          }
          if (textformat[0] == "gml") {
            char *export_txt = NULL;

            export_txt = poGeometry->exportToGML();
            txt[0] = export_txt;
            CPLFree(export_txt);

          }
          if (textformat[0] == "kml") {
            char *export_txt = NULL;

            export_txt = poGeometry->exportToKML();
            txt[0] = export_txt;
            CPLFree(export_txt);

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
          NumericVector extent = NumericVector::create(minx, maxx, miny, maxy);
          feature_xx.push_back(extent);
        }
        // FIXME: do with wk, perhaps post-hoc from WKB return from here
        // if (what[0] == "point") {
        //   Rcpp::List pts = Rcpp::List(1);
        //   pts = GetPointsInternal(poGeometry, 1);
        //   feature_xx.push_back(pts);
        // }
        if (what[0] == "type") {
          OGRwkbGeometryType gtyp = OGR_G_GetGeometryType(poGeometry);
          IntegerVector r_gtyp = IntegerVector(1);
          r_gtyp[0] = (int)gtyp;
          feature_xx.push_back(r_gtyp);
        }
      }

      OGRFeature::DestroyFeature( poFeature );
      lFeature = lFeature + 1;
    }  //    if (iFeature >= skip_n[0]) {  // we are at skip_n

    iFeature = iFeature + 1;
    if (limit_n[0] > 0 && lFeature >= limit_n[0]) {
      break;  // short-circuit for limit_n
    }
  }
  // clean up if SQL was used https://www.gdal.org/classGDALDataset.html#ab2c2b105b8f76a279e6a53b9b4a182e0
  if (sql[0] != "") {
    poDS->ReleaseResultSet(poLayer);
  }
  GDALClose( poDS );
  if (lFeature < 1) {
    if (skip_n[0] > 0) {
      Rcpp::stop("no features to be read (is 'skip_n' set too high?");
    }

    Rcpp::stop("no features to be read");
  }

  return(feature_xx.vector());
}



inline List gdal_read_names(CharacterVector dsn,
                           IntegerVector layer,
                           CharacterVector sql,
                           IntegerVector limit_n,
                           IntegerVector skip_n,
                           NumericVector ex)
{

  GDALDataset       *poDS;
  Rprintf("0\n");
  poDS = (GDALDataset*) GDALOpenEx(dsn[0], GDAL_OF_VECTOR | GDAL_OF_READONLY, NULL, NULL, NULL );
  Rprintf("open\n");
  if( poDS == NULL )
  {
    Rcpp::stop("Open failed.\n");
  }

  Rprintf("0\n");
  OGRLayer *poLayer = gdal_layer(poDS, layer, sql = sql, ex =  ex);
  Rprintf("layer\n");
  OGRFeature *poFeature;
  poLayer->ResetReading();

  //OGRFeatureDefn *poFDefn = poLayer->GetLayerDefn();
  CollectorList feature_xx;
  int iFeature = 0;
  int lFeature = 0;
  double   nFeature = force_layer_feature_count(poLayer);


  if (nFeature > MAX_INT) {
    Rcpp::warning("Number of features exceeds maximal number able to be read");
   nFeature = MAX_INT;
  }

  if (limit_n[0] > 0) {
    if (limit_n[0] < nFeature) {
      nFeature = limit_n[0];

    }
  }

  if (nFeature < 1) {
    if (skip_n[0] > 0) {
      Rcpp::stop("no features to be read (is 'skip_n' set too high?");
    }

    Rcpp::stop("no features to be read");
  }

  double aFID;
  Rcpp::NumericVector rFID(1);
  Rprintf("0\n");
  while( (poFeature = poLayer->GetNextFeature()) != NULL )
  {

    if (iFeature >= skip_n[0]) {
      aFID = (double) poFeature->GetFID();
      OGRFeature::DestroyFeature( poFeature );
      rFID[0] = aFID;
      feature_xx.push_back(Rcpp::clone(rFID));
      lFeature++;
    }
    iFeature++;
    if (limit_n[0] > 0 && lFeature >= limit_n[0]) {
      break;  // short-circuit for limit_n
    }
    Rprintf("%i\n", iFeature);
  }
  // clean up if SQL was used https://www.gdal.org/classGDALDataset.html#ab2c2b105b8f76a279e6a53b9b4a182e0
  if (sql[0] != "") {
    poDS->ReleaseResultSet(poLayer);
  }
  GDALClose( poDS );

  if (lFeature < 1) {
    if (skip_n[0] > 0) {
      Rcpp::stop("no features to be read (is 'skip_n' set too high?");
    }

    Rcpp::stop("no features to be read");
  }
  return(feature_xx.vector());
}


inline CharacterVector gdal_proj_to_wkt(CharacterVector proj_str) {
  OGRSpatialReference oSRS;
  char *pszWKT = NULL;
  oSRS.importFromProj4(proj_str[0]);
  oSRS.exportToWkt(&pszWKT);
  CharacterVector out =  Rcpp::CharacterVector::create(pszWKT);
  CPLFree(pszWKT);

  return out;
}

inline List gdal_projection_info(CharacterVector dsn,
                                IntegerVector layer,
                                CharacterVector sql)
{
  GDALDataset       *poDS;
  poDS = (GDALDataset*) GDALOpenEx(dsn[0], GDAL_OF_VECTOR, NULL, NULL, NULL );
  if( poDS == NULL )
  {
    Rcpp::stop("Open failed.\n");
  }
  NumericVector zero(1);
  zero[0] = 0.0;
  OGRLayer *poLayer = gdal_layer(poDS, layer, sql, zero);

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
   // Rcpp::warning("not null");
    // SRS is not NULL, so explore validation
    //  OGRErr err = SRS->Validate();
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

  // clean up if SQL was used https://www.gdal.org/classGDALDataset.html#ab2c2b105b8f76a279e6a53b9b4a182e0
  if (sql[0] != "") {
    poDS->ReleaseResultSet(poLayer);
  }
  GDALClose( poDS );
  return info_out;
}


inline CharacterVector gdal_report_fields(Rcpp::CharacterVector dsource,
                                             Rcpp::IntegerVector layer = 0,
                                             Rcpp::CharacterVector sql = "")
{

  GDALDataset       *poDS;
  poDS = (GDALDataset*) GDALOpenEx(dsource[0], GDAL_OF_VECTOR, NULL, NULL, NULL );
  if( poDS == NULL )
  {
    Rcpp::stop("Open failed.\n");
  }

  OGRLayer *poLayer = gdal_layer(poDS, layer, sql, NumericVector::create(0));

  OGRFeature *poFeature;
  poLayer->ResetReading();

  OGRFeatureDefn *poFDefn = poLayer->GetLayerDefn();
  //int iFeature = 0;
  poFeature = poLayer->GetNextFeature();

  int fieldcount = poFDefn->GetFieldCount();
  Rcpp::CharacterVector out(fieldcount);
  Rcpp::CharacterVector fieldnames(fieldcount);
  // as at 2019-08-23 https://gdal.org/ogr__core_8h.html#a787194bea637faf12d61643124a7c9fc
  //   OFTInteger Simple32bitinteger
  //   OFTIntegerList Listof32bitintegers
  //   OFTReal DoublePrecisionfloatingpoint
  //   OFTRealList Listofdoubles
  //   OFTString StringofASCIIchars
  //   OFTStringList Arrayofstrings
  //   OFTWideString deprecated
  //   OFTWideStringList deprecated
  //   OFTBinary RawBinarydata
  //   OFTDate Date
  //   OFTTime Time
  //   OFTDateTime DateandTime
  //   OFTInteger64 Single64bitinteger
  //   OFTInteger64List Listof64bitintegers
  //
  if (poFeature != NULL)
  {

    int iField;
    for( iField = 0; iField < fieldcount; iField++ )
    {
      OGRFieldDefn *poFieldDefn = poFDefn->GetFieldDefn( iField );
      fieldnames[iField] = poFieldDefn->GetNameRef();
      if( poFieldDefn->GetType() == OFTBinary ) {
        out[iField] = "OFTBinary";
      }
      if( poFieldDefn->GetType() == OFTDate ) {
        out[iField] = "OFTDate";
      }
      if( poFieldDefn->GetType() == OFTDateTime ) {
        out[iField] = "OFTDateTime";
      }
      if( poFieldDefn->GetType() == OFTInteger ) {
        out[iField] = "OFTInteger";
      }
      if( poFieldDefn->GetType() == OFTInteger64 ) {
        out[iField] = "OFTInteger64";
      }
      if( poFieldDefn->GetType() == OFTInteger64List ) {
        out[iField] = "OFTInteger64List";
      }
      if( poFieldDefn->GetType() == OFTIntegerList ) {
        out[iField] = "OFTIntegerList";
      }
      if( poFieldDefn->GetType() == OFTReal ) {
        out[iField] = "OFTReal";
      }
      if( poFieldDefn->GetType() == OFTRealList ) {
        out[iField] = "OFTRealList";
      }
      if( poFieldDefn->GetType() == OFTString ) {
        out[iField] = "OFTString";
      }
      if( poFieldDefn->GetType() == OFTStringList ) {
        out[iField] = "OFTStringList";
      }
      if( poFieldDefn->GetType() == OFTTime ) {
        out[iField] = "OFTTime";
      }
      // if( poFieldDefn->GetType() == OFTWideString ) {
      //   out[iField] = "OFTWideString";
      // }
      // if( poFieldDefn->GetType() == OFTWideStringList ) {
      //   out[iField] = "OFTWideStringList";
      // }
    }

    OGRFeature::DestroyFeature( poFeature );

  }
  out.attr("names") = fieldnames;
  // clean up if SQL was used https://www.gdal.org/classGDALDataset.html#ab2c2b105b8f76a279e6a53b9b4a182e0
  if (sql[0] != "") {
    poDS->ReleaseResultSet(poLayer);
  }
  GDALClose( poDS );
  return(out);
}

inline CharacterVector  gdal_vsi_list(CharacterVector urlpath)
{
  char **VSI_paths = VSIReadDir(urlpath[0]);
  int ipath = 0; // iterate though MetadataDomainList
  while (VSI_paths && VSI_paths[ipath] != NULL) {
    ipath++;
  }

  Rcpp::CharacterVector names(ipath);
  for (int i = 0; i < ipath; i++) {
    names[i] = VSI_paths[i];
  }
  CSLDestroy(VSI_paths);
  return names;

}

inline CharacterVector gdal_sds_list(const char* pszFilename)
{
  GDALDataset  *poDataset;

  poDataset = (GDALDataset *) GDALOpen( pszFilename, GA_ReadOnly );
  if( poDataset == NULL )
  {
    Rcpp::stop("cannot open dataset");
  }

  char **MDdomain = GDALGetMetadataDomainList(poDataset);

  int mdi = 0; // iterate though MetadataDomainList
  bool has_sds = false;
  while (MDdomain && MDdomain[mdi] != NULL) {
    if (strcmp(MDdomain[mdi], "SUBDATASETS") == 0) {
      has_sds = true;
    }
    mdi++;
  }
  //cleanup
  CSLDestroy(MDdomain);

  int dscount = 1;
  if (has_sds) {
    char **SDS = GDALGetMetadata(poDataset, "SUBDATASETS");
    int sdi = 0;
    while (SDS && SDS[sdi] != NULL) {
      sdi++; // count
    }
    //this seems to be the wrong context in which to do this?
    //CSLDestroy(SDS);
    dscount = sdi;
  }
  Rcpp::CharacterVector ret(dscount);
  if (has_sds) {
    // we have subdatasets, so list them all
    char **SDS2 = GDALGetMetadata(poDataset, "SUBDATASETS");
    for (int ii = 0; ii < dscount; ii++) {
      ret(ii) = SDS2[ii];
    }
    //this seems to be the wrong context in which to do this?
    //CSLDestroy(SDS2);
  } else {
    ret[0] = pszFilename;
  }
  GDALClose( (GDALDatasetH) poDataset );
  return ret;
}


inline List gdal_raster_info(CharacterVector dsn, LogicalVector min_max)
{
  GDALDatasetH hDataset;

  hDataset = GDALOpenEx(dsn[0], GA_ReadOnly, nullptr, NULL, nullptr);

  if( hDataset == nullptr )
  {
    Rcpp::stop("cannot open dataset");
  }
  int nXSize = GDALGetRasterXSize(hDataset);
  int nYSize = GDALGetRasterYSize(hDataset);


  double        adfGeoTransform[6];

  //poDataset->GetGeoTransform( adfGeoTransform );
  GDALGetGeoTransform(hDataset, adfGeoTransform );

  // bail out NOW (we have no SDS and/or no rasters)
  // #f <- system.file("h5ex_t_enum.h5", package = "h5")
  if (GDALGetRasterCount(hDataset) < 1) {
    Rcpp::stop("no rasters found in dataset");
  }
  Rcpp::DoubleVector trans(6);
  for (int ii = 0; ii < 6; ii++) trans[ii] = adfGeoTransform[ii];

  char **pfilelist = GDALGetFileList(hDataset);
  CharacterVector FileList = CharacterVector::create(*pfilelist);

  GDALRasterBandH  hBand;
  int             nBlockXSize, nBlockYSize;
  //int             bGotMin, bGotMax;
  double          adfMinMax[2];

  hBand = GDALGetRasterBand(hDataset, 1);
  // if we don't bail out above with no rasters things go bad here
  GDALGetBlockSize(hBand, &nBlockXSize, &nBlockYSize);
  if (min_max[0]) {
    GDALComputeRasterMinMax(hBand, TRUE, adfMinMax);
  }

  int nn = 10;
  Rcpp::List out(nn);
  Rcpp::CharacterVector names(nn);
  out[0] = trans;
  names[0] = "geotransform";
  out[1] = Rcpp::IntegerVector::create(nXSize, nYSize);
  names[1] = "dimXY";
  //GDALGetMetadataDomainList(hBand,  )


  // FIXME: this is unused, how to get the compression type if present?
  char **MDdomain = GDALGetMetadataDomainList(hBand);

  int mdi = 0; // iterate though MetadataDomainList
  bool has_compress = false;
  while (MDdomain && MDdomain[mdi] != NULL) {
    if (strcmp(MDdomain[mdi], "COMPRESS") == 0) {
      has_compress = true;
    }
    mdi++;
  }
  //cleanup
  CSLDestroy(MDdomain);


  DoubleVector vmmx(2);
  if (min_max[0]) {
    vmmx[0] = adfMinMax[0];
    vmmx[1] = adfMinMax[1];
  } else {
    vmmx[0] = NA_REAL;
    vmmx[1] = NA_REAL;
  }
  out[2] = vmmx;
  names[2] = "minmax";

  out[3] = Rcpp::IntegerVector::create(nBlockXSize, nBlockYSize);
  names[3] = "tilesXY";

  const char *proj;
  proj = GDALGetProjectionRef(hDataset);
  //https://gis.stackexchange.com/questions/164279/how-do-i-create-ogrspatialreference-from-raster-files-georeference-c
  //char *proj_tmp = (char *) proj;
  out[4] = Rcpp::CharacterVector::create(proj);
  names[4] = "projection";

  // get band number
  int nBands = GDALGetRasterCount(hDataset);
  out[5] = nBands;
  names[5] = "bands";

  //char *stri;
  //OGRSpatialReference oSRS;
  //oSRS.importFromWkt(&proj_tmp);
  //oSRS.exportToProj4(&stri);
  out[6] =  Rcpp::CharacterVector::create(""); //Rcpp::CharacterVector::create(stri);
  names[6] = "proj4";

  int succ;
  out[7] = GDALGetRasterNoDataValue(hBand, &succ);
  names[7] = "nodata_value";

  int ocount = GDALGetOverviewCount(hBand);
  // f <- system.file("extdata/volcano_overview.tif", package = "vapour", mustWork = TRUE)
  IntegerVector oviews = IntegerVector(ocount * 2);
  //oviews[0] = ocount;
  if (ocount > 0) {
    GDALRasterBandH  oBand;
    for (int ii = 0; ii < ocount; ii++) {
      oBand = GDALGetOverview(hBand, ii);


      int xsize = GDALGetRasterBandXSize(oBand);
      int ysize = GDALGetRasterBandYSize(oBand);

      oviews[(ii * 2) + 0 ] = xsize;
      oviews[(ii * 2) + 1 ] = ysize;



    }
  }
  out[8] = oviews;
  names[8] = "overviews";

  out[9] = FileList;
  names[9] = "filelist";




  out.attr("names") = names;

  //CPLFree(stri);
  // close up
  GDALClose( hDataset );
  return out;

}


inline List gdal_raster_gcp(CharacterVector dsn) {
  // get GCPs if any
  GDALDatasetH hDataset;
  //GDALDataset  *poDataset;

  hDataset = GDALOpenEx( dsn[0], GA_ReadOnly, nullptr, NULL, nullptr);
  if( hDataset == nullptr )
  {
    Rcpp::stop("cannot open dataset");
  }

  int gcp_count;
  gcp_count = GDALGetGCPCount(hDataset);
  const char *srcWKT = GDALGetGCPProjection(hDataset);
  Rcpp::List gcpout(6);
  Rcpp::CharacterVector gcpnames(6);
  Rcpp::CharacterVector gcpCRS(1);
  gcpCRS[0] = srcWKT;
  gcpnames[0] = "Pixel";
  gcpnames[1] = "Line";
  gcpnames[2] = "X";
  gcpnames[3] = "Y";
  gcpnames[4] = "Z";
  gcpnames[5] = "CRS";
  gcpout.attr("names") = gcpnames;
  if (gcp_count > 0) {
    Rcpp::NumericVector GCPPixel(gcp_count);
    Rcpp::NumericVector GCPLine(gcp_count);
    Rcpp::NumericVector GCPX(gcp_count);
    Rcpp::NumericVector GCPY(gcp_count);
    Rcpp::NumericVector GCPZ(gcp_count);
    for (int igcp = 0; igcp < gcp_count; ++igcp) {
      const GDAL_GCP *gcp = GDALGetGCPs( hDataset ) + igcp;
      //const GDAL_GCP *gcp = poDataset->GetGCPs() + igcp;
      GCPPixel[igcp] = gcp->dfGCPPixel;
      GCPLine[igcp] = gcp->dfGCPLine;
      GCPX[igcp] = gcp->dfGCPX;
      GCPY[igcp] = gcp->dfGCPY;
      GCPZ[igcp] = gcp->dfGCPZ;
    }
    gcpout[0] = GCPPixel;
    gcpout[1] = GCPLine;
    gcpout[2] = GCPX;
    gcpout[3] = GCPY;
    gcpout[4] = GCPZ;
    gcpout[5] = gcpCRS;
    //gcp_proj = poDataset->GetGCPProjection();
  } else {
    Rprintf("No GCP (ground control points) found.\n");
  }
  GDALClose( hDataset );
  return gcpout;
}


inline List gdal_raster_io(CharacterVector dsn,
                   IntegerVector window,
                   IntegerVector band,
                   CharacterVector resample)
{

  int Xoffset = window[0];
  int Yoffset = window[1];
  int nXSize = window[2];
  int nYSize = window[3];

  int outXSize = window[4];
  int outYSize = window[5];

  GDALDataset  *poDataset;

  poDataset = (GDALDataset *) GDALOpen(dsn[0], GA_ReadOnly );
  if( poDataset == NULL )
  {
    Rcpp::stop("cannot open dataset");
  }

  GDALRasterBand  *poBand;
  poBand = poDataset->GetRasterBand( band[0] );
  GDALDataType band_type =  poBand->GetRasterDataType();

  if( poBand == NULL )
  {
    Rcpp::stop("cannot get band");
  }

  // how to do this is here:
  // https://stackoverflow.com/questions/45978178/how-to-pass-in-a-gdalresamplealg-to-gdals-rasterio
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


  double *double_scanline;
  int    *integer_scanline;

  List out(1);
  CPLErr err;

  bool band_type_not_supported = true;
  // here we catch byte, int* as R's 32-bit integer
  // or Float32/64 as R's 64-bit numeric
  if ((band_type == GDT_Byte) |
      (band_type == GDT_Int16) |
      (band_type == GDT_Int32) |
      (band_type == GDT_UInt16) |
      (band_type == GDT_UInt32)) {
    integer_scanline = (int *) CPLMalloc(sizeof(int)*
      static_cast<unsigned long>(outXSize)*
      static_cast<unsigned long>(outYSize));
    err = poBand->RasterIO( GF_Read, Xoffset, Yoffset, nXSize, nYSize,
                            integer_scanline, outXSize, outYSize, GDT_Int32,
                            0, 0, &psExtraArg);
    IntegerVector res(outXSize*outYSize);
    for (int i = 0; i < (outXSize*outYSize); i++) res[i] = integer_scanline[i];
    out[0] = res;
    band_type_not_supported = false;
  }
  if ((band_type == GDT_Float64) | (band_type == GDT_Float32)) {
    double_scanline = (double *) CPLMalloc(sizeof(double)*
      static_cast<unsigned long>(outXSize)*
      static_cast<unsigned long>(outYSize));
    err = poBand->RasterIO( GF_Read, Xoffset, Yoffset, nXSize, nYSize,
                            double_scanline, outXSize, outYSize, GDT_Float64,
                            0, 0, &psExtraArg);
    NumericVector res(outXSize*outYSize);
    for (int i = 0; i < (outXSize*outYSize); i++) res[i] = double_scanline[i];
    out[0] = res;

    band_type_not_supported = false;
  }


  // safe but lazy way of not supporting Complex, TypeCount or Unknown types
  // (see GDT_ checks above)
  if (band_type_not_supported) {
    Rcpp::stop("band type not supported (is it Complex? report at hypertidy/vapour/issues)");
  }
  if(err != CE_None) {
    // Report failure somehow.
    Rcpp::stop("raster read failed");
  }
  // close up
  GDALClose( (GDALDatasetH) poDataset );

  return out;
}


} // namespace gdalheaders
#endif
