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
  // input that is not a CRS gives NA, that is the answer and not a problem to
  // report
  OGRErr in_err, err;
  {
    gdalcommon::QuietErrors quiet;
    in_err = oSRS.SetFromUserInput(as_cstr(proj_str[0]));
#if GDAL_VERSION_MAJOR >= 3
    const char *options[3] = { "MULTILINE=YES", "FORMAT=WKT2", NULL };
    err = oSRS.exportToWkt(&pszWKT, options);
#else
    err = oSRS.exportToWkt(&pszWKT);
#endif
  }
  gdalcommon::CPLStringPtr wkt_owner(pszWKT);
  writable::strings out(1);
  if (in_err != OGRERR_NONE || err != OGRERR_NONE || pszWKT == nullptr) {
    out[0] = NA_STRING;
  } else {
    out[0] = pszWKT;
  }
  return out;
}

inline logicals gdal_crs_is_lonlat(SEXP proj_str) {
  const char* crs_in[] = {CHAR(STRING_ELT(proj_str, 0))};
  OGRSpatialReference oSRS;
  {
    gdalcommon::QuietErrors quiet;
    oSRS.SetFromUserInput(*crs_in);
  }
  writable::logicals out(1);
  out[0] = (Rboolean)oSRS.IsGeographic();
  return out;
}

inline list gdal_projection_info(strings dsn, integers layer, strings sql, strings open_options) {
  gdalcommon::DatasetPtr poDS = gdalcommon::open_dataset_or_stop(as_cstr(dsn[0]), GDAL_OF_VECTOR, open_options);

  writable::doubles zero(1);
  zero[0] = 0.0;
  gdalcommon::LayerHandle poLayer = gdal_layer(poDS.get(), layer, sql, zero);
  OGRLayerH hLayer = reinterpret_cast<OGRLayerH>(poLayer.get());
  OGRSpatialReferenceH hSRS = OGR_L_GetSpatialRef(hLayer);

  writable::list info_out(6);
  writable::strings outnames = {"Proj4", "MICoordSys", "PrettyWkt", "Wkt", "EPSG", "XML"};
  info_out.names() = outnames;

  if (hSRS == nullptr) {
    // no CRS - e.g. .shp with no .prj
  } else {
    char *proj4 = nullptr;
    OGRErr p4_err = OSRExportToProj4(hSRS, &proj4);
    gdalcommon::CPLStringPtr proj4_owner(proj4);
    if (p4_err != OGRERR_NONE || proj4 == nullptr) gdalcommon::stop_gdal("error export SRS to proj4");
    writable::strings p4(1); p4[0] = proj4;
    info_out[0] = p4;

    // MICoordSys and XML cannot express every CRS. Those two elements are left
    // NULL when the export fails, without GDAL's message or a warning.
    char *MI = nullptr;
    OGRErr mi_err;
    {
      gdalcommon::QuietErrors quiet;
      mi_err = OSRExportToMICoordSys(hSRS, &MI);
    }
    gdalcommon::CPLStringPtr MI_owner(MI);
    if (mi_err == OGRERR_NONE && MI != nullptr) {
      writable::strings mi(1); mi[0] = MI;
      info_out[1] = mi;
    }

    char *pwkt = nullptr;
    OGRErr pw_err = OSRExportToPrettyWkt(hSRS, &pwkt, 0);
    gdalcommon::CPLStringPtr pwkt_owner(pwkt);
    if (pw_err != OGRERR_NONE || pwkt == nullptr) gdalcommon::stop_gdal("error export SRS to PrettyWkt");
    writable::strings pw(1); pw[0] = pwkt;
    info_out[2] = pw;

    char *uwkt = nullptr;
    OGRErr uw_err = OSRExportToWkt(hSRS, &uwkt);
    gdalcommon::CPLStringPtr uwkt_owner(uwkt);
    if (uw_err != OGRERR_NONE || uwkt == nullptr) gdalcommon::stop_gdal("error export SRS to WKT");
    writable::strings uw(1); uw[0] = uwkt;
    info_out[3] = uw;

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
    OGRErr xml_err;
    {
      gdalcommon::QuietErrors quiet;
      xml_err = OSRExportToXML(hSRS, &xml, "");
    }
    gdalcommon::CPLStringPtr xml_owner(xml);
    if (xml_err == OGRERR_NONE && xml != nullptr) {
      writable::strings xm(1); xm[0] = xml;
      info_out[5] = xm;
    }
  }

  return info_out;
}

} // namespace gdallibrary
#endif
