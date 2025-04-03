#ifndef GDALLIBRARY_H
#define GDALLIBRARY_H
#include <Rcpp.h>
#include "ogrsf_frmts.h"
#include "gdal_priv.h"
#include "CollectorList.h"
#include "gdalraster/gdalraster.h"
#include "ogr_srs_api.h"

namespace gdallibrary {
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


inline IntegerVector gdal_set_config_option(CharacterVector option, CharacterVector value) 
{
  CPLSetConfigOption( option[0], value[0] );
  
  
  return 1;
}

inline CharacterVector gdal_get_config_option(CharacterVector option){
  CharacterVector out(1);
  const char *str = CPLGetConfigOption(option[0], nullptr);
  if (str) 
  {
    out[0] = str;
  }
  return out;
}


inline CharacterVector gdal_layer_geometry_name(OGRLayer *poLayer) {
  
  poLayer->ResetReading();
  
  OGRFeatureDefn *poFDefn = poLayer->GetLayerDefn();
  int gfields = poFDefn->GetGeomFieldCount();
  CharacterVector out(gfields);
  const char *geom_name;
  for (int ig = 0; ig < gfields; ig++) {
    OGRGeomFieldDefn *poGFDefn = poFDefn->GetGeomFieldDefn(ig);
    geom_name =    poGFDefn->GetNameRef();
    out[ig] = geom_name;
  }
  //OGRFeature::DestroyFeature( poFeature );
  return out;
}
inline NumericVector gdal_layer_extent(OGRLayer *poLayer) {
  
  OGREnvelope poEnvelope;
  OGRErr err; 
  err = poLayer ->GetExtent(&poEnvelope,true);
  if (err != CE_None) {
    Rprintf("problem in get extent\n");
  }
  NumericVector out(4); 
  out[0] = poEnvelope.MinX;
  out[1] = poEnvelope.MaxX;
  out[2] = poEnvelope.MinY;
  out[3] = poEnvelope.MaxY;
  return out;
}

// this force function takes cheap count, checks, then more expensive, checks,
// then iterates and resets reading
inline R_xlen_t force_layer_feature_count(OGRLayer *poLayer) {
  R_xlen_t out;
  out = poLayer->GetFeatureCount(false);
  if (out == -1) {
    out = poLayer->GetFeatureCount(true);
  }
  if (out == -1) {
    out = 0;
    poLayer->ResetReading();
    OGRFeature *poFeature;  
    while( (poFeature = poLayer->GetNextFeature()) != NULL )
    {
      out++; 
      OGRFeature::DestroyFeature( poFeature );
    }
    poLayer->ResetReading();
  }
  return out;
}
inline IntegerVector proj_version()
{
  Rcpp::IntegerVector out(3);

   
  int num1; int num2; int num3; 
  OSRGetPROJVersion(&num1, &num2, &num3); 
  out[0] = num1; 
  out[1] = num2; 
  out[2] = num3; 
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
  
  bool use_extent_filter = false;
  if (ex.length() == 4) {
    if (ex[1] <= ex[0] || ex[3] <= ex[2]) {
      if (ex[1] <= ex[0]) {
        Rcpp::warning("extent filter invalid (xmax <= xmin), ignoring");
      }
      if (ex[3] <= ex[2]) {
        Rcpp::warning("extent filter invalid (ymax <= ymin), ignoring");
      }
    } else {    
      use_extent_filter = true;
      ring.addPoint(ex[0], ex[2]); //xmin, ymin
      ring.addPoint(ex[0], ex[3]); //xmin, ymax
      ring.addPoint(ex[1], ex[3]); //xmax, ymax
      ring.addPoint(ex[1], ex[2]); //xmax, ymin
      ring.closeRings();
      poly.addRing(&ring);
    }
  }
  
  // Rcpp::Function vapour_getenv_sql_dialect("vapour_getenv_dialect"); 
  // const char *sql_dialect = (const char *) vapour_getenv_sql_dialect(); 
  // 
  Environment vapour = Environment::namespace_env("vapour");
  
  // Picking up  function from this package
  Function vapour_getenv_sql_dialect = vapour["vapour_getenv_sql_dialect"];
  //const char *sql_dialect = (const char *) vapour_getenv_sql_dialect();
  CharacterVector R_dialect = vapour_getenv_sql_dialect(); 
  const char *sql_dialect = (const char *)R_dialect[0]; 
  
  
  
  if (sql[0] != "") {
    if (use_extent_filter) {
      poLayer =  poDS->ExecuteSQL(sql[0],
                                  &poly,
                                  sql_dialect );
    } else {
      poLayer =  poDS->ExecuteSQL(sql[0],
                                  NULL,
                                  sql_dialect );
    }
    
    if (poLayer == NULL) {
      Rcpp::stop("SQL execution failed.\n");
    }
    
  } else {
    int nlayer = poDS->GetLayerCount();
    if (layer[0] >= nlayer) {
      //Rprintf("layer count: %i\n", nlayer);
      //Rprintf("layer index: %i\n", layer[0]);
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
    sname[idriver] = GDALGetDriverShortName(dr);
    lname[idriver] = GDALGetDriverLongName(dr);
    isvector[idriver] = (dr->GetMetadataItem(GDAL_DCAP_VECTOR) != NULL);
    israster[idriver] = (dr->GetMetadataItem(GDAL_DCAP_RASTER) != NULL);
    iscopy[idriver] = (dr->GetMetadataItem(GDAL_DCAP_CREATECOPY) != NULL);
    iscreate[idriver] = (dr->GetMetadataItem(GDAL_DCAP_CREATE) != NULL);
    isvirt[idriver] = (dr->GetMetadataItem(GDAL_DCAP_VIRTUALIO) != NULL);
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
inline Rcpp::List allocate_fields_list(OGRFeatureDefn *poFDefn, R_xlen_t n_features, bool int64_as_string,
                                       Rcpp::CharacterVector fid_column) {
  
  if (fid_column.size() > 1)
    Rcpp::stop("FID column name should be a length 1 character vector"); // #nocov
  
  // modified MDS
  //int n = poFDefn->GetFieldCount() + poFDefn->GetGeomFieldCount() + fid_column.size();
  int n = poFDefn->GetFieldCount(); 
  
  Rcpp::List out(n);
  Rcpp::CharacterVector names(n);
  for (int i = 0; i < poFDefn->GetFieldCount(); i++) {
    OGRFieldDefn *poFieldDefn = poFDefn->GetFieldDefn(i);
    
    /// stolen directly from sf and adapted thanks Edzer Pebesma
    switch (poFieldDefn->GetType()) {
    case OFTWideString:
    case OFTWideStringList: {
      // don't do anything these are deprecated (and probably will cause version issues ...)
    }
      break;
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
    case OFTTime: 
    case OFTDateTime: {
      Rcpp::NumericVector ret(n_features);
      Rcpp::CharacterVector cls(2);
      cls[0] = "POSIXct";
      cls[1] = "POSIXt";
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
    case OFTBinary:
      out[i] = Rcpp::List(n_features);
      break;
    case OFTString:
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
  OGRLayer *poLayer = gdal_layer(poDS, layer, sql, ex);
  
  OGRFeature *poFeature;
  
  //double  nFeature = force_layer_feature_count(poLayer);
  // trying to fix SQL problem 2020-10-05
  R_xlen_t nFeature = (R_xlen_t)poLayer->GetFeatureCount();
  
  
  //Rprintf("%i\n", nFeature);
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
  List out = allocate_fields_list(poFDefn, (int)nFeature, int64_as_string, fid_column_name);
  int iFeature = 0;  // always increment iFeature, it is position through the loop
  int lFeature = 0; // keep a count of the features we actually send out
  while((poFeature = poLayer->GetNextFeature()) != NULL)
  {
    
    if (lFeature >= nFeature) {
      break;
    }
    //OGRFeatureDefn *poFDefn = poLayer->GetLayerDefn();
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
        
        if( poFieldDefn->GetType() == OFTBinary ) {
          Rcpp::List nv;
          nv = out[iField];
          //nv = out[iField];
          int bytecount;
          const GByte *bin = poFeature->GetFieldAsBinary(iField, &bytecount);
          RawVector rb(bytecount);
          for (int ib = 0; ib < bytecount; ib++) {
            rb[ib] = bin[ib];
          }
          nv[lFeature] = rb;
        }
        
        
      }
      // so we start counting
      lFeature = lFeature + 1;
    }
    // always increment iFeature, it's position through the loop
    iFeature = iFeature + 1;
    OGRFeature::DestroyFeature( poFeature );
    //if (iFeature % 1000 == 0)   Rprintf("fields %i\n", iFeature);
    
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


inline NumericVector gdal_feature_count(CharacterVector dsn,
                                        IntegerVector layer, CharacterVector sql, NumericVector ex) {
  GDALDataset       *poDS = nullptr;
  poDS = (GDALDataset*) GDALOpenEx(dsn[0], GDAL_OF_READONLY | GDAL_OF_VECTOR, NULL, NULL, NULL );
  if( poDS == NULL )
  {
    Rcpp::stop("Open failed.\n");
  }
  
  OGRLayer *poLayer = gdal_layer(poDS, layer, sql, ex);
  
  //  double nFeature = force_layer_feature_count(poLayer);
  // trying to fix SQL problem 2020-10-05
  R_xlen_t nFeature = poLayer->GetFeatureCount();
  if (nFeature < 1) {
    nFeature = force_layer_feature_count(poLayer);
  }
  
  //Rprintf("%i\n", nFeature);
  // clean up if SQL was used https://www.gdal.org/classGDALDataset.html#ab2c2b105b8f76a279e6a53b9b4a182e0
  if (sql[0] != "") {
    poDS->ReleaseResultSet(poLayer);
  }
  GDALClose( (GDALDatasetH) poDS );
  
  NumericVector out(1);
  out[0] = static_cast<double>(nFeature);
  return(out);
}

inline CharacterVector gdal_driver(CharacterVector dsn)
{
  
  GDALDataset       *poDS;
  poDS = (GDALDataset*) GDALOpenEx(dsn[0], GDAL_OF_READONLY , NULL, NULL, NULL );  // gdal_driver(<any-type>)
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
  OGRLayer *poLayer = gdal_layer(poDS, layer, sql, ex);
  
  OGRFeature *poFeature;
  poLayer->ResetReading();
  
  //OGRFeatureDefn *poFDefn = poLayer->GetLayerDefn();
  CollectorList feature_xx;
  int iFeature = 0;
  int lFeature = 0;
  //int   nFeature = force_layer_feature_count(poLayer);
  // trying to fix SQL problem 2020-10-05
  R_xlen_t nFeature = poLayer->GetFeatureCount();
  if (nFeature < 1) {
    //Rprintf("force count\n");
    nFeature = force_layer_feature_count(poLayer);
  }
  
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
  
  
  //int warncount = 0;
  while( (poFeature = poLayer->GetNextFeature()) != NULL )
  {
    
    if (iFeature >= skip_n[0]) {  // we are at skip_n
      
      OGRGeometry *poGeometry;
      poGeometry = poFeature->GetGeometryRef();
      if (poGeometry == NULL) {
        //warncount++;
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

inline CharacterVector gdal_proj_to_wkt(CharacterVector proj_str) {
   OGRSpatialReference oSRS;
   char *pszWKT = nullptr;
   
   //const char*  crs_in[] = {CHAR(STRING_ELT(proj_str, 0))};
   
   oSRS.SetFromUserInput((const char*) proj_str[0]);
#if GDAL_VERSION_MAJOR >= 3
   const char *options[3] = { "MULTILINE=YES", "FORMAT=WKT2", NULL };
   OGRErr err = oSRS.exportToWkt(&pszWKT, options);
#else
   OGRErr err = oSRS.exportToWkt(&pszWKT);
#endif


  CharacterVector out = Rcpp::CharacterVector::create("not a WKT string"); 
  if (err) {
     out =  Rcpp::CharacterVector::create(NA_STRING);
   } else {
     out =  Rcpp::CharacterVector::create(pszWKT);
   }
   if (pszWKT != nullptr) CPLFree(pszWKT);
   
  return out;
}


// R version
// inline CharacterVector gdal_proj_to_wkt(SEXP proj_str) {
//   OGRSpatialReference oSRS;
//   char *pszWKT = nullptr;
//   const char*  crs_in[] = {CHAR(STRING_ELT(proj_str, 0))};
//   
//   oSRS.SetFromUserInput(*crs_in);
// #if GDAL_VERSION_MAJOR >= 3
//   const char *options[3] = { "MULTILINE=YES", "FORMAT=WKT2", NULL };
//   OGRErr err = oSRS.exportToWkt(&pszWKT, options);
// #else
//   OGRErr err = oSRS.exportToWkt(&pszWKT);
// #endif
//   
//   //CharacterVector out; 
//   SEXP out = PROTECT(Rf_allocVector(STRSXP, 1));
// 
//   if (err) {
//     SET_STRING_ELT(out, 0, NA_STRING);
//   } else {
//     SET_STRING_ELT(out, 0, Rf_mkChar(pszWKT));
//   }
//   UNPROTECT(1); 
//   return out;
// }

inline LogicalVector gdal_crs_is_lonlat(SEXP proj_str) {
  const char*  crs_in[] = {CHAR(STRING_ELT(proj_str, 0))};
  
  OGRSpatialReference oSRS; 
  oSRS.SetFromUserInput(*crs_in);
  //LogicalVector out = LogicalVector::create(false); 
  SEXP out = PROTECT(Rf_allocVector(LGLSXP, 1));
  SET_LOGICAL_ELT(out, 0, false); 
  if (oSRS.IsGeographic()) {
    SET_LOGICAL_ELT(out, 0, true); 

  }
  UNPROTECT(1); 
  return out;
}


inline List gdal_projection_info(CharacterVector dsn,
                                 IntegerVector layer,
                                 CharacterVector sql)
{
  
  GDALDataset     *poDS = nullptr;
  poDS = (GDALDataset*) GDALOpenEx(dsn[0], GDAL_OF_VECTOR, NULL, NULL, NULL );
  
  if( poDS == nullptr )
  {
    Rcpp::stop("Open failed.\n");
  }
  
  NumericVector zero(1);
  zero[0] = 0.0;
  OGRLayer *poLayer = gdal_layer(poDS, layer, sql, zero);
  OGRSpatialReference *SRS =  poLayer->GetSpatialRef();
  
  //  char *proj;  // this gets cleaned up lower in the SRS==NULL else
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
  if (SRS == nullptr) {
    //Rcpp::warning("null");
    // do nothing, or warn
    // e.g. .shp with no .prj
  }  else {
    //   // Rcpp::warning("not null");
    //   // SRS is not NULL, so explore validation
    //   //  OGRErr err = SRS->Validate();
    char *proj4 = nullptr;
    SRS->exportToProj4(&proj4);
    if (proj4 != nullptr) {
      info_out[0] = CharacterVector::create(proj4); 
      CPLFree(proj4);
    }
    char *MI = nullptr;
    SRS->exportToMICoordSys(&MI);
    if (MI != nullptr) {
      info_out[1] = CharacterVector::create(MI); 
      CPLFree(MI);
    } 
    char *pwkt = nullptr;
    SRS->exportToPrettyWkt(&pwkt);
    if (pwkt != nullptr) {
      info_out[2] = CharacterVector::create(pwkt); 
      CPLFree(pwkt);
    } 
    char *uwkt = nullptr;
    SRS->exportToWkt(&uwkt);
    if (uwkt != nullptr) {
      info_out[3] = CharacterVector::create(uwkt); 
      CPLFree(uwkt);
    } 
    int epsg = SRS->GetEPSGGeogCS();
    info_out[4] =  IntegerVector::create(epsg);  
    
    char *xml = nullptr;
    SRS->exportToXML(&xml);
    if (xml != nullptr) {
      info_out[5] = CharacterVector::create(xml); 
      CPLFree(xml);
    }
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
  
  // char** VSI_paths;
  // if ( STARTS_WITH(urlpath[0], "/vsizip/") ||
  //      STARTS_WITH(urlpath[0], "/vsitar/") )
  // {
  //   VSI_paths = VSIReadDirRecursive(urlpath[0]);
  // } else
  // {
  //   VSI_paths = VSIReadDirRecursive(urlpath[0]);
  // }
  //
  char** VSI_paths  = VSIReadDirRecursive(urlpath[0]);
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



// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------



} // namespace gdallibrary
#endif
