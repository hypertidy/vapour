#ifndef GDALLIBRARY_H
#define GDALLIBRARY_H
#include <cpp11.hpp>
#include "ogrsf_frmts.h"
#include "gdal_priv.h"
#include "CollectorList.h"
#include "gdalraster/gdalraster.h"
#include "ogr_srs_api.h"

namespace gdallibrary {
using namespace cpp11;
namespace writable = cpp11::writable;

constexpr int MAX_INT =  std::numeric_limits<int>::max ();

inline void gdal_register_all() {
  GDALAllRegister();
}

inline void ogr_register_all() {
  OGRRegisterAll();
}

inline void ogr_cleanup_all() {
  OGRCleanupAll();
}
inline void osr_cleanup() {
  OSRCleanup();
}


inline integers gdal_set_config_option(strings option, strings value)
{
  CPLSetConfigOption( std::string(option[0]).c_str(), std::string(value[0]).c_str() );
  writable::integers out(1);
  out[0] = 1;
  return out;
}

inline strings gdal_get_config_option(strings option){
  writable::strings out(1);
  const char *str = CPLGetConfigOption(std::string(option[0]).c_str(), nullptr);
  if (str)
  {
    out[0] = str;
  }
  return out;
}


inline strings gdal_layer_geometry_name(OGRLayer *poLayer) {

  poLayer->ResetReading();

  OGRFeatureDefn *poFDefn = poLayer->GetLayerDefn();
  int gfields = poFDefn->GetGeomFieldCount();
  writable::strings out(gfields);
  const char *geom_name;
  for (int ig = 0; ig < gfields; ig++) {
    OGRGeomFieldDefn *poGFDefn = poFDefn->GetGeomFieldDefn(ig);
    geom_name =    poGFDefn->GetNameRef();
    out[ig] = geom_name;
  }
  return out;
}
inline doubles gdal_layer_extent(OGRLayer *poLayer) {

  OGREnvelope poEnvelope;
  OGRErr err;
  err = poLayer ->GetExtent(&poEnvelope,true);
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

// this force function takes cheap count, checks, then more expensive, checks,
// then iterates and resets reading
inline R_xlen_t force_layer_feature_count(OGRLayer *poLayer) {
  R_xlen_t out;
  out = poLayer->GetFeatureCount(false);
  if (out == -1) {
    out = poLayer->GetFeatureCount(true);
  }
  if (out == -1) {
    out = 0;
    poLayer->ResetReading();
    OGRFeature *poFeature;
    while( (poFeature = poLayer->GetNextFeature()) != NULL )
    {
      out++;
      OGRFeature::DestroyFeature( poFeature );
    }
    poLayer->ResetReading();
  }
  return out;
}
inline integers proj_version()
{
  writable::integers out(3);
  int num1; int num2; int num3;
  OSRGetPROJVersion(&num1, &num2, &num3);
  out[0] = num1;
  out[1] = num2;
  out[2] = num3;
  return out;
}

inline strings gdal_version()
{
  writable::strings out(1);
  out[0] = GDALVersionInfo("--version");
  return out;
}


inline OGRLayer *gdal_layer(GDALDataset *poDS, integers layer, strings sql, doubles ex) {
  OGRLayer  *poLayer;
  OGRPolygon poly;
  OGRLinearRing ring;

  bool use_extent_filter = false;
  if (ex.size() == 4) {
    if (ex[1] <= ex[0] || ex[3] <= ex[2]) {
      if (ex[1] <= ex[0]) {
        cpp11::warning("extent filter invalid (xmax <= xmin), ignoring");
      }
      if (ex[3] <= ex[2]) {
        cpp11::warning("extent filter invalid (ymax <= ymin), ignoring");
      }
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
      poLayer =  poDS->ExecuteSQL(sql_str.c_str(),
                                  &poly,
                                  sql_dialect );
    } else {
      poLayer =  poDS->ExecuteSQL(sql_str.c_str(),
                                  NULL,
                                  sql_dialect );
    }

    if (poLayer == NULL) {
      cpp11::stop("SQL execution failed.\n");
    }

  } else {
    int nlayer = poDS->GetLayerCount();
    if (layer[0] >= nlayer) {
      cpp11::stop("layer index exceeds layer count");
    }

    poLayer =  poDS->GetLayer(layer[0]);
  }
  if (poLayer == NULL) {
    cpp11::stop("Layer open failed.\n");
  }
  return poLayer;
}


// ---- DSN open/close helpers ------------------------------------------------
inline GDALDataset* open_ogr_dataset(const char* dsn,
                                     unsigned int flags = GDAL_OF_VECTOR) {
  GDALDataset *poDS = (GDALDataset*) GDALOpenEx(dsn, flags, NULL, NULL, NULL);
  if (poDS == NULL) {
    cpp11::stop("Open failed.\n");
  }
  return poDS;
}

template <typename Fn>
inline auto with_ogr_layer(strings dsn, integers layer,
                           strings sql, doubles ex,
                           Fn fn) -> decltype(fn(std::declval<OGRLayer*>())) {
  GDALDataset *poDS = open_ogr_dataset(std::string(dsn[0]).c_str());
  OGRLayer *poLayer = gdal_layer(poDS, layer, sql, ex);
  auto out = fn(poLayer);
  if (std::string(sql[0]) != "") {
    poDS->ReleaseResultSet(poLayer);
  }
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
  if (std::string(sql[0]) != "") {
    poDS->ReleaseResultSet(poLayer);
  }
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


inline list gdal_list_drivers()
{
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
    "driver"_nm = sname,
    "name"_nm = lname,
    "vector"_nm = isvector,
    "raster"_nm = israster,
    "create"_nm = iscreate,
    "copy"_nm = iscopy,
    "virtual"_nm = isvirt
  };
  return out;
}

// allocate_fields_list: from sf allocate_out_list by Edzer Pebesma
inline list allocate_fields_list(OGRFeatureDefn *poFDefn, R_xlen_t n_features, bool int64_as_string,
                                       strings fid_column) {

  if (fid_column.size() > 1)
    cpp11::stop("FID column name should be a length 1 character vector");

  int n = poFDefn->GetFieldCount();

  writable::list out(n);
  writable::strings names(n);
  for (int i = 0; i < poFDefn->GetFieldCount(); i++) {
    OGRFieldDefn *poFieldDefn = poFDefn->GetFieldDefn(i);

    switch (poFieldDefn->GetType()) {
    case OFTWideString:
    case OFTWideStringList: {
    }
      break;
    case OFTInteger: {
      if (poFieldDefn->GetSubType() == OFSTBoolean)
        out[i] = writable::logicals(n_features);
      else
        out[i] = writable::integers(n_features);
    }
      break;
    case OFTDate: {
      writable::doubles ret(n_features);
      ret.attr("class") = "Date";
      out[i] = ret;
    } break;
    case OFTTime:
    case OFTDateTime: {
      writable::doubles ret(n_features);
      writable::strings cls = {"POSIXct", "POSIXt"};
      ret.attr("class") = cls;
      out[i] = ret;
    } break;
    case OFTInteger64:
      if (int64_as_string)
        out[i] = writable::strings(n_features);
      else
        out[i] = writable::doubles(n_features);
      break;
    case OFTReal:
      out[i] = writable::doubles(n_features);
      break;
    case OFTStringList:
    case OFTRealList:
    case OFTIntegerList:
    case OFTInteger64List:
      out[i] = writable::list(n_features);
      break;
    case OFTBinary:
      out[i] = writable::list(n_features);
      break;
    case OFTString:
      out[i] = writable::strings(n_features);
      break;
    }
    names[i] = poFieldDefn->GetNameRef();
  }

  out.names() = names;
  return out;
}



inline list gdal_read_fields(strings dsn,
                             integers layer,
                             strings sql,
                             integers limit_n,
                             integers skip_n,
                             doubles ex,
                             strings fid_column_name)
{
  GDALDataset       *poDS;
  poDS = (GDALDataset*) GDALOpenEx(std::string(dsn[0]).c_str(), GDAL_OF_VECTOR, NULL, NULL, NULL );
  if( poDS == NULL )
  {
    cpp11::stop("Open failed.\n");
  }
  OGRLayer *poLayer = gdal_layer(poDS, layer, sql, ex);

  OGRFeature *poFeature;

  R_xlen_t nFeature = (R_xlen_t)poLayer->GetFeatureCount();

  if (nFeature > MAX_INT) {
    cpp11::warning("Number of features exceeds maximal number able to be read");
    nFeature = MAX_INT;
  }
  if (limit_n[0] > 0) {
    if (limit_n[0] < nFeature) {
      nFeature = nFeature - skip_n[0];
      if (limit_n[0] < nFeature) {
        nFeature = limit_n[0];
      }
    }
  }

  if (nFeature < 1) {
    if (skip_n[0] > 0) {
      cpp11::stop("no features to be read (is 'skip_n' set too high?");
    }
    cpp11::stop("no features to be read");
  }

  OGRFeatureDefn *poFDefn = poLayer->GetLayerDefn();
  bool int64_as_string = false;
  writable::list out(allocate_fields_list(poFDefn, (int)nFeature, int64_as_string, fid_column_name));
  int iFeature = 0;
  int lFeature = 0;
  while((poFeature = poLayer->GetNextFeature()) != NULL)
  {
    if (lFeature >= nFeature) {
      break;
    }
    if (iFeature >= skip_n[0]) {
      int iField;
      for( iField = 0; iField < poFDefn->GetFieldCount(); iField++ )
      {
        OGRFieldDefn *poFieldDefn = poFDefn->GetFieldDefn( iField );
        if( poFieldDefn->GetType() == OFTInteger   ) {
          writable::integers nv(out[iField]);
          nv[lFeature] = poFeature->GetFieldAsInteger( iField );
        }

        if( poFieldDefn->GetType() == OFTReal || poFieldDefn->GetType() == OFTInteger64) {
          writable::doubles nv(out[iField]);
          nv[lFeature] = poFeature->GetFieldAsDouble( iField );
        }

        if( poFieldDefn->GetType() == OFTString || poFieldDefn->GetType() == OFTDate || poFieldDefn->GetType() == OFTTime || poFieldDefn->GetType() == OFTDateTime) {
          writable::strings nv(out[iField]);
          nv[lFeature] = poFeature->GetFieldAsString( iField );
        }

        if( poFieldDefn->GetType() == OFTBinary ) {
          writable::list nv(out[iField]);
          int bytecount;
          const GByte *bin = poFeature->GetFieldAsBinary(iField, &bytecount);
          writable::raws rb(bytecount);
          for (int ib = 0; ib < bytecount; ib++) {
            rb[ib] = bin[ib];
          }
          nv[lFeature] = rb;
        }
      }
      lFeature = lFeature + 1;
    }
    iFeature = iFeature + 1;
    OGRFeature::DestroyFeature( poFeature );
  }
  if (std::string(sql[0]) != "") {
    poDS->ReleaseResultSet(poLayer);
  }
  GDALClose( poDS );
  if (lFeature < 1) {
    if (skip_n[0] > 0) {
      cpp11::stop("no features to be read (is 'skip_n' set too high?");
    }
    cpp11::stop("no features to be read");
  }
  return out;
}


inline doubles gdal_feature_count(strings dsn,
                                        integers layer, strings sql, doubles ex) {
  return with_ogr_layer(dsn, layer, sql, ex,
    GDAL_OF_READONLY | GDAL_OF_VECTOR,
    [](OGRLayer *poLayer) -> doubles {
      R_xlen_t nFeature = poLayer->GetFeatureCount();
      if (nFeature < 1) {
        nFeature = force_layer_feature_count(poLayer);
      }
      writable::doubles out(1);
      out[0] = static_cast<double>(nFeature);
      return out;
    });
}

inline strings gdal_driver(strings dsn)
{
  return with_ogr_dataset(std::string(dsn[0]).c_str(), GDAL_OF_READONLY,
    [](GDALDataset *poDS) -> strings {
      writable::strings dname(1);
      dname[0] = poDS->GetDriverName();
      return dname;
    });
}

inline strings gdal_layer_names(strings dsn)
{
  return with_ogr_dataset(std::string(dsn[0]).c_str(), GDAL_OF_VECTOR,
    [](GDALDataset *poDS) -> strings {
      int nlayer = poDS->GetLayerCount();
      writable::strings lnames(nlayer);
      for (int ilayer = 0; ilayer < nlayer; ilayer++) {
        OGRLayer *poLayer = poDS->GetLayer(ilayer);
        lnames[ilayer] = poLayer->GetName();
      }
      return lnames;
    });
}




inline list gdal_read_geometry(strings dsn,
                               integers layer,
                               strings sql,
                               strings what,
                               strings textformat,
                               integers limit_n,
                               integers skip_n,
                               doubles ex)
{
  GDALDataset       *poDS;
  poDS = (GDALDataset*) GDALOpenEx(std::string(dsn[0]).c_str(), GDAL_OF_VECTOR, NULL, NULL, NULL );
  if( poDS == NULL )
  {
    cpp11::stop("Open failed.\n");
  }
  OGRLayer *poLayer = gdal_layer(poDS, layer, sql, ex);

  OGRFeature *poFeature;
  poLayer->ResetReading();

  CollectorList feature_xx;
  int iFeature = 0;
  int lFeature = 0;
  R_xlen_t nFeature = poLayer->GetFeatureCount();
  if (nFeature < 1) {
    nFeature = force_layer_feature_count(poLayer);
  }

  if (nFeature > MAX_INT) {
    nFeature = MAX_INT;
    cpp11::warning("Number of features exceeds maximal number able to be read");
  }

  if (limit_n[0] > 0) {
    if (limit_n[0] < nFeature) {
      nFeature = limit_n[0];
    }
  }
  if (nFeature < 1) {
    cpp11::stop("no features to be read");
  }

  std::string what_str = std::string(what[0]);
  std::string tf_str = std::string(textformat[0]);

  while( (poFeature = poLayer->GetNextFeature()) != NULL )
  {
    if (iFeature >= skip_n[0]) {

      OGRGeometry *poGeometry;
      poGeometry = poFeature->GetGeometryRef();
      if (poGeometry == NULL) {
        feature_xx.push_back(R_NilValue);
      } else {
        if (what_str == "geometry") {
          writable::raws raw(poGeometry->WkbSize());
          poGeometry->exportToWkb(wkbNDR, RAW(raw), wkbVariantIso);
          feature_xx.push_back(raw);
        }
        if (what_str == "text") {
          writable::strings txt(1);
          if (tf_str == "json") {
            char *export_txt = NULL;
            export_txt = poGeometry->exportToJson();
            txt[0] = export_txt;
            CPLFree(export_txt);
          }
          if (tf_str == "gml") {
            char *export_txt = NULL;
            export_txt = poGeometry->exportToGML();
            txt[0] = export_txt;
            CPLFree(export_txt);
          }
          if (tf_str == "kml") {
            char *export_txt = NULL;
            export_txt = poGeometry->exportToKML();
            txt[0] = export_txt;
            CPLFree(export_txt);
          }
          if (tf_str == "wkt") {
            char *pszGEOM_WKT = NULL;
            poGeometry->exportToWkt(&pszGEOM_WKT, wkbVariantIso );
            txt[0] = pszGEOM_WKT;
            CPLFree( pszGEOM_WKT );
          }
          feature_xx.push_back(txt);
        }
        if (what_str == "extent") {
          OGREnvelope env;
          OGR_G_GetEnvelope(poGeometry, &env);
          double minx, maxx, miny, maxy;
          if (poGeometry->IsEmpty()) {
            minx = NA_REAL; maxx = NA_REAL; miny = NA_REAL; maxy = NA_REAL;
          } else {
            minx = env.MinX; maxx = env.MaxX; miny = env.MinY; maxy = env.MaxY;
          }
          writable::doubles extent = {minx, maxx, miny, maxy};
          feature_xx.push_back(extent);
        }
        if (what_str == "type") {
          OGRwkbGeometryType gtyp = OGR_G_GetGeometryType(poGeometry);
          writable::integers r_gtyp(1);
          r_gtyp[0] = (int)gtyp;
          feature_xx.push_back(r_gtyp);
        }
      }

      OGRFeature::DestroyFeature( poFeature );
      lFeature = lFeature + 1;
    }

    iFeature = iFeature + 1;
    if (limit_n[0] > 0 && lFeature >= limit_n[0]) {
      break;
    }
  }
  if (std::string(sql[0]) != "") {
    poDS->ReleaseResultSet(poLayer);
  }
  GDALClose( poDS );
  if (lFeature < 1) {
    if (skip_n[0] > 0) {
      cpp11::stop("no features to be read (is 'skip_n' set too high?");
    }
    cpp11::stop("no features to be read");
  }

  return(feature_xx.vector());
}

inline strings gdal_proj_to_wkt(strings proj_str) {
   OGRSpatialReference oSRS;
   char *pszWKT = nullptr;

   oSRS.SetFromUserInput(std::string(proj_str[0]).c_str());
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
  const char*  crs_in[] = {CHAR(STRING_ELT(proj_str, 0))};

  OGRSpatialReference oSRS;
  oSRS.SetFromUserInput(*crs_in);
  writable::logicals out(1);
  out[0] = (Rboolean)oSRS.IsGeographic();
  return out;
}


inline list gdal_projection_info(strings dsn,
                                 integers layer,
                                 strings sql)
{

  GDALDataset     *poDS = nullptr;
  poDS = (GDALDataset*) GDALOpenEx(std::string(dsn[0]).c_str(), GDAL_OF_VECTOR, NULL, NULL, NULL );

  if( poDS == nullptr )
  {
    cpp11::stop("Open failed.\n");
  }

  writable::doubles zero(1);
  zero[0] = 0.0;
  OGRLayer *poLayer = gdal_layer(poDS, layer, sql, zero);
  OGRLayerH hLayer = reinterpret_cast<OGRLayerH>(poLayer);

  OGRSpatialReferenceH hSRS = OGR_L_GetSpatialRef(hLayer);
  writable::list info_out(6);
  writable::strings outnames = {"Proj4", "MICoordSys", "PrettyWkt", "Wkt", "EPSG", "XML"};

  info_out.names() = outnames;
  if (hSRS == nullptr) {
    // do nothing - e.g. .shp with no .prj
  }  else {
    char *proj4 = nullptr;
    if (OSRExportToProj4(hSRS, &proj4) != OGRERR_NONE)    {
      cpp11::stop("error export SRS to proj4");
    }
    writable::strings p4(1); p4[0] = proj4;
    info_out[0] = p4;
    CPLFree(proj4);

    char *MI = nullptr;
    if (OSRExportToMICoordSys(hSRS, &MI) != OGRERR_NONE) {
      cpp11::stop("error export SRS to MICoordSys");
    }
    writable::strings mi(1); mi[0] = MI;
    info_out[1] = mi;
    CPLFree(MI);

    char *pwkt = nullptr;
    if (OSRExportToPrettyWkt(hSRS, &pwkt, 0) != OGRERR_NONE) {
      cpp11::stop("error export SRS to PrettyWkt");
    }
    writable::strings pw(1); pw[0] = pwkt;
    info_out[2] = pw;
    CPLFree(pwkt);

    char *uwkt = nullptr;
    if (OSRExportToWkt(hSRS, &uwkt) != OGRERR_NONE) {
      cpp11::stop("error export SRS to WKT");
    }
    writable::strings uw(1); uw[0] = uwkt;
    info_out[3] = uw;
    CPLFree(uwkt);

    writable::integers epsg(1); epsg[0] = 0;
    info_out[4] = epsg;

    char *xml = nullptr;
    if (OSRExportToXML(hSRS, &xml, "") != OGRERR_NONE) {
      cpp11::warning("unable export SRS to XML");
    } else {
      writable::strings xm(1); xm[0] = xml;
      info_out[5] = xm;
      CPLFree(xml);
    }
  }  // end if hSRS != nullptr

  // clean up if SQL was used
  if (std::string(sql[0]) != "") {
    poDS->ReleaseResultSet(poLayer);
  }
  GDALClose( poDS );
  return info_out;
}


inline strings gdal_report_fields(strings dsource,
                                          integers layer,
                                          strings sql)
{
  writable::doubles zero(1);
  zero[0] = 0.0;
  return with_ogr_layer(dsource, layer, sql, zero,
    [](OGRLayer *poLayer) -> strings {
      OGRFeature *poFeature;
      poLayer->ResetReading();

      OGRFeatureDefn *poFDefn = poLayer->GetLayerDefn();
      poFeature = poLayer->GetNextFeature();

      int fieldcount = poFDefn->GetFieldCount();
      writable::strings out(fieldcount);
      writable::strings fieldnames(fieldcount);

      if (poFeature != NULL)
      {
        int iField;
        for( iField = 0; iField < fieldcount; iField++ )
        {
          OGRFieldDefn *poFieldDefn = poFDefn->GetFieldDefn( iField );
          fieldnames[iField] = poFieldDefn->GetNameRef();
          if( poFieldDefn->GetType() == OFTBinary )        out[iField] = "OFTBinary";
          if( poFieldDefn->GetType() == OFTDate )          out[iField] = "OFTDate";
          if( poFieldDefn->GetType() == OFTDateTime )      out[iField] = "OFTDateTime";
          if( poFieldDefn->GetType() == OFTInteger )       out[iField] = "OFTInteger";
          if( poFieldDefn->GetType() == OFTInteger64 )     out[iField] = "OFTInteger64";
          if( poFieldDefn->GetType() == OFTInteger64List ) out[iField] = "OFTInteger64List";
          if( poFieldDefn->GetType() == OFTIntegerList )   out[iField] = "OFTIntegerList";
          if( poFieldDefn->GetType() == OFTReal )          out[iField] = "OFTReal";
          if( poFieldDefn->GetType() == OFTRealList )      out[iField] = "OFTRealList";
          if( poFieldDefn->GetType() == OFTString )        out[iField] = "OFTString";
          if( poFieldDefn->GetType() == OFTStringList )    out[iField] = "OFTStringList";
          if( poFieldDefn->GetType() == OFTTime )          out[iField] = "OFTTime";
        }
        OGRFeature::DestroyFeature( poFeature );
      }
      out.names() = fieldnames;
      return out;
    });
}

inline strings gdal_vsi_list(strings urlpath)
{
  char** VSI_paths  = VSIReadDirRecursive(std::string(urlpath[0]).c_str());
  int ipath = 0;
  while (VSI_paths && VSI_paths[ipath] != NULL) {
    ipath++;
  }

  writable::strings names(ipath);
  for (int i = 0; i < ipath; i++) {
    names[i] = VSI_paths[i];
  }

  CSLDestroy(VSI_paths);
  return names;
}


} // namespace gdallibrary
#endif
