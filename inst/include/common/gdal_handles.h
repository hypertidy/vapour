#ifndef VAPOUR_GDAL_HANDLES_H
#define VAPOUR_GDAL_HANDLES_H

// Owning handles for GDAL resources, and the one place a data source is opened.
//
// cpp11::stop(), cpp11::warning() under options(warn = 2), an allocation
// failure and an error in an R function called from C++ all leave by throwing
// a C++ exception. Anything held in one of these handles is released on that
// path as well as on the normal one, so no early exit can leak a dataset, a
// feature, a SQL result set or an options struct.

#include <memory>
#include <string>

#include <cpp11.hpp>
#include "gdal_priv.h"
#include "gdal_pam.h"
#include "gdal_utils.h"
#include "ogrsf_frmts.h"
#include "cpl_string.h"

#include "common/common_vapour.h"
#include "common/gdal_messages.h"

namespace gdalcommon {

// Dropping the reference closes a dataset nothing else holds. A dataset that a
// VRT made from it still refers to stays open until that VRT is closed, where
// GDALClose() would destroy it underneath the VRT.
struct DatasetCloser {
  void operator()(GDALDataset *p) const { if (p != nullptr) GDALReleaseDataset((GDALDatasetH)p); }
};
typedef std::unique_ptr<GDALDataset, DatasetCloser> DatasetPtr;

struct FeatureDestroyer {
  void operator()(OGRFeature *p) const { if (p != nullptr) OGRFeature::DestroyFeature(p); }
};
typedef std::unique_ptr<OGRFeature, FeatureDestroyer> FeaturePtr;

struct WarpAppOptionsFreer {
  void operator()(GDALWarpAppOptions *p) const { if (p != nullptr) GDALWarpAppOptionsFree(p); }
};
typedef std::unique_ptr<GDALWarpAppOptions, WarpAppOptionsFreer> WarpAppOptionsPtr;

struct TranslateOptionsFreer {
  void operator()(GDALTranslateOptions *p) const { if (p != nullptr) GDALTranslateOptionsFree(p); }
};
typedef std::unique_ptr<GDALTranslateOptions, TranslateOptionsFreer> TranslateOptionsPtr;

struct InfoOptionsFreer {
  void operator()(GDALInfoOptions *p) const { if (p != nullptr) GDALInfoOptionsFree(p); }
};
typedef std::unique_ptr<GDALInfoOptions, InfoOptionsFreer> InfoOptionsPtr;

struct BuildVRTOptionsFreer {
  void operator()(GDALBuildVRTOptions *p) const { if (p != nullptr) GDALBuildVRTOptionsFree(p); }
};
typedef std::unique_ptr<GDALBuildVRTOptions, BuildVRTOptionsFreer> BuildVRTOptionsPtr;

// memory GDAL allocated and expects the caller to CPLFree()
struct CPLFreer {
  void operator()(char *p) const { CPLFree(p); }
};
typedef std::unique_ptr<char, CPLFreer> CPLStringPtr;

// A layer of a dataset. When it came from ExecuteSQL() it is a result set that
// the dataset must be asked to release; a plain layer is owned by the dataset.
// Declare it after the DatasetPtr it belongs to, so that it goes first.
class LayerHandle {
public:
  LayerHandle() {}
  LayerHandle(GDALDataset *ds, OGRLayer *layer, bool result_set)
    : ds_(ds), layer_(layer), result_set_(result_set) {}
  ~LayerHandle() { release(); }

  LayerHandle(const LayerHandle &) = delete;
  LayerHandle &operator=(const LayerHandle &) = delete;
  LayerHandle(LayerHandle &&other) noexcept
    : ds_(other.ds_), layer_(other.layer_), result_set_(other.result_set_) {
    other.layer_ = nullptr;
  }
  LayerHandle &operator=(LayerHandle &&other) noexcept {
    if (this != &other) {
      release();
      ds_ = other.ds_; layer_ = other.layer_; result_set_ = other.result_set_;
      other.layer_ = nullptr;
    }
    return *this;
  }

  OGRLayer *get() const { return layer_; }
  OGRLayer *operator->() const { return layer_; }
  explicit operator bool() const { return layer_ != nullptr; }

private:
  void release() noexcept {
    if (layer_ != nullptr && result_set_ && ds_ != nullptr) ds_->ReleaseResultSet(layer_);
    layer_ = nullptr;
  }
  GDALDataset *ds_ = nullptr;
  OGRLayer *layer_ = nullptr;
  bool result_set_ = false;
};

// An R character vector as a GDAL string list. Missing and empty strings are
// dropped, so character(), "" and NA all mean "none".
inline CPLStringList as_string_list(const cpp11::strings &x) {
  CPLStringList out;
  for (R_xlen_t i = 0; i < x.size(); i++) {
    SEXP el = STRING_ELT(x, i);
    if (el == NA_STRING) continue;
    const char *value = CHAR(el);
    if (value[0] == '\0') continue;
    out.AddString(value);
  }
  return out;
}

// Keep changes made to an open source in memory only. GDAL otherwise saves
// metadata set on a read-only dataset to an .aux.xml file beside the source
// when the dataset closes.
inline void no_sidecar_save(GDALDataset *ds) {
  GDALPamDataset *pam = dynamic_cast<GDALPamDataset *>(ds);
  if (pam != nullptr) pam->SetPamFlags(pam->GetPamFlags() | GPF_NOSAVE);
}

// ---- open -------------------------------------------------------------------
// Every data source vapour opens is opened here. 'flags' are GDAL_OF_* values:
// the kind of source (GDAL_OF_RASTER, GDAL_OF_VECTOR, or neither for any kind)
// and GDAL_OF_UPDATE to write. GDAL is always asked to say why an open failed.
// The handle is empty when the source cannot be opened.
inline DatasetPtr open_dataset(const char *dsn, unsigned int flags,
                               const cpp11::strings &open_options,
                               const char *const *allowed_drivers = nullptr) {
  CPLStringList oo = as_string_list(open_options);
  GDALDataset *ds = (GDALDataset *) GDALOpenEx(dsn, flags | GDAL_OF_VERBOSE_ERROR,
                                               allowed_drivers, oo.List(), nullptr);
  return DatasetPtr(ds);
}

// as open_dataset(), but an R error when the source cannot be opened
inline DatasetPtr open_dataset_or_stop(const char *dsn, unsigned int flags,
                                       const cpp11::strings &open_options,
                                       const char *const *allowed_drivers = nullptr) {
  DatasetPtr ds = open_dataset(dsn, flags, open_options, allowed_drivers);
  if (!ds) stop_gdal(std::string("cannot open '") + dsn + "'");
  return ds;
}

} // namespace gdalcommon
#endif
