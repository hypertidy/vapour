#ifndef GDAL_DRIVERS_H
#define GDAL_DRIVERS_H
#include <cpp11.hpp>
#include "common/common_vapour.h"
#include "common/gdal_handles.h"
#include "gdal_priv.h"
#include "ogr_srs_api.h"
#include "gdallibrary/gdal_layer_utils.h"

namespace gdallibrary {
using namespace cpp11;
namespace writable = cpp11::writable;

inline void gdal_register_all() { GDALAllRegister(); }
inline void ogr_register_all() { OGRRegisterAll(); }
inline void ogr_cleanup_all() { OGRCleanupAll(); }
inline void osr_cleanup() { OSRCleanup(); }

inline integers gdal_set_config_option(strings option, strings value) {
  CPLSetConfigOption(as_cstr(option[0]), as_cstr(value[0]));
  writable::integers out(1);
  out[0] = 1;
  return out;
}

inline strings gdal_get_config_option(strings option) {
  writable::strings out(1);
  const char *str = CPLGetConfigOption(as_cstr(option[0]), nullptr);
  if (str) out[0] = str;
  return out;
}

inline integers proj_version() {
  writable::integers out(3);
  int num1, num2, num3;
  OSRGetPROJVersion(&num1, &num2, &num3);
  out[0] = num1; out[1] = num2; out[2] = num3;
  return out;
}

inline strings gdal_version() {
  writable::strings out(1);
  out[0] = GDALVersionInfo("--version");
  return out;
}

inline list gdal_list_drivers() {
  int n = GetGDALDriverManager()->GetDriverCount();
  writable::strings sname(n);
  writable::strings lname(n);
  writable::logicals isvector(n);
  writable::logicals israster(n);
  writable::logicals iscopy(n);
  writable::logicals iscreate(n);
  writable::logicals isvirt(n);
  writable::strings dialects(n);
  for (int idriver = 0; idriver < n; idriver++) {
    GDALDriver *dr = GetGDALDriverManager()->GetDriver(idriver);
    sname[idriver] = GDALGetDriverShortName(dr);
    lname[idriver] = GDALGetDriverLongName(dr);
    isvector[idriver] = (Rboolean)(dr->GetMetadataItem(GDAL_DCAP_VECTOR) != NULL);
    israster[idriver] = (Rboolean)(dr->GetMetadataItem(GDAL_DCAP_RASTER) != NULL);
    iscopy[idriver] = (Rboolean)(dr->GetMetadataItem(GDAL_DCAP_CREATECOPY) != NULL);
    iscreate[idriver] = (Rboolean)(dr->GetMetadataItem(GDAL_DCAP_CREATE) != NULL);
    isvirt[idriver] = (Rboolean)(dr->GetMetadataItem(GDAL_DCAP_VIRTUALIO) != NULL);
    // space separated, GDAL >= 3.6 (the item is absent before that, and for raster drivers)
    const char *dialect = dr->GetMetadataItem("DMD_SUPPORTED_SQL_DIALECTS");
    if (dialect == NULL) {
      dialects[idriver] = NA_STRING;
    } else {
      dialects[idriver] = dialect;
    }
  }
  using namespace cpp11::literals;
  writable::list out = {
    "driver"_nm = sname, "name"_nm = lname,
    "vector"_nm = isvector, "raster"_nm = israster,
    "create"_nm = iscreate, "copy"_nm = iscopy,
    "virtual"_nm = isvirt,
    "sql_dialects"_nm = dialects
  };
  return out;
}

inline strings gdal_driver(strings dsn, strings open_options) {
  return with_ogr_dataset(as_cstr(dsn[0]), GDAL_OF_READONLY, open_options,
    [](GDALDataset *poDS) -> strings {
      writable::strings dname(1);
      dname[0] = poDS->GetDriverName();
      return dname;
    });
}

inline strings gdal_vector_filelist(strings dsn, strings open_options) {
  return with_ogr_dataset(as_cstr(dsn[0]), GDAL_OF_VECTOR, open_options,
    [](GDALDataset *poDS) -> strings {
      CPLStringList filelist(poDS->GetFileList());
      int nfile = filelist.size();
      writable::strings files(nfile);
      for (int ifile = 0; ifile < nfile; ifile++) {
        files[ifile] = filelist[ifile];
      }
      return files;
    });
}

inline strings gdal_layer_names(strings dsn, strings open_options) {
  return with_ogr_dataset(as_cstr(dsn[0]), GDAL_OF_VECTOR, open_options,
    [](GDALDataset *poDS) -> strings {
      int nlayer = poDS->GetLayerCount();
      writable::strings lnames(nlayer);
      for (int ilayer = 0; ilayer < nlayer; ilayer++) {
        OGRLayer *poLayer = poDS->GetLayer(ilayer);
        if (poLayer == nullptr) {
          lnames[ilayer] = NA_STRING;
        } else {
          lnames[ilayer] = poLayer->GetName();
        }
      }
      return lnames;
    });
}

inline strings gdal_vsi_list(strings urlpath) {
  CPLStringList VSI_paths(VSIReadDirRecursive(as_cstr(urlpath[0])));
  int npath = VSI_paths.size();
  writable::strings names(npath);
  for (int i = 0; i < npath; i++) names[i] = VSI_paths[i];
  return names;
}

} // namespace gdallibrary
#endif
