#ifndef GDAL_CRS_H
#define GDAL_CRS_H
#include <cpp11.hpp>
#include "common/common_vapour.h"
#include "gdal_priv.h"
#include "ogr_srs_api.h"
#include "gdallibrary/gdal_layer_utils.h"

namespace gdallibrary {
using namespace cpp11;
namespace writable = cpp11::writable;

inline strings gdal_proj_to_wkt(strings proj_str) {
  OGRSpatialReference oSRS;
  char *pszWKT = nullptr;
  oSRS.SetFromUserInput(as_cstr(proj_str[0]));
#if GDAL_VERSION_MAJOR >= 3
  const char *options[3] = { "MULTILINE=YES", "FORMAT=WKT2", NULL };
  OGRErr err = oSRS.exportToWkt(&pszWKT, options);
#else
  OGRErr err = oSRS.exportToWkt(&pszWKT);
#endif
  writable::strings out(1);
  if (err) {
    out[0] = NA_STRING;
  } else {
    out[0] = pszWKT;
  }
  if (pszWKT != nullptr) CPLFree(pszWKT);
  return out;
}

inline logicals gdal_crs_is_lonlat(SEXP proj_str) {
  const char* crs_in[] = {CHAR(STRING_ELT(proj_str, 0))};
  OGRSpatialReference oSRS;
  oSRS.SetFromUserInput(*crs_in);
  writable::logicals out(1);
  out[0] = (Rboolean)oSRS.IsGeographic();
  return out;
}

inline list gdal_projection_info(strings dsn, integers layer, strings sql) {
  GDALDataset *poDS = nullptr;
  poDS = (GDALDataset*) GDALOpenEx(as_cstr(dsn[0]), GDAL_OF_VECTOR, NULL, NULL, NULL);
  if (poDS == nullptr) cpp11::stop("Open failed.\n");

  writable::doubles zero(1);
  zero[0] = 0.0;
  OGRLayer *poLayer = gdal_layer(poDS, layer, sql, zero);
  OGRLayerH hLayer = reinterpret_cast<OGRLayerH>(poLayer);
  OGRSpatialReferenceH hSRS = OGR_L_GetSpatialRef(hLayer);

  writable::list info_out(6);
  writable::strings outnames = {"Proj4", "MICoordSys", "PrettyWkt", "Wkt", "EPSG", "XML"};
  info_out.names() = outnames;

  if (hSRS == nullptr) {
    // no CRS - e.g. .shp with no .prj
  } else {
    char *proj4 = nullptr;
    if (OSRExportToProj4(hSRS, &proj4) != OGRERR_NONE) cpp11::stop("error export SRS to proj4");
    writable::strings p4(1); p4[0] = proj4;
    info_out[0] = p4;
    CPLFree(proj4);

    // MICoordSys and XML cannot express every CRS. Those two elements are left
    // NULL when the export fails, without GDAL's message or a warning.
    char *MI = nullptr;
    CPLPushErrorHandler(CPLQuietErrorHandler);
    OGRErr mi_err = OSRExportToMICoordSys(hSRS, &MI);
    CPLPopErrorHandler();
    if (mi_err == OGRERR_NONE && MI != nullptr) {
      writable::strings mi(1); mi[0] = MI;
      info_out[1] = mi;
    }
    CPLFree(MI);

    char *pwkt = nullptr;
    if (OSRExportToPrettyWkt(hSRS, &pwkt, 0) != OGRERR_NONE) cpp11::stop("error export SRS to PrettyWkt");
    writable::strings pw(1); pw[0] = pwkt;
    info_out[2] = pw;
    CPLFree(pwkt);

    char *uwkt = nullptr;
    if (OSRExportToWkt(hSRS, &uwkt) != OGRERR_NONE) cpp11::stop("error export SRS to WKT");
    writable::strings uw(1); uw[0] = uwkt;
    info_out[3] = uw;
    CPLFree(uwkt);

    // the EPSG code when the CRS carries one, or when GDAL can identify one
    // for it, otherwise NA
    writable::integers epsg(1); epsg[0] = NA_INTEGER;
    OGRSpatialReferenceH hSRSid = OSRClone(hSRS);
    if (hSRSid != nullptr) {
      OSRAutoIdentifyEPSG(hSRSid);
      const char *auth_name = OSRGetAuthorityName(hSRSid, nullptr);
      const char *auth_code = OSRGetAuthorityCode(hSRSid, nullptr);
      if (auth_name != nullptr && auth_code != nullptr && EQUAL(auth_name, "EPSG")) {
        epsg[0] = atoi(auth_code);
      }
      OSRDestroySpatialReference(hSRSid);
    }
    info_out[4] = epsg;

    char *xml = nullptr;
    CPLPushErrorHandler(CPLQuietErrorHandler);
    OGRErr xml_err = OSRExportToXML(hSRS, &xml, "");
    CPLPopErrorHandler();
    if (xml_err == OGRERR_NONE && xml != nullptr) {
      writable::strings xm(1); xm[0] = xml;
      info_out[5] = xm;
    }
    CPLFree(xml);
  }

  if (std::string(sql[0]) != "") poDS->ReleaseResultSet(poLayer);
  GDALClose(poDS);
  return info_out;
}

} // namespace gdallibrary
#endif
