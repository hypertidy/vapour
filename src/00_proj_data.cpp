#include <cpp11.hpp>
#include "gdal_priv.h"
#include "ogr_srs_api.h"
#include "cpl_conv.h"
#include "cpl_error.h"
#include "cpl_string.h"
#include "common/common_vapour.h"

using namespace cpp11;

// Helpers for managing the (optional) bundled copy of the PROJ data
// directory in inst/proj, and for checking that it is usable by the
// libproj that GDAL is linked against.
//
// All of this goes through GDAL's OSR layer rather than proj.h directly
// so vapour does not need to link or include PROJ itself, and so the
// search path is scoped to this GDAL's PROJ contexts rather than set
// process-wide via the PROJ_LIB environment variable.

// Set the PROJ search path for GDAL's PROJ contexts. An empty character
// vector resets to the PROJ/GDAL defaults.
[[cpp11::register]]
logicals set_proj_search_paths_cpp(strings paths) {
  writable::logicals out(1);
  out[0] = FALSE;
#if GDAL_VERSION_NUM >= 3000000
  if (paths.size() == 0) {
    OSRSetPROJSearchPaths(nullptr);
  } else {
    char **papszPaths = nullptr;
    for (R_xlen_t i = 0; i < paths.size(); i++) {
      papszPaths = CSLAddString(papszPaths, as_cstr(paths[i]));
    }
    OSRSetPROJSearchPaths(papszPaths);
    CSLDestroy(papszPaths);
  }
  out[0] = TRUE;
#endif
  return out;
}

[[cpp11::register]]
strings get_proj_search_paths_cpp() {
  writable::strings out;
#if GDAL_VERSION_NUM >= 3000000
  char **papszPaths = OSRGetPROJSearchPaths();
  if (papszPaths != nullptr) {
    for (int i = 0; papszPaths[i] != nullptr; i++) {
      out.push_back(papszPaths[i]);
    }
    CSLDestroy(papszPaths);
  }
#endif
  return out;
}

// Can the current PROJ search path serve a database lookup? This is the
// cheapest honest test: PROJ validates proj.db's layout version lazily,
// on first database access, not when the search path is set. Importing
// EPSG:4326 forces that access. Errors are silenced because a failure
// here is expected and handled by the caller.
//
// GDAL caches importFromEPSG results process-wide, so a lookup that
// succeeded earlier (in this package or, on a shared libgdal, in another
// package) would mask a bad search path; OSRCleanup() drops that cache
// so the probe really hits the database.
[[cpp11::register]]
logicals probe_proj_db_cpp() {
  writable::logicals out(1);
  out[0] = FALSE;
  OSRCleanup();
  CPLPushErrorHandler(CPLQuietErrorHandler);
  OGRSpatialReferenceH hSRS = OSRNewSpatialReference(nullptr);
  if (hSRS != nullptr) {
    OGRErr err = OSRImportFromEPSG(hSRS, 4326);
    out[0] = (err == OGRERR_NONE);
    OSRDestroySpatialReference(hSRS);
  }
  CPLPopErrorHandler();
  return out;
}

// Read the metadata table of a proj.db file through GDAL's SQLite
// driver, so we can report which PROJ built it (PROJ.VERSION) and its
// DATABASE.LAYOUT.VERSION.MAJOR/MINOR without linking sqlite3 or proj
// ourselves. Returns a named character vector; empty if the file cannot
// be opened or the SQLite driver is unavailable.
[[cpp11::register]]
strings proj_db_metadata_cpp(strings path) {
  writable::strings out;
  writable::strings nms;
  CPLPushErrorHandler(CPLQuietErrorHandler);
  const char *apszAllowed[] = {"SQLite", nullptr};
  GDALDatasetH hDS = GDALOpenEx(as_cstr(path[0]), GDAL_OF_VECTOR | GDAL_OF_READONLY,
                                apszAllowed, nullptr, nullptr);
  if (hDS != nullptr) {
    OGRLayerH hLayer = GDALDatasetExecuteSQL(hDS,
      "SELECT key, value FROM metadata", nullptr, nullptr);
    if (hLayer != nullptr) {
      OGRFeatureH hFeat;
      OGR_L_ResetReading(hLayer);
      while ((hFeat = OGR_L_GetNextFeature(hLayer)) != nullptr) {
        const char *key = OGR_F_GetFieldAsString(hFeat, 0);
        const char *val = OGR_F_GetFieldAsString(hFeat, 1);
        nms.push_back(key ? key : "");
        out.push_back(val ? val : "");
        OGR_F_Destroy(hFeat);
      }
      GDALDatasetReleaseResultSet(hDS, hLayer);
    }
    GDALClose(hDS);
  }
  CPLPopErrorHandler();
  out.names() = nms;
  return out;
}

// Clear a GDAL config option (CPLSetConfigOption with NULL), which the
// existing set_gdal_config_cpp cannot express from an R string.
[[cpp11::register]]
logicals unset_gdal_config_cpp(strings option) {
  CPLSetConfigOption(as_cstr(option[0]), nullptr);
  writable::logicals out(1);
  out[0] = TRUE;
  return out;
}
