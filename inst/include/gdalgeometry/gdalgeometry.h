#ifndef GDALGEOMETRY_H
#define GDALGEOMETRY_H
#include <Rcpp.h>
#include "ogrsf_frmts.h"
//#include "geos.h"
//#include "ogr_api.h"
#include "gdal_priv.h"
// #include "ogr_spatialref.h" // for OGRSpatialReference
// #include "cpl_conv.h" // for CPLFree()

#include "gdallibrary/gdallibrary.h"
#include "gdalmiscutils/gdalmiscutils.h"
namespace gdalgeometry {
using namespace Rcpp;

// low level geometry converters from feature ----------------------------------------------
//FIXME
// o add cast capability?
// o add ExportToGEOS?
// o boundary, buffer, centroid, convexhull, delaunaytriangulation, polygonize, segmentize,
//   simplify(preservetopology), totriangulatedsurface
// o various To<type> casts,
// o allow control over wkbNDR and wkbVariant
// see here for the constants for the format variants
// http://www.gdal.org/ogr__core_8h.html#a6716bd3399c31e7bc8b0fd94fd7d9ba6a7459e8d11fa69e89271771c8d0f265d8
inline RawVector gdal_geometry_raw(OGRFeature *poFeature) {
  if (poFeature->GetGeometryRef()) {
    Rcpp::RawVector raw(poFeature->GetGeometryRef()->WkbSize());
    poFeature->GetGeometryRef()->exportToWkb(wkbNDR, &(raw[0]), wkbVariantIso);
    return raw;
    
  } else {
    Rcpp::RawVector raw(0); 
    return raw; 
  }
}


// SEXP geos_common_xptr(GEOSGeometry* geometry) {
//   SEXP externalPtr = R_MakeExternalPtr((void *) geometry, R_NilValue, R_NilValue);
//   return externalPtr;
// }
// inline SEXP gdal_geometry_geos(OGRFeature *poFeature) {
//   if (poFeature->GetGeometryRef()) {
//     GEOSContextHandle_t ctxt = OGRGeometry::createGEOSContext();
//     GEOSGeom geosGeom = geom.exportToGEOS(ctxt);
//     OGRGeometry::freeGEOSContext(ctxt);
//     
//     GEOSGeom geosGeom  = poFeature->GetGeometryRef()->exportToGEOS(ctxt); 
//     GEOSGeom_destroy_r(ctxt);
//     if (nullptr != geosGeom) {
//       return geos_common_xptr(geosGeom); 
//     }
//   } 
//   return R_NilValue;  
// }
inline CharacterVector gdal_geometry_wkt(OGRFeature *poFeature) {
  CharacterVector wkt(1);
  if (poFeature->GetGeometryRef()) {
    char *pszGEOM_WKT = NULL;
    poFeature->GetGeometryRef()->exportToWkt(&pszGEOM_WKT, wkbVariantIso );
    wkt[0] = pszGEOM_WKT;
    CPLFree( pszGEOM_WKT );
  } else {
    wkt[0] = NA_STRING; 
  }
  return wkt;
}
inline CharacterVector gdal_geometry_txt(OGRFeature *poFeature, CharacterVector format) {
  char *export_txt = NULL;
  CharacterVector txt(1);
  if (poFeature->GetGeometryRef()) {
  if (format[0] == "gml") {
    export_txt = poFeature->GetGeometryRef()->exportToGML();
  }
  if (format[0] == "json") {
    export_txt = poFeature->GetGeometryRef()->exportToJson();
  }
  if (format[0] == "kml") {
    export_txt = poFeature->GetGeometryRef()->exportToKML();
  }
  txt[0] = export_txt;
  } else {
    txt[0] = NA_STRING; 
  }
  CPLFree(export_txt);
  return txt;
}
inline IntegerVector gdal_geometry_type(OGRFeature*poFeature) {
  OGRGeometry *poGeom = poFeature->GetGeometryRef(); 
  
  
  OGRBoolean empty;
  if (poGeom) {
    empty = poFeature->GetGeometryRef()->IsEmpty(); 
  } else {
    empty = 1; 
  }
  
  Rcpp::IntegerVector r_gtyp = Rcpp::IntegerVector(1);
  if (empty) {
    r_gtyp = NA_INTEGER; 
  } else {
    OGRwkbGeometryType gtyp = OGR_G_GetGeometryType( poFeature->GetGeometryRef());
    r_gtyp[0] = (int)gtyp;
  }
  return r_gtyp;
}
inline NumericVector gdal_geometry_extent(OGRFeature *poFeature) {
  
  double minx, maxx, miny, maxy;
  minx = NA_REAL;
  maxx = NA_REAL;
  miny = NA_REAL;
  maxy = NA_REAL;
  
  if (poFeature->GetGeometryRef()) {
   OGREnvelope env;
   OGR_G_GetEnvelope(poFeature->GetGeometryRef(), &env);
   if (poFeature->GetGeometryRef()->IsEmpty()) {
     
   } else {
     minx = env.MinX;
     maxx = env.MaxX;
     miny = env.MinY;
     maxy = env.MaxY;
   }
  }
  NumericVector extent = NumericVector::create(minx, maxx, miny, maxy);
  return extent;
}

// --------------------------------------------------------------------------------------


inline NumericVector layer_read_fids_ij(OGRLayer *poLayer, NumericVector ij) {
  R_xlen_t st = (R_xlen_t)ij[0]; 
  R_xlen_t en = (R_xlen_t)ij[1]; 
  NumericVector out(en - st + 1);
  std::fill( out.begin(), out.end(), NumericVector::get_na() );
  OGRFeature *poFeature;
  R_xlen_t cnt = 0;
  R_xlen_t ii = 0;
  
  while(ii <= en && (poFeature = poLayer->GetNextFeature()) != NULL ) {
    if (ii >= st) {
      out[cnt] = (double)poFeature->GetFID();
      cnt++;
    }
    ii++;
    OGRFeature::DestroyFeature(poFeature);
  }
  if (cnt < out.length()) {
    Rcpp::warning("not as many FIDs as requested");
  }
  return out;
}

/// layer READ FIDS ----------------------------------------------------------------------------
inline NumericVector layer_read_fids_all(OGRLayer *poLayer) {
  R_xlen_t nFeature = poLayer->GetFeatureCount();
  
  NumericVector ij(2);
  ij[0] = 0;
  /// if there are no features we get 0 fields allocation
  ij[1] = (double)(nFeature - 1); 
  
  NumericVector out = layer_read_fids_ij(poLayer, ij);
  return out;
}

inline NumericVector layer_read_fids_ia(OGRLayer *poLayer, NumericVector ia) {
  
  NumericVector out(ia.length());
  std::fill( out.begin(), out.end(), NumericVector::get_na() );
  OGRFeature *poFeature;
  R_xlen_t ii = 0;
  R_xlen_t cnt = 0;
  while( (poFeature = poLayer->GetNextFeature()) != NULL ) {
    if (ii == (R_xlen_t)(ia[cnt])) {
      out[cnt] = (double)poFeature->GetFID();
      cnt++;
    }
    ii++;
    OGRFeature::DestroyFeature(poFeature);
  }
  if (cnt < out.length()) {
    Rcpp::warning("not all FIDS found");
  }
  return out;
}


inline List feature_read_geom(OGRFeature *poFeature, CharacterVector format) {
  List out(1); 
  // work through format
  // FIXME: get rid of "geometry"
  if ((format[0] == "wkb")) { // || (format[0] == "geometry")) {
    out[0] = gdal_geometry_raw(poFeature);
  }
  if (format[0] == "wkt") {
    out[0] = gdal_geometry_wkt(poFeature);
  }
  // FIXME: maybe call it envelope not extent
  if (format[0] == "extent") {
    out[0] = gdal_geometry_extent(poFeature);
  }
  // these are all just text variants (wkt uses a different mech)
  if ((format[0] == "gml") || (format[0] == "json") || (format[0] == "kml")) {
    out[0] = gdal_geometry_txt(poFeature, format);
  }
  if (format[0] == "type") {
    out[0] = gdal_geometry_type(poFeature);
  }
  
  return out;
}
/// layer READ GEOMS ----------------------------------------------------------------------------

inline List layer_read_geom_ij(OGRLayer *poLayer, CharacterVector format, NumericVector ij) {
  OGRFeature *poFeature;
  R_xlen_t st = (R_xlen_t)ij[0]; 
  R_xlen_t en = (R_xlen_t)ij[1]; 
  
  List out(en - st + 1);
  R_xlen_t ii = 0;
  R_xlen_t cnt = 0;
  
  while(ii <= en &&  (poFeature = poLayer->GetNextFeature()) != NULL ) {
    if (ii >= st) {
      out[cnt] = feature_read_geom(poFeature, format)[0]; 
      cnt++;
    }
    OGRFeature::DestroyFeature(poFeature);
    ii++;
  }
  if (cnt < out.length()) {
    Rcpp::warning("not as many geoms as requested");
  }
  return out;
}

inline List layer_read_geom_all(OGRLayer *poLayer, CharacterVector format) {
  R_xlen_t nFeature = poLayer->GetFeatureCount();
  
  NumericVector ij(2);
  ij[0] = 0;
  /// if there are no features we get 0 fields allocation
  ij[1] = (double)(nFeature - 1); 
  
  List out = layer_read_geom_ij(poLayer, format, ij);
  return out;
}

inline List layer_read_geom_ia(OGRLayer *poLayer, CharacterVector format, NumericVector ia) {
  OGRFeature *poFeature;
  
  poLayer->ResetReading();
  
  List out(ia.length());
  R_xlen_t ii = 0;
  R_xlen_t cnt = 0;
  
  while( (poFeature = poLayer->GetNextFeature()) != NULL ) {
    if (ii == static_cast<R_xlen_t>(ia[cnt])) {
      out[cnt] = feature_read_geom(poFeature, format)[0]; 
      cnt++;
      
    }
    ii++;
    OGRFeature::DestroyFeature(poFeature);
    if (cnt == ia.length()) break; 
  }
  
  return out;
}

inline List layer_read_geom_fa(OGRLayer *poLayer, CharacterVector format, NumericVector fa) {
  OGRFeature *poFeature;
  //poLayer->ResetReading();
  
  List out(fa.length());
  
  for (R_xlen_t ii = 0; ii < fa.length(); ii++) {
    GIntBig feature_id = (GIntBig)fa[ii];
    poFeature = poLayer->GetFeature(feature_id);
    out[ii] = feature_read_geom(poFeature, format)[0]; 
    OGRFeature::DestroyFeature(poFeature);
  }
  return out;
}

/// layer READ FIELDS ----------------------------------------------------------------------------

// _alll uses _ij so this comes first
inline List layer_read_fields_ij(OGRLayer *poLayer, CharacterVector fid_column_name,
                                 NumericVector ij) {
  
  R_xlen_t st = (R_xlen_t)ij[0]; 
  R_xlen_t en = (R_xlen_t)ij[1]; 
  OGRFeatureDefn *poFDefn = poLayer->GetLayerDefn();
  bool int64_as_string = false;
  // special case if ij[1] < ij[0] 
  R_xlen_t nfeatures = en - st + 1;
  if (en < st) nfeatures = 0; 
  List out = gdallibrary::allocate_fields_list(poFDefn, nfeatures, 
                                               int64_as_string, fid_column_name);
  OGRFeature *poFeature;
  R_xlen_t cnt = 0;
  R_xlen_t ii = 0;
  int iField;
  int bytecount; 
  int not_NA; 
  
  // for OFTDateTime
  Rcpp::Function ISOdatetime("ISOdatetime");
  Rcpp::Function ISOdate("ISOdate");
  while(ii <= en &&  (poFeature = poLayer->GetNextFeature()) != NULL ) {
    if (ii >= st) {
      for(iField = 0; iField < poFDefn->GetFieldCount(); iField++ )
      {
        OGRFieldDefn *poFieldDefn = poFDefn->GetFieldDefn( iField );
        not_NA = poFeature->IsFieldSetAndNotNull(iField);
        
        /// stolen directly from sf and adapted thanks Edzer Pebesma
        switch( poFieldDefn->GetType() ) {
        case OFTWideString:
        case OFTWideStringList: {
          // don't do anything these are deprecated (and probably will cause version issues ...)
        }
          break;
        case OFTString: {
          CharacterVector cv;
          cv = out[iField];
          if (not_NA) {
            cv[cnt] = poFeature->GetFieldAsString( iField );
          } else {
            cv[cnt] = NA_STRING;
          }
          
        } break;
        case OFTInteger: {
          IntegerVector iv;
          iv = out[iField];
          if (! not_NA) {
            iv[cnt] = NA_INTEGER;
            break;
          }
          // does this need subtype handling (should allow raw instead of integer for OSTBoolean)
          iv[cnt] = poFeature->GetFieldAsInteger( iField );

        }  break;
        case OFTTime:
        case OFTDateTime:
        case OFTDate: {
          NumericVector nv;
          nv = out[iField];

          if (! not_NA) {
            nv[cnt] = NA_REAL;
            break;
          }
       
          int Year, Month, Day, Hour, Minute, TZFlag;
          float Second;
          const char *tzone = "";
         
          poFeature->GetFieldAsDateTime(iField, &Year, &Month, &Day, &Hour, &Minute,
                                        &Second, &TZFlag);
          if (TZFlag == 100)
            tzone = "UTC";
          if (poFieldDefn->GetType() == OFTDateTime || poFieldDefn->GetType() == OFTTime) {
            if (cnt == 0 && poFieldDefn->GetType() == OFTTime) {
              Rcpp::warning("field of type 'OFTTime' converted to POSIXct: %s", poFDefn->GetFieldDefn(iField)->GetNameRef()); 
            }
            Rcpp::NumericVector ret = ISOdatetime(Year, Month, Day, Hour, Minute, Second, tzone); 
            nv[cnt] = ret[0];
          } else {
            Rcpp::NumericVector ret = ISOdate(Year, Month, Day); 
            nv[cnt] = ret[0];
          }
          break;
        }
          break;
        case OFTInteger64: {
          NumericVector iiv;
          iiv = out[iField];
          if (! not_NA) {
            iiv[cnt] = NA_REAL;
            break;
          }
          iiv[cnt] = poFeature->GetFieldAsDouble( iField );
        }
          break;
        case OFTReal: {
          NumericVector nv;
          nv = out[iField];
          if (! not_NA) {
            nv[cnt] = NA_REAL;
            break;
          }
          nv[cnt] = poFeature->GetFieldAsDouble( iField );

        }
          break;
          case OFTStringList: {
            // List slv;
            // slv = out[iField];
            //
            // slv[cnt] = poFeature->GetFieldAsStringList(iField);
            //
          }
            break;
          case OFTRealList: {
            // List rlv;
            // rlv = out[iField];
            // int rlcount;
            // rlv[cnt] = poFeature->GetFieldAsDoubleList(iField, &rlcount);

          }
            break;
          case OFTIntegerList: {
            // List ilv;
            // ilv = out[iField];
            // int ilcount;
            // ilv[cnt] = poFeature->GetFieldAsIntegerList(iField, &ilcount);
          }
            break;
          case OFTInteger64List: {
            // List iilv;
            // iilv = out[iField];
            // int iilcount;
            // iilv[cnt] = poFeature->GetFieldAsInteger64List(iField, &iilcount);

          }
            break;

        case OFTBinary: {
          Rcpp::List bv;
          bv = out[iField];

          const GByte *bin = poFeature->GetFieldAsBinary(iField, &bytecount);
          RawVector rb(bytecount);
          for (int ib = 0; ib < bytecount; ib++) {
            rb[ib] = bin[ib];
          }
          bv[cnt] = rb;
        } break; 
      }  // end switch
      }
      cnt++;
    } // end if
    ii++;
    OGRFeature::DestroyFeature(poFeature);
  } // end get next feature
  if (cnt < nfeatures) {
    Rcpp::warning("not as many features as requested");
  }
  return out;
}


inline List layer_read_fields_all(OGRLayer *poLayer, CharacterVector fid_column_name) {
  R_xlen_t nFeature = poLayer->GetFeatureCount();

  NumericVector ij(2);
  ij[0] = 0;
  /// if there are no features we get 0 fields allocation
  ij[1] = (double)(nFeature - 1); 
 
  List out = layer_read_fields_ij(poLayer, fid_column_name, ij);
  return out;
}
inline List layer_read_fields_ia(OGRLayer *poLayer, CharacterVector fid_column_name,
                                 NumericVector ia) {
  R_xlen_t   nFeature = ia.length();
  
  OGRFeatureDefn *poFDefn = poLayer->GetLayerDefn();
  bool int64_as_string = false;
  List out = gdallibrary::allocate_fields_list(poFDefn, nFeature, int64_as_string, fid_column_name);
  
  OGRFeature *poFeature;
  
  R_xlen_t ii = 0;
  R_xlen_t cnt = 0;
  int iField;
  
  while( (poFeature = poLayer->GetNextFeature()) != NULL ) {
    
    if (ii == static_cast<R_xlen_t>(ia[cnt])) {
      
      // FIXME copy the wider field support from sf (but how to make this a function)
      for( iField = 0; iField < poFDefn->GetFieldCount(); iField++ )
      {
        OGRFieldDefn *poFieldDefn = poFDefn->GetFieldDefn( iField );
        if( poFieldDefn->GetType() == OFTInteger   ) {
          IntegerVector nv;
          nv = out[iField];
          nv[cnt] = poFeature->GetFieldAsInteger( iField );
        }
        
        if( poFieldDefn->GetType() == OFTReal || poFieldDefn->GetType() == OFTInteger64) {
          NumericVector nv;
          nv = out[iField];
          nv[cnt] = poFeature->GetFieldAsDouble( iField );
        }
        
        if( poFieldDefn->GetType() == OFTString || poFieldDefn->GetType() == OFTDate || poFieldDefn->GetType() == OFTTime || poFieldDefn->GetType() == OFTDateTime) {
          CharacterVector nv;
          nv = out[iField];
          nv[cnt] = poFeature->GetFieldAsString( iField );
          
        }
      }
      
      cnt++;
    }
    
    OGRFeature::DestroyFeature(poFeature);
    ii++;
  }
  return out;
}

inline List layer_read_fields_fa(OGRLayer *poLayer, CharacterVector fid_column_name,
                                 NumericVector fa) {
  R_xlen_t   nFeature = fa.length();
  
  OGRFeatureDefn *poFDefn = poLayer->GetLayerDefn();
  bool int64_as_string = false;
  List out = gdallibrary::allocate_fields_list(poFDefn, nFeature, int64_as_string, fid_column_name);
  
  OGRFeature *poFeature;
  R_xlen_t cnt = 0;
  int iField;
  
  for (R_xlen_t ii = 0; ii < fa.length(); ii++) {
    GIntBig feature_id = (GIntBig)fa[ii];
    poFeature = poLayer->GetFeature(feature_id);
    if (NULL == poFeature) {
      Rcpp::warning("FID not found %i", fa[ii]);
    } else {
      // FIXME copy the wider field support from sf (but how to make this a function)
      for( iField = 0; iField < poFDefn->GetFieldCount(); iField++ )
      {
        OGRFieldDefn *poFieldDefn = poFDefn->GetFieldDefn( iField );
        if( poFieldDefn->GetType() == OFTInteger   ) {
          IntegerVector nv;
          nv = out[iField];
          nv[cnt] = poFeature->GetFieldAsInteger( iField );
        }
        
        if( poFieldDefn->GetType() == OFTReal || poFieldDefn->GetType() == OFTInteger64) {
          NumericVector nv;
          nv = out[iField];
          nv[cnt] = poFeature->GetFieldAsDouble( iField );
        }
        
        if( poFieldDefn->GetType() == OFTString || poFieldDefn->GetType() == OFTDate || poFieldDefn->GetType() == OFTTime || poFieldDefn->GetType() == OFTDateTime) {
          CharacterVector nv;
          nv = out[iField];
          nv[cnt] = poFeature->GetFieldAsString( iField );
          
        }
      }
      OGRFeature::DestroyFeature(poFeature);
    }
    cnt++;
  }
  return out;
}


// DSN read
inline NumericVector dsn_read_fids_all(CharacterVector dsn, IntegerVector layer,
                                       CharacterVector sql, NumericVector ex) {
  
  GDALDataset       *poDS;
  poDS = (GDALDataset*) GDALOpenEx(dsn[0], GDAL_OF_VECTOR, NULL, NULL, NULL );
  if( poDS == NULL )
  {
    Rcpp::stop("Open failed.\n");
  }
  OGRLayer *poLayer = gdallibrary::gdal_layer(poDS, layer, sql, ex);
  NumericVector out = layer_read_fids_all(poLayer);
  // clean up if SQL was used https://www.gdal.org/classGDALDataset.html#ab2c2b105b8f76a279e6a53b9b4a182e0
  if (sql[0] != "") {
    poDS->ReleaseResultSet(poLayer);
  }
  GDALClose(poDS);
  return out;
}
inline NumericVector dsn_read_fids_ij(CharacterVector dsn, IntegerVector layer,
                                      CharacterVector sql, NumericVector ex,
                                      NumericVector ij) {
  
  GDALDataset       *poDS;
  poDS = (GDALDataset*) GDALOpenEx(dsn[0], GDAL_OF_VECTOR, NULL, NULL, NULL );
  if( poDS == NULL )
  {
    Rcpp::stop("Open failed.\n");
  }
  OGRLayer *poLayer = gdallibrary::gdal_layer(poDS, layer, sql, ex);
  NumericVector out = layer_read_fids_ij(poLayer, ij);
  // clean up if SQL was used https://www.gdal.org/classGDALDataset.html#ab2c2b105b8f76a279e6a53b9b4a182e0
  if (sql[0] != "") {
    poDS->ReleaseResultSet(poLayer);
  }
  GDALClose(poDS);
  return out;
}

inline NumericVector dsn_read_fids_ia(CharacterVector dsn, IntegerVector layer,
                                      CharacterVector sql, NumericVector ex,
                                      NumericVector ia) {
  
  GDALDataset       *poDS;
  poDS = (GDALDataset*) GDALOpenEx(dsn[0], GDAL_OF_VECTOR, NULL, NULL, NULL );
  if( poDS == NULL )
  {
    Rcpp::stop("Open failed.\n");
  }
  OGRLayer *poLayer = gdallibrary::gdal_layer(poDS, layer, sql, ex);
  NumericVector out = layer_read_fids_ia(poLayer, ia);
  // clean up if SQL was used https://www.gdal.org/classGDALDataset.html#ab2c2b105b8f76a279e6a53b9b4a182e0
  if (sql[0] != "") {
    poDS->ReleaseResultSet(poLayer);
  }
  GDALClose(poDS);
  return out;
}
inline List dsn_read_geom_all(CharacterVector dsn, IntegerVector layer,
                              CharacterVector sql, NumericVector ex,
                              CharacterVector format) {
  
  GDALDataset       *poDS;
  poDS = (GDALDataset*) GDALOpenEx(dsn[0], GDAL_OF_VECTOR, NULL, NULL, NULL );
  if( poDS == NULL )
  {
    Rcpp::stop("Open failed.\n");
  }
  OGRLayer *poLayer = gdallibrary::gdal_layer(poDS, layer, sql, ex);
  List out = layer_read_geom_all(poLayer, format);
  // clean up if SQL was used https://www.gdal.org/classGDALDataset.html#ab2c2b105b8f76a279e6a53b9b4a182e0
  if (sql[0] != "") {
    poDS->ReleaseResultSet(poLayer);
  }
  GDALClose(poDS);
  return out;
}
inline List dsn_read_geom_ij(CharacterVector dsn, IntegerVector layer,
                             CharacterVector sql, NumericVector ex,
                             CharacterVector format, NumericVector ij) {
  
  GDALDataset       *poDS;
  poDS = (GDALDataset*) GDALOpenEx(dsn[0], GDAL_OF_VECTOR, NULL, NULL, NULL );
  if( poDS == NULL )
  {
    Rcpp::stop("Open failed.\n");
  }
  OGRLayer *poLayer = gdallibrary::gdal_layer(poDS, layer, sql, ex);
  List out = layer_read_geom_ij(poLayer, format, ij);
  // clean up if SQL was used https://www.gdal.org/classGDALDataset.html#ab2c2b105b8f76a279e6a53b9b4a182e0
  if (sql[0] != "") {
    poDS->ReleaseResultSet(poLayer);
  }
  GDALClose(poDS);
  return out;
}


inline List dsn_read_geom_ia(CharacterVector dsn, IntegerVector layer,
                             CharacterVector sql, NumericVector ex,
                             CharacterVector format, NumericVector ia) {
  
  GDALDataset       *poDS;
  poDS = (GDALDataset*) GDALOpenEx(dsn[0], GDAL_OF_VECTOR, NULL, NULL, NULL );
  if( poDS == NULL )
  {
    Rcpp::stop("Open failed.\n");
  }
  OGRLayer *poLayer = gdallibrary::gdal_layer(poDS, layer, sql, ex);
  List out = layer_read_geom_ia(poLayer, format, ia);
  // clean up if SQL was used https://www.gdal.org/classGDALDataset.html#ab2c2b105b8f76a279e6a53b9b4a182e0
  if (sql[0] != "") {
    poDS->ReleaseResultSet(poLayer);
  }
  GDALClose(poDS);
  return out;
}

inline List dsn_read_geom_fa(CharacterVector dsn, IntegerVector layer,
                             CharacterVector sql, NumericVector ex,
                             CharacterVector format, NumericVector fa) {
  
  GDALDataset       *poDS;
  poDS = (GDALDataset*) GDALOpenEx(dsn[0], GDAL_OF_VECTOR, NULL, NULL, NULL );
  if( poDS == NULL )
  {
    Rcpp::stop("Open failed.\n");
  }
  OGRLayer *poLayer = gdallibrary::gdal_layer(poDS, layer, sql, ex);
  List out = layer_read_geom_fa(poLayer, format, fa);
  // clean up if SQL was used https://www.gdal.org/classGDALDataset.html#ab2c2b105b8f76a279e6a53b9b4a182e0
  if (sql[0] != "") {
    poDS->ReleaseResultSet(poLayer);
  }
  GDALClose(poDS);
  return out;
}

/// dsn READ FIELDS ----------------------------------------------------------------------------
inline List dsn_read_fields_all(CharacterVector dsn, IntegerVector layer,
                                CharacterVector sql, NumericVector ex,
                                CharacterVector fid_column_name) {
  
  GDALDataset       *poDS;
  poDS = (GDALDataset*) GDALOpenEx(dsn[0], GDAL_OF_VECTOR, NULL, NULL, NULL );
  
  if( poDS == NULL )
  {
    Rcpp::stop("Open failed.\n");
  }
  OGRLayer *poLayer = gdallibrary::gdal_layer(poDS, layer, sql, ex);
  
  List out = layer_read_fields_all(poLayer, fid_column_name);
  
  // clean up if SQL was used https://www.gdal.org/classGDALDataset.html#ab2c2b105b8f76a279e6a53b9b4a182e0
  if (sql[0] != "") {
    poDS->ReleaseResultSet(poLayer);
  }
  GDALClose(poDS);
  return out;
}

inline List dsn_read_fields_ij(CharacterVector dsn, IntegerVector layer,
                               CharacterVector sql, NumericVector ex,
                               CharacterVector fid_column_name, NumericVector ij) {
  
  GDALDataset       *poDS;
  poDS = (GDALDataset*) GDALOpenEx(dsn[0], GDAL_OF_VECTOR, NULL, NULL, NULL );
  if( poDS == NULL )
  {
    Rcpp::stop("Open failed.\n");
  }
  OGRLayer *poLayer = gdallibrary::gdal_layer(poDS, layer, sql, ex);
  List out = layer_read_fields_ij(poLayer, fid_column_name, ij);
  // clean up if SQL was used https://www.gdal.org/classGDALDataset.html#ab2c2b105b8f76a279e6a53b9b4a182e0
  if (sql[0] != "") {
    poDS->ReleaseResultSet(poLayer);
  }
  GDALClose(poDS);
  return out;
}
inline List dsn_read_fields_ia(CharacterVector dsn, IntegerVector layer,
                               CharacterVector sql, NumericVector ex,
                               CharacterVector fid_column_name, NumericVector ia) {
  
  GDALDataset       *poDS;
  poDS = (GDALDataset*) GDALOpenEx(dsn[0], GDAL_OF_VECTOR, NULL, NULL, NULL );
  if( poDS == NULL )
  {
    Rcpp::stop("Open failed.\n");
  }
  OGRLayer *poLayer = gdallibrary::gdal_layer(poDS, layer, sql, ex);
  List out = layer_read_fields_ia(poLayer, fid_column_name, ia);
  // clean up if SQL was used https://www.gdal.org/classGDALDataset.html#ab2c2b105b8f76a279e6a53b9b4a182e0
  if (sql[0] != "") {
    poDS->ReleaseResultSet(poLayer);
  }
  GDALClose(poDS);
  return out;
}

inline List dsn_read_fields_fa(CharacterVector dsn, IntegerVector layer,
                               CharacterVector sql, NumericVector ex,
                               CharacterVector fid_column_name, NumericVector fa) {
  
  GDALDataset       *poDS;
  poDS = (GDALDataset*) GDALOpenEx(dsn[0], GDAL_OF_VECTOR, NULL, NULL, NULL );
  if( poDS == NULL )
  {
    Rcpp::stop("Open failed.\n");
  }
  OGRLayer *poLayer = gdallibrary::gdal_layer(poDS, layer, sql, ex);
  List out = layer_read_fields_fa(poLayer, fid_column_name, fa);
  // clean up if SQL was used https://www.gdal.org/classGDALDataset.html#ab2c2b105b8f76a279e6a53b9b4a182e0
  if (sql[0] != "") {
    poDS->ReleaseResultSet(poLayer);
  }
  GDALClose(poDS);
  return out;
}

}

#endif
