#ifndef GDAL_FIELD_ALLOC_H
#define GDAL_FIELD_ALLOC_H
#include <cpp11.hpp>
#include "ogrsf_frmts.h"
#include "gdallibrary/gdal_layer_utils.h"

namespace gdallibrary {
using namespace cpp11;
namespace writable = cpp11::writable;

// Vectors start out missing. A layer can yield fewer features than it counted
// (a driver may count by bounding box and then filter by geometry), and the
// slots that are never written must not hold whatever was in memory.
inline SEXP na_logicals(R_xlen_t n) {
  writable::logicals x(n);
  for (R_xlen_t i = 0; i < n; i++) x[i] = NA_LOGICAL;
  return x;
}
inline SEXP na_integers(R_xlen_t n) {
  writable::integers x(n);
  for (R_xlen_t i = 0; i < n; i++) x[i] = NA_INTEGER;
  return x;
}
inline writable::doubles na_doubles(R_xlen_t n) {
  writable::doubles x(n);
  for (R_xlen_t i = 0; i < n; i++) x[i] = NA_REAL;
  return x;
}
inline SEXP na_strings(R_xlen_t n) {
  writable::strings x(n);
  for (R_xlen_t i = 0; i < n; i++) x[i] = NA_STRING;
  return x;
}

// allocate_fields_list: from sf allocate_out_list by Edzer Pebesma
inline list allocate_fields_list(OGRFeatureDefn *poFDefn, R_xlen_t n_features, bool int64_as_string,
                                 strings fid_column) {
  if (fid_column.size() > 1)
    cpp11::stop("FID column name should be a length 1 character vector");

  int n = poFDefn->GetFieldCount();
  writable::list out(n);
  writable::strings names(n);
  for (int i = 0; i < n; i++) {
    OGRFieldDefn *poFieldDefn = poFDefn->GetFieldDefn(i);
    switch (poFieldDefn->GetType()) {
    case OFTWideString:
    case OFTWideStringList:
      break;
    case OFTInteger:
      if (poFieldDefn->GetSubType() == OFSTBoolean)
        out[i] = na_logicals(n_features);
      else
        out[i] = na_integers(n_features);
      break;
    case OFTDate: {
      writable::doubles ret = na_doubles(n_features);
      ret.attr("class") = "Date";
      out[i] = ret;
    } break;
    case OFTTime:
    case OFTDateTime: {
      writable::doubles ret = na_doubles(n_features);
      writable::strings cls = {"POSIXct", "POSIXt"};
      ret.attr("class") = cls;
      out[i] = ret;
    } break;
    case OFTInteger64:
      out[i] = int64_as_string ? na_strings(n_features) : (SEXP)na_doubles(n_features);
      break;
    case OFTReal:
      out[i] = na_doubles(n_features);
      break;
    case OFTStringList:
    case OFTRealList:
    case OFTIntegerList:
    case OFTInteger64List:
    case OFTBinary:
      out[i] = writable::list(n_features);
      break;
    case OFTString:
      out[i] = na_strings(n_features);
      break;
    }
    names[i] = poFieldDefn->GetNameRef();
  }
  out.names() = names;
  return out;
}

inline doubles gdal_feature_count(strings dsn, integers layer, strings sql, doubles ex,
                                  strings open_options) {
  return with_ogr_layer(dsn, layer, sql, ex, open_options,
    GDAL_OF_READONLY | GDAL_OF_VECTOR,
    [](OGRLayer *poLayer) -> doubles {
      R_xlen_t nFeature = poLayer->GetFeatureCount();
      if (nFeature < 1) nFeature = force_layer_feature_count(poLayer);
      writable::doubles out(1);
      out[0] = static_cast<double>(nFeature);
      return out;
    });
}

inline strings gdal_report_fields(strings dsource, integers layer, strings sql,
                                  strings open_options) {
  writable::doubles zero(1);
  zero[0] = 0.0;
  return with_ogr_layer(dsource, layer, sql, zero, open_options,
    [](OGRLayer *poLayer) -> strings {
      poLayer->ResetReading();
      OGRFeatureDefn *poFDefn = poLayer->GetLayerDefn();
      gdalcommon::FeaturePtr poFeature(poLayer->GetNextFeature());
      int fieldcount = poFDefn->GetFieldCount();
      writable::strings out(fieldcount);
      writable::strings fieldnames(fieldcount);
      if (poFeature) {
        for (int iField = 0; iField < fieldcount; iField++) {
          OGRFieldDefn *poFieldDefn = poFDefn->GetFieldDefn(iField);
          fieldnames[iField] = poFieldDefn->GetNameRef();
          if (poFieldDefn->GetType() == OFTBinary)        out[iField] = "OFTBinary";
          if (poFieldDefn->GetType() == OFTDate)          out[iField] = "OFTDate";
          if (poFieldDefn->GetType() == OFTDateTime)      out[iField] = "OFTDateTime";
          if (poFieldDefn->GetType() == OFTInteger)       out[iField] = "OFTInteger";
          if (poFieldDefn->GetType() == OFTInteger64)     out[iField] = "OFTInteger64";
          if (poFieldDefn->GetType() == OFTInteger64List) out[iField] = "OFTInteger64List";
          if (poFieldDefn->GetType() == OFTIntegerList)   out[iField] = "OFTIntegerList";
          if (poFieldDefn->GetType() == OFTReal)          out[iField] = "OFTReal";
          if (poFieldDefn->GetType() == OFTRealList)      out[iField] = "OFTRealList";
          if (poFieldDefn->GetType() == OFTString)        out[iField] = "OFTString";
          if (poFieldDefn->GetType() == OFTStringList)    out[iField] = "OFTStringList";
          if (poFieldDefn->GetType() == OFTTime)          out[iField] = "OFTTime";
        }
      }
      out.names() = fieldnames;
      return out;
    });
}

} // namespace gdallibrary
#endif
