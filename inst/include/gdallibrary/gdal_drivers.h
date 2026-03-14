#ifndef GDAL_DRIVERS_H
#define GDAL_DRIVERS_H
#include <cpp11.hpp>
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
  CPLSetConfigOption(std::string(option[0]).c_str(), std::string(value[0]).c_str());
  writable::integers out(1);
  out[0] = 1;
  return out;
}

inline strings gdal_get_config_option(strings option) {
  writable::strings out(1);
  const char *str = CPLGetConfigOption(std::string(option[0]).c_str(), nullptr);
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
  for (int idriver = 0; idriver < n; idriver++) {
    GDALDriver *dr = GetGDALDriverManager()->GetDriver(idriver);
    sname[idriver] = GDALGetDriverShortName(dr);
    lname[idriver] = GDALGetDriverLongName(dr);
    isvector[idriver] = (Rboolean)(dr->GetMetadataItem(GDAL_DCAP_VECTOR) != NULL);
    israster[idriver] = (Rboolean)(dr->GetMetadataItem(GDAL_DCAP_RASTER) != NULL);
    iscopy[idriver] = (Rboolean)(dr->GetMetadataItem(GDAL_DCAP_CREATECOPY) != NULL);
    iscreate[idriver] = (Rboolean)(dr->GetMetadataItem(GDAL_DCAP_CREATE) != NULL);
    isvirt[idriver] = (Rboolean)(dr->GetMetadataItem(GDAL_DCAP_VIRTUALIO) != NULL);
  }
  using namespace cpp11::literals;
  writable::list out = {
    "driver"_nm = sname, "name"_nm = lname,
    "vector"_nm = isvector, "raster"_nm = israster,
    "create"_nm = iscreate, "copy"_nm = iscopy,
    "virtual"_nm = isvirt
  };
  return out;
}

inline strings gdal_driver(strings dsn) {
  return with_ogr_dataset(std::string(dsn[0]).c_str(), GDAL_OF_READONLY,
    [](GDALDataset *poDS) -> strings {
      writable::strings dname(1);
      dname[0] = poDS->GetDriverName();
      return dname;
    });
}

inline strings gdal_layer_names(strings dsn) {
  return with_ogr_dataset(std::string(dsn[0]).c_str(), GDAL_OF_VECTOR,
    [](GDALDataset *poDS) -> strings {
      int nlayer = poDS->GetLayerCount();
      writable::strings lnames(nlayer);
      for (int ilayer = 0; ilayer < nlayer; ilayer++) {
        lnames[ilayer] = poDS->GetLayer(ilayer)->GetName();
      }
      return lnames;
    });
}

inline strings gdal_vsi_list(strings urlpath) {
  char** VSI_paths = VSIReadDirRecursive(std::string(urlpath[0]).c_str());
  int ipath = 0;
  while (VSI_paths && VSI_paths[ipath] != NULL) ipath++;
  writable::strings names(ipath);
  for (int i = 0; i < ipath; i++) names[i] = VSI_paths[i];
  CSLDestroy(VSI_paths);
  return names;
}

} // namespace gdallibrary
#endif
