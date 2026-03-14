#ifndef GDAL_LAYER_UTILS_H
#define GDAL_LAYER_UTILS_H
#include <cpp11.hpp>
#include "ogrsf_frmts.h"
#include "gdal_priv.h"

namespace gdallibrary {
using namespace cpp11;
namespace writable = cpp11::writable;

constexpr int MAX_INT = std::numeric_limits<int>::max();

inline strings gdal_layer_geometry_name(OGRLayer *poLayer) {
  poLayer->ResetReading();
  OGRFeatureDefn *poFDefn = poLayer->GetLayerDefn();
  int gfields = poFDefn->GetGeomFieldCount();
  writable::strings out(gfields);
  for (int ig = 0; ig < gfields; ig++) {
    OGRGeomFieldDefn *poGFDefn = poFDefn->GetGeomFieldDefn(ig);
    out[ig] = poGFDefn->GetNameRef();
  }
  return out;
}

inline doubles gdal_layer_extent(OGRLayer *poLayer) {
  OGREnvelope poEnvelope;
  OGRErr err = poLayer->GetExtent(&poEnvelope, true);
  if (err != CE_None) {
    Rprintf("problem in get extent\n");
  }
  writable::doubles out(4);
  out[0] = poEnvelope.MinX;
  out[1] = poEnvelope.MaxX;
  out[2] = poEnvelope.MinY;
  out[3] = poEnvelope.MaxY;
  return out;
}

inline R_xlen_t force_layer_feature_count(OGRLayer *poLayer) {
  R_xlen_t out = poLayer->GetFeatureCount(false);
  if (out == -1) {
    out = poLayer->GetFeatureCount(true);
  }
  if (out == -1) {
    out = 0;
    poLayer->ResetReading();
    OGRFeature *poFeature;
    while ((poFeature = poLayer->GetNextFeature()) != NULL) {
      out++;
      OGRFeature::DestroyFeature(poFeature);
    }
    poLayer->ResetReading();
  }
  return out;
}

inline OGRLayer *gdal_layer(GDALDataset *poDS, integers layer, strings sql, doubles ex) {
  OGRLayer *poLayer;
  OGRPolygon poly;
  OGRLinearRing ring;

  bool use_extent_filter = false;
  if (ex.size() == 4) {
    if (ex[1] <= ex[0] || ex[3] <= ex[2]) {
      if (ex[1] <= ex[0]) cpp11::warning("extent filter invalid (xmax <= xmin), ignoring");
      if (ex[3] <= ex[2]) cpp11::warning("extent filter invalid (ymax <= ymin), ignoring");
    } else {
      use_extent_filter = true;
      ring.addPoint(ex[0], ex[2]);
      ring.addPoint(ex[0], ex[3]);
      ring.addPoint(ex[1], ex[3]);
      ring.addPoint(ex[1], ex[2]);
      ring.closeRings();
      poly.addRing(&ring);
    }
  }

  auto vapour_getenv_sql_dialect = cpp11::package("vapour")["vapour_getenv_sql_dialect"];
  cpp11::strings R_dialect(vapour_getenv_sql_dialect());
  const char *sql_dialect = CHAR(STRING_ELT(R_dialect, 0));

  std::string sql_str = std::string(sql[0]);
  if (sql_str != "") {
    if (use_extent_filter) {
      poLayer = poDS->ExecuteSQL(sql_str.c_str(), &poly, sql_dialect);
    } else {
      poLayer = poDS->ExecuteSQL(sql_str.c_str(), NULL, sql_dialect);
    }
    if (poLayer == NULL) cpp11::stop("SQL execution failed.\n");
  } else {
    int nlayer = poDS->GetLayerCount();
    if (layer[0] >= nlayer) cpp11::stop("layer index exceeds layer count");
    poLayer = poDS->GetLayer(layer[0]);
  }
  if (poLayer == NULL) cpp11::stop("Layer open failed.\n");
  return poLayer;
}

// ---- DSN open/close helpers ------------------------------------------------
inline GDALDataset* open_ogr_dataset(const char* dsn,
                                     unsigned int flags = GDAL_OF_VECTOR) {
  GDALDataset *poDS = (GDALDataset*) GDALOpenEx(dsn, flags, NULL, NULL, NULL);
  if (poDS == NULL) cpp11::stop("Open failed.\n");
  return poDS;
}

template <typename Fn>
inline auto with_ogr_layer(strings dsn, integers layer,
                           strings sql, doubles ex,
                           Fn fn) -> decltype(fn(std::declval<OGRLayer*>())) {
  GDALDataset *poDS = open_ogr_dataset(std::string(dsn[0]).c_str());
  OGRLayer *poLayer = gdal_layer(poDS, layer, sql, ex);
  auto out = fn(poLayer);
  if (std::string(sql[0]) != "") poDS->ReleaseResultSet(poLayer);
  GDALClose(poDS);
  return out;
}

template <typename Fn>
inline auto with_ogr_layer(strings dsn, integers layer,
                           strings sql, doubles ex,
                           unsigned int flags,
                           Fn fn) -> decltype(fn(std::declval<OGRLayer*>())) {
  GDALDataset *poDS = open_ogr_dataset(std::string(dsn[0]).c_str(), flags);
  OGRLayer *poLayer = gdal_layer(poDS, layer, sql, ex);
  auto out = fn(poLayer);
  if (std::string(sql[0]) != "") poDS->ReleaseResultSet(poLayer);
  GDALClose(poDS);
  return out;
}

template <typename Fn>
inline auto with_ogr_dataset(const char* dsn, unsigned int flags,
                             Fn fn) -> decltype(fn(std::declval<GDALDataset*>())) {
  GDALDataset *poDS = open_ogr_dataset(dsn, flags);
  auto out = fn(poDS);
  GDALClose(poDS);
  return out;
}

} // namespace gdallibrary
#endif
