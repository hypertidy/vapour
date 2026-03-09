#ifndef GDALGEOMETRY_H
#define GDALGEOMETRY_H
#include <cpp11.hpp>
#include "ogrsf_frmts.h"
#include "gdal_priv.h"

#include "gdallibrary/gdallibrary.h"
#include "gdalmiscutils/gdalmiscutils.h"
namespace gdalgeometry {
using namespace cpp11;
namespace writable = cpp11::writable;

inline raws gdal_geometry_raw(OGRFeature *poFeature) {
  if (poFeature->GetGeometryRef()) {
    writable::raws raw(poFeature->GetGeometryRef()->WkbSize());
    poFeature->GetGeometryRef()->exportToWkb(wkbNDR, RAW(raw), wkbVariantIso);
    return raw;
  } else {
    writable::raws raw(0);
    return raw;
  }
}

inline strings gdal_geometry_wkt(OGRFeature *poFeature) {
  writable::strings wkt(1);
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
inline strings gdal_geometry_txt(OGRFeature *poFeature, strings format) {
  char *export_txt = NULL;
  writable::strings txt(1);
  std::string fmt = std::string(format[0]);
  if (poFeature->GetGeometryRef()) {
    if (fmt == "gml")  export_txt = poFeature->GetGeometryRef()->exportToGML();
    if (fmt == "json") export_txt = poFeature->GetGeometryRef()->exportToJson();
    if (fmt == "kml")  export_txt = poFeature->GetGeometryRef()->exportToKML();
    txt[0] = export_txt;
  } else {
    txt[0] = NA_STRING;
  }
  CPLFree(export_txt);
  return txt;
}
inline integers gdal_geometry_type(OGRFeature*poFeature) {
  OGRGeometry *poGeom = poFeature->GetGeometryRef();
  OGRBoolean empty;
  if (poGeom) {
    empty = poFeature->GetGeometryRef()->IsEmpty();
  } else {
    empty = 1;
  }
  writable::integers r_gtyp(1);
  if (empty) {
    r_gtyp[0] = NA_INTEGER;
  } else {
    OGRwkbGeometryType gtyp = OGR_G_GetGeometryType( poFeature->GetGeometryRef());
    r_gtyp[0] = (int)gtyp;
  }
  return r_gtyp;
}
inline doubles gdal_geometry_extent(OGRFeature *poFeature) {
  double minx = NA_REAL, maxx = NA_REAL, miny = NA_REAL, maxy = NA_REAL;
  if (poFeature->GetGeometryRef()) {
   OGREnvelope env;
   OGR_G_GetEnvelope(poFeature->GetGeometryRef(), &env);
   if (!poFeature->GetGeometryRef()->IsEmpty()) {
     minx = env.MinX; maxx = env.MaxX; miny = env.MinY; maxy = env.MaxY;
   }
  }
  writable::doubles extent = {minx, maxx, miny, maxy};
  return extent;
}

// --------------------------------------------------------------------------------------

inline doubles layer_read_fids_ij(OGRLayer *poLayer, doubles ij) {
  R_xlen_t st = (R_xlen_t)ij[0];
  R_xlen_t en = (R_xlen_t)ij[1];
  writable::doubles out(en - st + 1);
  for (R_xlen_t k = 0; k < out.size(); k++) out[k] = NA_REAL;
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
  if (cnt < out.size()) {
    cpp11::warning("not as many FIDs as requested");
  }
  return out;
}

inline doubles layer_read_fids_all(OGRLayer *poLayer) {
  R_xlen_t nFeature = poLayer->GetFeatureCount();
  writable::doubles ij = {0.0, (double)(nFeature - 1)};
  return layer_read_fids_ij(poLayer, ij);
}

inline doubles layer_read_fids_ia(OGRLayer *poLayer, doubles ia) {
  writable::doubles out(ia.size());
  for (R_xlen_t k = 0; k < out.size(); k++) out[k] = NA_REAL;
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
  if (cnt < out.size()) {
    cpp11::warning("not all FIDS found");
  }
  return out;
}


inline list feature_read_geom(OGRFeature *poFeature, strings format) {
  writable::list out(1);
  std::string fmt = std::string(format[0]);
  if (fmt == "wkb")    out[0] = gdal_geometry_raw(poFeature);
  if (fmt == "wkt")    out[0] = gdal_geometry_wkt(poFeature);
  if (fmt == "extent") out[0] = gdal_geometry_extent(poFeature);
  if (fmt == "gml" || fmt == "json" || fmt == "kml") out[0] = gdal_geometry_txt(poFeature, format);
  if (fmt == "type")   out[0] = gdal_geometry_type(poFeature);
  return out;
}

inline list layer_read_geom_ij(OGRLayer *poLayer, strings format, doubles ij) {
  OGRFeature *poFeature;
  R_xlen_t st = (R_xlen_t)ij[0];
  R_xlen_t en = (R_xlen_t)ij[1];
  writable::list out(en - st + 1);
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
  if (cnt < out.size()) {
    cpp11::warning("not as many geoms as requested");
  }
  return out;
}

inline list layer_read_geom_all(OGRLayer *poLayer, strings format) {
  R_xlen_t nFeature = poLayer->GetFeatureCount();
  writable::doubles ij = {0.0, (double)(nFeature - 1)};
  return layer_read_geom_ij(poLayer, format, ij);
}

inline list layer_read_geom_ia(OGRLayer *poLayer, strings format, doubles ia) {
  OGRFeature *poFeature;
  poLayer->ResetReading();
  writable::list out(ia.size());
  R_xlen_t ii = 0;
  R_xlen_t cnt = 0;
  while( (poFeature = poLayer->GetNextFeature()) != NULL ) {
    if (ii == static_cast<R_xlen_t>(ia[cnt])) {
      out[cnt] = feature_read_geom(poFeature, format)[0];
      cnt++;
    }
    ii++;
    OGRFeature::DestroyFeature(poFeature);
    if (cnt == ia.size()) break;
  }
  return out;
}

inline list layer_read_geom_fa(OGRLayer *poLayer, strings format, doubles fa) {
  OGRFeature *poFeature;
  writable::list out(fa.size());
  for (R_xlen_t ii = 0; ii < fa.size(); ii++) {
    GIntBig feature_id = (GIntBig)fa[ii];
    poFeature = poLayer->GetFeature(feature_id);
    out[ii] = feature_read_geom(poFeature, format)[0];
    OGRFeature::DestroyFeature(poFeature);
  }
  return out;
}

/// layer READ FIELDS ----------------------------------------------------------------------------

inline list layer_read_fields_ij(OGRLayer *poLayer, strings fid_column_name,
                                 doubles ij) {
  R_xlen_t st = (R_xlen_t)ij[0];
  R_xlen_t en = (R_xlen_t)ij[1];
  OGRFeatureDefn *poFDefn = poLayer->GetLayerDefn();
  bool int64_as_string = false;
  R_xlen_t nfeatures = en - st + 1;
  if (en < st) nfeatures = 0;
  writable::list out(gdallibrary::allocate_fields_list(poFDefn, nfeatures,
                                               int64_as_string, fid_column_name));
  OGRFeature *poFeature;
  R_xlen_t cnt = 0;
  R_xlen_t ii = 0;
  int iField;
  int bytecount;
  int not_NA;

  auto ISOdatetime = cpp11::package("base")["ISOdatetime"];
  auto ISOdate = cpp11::package("base")["ISOdate"];

  while(ii <= en &&  (poFeature = poLayer->GetNextFeature()) != NULL ) {
    if (ii >= st) {
      for(iField = 0; iField < poFDefn->GetFieldCount(); iField++ )
      {
        OGRFieldDefn *poFieldDefn = poFDefn->GetFieldDefn( iField );
        not_NA = poFeature->IsFieldSetAndNotNull(iField);

        switch( poFieldDefn->GetType() ) {
        case OFTWideString:
        case OFTWideStringList: {
        }
          break;
        case OFTString: {
          writable::strings cv(out[iField]);
          if (not_NA) {
            cv[cnt] = poFeature->GetFieldAsString( iField );
          } else {
            cv[cnt] = NA_STRING;
          }
        } break;
        case OFTInteger: {
          writable::integers iv(out[iField]);
          if (! not_NA) {
            iv[cnt] = NA_INTEGER;
            break;
          }
          iv[cnt] = poFeature->GetFieldAsInteger( iField );
        }  break;
        case OFTTime:
        case OFTDateTime:
        case OFTDate: {
          writable::doubles nv(out[iField]);
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
              cpp11::warning("field of type 'OFTTime' converted to POSIXct: %s", poFDefn->GetFieldDefn(iField)->GetNameRef());
            }
            cpp11::sexp ret = ISOdatetime(Year, Month, Day, Hour, Minute, (double)Second, tzone);
            nv[cnt] = cpp11::doubles(ret)[0];
          } else {
            cpp11::sexp ret = ISOdate(Year, Month, Day);
            nv[cnt] = cpp11::doubles(ret)[0];
          }
          break;
        }
          break;
        case OFTInteger64: {
          writable::doubles iiv(out[iField]);
          if (! not_NA) {
            iiv[cnt] = NA_REAL;
            break;
          }
          iiv[cnt] = poFeature->GetFieldAsDouble( iField );
        }
          break;
        case OFTReal: {
          writable::doubles nv(out[iField]);
          if (! not_NA) {
            nv[cnt] = NA_REAL;
            break;
          }
          nv[cnt] = poFeature->GetFieldAsDouble( iField );
        }
          break;
          case OFTStringList:
          case OFTRealList:
          case OFTIntegerList:
          case OFTInteger64List:
            break;
        case OFTBinary: {
          writable::list bv(out[iField]);
          const GByte *bin = poFeature->GetFieldAsBinary(iField, &bytecount);
          writable::raws rb(bytecount);
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
    cpp11::warning("not as many features as requested");
  }
  return out;
}


inline list layer_read_fields_all(OGRLayer *poLayer, strings fid_column_name) {
  R_xlen_t nFeature = poLayer->GetFeatureCount();
  writable::doubles ij = {0.0, (double)(nFeature - 1)};
  return layer_read_fields_ij(poLayer, fid_column_name, ij);
}
inline list layer_read_fields_ia(OGRLayer *poLayer, strings fid_column_name,
                                 doubles ia) {
  R_xlen_t   nFeature = ia.size();
  OGRFeatureDefn *poFDefn = poLayer->GetLayerDefn();
  bool int64_as_string = false;
  writable::list out(gdallibrary::allocate_fields_list(poFDefn, nFeature, int64_as_string, fid_column_name));
  OGRFeature *poFeature;
  R_xlen_t ii = 0;
  R_xlen_t cnt = 0;
  int iField;
  while( (poFeature = poLayer->GetNextFeature()) != NULL ) {
    if (ii == static_cast<R_xlen_t>(ia[cnt])) {
      for( iField = 0; iField < poFDefn->GetFieldCount(); iField++ )
      {
        OGRFieldDefn *poFieldDefn = poFDefn->GetFieldDefn( iField );
        if( poFieldDefn->GetType() == OFTInteger ) {
          writable::integers nv(out[iField]);
          nv[cnt] = poFeature->GetFieldAsInteger( iField );
        }
        if( poFieldDefn->GetType() == OFTReal || poFieldDefn->GetType() == OFTInteger64) {
          writable::doubles nv(out[iField]);
          nv[cnt] = poFeature->GetFieldAsDouble( iField );
        }
        if( poFieldDefn->GetType() == OFTString || poFieldDefn->GetType() == OFTDate || poFieldDefn->GetType() == OFTTime || poFieldDefn->GetType() == OFTDateTime) {
          writable::strings nv(out[iField]);
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

inline list layer_read_fields_fa(OGRLayer *poLayer, strings fid_column_name,
                                 doubles fa) {
  R_xlen_t   nFeature = fa.size();
  OGRFeatureDefn *poFDefn = poLayer->GetLayerDefn();
  bool int64_as_string = false;
  writable::list out(gdallibrary::allocate_fields_list(poFDefn, nFeature, int64_as_string, fid_column_name));
  OGRFeature *poFeature;
  R_xlen_t cnt = 0;
  int iField;
  for (R_xlen_t ii = 0; ii < fa.size(); ii++) {
    GIntBig feature_id = (GIntBig)fa[ii];
    poFeature = poLayer->GetFeature(feature_id);
    if (NULL == poFeature) {
      cpp11::warning("FID not found %i", (int)fa[ii]);
    } else {
      for( iField = 0; iField < poFDefn->GetFieldCount(); iField++ )
      {
        OGRFieldDefn *poFieldDefn = poFDefn->GetFieldDefn( iField );
        if( poFieldDefn->GetType() == OFTInteger ) {
          writable::integers nv(out[iField]);
          nv[cnt] = poFeature->GetFieldAsInteger( iField );
        }
        if( poFieldDefn->GetType() == OFTReal || poFieldDefn->GetType() == OFTInteger64) {
          writable::doubles nv(out[iField]);
          nv[cnt] = poFeature->GetFieldAsDouble( iField );
        }
        if( poFieldDefn->GetType() == OFTString || poFieldDefn->GetType() == OFTDate || poFieldDefn->GetType() == OFTTime || poFieldDefn->GetType() == OFTDateTime) {
          writable::strings nv(out[iField]);
          nv[cnt] = poFeature->GetFieldAsString( iField );
        }
      }
      OGRFeature::DestroyFeature(poFeature);
    }
    cnt++;
  }
  return out;
}


// DSN read - all use with_ogr_layer
inline doubles dsn_read_fids_all(strings dsn, integers layer, strings sql, doubles ex) {
  return gdallibrary::with_ogr_layer(dsn, layer, sql, ex,
    [](OGRLayer *poLayer) { return layer_read_fids_all(poLayer); });
}
inline doubles dsn_read_fids_ij(strings dsn, integers layer, strings sql, doubles ex, doubles ij) {
  return gdallibrary::with_ogr_layer(dsn, layer, sql, ex,
    [&ij](OGRLayer *poLayer) { return layer_read_fids_ij(poLayer, ij); });
}
inline doubles dsn_read_fids_ia(strings dsn, integers layer, strings sql, doubles ex, doubles ia) {
  return gdallibrary::with_ogr_layer(dsn, layer, sql, ex,
    [&ia](OGRLayer *poLayer) { return layer_read_fids_ia(poLayer, ia); });
}
inline list dsn_read_geom_all(strings dsn, integers layer, strings sql, doubles ex, strings format) {
  return gdallibrary::with_ogr_layer(dsn, layer, sql, ex,
    [&format](OGRLayer *poLayer) { return layer_read_geom_all(poLayer, format); });
}
inline list dsn_read_geom_ij(strings dsn, integers layer, strings sql, doubles ex, strings format, doubles ij) {
  return gdallibrary::with_ogr_layer(dsn, layer, sql, ex,
    [&format, &ij](OGRLayer *poLayer) { return layer_read_geom_ij(poLayer, format, ij); });
}
inline list dsn_read_geom_ia(strings dsn, integers layer, strings sql, doubles ex, strings format, doubles ia) {
  return gdallibrary::with_ogr_layer(dsn, layer, sql, ex,
    [&format, &ia](OGRLayer *poLayer) { return layer_read_geom_ia(poLayer, format, ia); });
}
inline list dsn_read_geom_fa(strings dsn, integers layer, strings sql, doubles ex, strings format, doubles fa) {
  return gdallibrary::with_ogr_layer(dsn, layer, sql, ex,
    [&format, &fa](OGRLayer *poLayer) { return layer_read_geom_fa(poLayer, format, fa); });
}
inline list dsn_read_fields_all(strings dsn, integers layer, strings sql, doubles ex, strings fid_column_name) {
  return gdallibrary::with_ogr_layer(dsn, layer, sql, ex,
    [&fid_column_name](OGRLayer *poLayer) { return layer_read_fields_all(poLayer, fid_column_name); });
}
inline list dsn_read_fields_ij(strings dsn, integers layer, strings sql, doubles ex, strings fid_column_name, doubles ij) {
  return gdallibrary::with_ogr_layer(dsn, layer, sql, ex,
    [&fid_column_name, &ij](OGRLayer *poLayer) { return layer_read_fields_ij(poLayer, fid_column_name, ij); });
}
inline list dsn_read_fields_ia(strings dsn, integers layer, strings sql, doubles ex, strings fid_column_name, doubles ia) {
  return gdallibrary::with_ogr_layer(dsn, layer, sql, ex,
    [&fid_column_name, &ia](OGRLayer *poLayer) { return layer_read_fields_ia(poLayer, fid_column_name, ia); });
}
inline list dsn_read_fields_fa(strings dsn, integers layer, strings sql, doubles ex, strings fid_column_name, doubles fa) {
  return gdallibrary::with_ogr_layer(dsn, layer, sql, ex,
    [&fid_column_name, &fa](OGRLayer *poLayer) { return layer_read_fields_fa(poLayer, fid_column_name, fa); });
}

}

#endif
