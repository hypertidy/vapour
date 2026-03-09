#ifndef GDALVECTORSTREAM_H
#define GDALVECTORSTREAM_H

// WIP https://gdal.org/development/rfc/rfc86_column_oriented_api.html
// written by Dewey Dunnington and the GDAL project

#include <cpp11.hpp>
#include <ogrsf_frmts.h>
#include "common/common_vapour.h"
#if GDAL_VERSION_NUM >= GDAL_COMPUTE_VERSION(3,6,0)
#include <ogr_recordbatch.h>
#endif

using namespace cpp11;
namespace writable = cpp11::writable;

namespace gdalvectorstream {

#if GDAL_VERSION_NUM >= GDAL_COMPUTE_VERSION(3,6,0)


class GDALStreamWrapper {
public:
  static void Make(struct ArrowArrayStream* stream, cpp11::list shelter,
                   struct ArrowArrayStream* stream_out) {
    stream_out->get_schema = &get_schema_wrap;
    stream_out->get_next = &get_next_wrap;
    stream_out->get_last_error = &get_last_error_wrap;
    stream_out->release = &release_wrap;
    stream_out->private_data = new GDALStreamWrapper(stream, shelter);
  }

  ~GDALStreamWrapper() {
    stream_.release(&stream_);
    GDALDataset* poDS = (GDALDataset*)R_ExternalPtrAddr(shelter_[0]);
    GDALClose(poDS);
    R_SetExternalPtrAddr(shelter_[0], nullptr);
  }

private:
  struct ArrowArrayStream stream_;
  cpp11::list shelter_;

  GDALStreamWrapper(struct ArrowArrayStream* stream, cpp11::list shelter):
    shelter_(shelter) {
    memcpy(&stream_, stream, sizeof(struct ArrowArrayStream));
    stream->release = nullptr;
  }

  int get_schema(struct ArrowSchema* out) {
    return stream_.get_schema(&stream_, out);
  }

  int get_next(struct ArrowArray* out) {
    return stream_.get_next(&stream_, out);
  }

  const char* get_last_error() {
    return stream_.get_last_error(&stream_);
  }

  static int get_schema_wrap(struct ArrowArrayStream* stream, struct ArrowSchema* out) {
    return reinterpret_cast<GDALStreamWrapper*>(stream->private_data)->get_schema(out);
  }

  static int get_next_wrap(struct ArrowArrayStream* stream, struct ArrowArray* out) {
    return reinterpret_cast<GDALStreamWrapper*>(stream->private_data)->get_next(out);
  }

  static const char* get_last_error_wrap(struct ArrowArrayStream* stream) {
    return reinterpret_cast<GDALStreamWrapper*>(stream->private_data)->get_last_error();
  }

