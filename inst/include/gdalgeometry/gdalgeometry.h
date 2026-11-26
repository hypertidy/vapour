#ifndef GDALGEOMETRY_H
#define GDALGEOMETRY_H
#include <cpp11.hpp>
#include "ogrsf_frmts.h"
#include "gdal_priv.h"

#include "common/gdal_handles.h"
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
    writable::raws raw(static_cast<R_xlen_t>(0));
    return raw;
  }
}

inline strings gdal_geometry_wkt(OGRFeature *poFeature) {
  writable::strings wkt(1);
  if (poFeature->GetGeometryRef()) {
    char *pszGEOM_WKT = NULL;
    OGRErr err = poFeature->GetGeometryRef()->exportToWkt(&pszGEOM_WKT, wkbVariantIso );
    gdalcommon::CPLStringPtr wkt_owner(pszGEOM_WKT);
    if (err == OGRERR_NONE && pszGEOM_WKT != NULL) {
      wkt[0] = pszGEOM_WKT;
    } else {
      wkt[0] = NA_STRING;
    }
  } else {
    wkt[0] = NA_STRING;
  }
  return wkt;
}
inline strings gdal_geometry_txt(OGRFeature *poFeature, strings format) {
  writable::strings txt(1);
  txt[0] = NA_STRING;
  std::string fmt = std::string(format[0]);
  if (poFeature->GetGeometryRef()) {
    char *export_txt = NULL;
    if (fmt == "gml")  export_txt = poFeature->GetGeometryRef()->exportToGML();
    if (fmt == "json") export_txt = poFeature->GetGeometryRef()->exportToJson();
    if (fmt == "kml")  export_txt = poFeature->GetGeometryRef()->exportToKML();
    gdalcommon::CPLStringPtr txt_owner(export_txt);
    // a geometry GDAL cannot write in this format is missing, as an absent one is
    if (export_txt != NULL) txt[0] = export_txt;
  }
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
  R_xlen_t n = en - st + 1;
  if (n < 0) n = 0;
  writable::doubles out(n);
  for (R_xlen_t k = 0; k < out.size(); k++) out[k] = NA_REAL;
  R_xlen_t cnt = 0;
  R_xlen_t ii = 0;
  while (ii <= en) {
    gdalcommon::FeaturePtr poFeature(poLayer->GetNextFeature());
    if (!poFeature) break;
    if (ii >= st) {
      out[cnt] = (double)poFeature->GetFID();
      cnt++;
    }
    ii++;
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
  R_xlen_t ii = 0;
  R_xlen_t cnt = 0;
  while (cnt < ia.size()) {
    gdalcommon::FeaturePtr poFeature(poLayer->GetNextFeature());
    if (!poFeature) break;
    if (ii == (R_xlen_t)(ia[cnt])) {
      out[cnt] = (double)poFeature->GetFID();
      cnt++;
    }
    ii++;
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
  R_xlen_t st = (R_xlen_t)ij[0];
  R_xlen_t en = (R_xlen_t)ij[1];
  R_xlen_t n = en - st + 1;
  if (n < 0) n = 0;
  writable::list out(n);
  R_xlen_t ii = 0;
  R_xlen_t cnt = 0;
  while (ii <= en) {
    gdalcommon::FeaturePtr poFeature(poLayer->GetNextFeature());
    if (!poFeature) break;
    if (ii >= st) {
      out[cnt] = feature_read_geom(poFeature.get(), format)[0];
      cnt++;
    }
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
  poLayer->ResetReading();
  writable::list out(ia.size());
  R_xlen_t ii = 0;
  R_xlen_t cnt = 0;
  while (cnt < ia.size()) {
    gdalcommon::FeaturePtr poFeature(poLayer->GetNextFeature());
    if (!poFeature) break;
    if (ii == static_cast<R_xlen_t>(ia[cnt])) {
      out[cnt] = feature_read_geom(poFeature.get(), format)[0];
      cnt++;
    }
    ii++;
  }
  return out;
}

inline list layer_read_geom_fa(OGRLayer *poLayer, strings format, doubles fa) {
  writable::list out(fa.size());
  for (R_xlen_t ii = 0; ii < fa.size(); ii++) {
    GIntBig feature_id = (GIntBig)fa[ii];
    gdalcommon::FeaturePtr poFeature(poLayer->GetFeature(feature_id));
    if (!poFeature) {
      // the element stays NULL
      cpp11::warning("FID not found %.0f", (double)fa[ii]);
    } else {
      out[ii] = feature_read_geom(poFeature.get(), format)[0];
    }
  }
  return out;
}

/// layer READ FIELDS ----------------------------------------------------------------------------

// Write one feature's field values into the pre-allocated output list at position cnt.
// ISOdatetime_fn and ISOdate_fn are resolved once and passed through to avoid repeated
// R function lookup inside tight loops.
inline void write_feature_fields(OGRFeature *poFeature, OGRFeatureDefn *poFDefn,
                                 writable::list& out, R_xlen_t cnt,
                                 cpp11::function& ISOdatetime_fn,
                                 cpp11::function& ISOdate_fn) {
  int bytecount;
  for (int iField = 0; iField < poFDefn->GetFieldCount(); iField++) {
    OGRFieldDefn *poFieldDefn = poFDefn->GetFieldDefn(iField);
    int not_NA = poFeature->IsFieldSetAndNotNull(iField);
    
    switch (poFieldDefn->GetType()) {
    case OFTWideString:
    case OFTWideStringList:
      break;
    case OFTString: {
      writable::strings cv(out[iField]);
      if (not_NA) { cv[cnt] = poFeature->GetFieldAsString(iField); }
      else { cv[cnt] = NA_STRING; }
    } break;
    case OFTInteger: {
      // a boolean subtype was allocated as a logical vector
      if (poFieldDefn->GetSubType() == OFSTBoolean) {
        writable::logicals lv(out[iField]);
        if (!not_NA) { lv[cnt] = NA_LOGICAL; break; }
        lv[cnt] = (Rboolean)(poFeature->GetFieldAsInteger(iField) != 0);
        break;
      }
      writable::integers iv(out[iField]);
      if (!not_NA) { iv[cnt] = NA_INTEGER; break; }
      iv[cnt] = poFeature->GetFieldAsInteger(iField);
    } break;
    case OFTTime:
    case OFTDateTime:
    case OFTDate: {
      writable::doubles nv(out[iField]);
      if (!not_NA) { nv[cnt] = NA_REAL; break; }
      int Year, Month, Day, Hour, Minute, TZFlag;
      float Second;
      const char *tzone = "";
      poFeature->GetFieldAsDateTime(iField, &Year, &Month, &Day, &Hour, &Minute,
                                    &Second, &TZFlag);
      if (TZFlag == 100) tzone = "UTC";
      if (poFieldDefn->GetType() == OFTDateTime || poFieldDefn->GetType() == OFTTime) {
        if (cnt == 0 && poFieldDefn->GetType() == OFTTime) {
          cpp11::warning("field of type 'OFTTime' converted to POSIXct: %s",
                         poFDefn->GetFieldDefn(iField)->GetNameRef());
        }
        cpp11::sexp ret = ISOdatetime_fn(Year, Month, Day, Hour, Minute, (double)Second, tzone);
        nv[cnt] = cpp11::doubles(ret)[0];
      } else {
        cpp11::sexp ret = ISOdate_fn(Year, Month, Day);
        nv[cnt] = cpp11::doubles(ret)[0];
      }
    } break;
    case OFTInteger64: {
      writable::doubles iiv(out[iField]);
      if (!not_NA) { iiv[cnt] = NA_REAL; break; }
      iiv[cnt] = poFeature->GetFieldAsDouble(iField);
    } break;
    case OFTReal: {
      writable::doubles nv(out[iField]);
      if (!not_NA) { nv[cnt] = NA_REAL; break; }
      nv[cnt] = poFeature->GetFieldAsDouble(iField);
    } break;
    case OFTStringList:
    case OFTRealList:
    case OFTIntegerList:
    case OFTInteger64List:
      break;
    case OFTBinary: {
      writable::list bv(out[iField]);
      const GByte *bin = poFeature->GetFieldAsBinary(iField, &bytecount);
      writable::raws rb(bytecount);
      for (int ib = 0; ib < bytecount; ib++) rb[ib] = bin[ib];
      bv[cnt] = rb;
    } break;
    }
  }
}

inline list layer_read_fields_ij(OGRLayer *poLayer, strings fid_column_name,
                                 doubles ij) {
  R_xlen_t st = (R_xlen_t)ij[0];
  R_xlen_t en = (R_xlen_t)ij[1];
  OGRFeatureDefn *poFDefn = poLayer->GetLayerDefn();
  R_xlen_t nfeatures = en - st + 1;
  if (en < st) nfeatures = 0;
  writable::list out(gdallibrary::allocate_fields_list(poFDefn, nfeatures,
                                                       false, fid_column_name));
  cpp11::function ISOdatetime_fn(cpp11::package("base")["ISOdatetime"]);
  cpp11::function ISOdate_fn(cpp11::package("base")["ISOdate"]);

  R_xlen_t cnt = 0;
  R_xlen_t ii = 0;
  while (ii <= en) {
    gdalcommon::FeaturePtr poFeature(poLayer->GetNextFeature());
    if (!poFeature) break;
    if (ii >= st) {
      write_feature_fields(poFeature.get(), poFDefn, out, cnt, ISOdatetime_fn, ISOdate_fn);
      cnt++;
    }
    ii++;
  }
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
  R_xlen_t nFeature = ia.size();
  OGRFeatureDefn *poFDefn = poLayer->GetLayerDefn();
  writable::list out(gdallibrary::allocate_fields_list(poFDefn, nFeature, false, fid_column_name));
  cpp11::function ISOdatetime_fn(cpp11::package("base")["ISOdatetime"]);
  cpp11::function ISOdate_fn(cpp11::package("base")["ISOdate"]);

  R_xlen_t ii = 0;
  R_xlen_t cnt = 0;
  while (cnt < nFeature) {
    gdalcommon::FeaturePtr poFeature(poLayer->GetNextFeature());
    if (!poFeature) break;
    if (ii == static_cast<R_xlen_t>(ia[cnt])) {
      write_feature_fields(poFeature.get(), poFDefn, out, cnt, ISOdatetime_fn, ISOdate_fn);
      cnt++;
    }
    ii++;
  }
  return out;
}

inline list layer_read_fields_fa(OGRLayer *poLayer, strings fid_column_name,
                                 doubles fa) {
  R_xlen_t nFeature = fa.size();
  OGRFeatureDefn *poFDefn = poLayer->GetLayerDefn();
  writable::list out(gdallibrary::allocate_fields_list(poFDefn, nFeature, false, fid_column_name));
  cpp11::function ISOdatetime_fn(cpp11::package("base")["ISOdatetime"]);
  cpp11::function ISOdate_fn(cpp11::package("base")["ISOdate"]);

  R_xlen_t cnt = 0;
  for (R_xlen_t ii = 0; ii < fa.size(); ii++) {
    GIntBig feature_id = (GIntBig)fa[ii];
    gdalcommon::FeaturePtr poFeature(poLayer->GetFeature(feature_id));
    if (!poFeature) {
      cpp11::warning("FID not found %.0f", (double)fa[ii]);
    } else {
      write_feature_fields(poFeature.get(), poFDefn, out, cnt, ISOdatetime_fn, ISOdate_fn);
    }
    cnt++;
  }
  return out;
}


// DSN read - all use with_ogr_layer
inline doubles dsn_read_fids_all(strings dsn, integers layer, strings sql, doubles ex, strings oo) {
  return gdallibrary::with_ogr_layer(dsn, layer, sql, ex, oo,
                                     [](OGRLayer *poLayer) { return layer_read_fids_all(poLayer); });
}
inline doubles dsn_read_fids_ij(strings dsn, integers layer, strings sql, doubles ex, strings oo, doubles ij) {
  return gdallibrary::with_ogr_layer(dsn, layer, sql, ex, oo,
                                     [&ij](OGRLayer *poLayer) { return layer_read_fids_ij(poLayer, ij); });
}
inline doubles dsn_read_fids_ia(strings dsn, integers layer, strings sql, doubles ex, strings oo, doubles ia) {
  return gdallibrary::with_ogr_layer(dsn, layer, sql, ex, oo,
                                     [&ia](OGRLayer *poLayer) { return layer_read_fids_ia(poLayer, ia); });
}
inline list dsn_read_geom_all(strings dsn, integers layer, strings sql, doubles ex, strings oo, strings format) {
  return gdallibrary::with_ogr_layer(dsn, layer, sql, ex, oo,
                                     [&format](OGRLayer *poLayer) { return layer_read_geom_all(poLayer, format); });
}
inline list dsn_read_geom_ij(strings dsn, integers layer, strings sql, doubles ex, strings oo, strings format, doubles ij) {
  return gdallibrary::with_ogr_layer(dsn, layer, sql, ex, oo,
                                     [&format, &ij](OGRLayer *poLayer) { return layer_read_geom_ij(poLayer, format, ij); });
}
inline list dsn_read_geom_ia(strings dsn, integers layer, strings sql, doubles ex, strings oo, strings format, doubles ia) {
  return gdallibrary::with_ogr_layer(dsn, layer, sql, ex, oo,
                                     [&format, &ia](OGRLayer *poLayer) { return layer_read_geom_ia(poLayer, format, ia); });
}
inline list dsn_read_geom_fa(strings dsn, integers layer, strings sql, doubles ex, strings oo, strings format, doubles fa) {
  return gdallibrary::with_ogr_layer(dsn, layer, sql, ex, oo,
                                     [&format, &fa](OGRLayer *poLayer) { return layer_read_geom_fa(poLayer, format, fa); });
}
inline list dsn_read_fields_all(strings dsn, integers layer, strings sql, doubles ex, strings oo, strings fid_column_name) {
  return gdallibrary::with_ogr_layer(dsn, layer, sql, ex, oo,
                                     [&fid_column_name](OGRLayer *poLayer) { return layer_read_fields_all(poLayer, fid_column_name); });
}
inline list dsn_read_fields_ij(strings dsn, integers layer, strings sql, doubles ex, strings oo, strings fid_column_name, doubles ij) {
  return gdallibrary::with_ogr_layer(dsn, layer, sql, ex, oo,
                                     [&fid_column_name, &ij](OGRLayer *poLayer) { return layer_read_fields_ij(poLayer, fid_column_name, ij); });
}
inline list dsn_read_fields_ia(strings dsn, integers layer, strings sql, doubles ex, strings oo, strings fid_column_name, doubles ia) {
  return gdallibrary::with_ogr_layer(dsn, layer, sql, ex, oo,
                                     [&fid_column_name, &ia](OGRLayer *poLayer) { return layer_read_fields_ia(poLayer, fid_column_name, ia); });
}
inline list dsn_read_fields_fa(strings dsn, integers layer, strings sql, doubles ex, strings oo, strings fid_column_name, doubles fa) {
  return gdallibrary::with_ogr_layer(dsn, layer, sql, ex, oo,
                                     [&fid_column_name, &fa](OGRLayer *poLayer) { return layer_read_fields_fa(poLayer, fid_column_name, fa); });
}

}

#endif
