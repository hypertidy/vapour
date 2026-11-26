#ifndef GDAL_LAYER_UTILS_H
#define GDAL_LAYER_UTILS_H
#include <cpp11.hpp>
#include "common/common_vapour.h"
#include "common/gdal_handles.h"
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

// xmin, xmax, ymin, ymax, or all NA when the layer has no extent: no geometry
// column, no features, or a driver that cannot say
inline doubles gdal_layer_extent(OGRLayer *poLayer) {
  OGREnvelope poEnvelope;
  writable::doubles out = {NA_REAL, NA_REAL, NA_REAL, NA_REAL};
  OGRErr err = poLayer->GetExtent(&poEnvelope, true);
  if (err == OGRERR_NONE) {
    out[0] = poEnvelope.MinX;
    out[1] = poEnvelope.MaxX;
    out[2] = poEnvelope.MinY;
    out[3] = poEnvelope.MaxY;
  }
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
    while (true) {
      gdalcommon::FeaturePtr poFeature(poLayer->GetNextFeature());
      if (!poFeature) break;
      out++;
    }
    poLayer->ResetReading();
  }
  return out;
}

// The layer to read: the result of 'sql' when that is given, otherwise the
// layer at the 0-based index 'layer'. The handle releases a SQL result set
// when it goes out of scope.
inline gdalcommon::LayerHandle gdal_layer(GDALDataset *poDS, integers layer, strings sql, doubles ex) {
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

  std::string sql_str = std::string(sql[0]);
  if (sql_str != "") {
    auto vapour_getenv_sql_dialect = cpp11::package("vapour")["vapour_getenv_sql_dialect"];
    cpp11::strings R_dialect(vapour_getenv_sql_dialect());
    const char *sql_dialect = as_cstr(R_dialect[0]);
    OGRLayer *poLayer = poDS->ExecuteSQL(sql_str.c_str(),
                                         use_extent_filter ? &poly : nullptr,
                                         sql_dialect);
    if (poLayer == nullptr) gdalcommon::stop_gdal("SQL execution failed");
    return gdalcommon::LayerHandle(poDS, poLayer, true);
  }

  int nlayer = poDS->GetLayerCount();
  if (layer[0] == NA_INTEGER || layer[0] < 0) cpp11::stop("layer index must be 0 or greater");
  if (layer[0] >= nlayer) cpp11::stop("layer index exceeds layer count");
  OGRLayer *poLayer = poDS->GetLayer(layer[0]);
  if (poLayer == nullptr) gdalcommon::stop_gdal("Layer open failed");
  return gdalcommon::LayerHandle(poDS, poLayer, false);
}

// ---- DSN open helpers -------------------------------------------------------
// Open 'dsn', find the layer, run 'fn' on it. The dataset and any SQL result
// set are released however 'fn' leaves.
template <typename Fn>
inline auto with_ogr_layer(strings dsn, integers layer,
                           strings sql, doubles ex,
                           strings open_options,
                           unsigned int flags,
                           Fn fn) -> decltype(fn(std::declval<OGRLayer*>())) {
  gdalcommon::DatasetPtr poDS = gdalcommon::open_dataset_or_stop(as_cstr(dsn[0]), flags, open_options);
  gdalcommon::LayerHandle poLayer = gdal_layer(poDS.get(), layer, sql, ex);
  return fn(poLayer.get());
}

template <typename Fn>
inline auto with_ogr_layer(strings dsn, integers layer,
                           strings sql, doubles ex,
                           strings open_options,
                           Fn fn) -> decltype(fn(std::declval<OGRLayer*>())) {
  return with_ogr_layer(dsn, layer, sql, ex, open_options, GDAL_OF_VECTOR, fn);
}

template <typename Fn>
inline auto with_ogr_dataset(const char* dsn, unsigned int flags,
                             strings open_options,
                             Fn fn) -> decltype(fn(std::declval<GDALDataset*>())) {
  gdalcommon::DatasetPtr poDS = gdalcommon::open_dataset_or_stop(dsn, flags, open_options);
  return fn(poDS.get());
}

} // namespace gdallibrary
#endif