  static void release_wrap(struct ArrowArrayStream* stream) {
    delete reinterpret_cast<GDALStreamWrapper*>(stream->private_data);
    stream->release = nullptr;
  }
};


static void finalize_dataset_xptr(SEXP dataset_xptr) {
  GDALDataset *poDS = (GDALDataset*)R_ExternalPtrAddr(dataset_xptr);
  if (poDS != nullptr) {
    GDALClose(poDS);
  }
}

inline list ogr_layer_setup(strings datasource,
                                  strings layer,
                               strings query,
                               std::vector<std::string> options,
                               bool quiet,
                               std::vector<std::string> drivers,
                               doubles ex,
                               int width) {
  std::vector <char *> open_options;
  if (options.size() > 0) {
     open_options = string_to_charptr(options);
  }

  std::vector <char *> drivers_v = string_to_charptr(drivers);
  GDALDataset *poDS;

  poDS = (GDALDataset *) GDALOpenEx( std::string(datasource[0]).c_str(), GDAL_OF_VECTOR | GDAL_OF_READONLY,
          drivers.size() ? drivers_v.data() : NULL,
          open_options.size() ? open_options.data() : NULL,
          NULL );

  if( poDS == NULL ) {
    cpp11::stop("Cannot open %s; ", std::string(datasource[0]).c_str());
  }

  SEXP dataset_xptr = PROTECT(R_MakeExternalPtr(poDS, R_NilValue, R_NilValue));
  R_RegisterCFinalizer(dataset_xptr, &finalize_dataset_xptr);

  writable::strings layer_w;
  bool layer_empty = (layer.size() == 0);
  bool query_na = (query.size() > 0 && STRING_ELT(query, 0) == NA_STRING);

  if (layer_empty && query_na) {
    switch (poDS->GetLayerCount()) {
    case 0: {
    cpp11::stop("No layers in datasource.");
  }
    case 1: {
      OGRLayer *poLayer = poDS->GetLayer(0);
      layer_w.push_back(poLayer->GetName());
      break;
    }
    default: {
      OGRLayer *poLayer = poDS->GetLayer(0);
      layer_w.push_back(poLayer->GetName());
      if (! quiet) {
        Rprintf("Multiple layers are present in data source %s, ", std::string(datasource[0]).c_str());
        Rprintf("reading layer `%s'.\n", std::string(layer_w[0]).c_str());
        Rprintf("Use `st_layers' to list all layer names and their type in a data source.\n");
        Rprintf("Set the `layer' argument in `st_read' to read a particular layer.\n");
      }
      auto warning_fn = cpp11::package("base")["warning"];
      warning_fn("automatically selected the first layer in a data source containing more than one.");
    }
    }
  } else {
    for (R_xlen_t i = 0; i < layer.size(); i++) layer_w.push_back(std::string(layer[i]));
  }

  OGRPolygon poly;
  OGRLinearRing ring;
  writable::strings query_w(1);
  query_w[0] = (query.size() > 0) ? SEXP(query[0]) : NA_STRING;

  if (ex.size() == 4) {
        if (ex[1] <= ex[0] || ex[3] <= ex[2]) {
          if (ex[1] <= ex[0]) cpp11::warning("extent filter invalid (xmax <= xmin), ignoring");
          if (ex[3] <= ex[2]) cpp11::warning("extent filter invalid (ymax <= ymin), ignoring");
        } else {
          ring.addPoint(ex[0], ex[2]);
          ring.addPoint(ex[0], ex[3]);
          ring.addPoint(ex[1], ex[3]);
          ring.addPoint(ex[1], ex[2]);
          ring.closeRings();
          poly.addRing(&ring);
          if (STRING_ELT(query_w, 0) == NA_STRING) {
            std::string auto_sql = CPLSPrintf("SELECT * FROM %s", std::string(layer_w[0]).c_str());
            query_w[0] = auto_sql.c_str();
          }
        }
  }

  OGRLayer *poLayer;
  bool query_is_na = (STRING_ELT(query_w, 0) == NA_STRING);
  if (!query_is_na) {
    poLayer = poDS->ExecuteSQL(std::string(query_w[0]).c_str(), &poly, NULL);
    if (poLayer == NULL)
      cpp11::stop("Query execution failed, cannot open layer.\n");
    if (layer_w.size())
      cpp11::warning("argument layer is ignored when query is specified\n");
  } else {
    poLayer = poDS->GetLayerByName(std::string(layer_w[0]).c_str());
  }
  if (poLayer == NULL) {
    Rprintf("Cannot open layer %s\n", std::string(layer_w[0]).c_str());
    cpp11::stop("Opening layer failed.\n");
  }

  if (! quiet) {
    if (!query_is_na)
      Rprintf("Reading query `%s'\nfrom data source ", std::string(query_w[0]).c_str());
    else
      Rprintf("Reading layer `%s' from data source ", std::string(layer_w[0]).c_str());
    Rprintf("`%s' ", std::string(datasource[0]).c_str());
    Rprintf("using driver `%s'\n", poDS->GetDriverName());
  }

  SEXP layer_xptr = PROTECT(R_MakeExternalPtr(poLayer, R_NilValue, dataset_xptr));
  Rprintf("%s", "ogr_layer_setup:\n");
  writable::list result = {dataset_xptr, layer_xptr};
  UNPROTECT(2);
  return result;
}

inline list read_gdal_stream(
    cpp11::sexp stream_xptr,
    strings datasource,
    strings layer,
    strings query,
    std::vector<std::string> options,
    bool quiet,
    std::vector<std::string> drivers,
    doubles extent,
    bool dsn_exists,
    strings fid_column,
    int width) {

  const char* array_stream_options[] = {"INCLUDE_FID=NO", nullptr};
  if (fid_column.size() == 1) {
    array_stream_options[0] = "INCLUDE_FID=YES";
  }

  list prep = ogr_layer_setup(datasource, layer, query,
                                        options, quiet, drivers, extent, width);

  OGRLayer* poLayer = (OGRLayer*)R_ExternalPtrAddr(prep[1]);
  auto stream_out = reinterpret_cast<struct ArrowArrayStream*>(
    R_ExternalPtrAddr(stream_xptr));

  OGRLayerH hLayer = reinterpret_cast<OGRLayerH>(poLayer);
  OGRSpatialReferenceH hSRS = OGR_L_GetSpatialRef(hLayer);

  char* wkt_out;
  std::string wkt_str;
  if (OSRExportToWkt(hSRS, &wkt_out) != OGRERR_NONE) {
    cpp11::stop("error export SRS to Wkt");
  }
  wkt_str = wkt_out;
  CPLFree(wkt_out);

  struct ArrowArrayStream stream_temp;
  if (!poLayer->GetArrowStream(&stream_temp, array_stream_options)) {
    cpp11::stop("Failed to open ArrayStream from Layer");
  }

  GDALStreamWrapper::Make(&stream_temp, prep, stream_out);

  double num_features;
  if (query.size() == 0) {
    num_features = (double) poLayer->GetFeatureCount(false);
  } else {
    num_features = -1;
  }

  writable::strings wkt_cv(1);
  wkt_cv[0] = wkt_str.c_str();
  writable::doubles nf = {num_features};
  writable::list result = {wkt_cv, nf};
  return result;
}

#else

inline list read_gdal_stream(
    cpp11::sexp stream_xptr,
    strings datasource,
    strings layer,
    strings query,
    std::vector<std::string> options,
    bool quiet,
    std::vector<std::string> drivers,
    doubles extent,
    bool dsn_exists,
    strings fid_column,
    int width) {
  cpp11::stop("read_stream() requires GDAL >= 3.6");
}

#endif

} // gdalvectorstream


#endif
