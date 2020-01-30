#include <limits>

#include "ogrsf_frmts.h"
#include "ogr_api.h"
#include <Rcpp.h>
#include "CollectorList.h"


using namespace Rcpp;

constexpr int MAX_INT =  std::numeric_limits<int>::max ();



// static const char* asString(SEXP sxpString, const int i = 0) {
//
//     //if (isNull(sxpString)) return NULL;
//
//     return(CHAR(STRING_ELT(sxpString, i)));
//
//   }

//
// extern "C" void run_name(GDALDriver *driver) {
//   driver->GetDescription();
// }
// extern "C" SEXP vapour_get_driver(SEXP sxpDriverName) {
//
//   GDALAllRegister();
//
//     const char *pDriverName = asString(sxpDriverName);
//
//     //installErrorHandler();
//     GDALDriver *pDriver = (GDALDriver *) GDALGetDriverByName(pDriverName);
//     //uninstallErrorHandlerAndTriggerError();
//
//     if (pDriver == NULL) {
//       Rcpp::stop("aa/n");
// //      error("No driver registered with name: %s\n", pDriverName);
//     }
//     SEXP sxpHandle = R_MakeExternalPtr((void *) pDriver,
//                                        R_NilValue,
//                                        R_NilValue);
//
//     return(sxpHandle);
//
//   }
//


// thk686
static void attachPoints(SEXP res, OGRGeometryH hG)
{
  SEXP x, y, z;
  int stride = sizeof(double),
    n = OGR_G_GetPointCount(hG);
  if ( n == 0 ) return;
  x = PROTECT(Rf_allocVector(REALSXP, n));
  y = PROTECT(Rf_allocVector(REALSXP, n));
  z = PROTECT(Rf_allocVector(REALSXP, n));
  OGR_G_GetPoints(hG, REAL(x), stride, REAL(y), stride, REAL(z), stride);
  switch ( Rf_length(res) )
  {
  case 3:;
    SET_VECTOR_ELT(res, 0, x);
    SET_VECTOR_ELT(res, 1, y);
    SET_VECTOR_ELT(res, 2, z);
    break;
  case 2:;
    SET_VECTOR_ELT(res, 0, x);
    SET_VECTOR_ELT(res, 1, y);
    break;
  default:;
  Rf_warning("Empty geometry\n");
  break;
  }
  UNPROTECT(3);
  return;
}
// thk686
static void attachNames(SEXP x)
{
  int dim = Rf_length(x);
  SEXP names = PROTECT(Rf_allocVector(STRSXP, dim));
  switch ( dim )
  {
  case 3:;
    SET_STRING_ELT(names, 2, Rf_mkChar("z"));
  case 2:;
    SET_STRING_ELT(names, 1, Rf_mkChar("y"));
    SET_STRING_ELT(names, 0, Rf_mkChar("x"));
    break;
  default:;
  Rf_warning("Empty geometry\n");
  break;
  }
  Rf_setAttrib(x, R_NamesSymbol, names);
  UNPROTECT(1);
  return;
}
// thk686
static SEXP GetPointsInternal(OGRGeometryH hG, int nested)
{
  SEXP res;
  int n = OGR_G_GetGeometryCount(hG);
  switch ( n )
  {
  case 0:
  {
    int dim = OGR_G_GetCoordinateDimension(hG);
    res = PROTECT(Rf_allocVector(VECSXP, dim));
    attachPoints(res, hG);
    attachNames(res);
    UNPROTECT(1);
  }
    break;
  case 1:
    if (nested == 0) {
      {
        OGRGeometryH hR = OGR_G_GetGeometryRef(hG, 0);
        res = GetPointsInternal(hR, nested);
      }
      break;
    } // else: fall through
  default:
    {
      res = PROTECT(Rf_allocVector(VECSXP, n));
      for ( int i = 0; i != n; ++i )
      {
        OGRGeometryH hR = OGR_G_GetGeometryRef(hG, i);
        SET_VECTOR_ELT(res, i, GetPointsInternal(hR, nested));
      }
      UNPROTECT(1);
    }
    break;
  }
  return res;
}


// [[Rcpp::export]]
Rcpp::CharacterVector vapour_driver_cpp(Rcpp::CharacterVector dsource)
{

  GDALAllRegister();
  GDALDataset       *poDS;
  poDS = (GDALDataset*) GDALOpenEx(dsource[0], GDAL_OF_VECTOR, NULL, NULL, NULL );
  if( poDS == NULL )
  {
    Rcpp::stop("Open failed.\n");
  }
  Rcpp::CharacterVector dname(1);
  dname[0] = poDS->GetDriverName();
  GDALClose(poDS);
  return(dname);
}


// [[Rcpp::export]]
Rcpp::CharacterVector vapour_layer_names_cpp(Rcpp::CharacterVector dsource,
                                             Rcpp::CharacterVector sql = "")
{

  GDALAllRegister();
  GDALDataset       *poDS;
  poDS = (GDALDataset*) GDALOpenEx(dsource[0], GDAL_OF_VECTOR, NULL, NULL, NULL );
  if( poDS == NULL )
  {
    Rcpp::stop("Open failed.\n");
  }
  OGRLayer  *poLayer;
  if (sql[0] != "") {

    poLayer =  poDS->ExecuteSQL(sql[0],
                                NULL,
                                NULL);

    if (poLayer == NULL) {
      Rcpp::stop("SQL execution failed.\n");
    }
    // clean up if SQL was used https://www.gdal.org/classGDALDataset.html#ab2c2b105b8f76a279e6a53b9b4a182e0
    poDS->ReleaseResultSet(poLayer);

  }
  int nlayer = poDS->GetLayerCount();
  Rcpp::CharacterVector lnames = Rcpp::CharacterVector(nlayer);

  for (int ilayer = 0; ilayer < nlayer; ilayer++) {
    poLayer = poDS->GetLayer(ilayer);
    lnames[ilayer] = poLayer->GetName();
  }
  GDALClose(poDS);
  return(lnames);
}

// copied from Edzer Pebesma, https://github.com/r-spatial/sf/blob/master/src/gdal_read.cpp
Rcpp::List allocate_attribute(OGRFeatureDefn *poFDefn, int n_features, bool int64_as_string) {

  int n = poFDefn->GetFieldCount(); //+ poFDefn->GetGeomFieldCount();
  Rcpp::List out(n);
  Rcpp::CharacterVector names(n);
  for (int i = 0; i < poFDefn->GetFieldCount(); i++) {
    OGRFieldDefn *poFieldDefn = poFDefn->GetFieldDefn(i);
    switch (poFieldDefn->GetType()) {
    case OFTInteger:
      out[i] = Rcpp::IntegerVector(n_features);
      break;
    case OFTDate: {
      Rcpp::NumericVector ret(n_features);
      ret.attr("class") = "Date";
      out[i] = ret;
    } break;
    case OFTDateTime: {
      Rcpp::NumericVector ret(n_features);
      Rcpp::CharacterVector cls(2);
      cls(0) = "POSIXct";
      cls(1) = "POSIXt";
      ret.attr("class") = cls;
      out[i] = ret;
    } break;
    case OFTInteger64: // fall through: converts Int64 -> double
      if (int64_as_string)
        out[i] = Rcpp::CharacterVector(n_features);
      else
        out[i] = Rcpp::NumericVector(n_features);
      break;
    case OFTReal:
      out[i] = Rcpp::NumericVector(n_features);
      break;
    case OFTString:
      out[i] = Rcpp::CharacterVector(n_features);
      break;
    case OFTStringList:
    case OFTRealList:
    case OFTIntegerList:
      out[i] = Rcpp::List(n_features);
      break;
    default:
      Rcpp::stop("Unrecognized field type\n"); // #nocov
    break;
    }
    names[i] = poFieldDefn->GetNameRef();
  }

  out.attr("names") = names;
  return out;
}
// [[Rcpp::export]]
DoubleVector find_feature_count_cpp(Rcpp::CharacterVector dsource,
                                    Rcpp::IntegerVector layer = 0,
                                    Rcpp::LogicalVector iterate = true) {
  GDALAllRegister();
  GDALDataset       *poDS;
  poDS = (GDALDataset*) GDALOpenEx(dsource[0], GDAL_OF_VECTOR, NULL, NULL, NULL );
  if( poDS == NULL )
  {
    Rcpp::stop("Open failed.\n");
  }


  OGRLayer  *aLayer;
  aLayer =  poDS->GetLayer(layer[0]);
  OGRFeature *aFeature;
  aLayer->ResetReading();
  double nFeature = (double)aLayer->GetFeatureCount();
  if (iterate) {
    nFeature = 0;
    while( (aFeature = aLayer->GetNextFeature()) != NULL )
    {
      nFeature++;
      OGRFeature::DestroyFeature(aFeature);
    }

  }
  GDALClose( poDS );

  return(nFeature);
}

// [[Rcpp::export]]
List vapour_read_attributes_cpp(Rcpp::CharacterVector dsource,
                                Rcpp::IntegerVector layer = 0,
                                Rcpp::CharacterVector sql = "",
                                Rcpp::IntegerVector limit_n = 0,
                                Rcpp::IntegerVector skip_n = 0,
                                Rcpp::NumericVector ex = 0)
{
  GDALAllRegister();
  GDALDataset       *poDS;
  poDS = (GDALDataset*) GDALOpenEx(dsource[0], GDAL_OF_VECTOR, NULL, NULL, NULL );
  if( poDS == NULL )
  {
    Rcpp::stop("Open failed.\n");
  }


  OGRLayer  *poLayer;
  OGRPolygon poly;
  OGRLinearRing ring;

  if (ex.length() == 4) {

    ring.addPoint(ex[0], ex[2]); //xmin, ymin
    ring.addPoint(ex[0], ex[3]); //xmin, ymax
    ring.addPoint(ex[1], ex[3]); //xmax, ymax
    ring.addPoint(ex[1], ex[2]); //xmax, ymin

    ring.closeRings();
    poly.addRing(&ring);
  }

  if (sql[0] != "") {
    if (ex.length() == 4) {
      poLayer =  poDS->ExecuteSQL(sql[0],
                                  &poly,
                                  NULL );
    } else {
      poLayer =  poDS->ExecuteSQL(sql[0],
                                  NULL,
                                  NULL );
    }

    if (poLayer == NULL) {
      Rcpp::stop("SQL execution failed.\n");
    }

  } else {
    poLayer =  poDS->GetLayer(layer[0]);
  }
  if (poLayer == NULL) {
    Rcpp::stop("Layer open failed.\n");
  }

  OGRFeature *poFeature;
  poLayer->ResetReading();
  double nFeature = (double)poLayer->GetFeatureCount();
nFeature = -1;
  if (nFeature == -1) {

   nFeature = 0;
    // we have to find out first because this driver doesn't support GetFeatureCount
    // https://trac.osgeo.org/gdal/wiki/rfc66_randomlayerreadwrite
    while( (poFeature = poLayer->GetNextFeature()) != NULL )
    {
      nFeature++;
      OGRFeature::DestroyFeature( poFeature );
    }

    poLayer->ResetReading();

  }

  if (nFeature > MAX_INT)
    Rcpp::stop("Number of features exceeds maximal number able to be read");


// this is poorly laid out but works, check twice to avoid
// over allocating as per #60
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
      Rcpp::stop("no features to be read (is 'skip_n' set too high?");
    }
    Rcpp::stop("no features to be read");
  }


  OGRFeatureDefn *poFDefn = poLayer->GetLayerDefn();
  bool int64_as_string = false;
  Rcpp::List out = allocate_attribute(poFDefn, nFeature, int64_as_string);
  int iFeature = 0;  // always increment iFeature, it is position through the loop
  int lFeature = 0; // keep a count of the features we actually send out
  while((poFeature = poLayer->GetNextFeature()) != NULL)
  {


    if (lFeature >= nFeature) {
      break;
    }
    OGRFeatureDefn *poFDefn = poLayer->GetLayerDefn();
    // only increment lFeature if we actually keep this one
    if (iFeature >= skip_n[0]) {  // we are at skip_n

      int iField;
      for( iField = 0; iField < poFDefn->GetFieldCount(); iField++ )
      {
        OGRFieldDefn *poFieldDefn = poFDefn->GetFieldDefn( iField );
        if( poFieldDefn->GetType() == OFTInteger   ) {
          Rcpp::IntegerVector nv;
          nv = out[iField];
          nv[lFeature] = poFeature->GetFieldAsInteger( iField );
        }

        if( poFieldDefn->GetType() == OFTReal || poFieldDefn->GetType() == OFTInteger64) {
          Rcpp::NumericVector nv;
          nv = out[iField];
          nv[lFeature] = poFeature->GetFieldAsDouble( iField );
        }

        if( poFieldDefn->GetType() == OFTString || poFieldDefn->GetType() == OFTDate || poFieldDefn->GetType() == OFTTime || poFieldDefn->GetType() == OFTDateTime) {
          Rcpp::CharacterVector nv;
          nv = out[iField];
          nv[lFeature] = poFeature->GetFieldAsString( iField );

        }
      }
      // so we start counting
      lFeature = lFeature + 1;
    }
    // always increment iFeature, it's position through the loop
    iFeature = iFeature + 1;
    OGRFeature::DestroyFeature( poFeature );

  }

  // clean up if SQL was used https://www.gdal.org/classGDALDataset.html#ab2c2b105b8f76a279e6a53b9b4a182e0
  if (sql[0] != "") {
    poDS->ReleaseResultSet(poLayer);
  }
  GDALClose( poDS );

  if (lFeature < 1) {

    if (skip_n[0] > 0) {
      Rcpp::stop("no features to be read (is 'skip_n' set too high?");
    }
    Rcpp::stop("no features to be read");
  }

  return(out);
}



// [[Rcpp::export]]
List vapour_read_geometry_cpp(Rcpp::CharacterVector dsource,
                              Rcpp::IntegerVector layer = 0,
                              Rcpp::CharacterVector sql = "",
                              Rcpp::CharacterVector what = "geometry",
                              Rcpp::CharacterVector textformat = "json",
                              Rcpp::IntegerVector limit_n = 0,
                              Rcpp::IntegerVector skip_n = 0,
                              Rcpp::NumericVector ex = 0)
{
  GDALAllRegister();
  GDALDataset       *poDS;
  poDS = (GDALDataset*) GDALOpenEx(dsource[0], GDAL_OF_VECTOR, NULL, NULL, NULL );
  if( poDS == NULL )
  {
    Rcpp::stop("Open failed.\n");
  }
  OGRLayer  *poLayer;


  OGRPolygon poly;
  OGRLinearRing ring;

  if (ex.length() == 4) {

    ring.addPoint(ex[0], ex[2]); //xmin, ymin
    ring.addPoint(ex[0], ex[3]); //xmin, ymax
    ring.addPoint(ex[1], ex[3]); //xmax, ymax
    ring.addPoint(ex[1], ex[2]); //xmax, ymin

    ring.closeRings();
    poly.addRing(&ring);
  }

  if (sql[0] != "") {
    if (ex.length() == 4) {
      poLayer =  poDS->ExecuteSQL(sql[0],
                                  &poly,
                                  NULL );
    } else {
      poLayer =  poDS->ExecuteSQL(sql[0],
                                  NULL,
                                  NULL );
    }

    if (poLayer == NULL) {
      Rcpp::stop("SQL execution failed.\n");
    }

  } else {
    poLayer =  poDS->GetLayer(layer[0]);
  }
  if (poLayer == NULL) {
    Rcpp::stop("Layer open failed.\n");
  }
  OGRFeature *poFeature;
  poLayer->ResetReading();

  //OGRFeatureDefn *poFDefn = poLayer->GetLayerDefn();
  CollectorList feature_xx;
  int iFeature = 0;
  int lFeature = 0;
  int nFeature = (int)poLayer->GetFeatureCount();
  if (nFeature > MAX_INT)
    Rcpp::stop("Number of features exceeds maximal number able to be read");
  if (nFeature == -1) {
    nFeature = 0;
    // we have to find out first because this driver doesn't support GetFeatureCount
    // https://trac.osgeo.org/gdal/wiki/rfc66_randomlayerreadwrite
    while( (poFeature = poLayer->GetNextFeature()) != NULL )
    {
      nFeature++;
      OGRFeature::DestroyFeature( poFeature );
    }
    poLayer->ResetReading();

  }


  if (limit_n[0] > 0) {
    if (limit_n[0] < nFeature) {
      nFeature = limit_n[0];
    }
  }
  if (nFeature < 1) {
    Rcpp::stop("no features to be read");
  }


  int warncount = 0;
  while( (poFeature = poLayer->GetNextFeature()) != NULL )
  {

    if (iFeature >= skip_n[0]) {  // we are at skip_n

      OGRGeometry *poGeometry;
      poGeometry = poFeature->GetGeometryRef();
      if (poGeometry == NULL) {
        warncount++;
        feature_xx.push_back(R_NilValue);
        //if (warncount == 1) Rcpp::warning("at least one geometry is NULL, perhaps the 'sql' argument excludes the native geometry?\n(use 'SELECT * FROM ..') ");
      } else {
        // GEOMETRY
        // geometry native binary
        // text     various text forms
        // extent   simple bbox
        if (what[0] == "geometry") {
          //https://github.com/r-spatial/sf/blob/798068d3044a65797c52bf3b42bc4a5d83b45e9a/src/gdal.cpp#L207
          Rcpp::RawVector raw(poGeometry->WkbSize());

          //todo we probably need better err handling see sf handle_error
          poGeometry->exportToWkb(wkbNDR, &(raw[0]), wkbVariantIso);
          feature_xx.push_back(raw);

        }
        if (what[0] == "text") {
          Rcpp::CharacterVector txt(1);
          if (textformat[0] == "json") {
            char *export_txt = NULL;
            export_txt = poGeometry->exportToJson();
            txt[0] = export_txt;
            CPLFree(export_txt);
          }
          if (textformat[0] == "gml") {
            char *export_txt = NULL;

            export_txt = poGeometry->exportToGML();
            txt[0] = export_txt;
            CPLFree(export_txt);

          }
          if (textformat[0] == "kml") {
            char *export_txt = NULL;

            export_txt = poGeometry->exportToKML();
            txt[0] = export_txt;
            CPLFree(export_txt);

          }
          if (textformat[0] == "wkt") {
            //     // see buffer handling for SRS here which is where
            //     // I got inspiration from : http://www.gdal.org/gdal_tutorial.html
            char *pszGEOM_WKT = NULL;
            //     // see here for the constants for the format variants
            //     // http://www.gdal.org/ogr__core_8h.html#a6716bd3399c31e7bc8b0fd94fd7d9ba6a7459e8d11fa69e89271771c8d0f265d8
            poGeometry->exportToWkt(&pszGEOM_WKT, wkbVariantIso );
            txt[0] = pszGEOM_WKT;
            CPLFree( pszGEOM_WKT );

          }
          feature_xx.push_back(txt);


        }
        if (what[0] == "extent") {
          OGREnvelope env;
          OGR_G_GetEnvelope(poGeometry, &env);
          // if geometry is empty, set the envelope to undefined (otherwise all 0s)
          double minx, maxx, miny, maxy;
          if (poGeometry->IsEmpty()) {

            minx = NA_REAL;
            maxx = NA_REAL;
            miny = NA_REAL;
            maxy = NA_REAL;

          } else {
            minx = env.MinX;
            maxx = env.MaxX;
            miny = env.MinY;
            maxy = env.MaxY;
          }

          Rcpp::NumericVector extent = NumericVector::create(minx, maxx, miny, maxy);
          feature_xx.push_back(extent);


        }
        if (what[0] == "point") {
          Rcpp::List pts = Rcpp::List(1);
          pts = GetPointsInternal(poGeometry, 1);
          feature_xx.push_back(pts);
        }
        if (what[0] == "type") {
          OGRwkbGeometryType gtyp = OGR_G_GetGeometryType(poGeometry);
          Rcpp::IntegerVector r_gtyp = Rcpp::IntegerVector(1);
          r_gtyp[0] = (int)gtyp;
          feature_xx.push_back(r_gtyp);
        }
      }
      OGRFeature::DestroyFeature( poFeature );
      lFeature = lFeature + 1;
    }  //    if (iFeature >= skip_n[0]) {  // we are at skip_n

    iFeature = iFeature + 1;
    if (limit_n[0] > 0 && lFeature >= limit_n[0]) {
      break;  // short-circuit for limit_n
    }

  }

  // clean up if SQL was used https://www.gdal.org/classGDALDataset.html#ab2c2b105b8f76a279e6a53b9b4a182e0
  if (sql[0] != "") {
    poDS->ReleaseResultSet(poLayer);
  }
  GDALClose( poDS );

  if (lFeature < 1) {
    if (skip_n[0] > 0) {
      Rcpp::stop("no features to be read (is 'skip_n' set too high?");
    }

    Rcpp::stop("no features to be read");
  }

  return(feature_xx.vector());
}



// [[Rcpp::export]]
List vapour_projection_info_cpp(Rcpp::CharacterVector dsource,
                                Rcpp::IntegerVector layer = 0,
                                Rcpp::CharacterVector sql = "")
{
  GDALAllRegister();
  GDALDataset       *poDS;
  poDS = (GDALDataset*) GDALOpenEx(dsource[0], GDAL_OF_VECTOR, NULL, NULL, NULL );
  if( poDS == NULL )
  {
    Rcpp::stop("Open failed.\n");
  }
  OGRLayer  *poLayer;
  if (sql[0] != "") {
    poLayer =  poDS->ExecuteSQL(sql[0],
                                NULL,
                                NULL );

    if (poLayer == NULL) {
      Rcpp::stop("SQL execution failed.\n");
    }

  } else {
    poLayer =  poDS->GetLayer(layer[0]);
  }
  if (poLayer == NULL) {
    Rcpp::stop("Layer open failed.\n");
  }
  OGRSpatialReference *SRS =  poLayer->GetSpatialRef();

  char *proj;  // this gets cleaned up lower in the SRS==NULL else
  List info_out(6);
  CharacterVector outproj(1);
  CharacterVector outnames(6);
  outnames[0] = "Proj4";
  outnames[1] = "MICoordSys";
  outnames[2] = "PrettyWkt";
  outnames[3] = "Wkt";
  outnames[4] = "EPSG";
  outnames[5] = "XML";
  info_out.attr("names") = outnames;

  if (SRS == NULL) {
    //Rcpp::warning("null");
    // do nothing, or warn
    // e.g. .shp with no .prj
  } else {
    Rcpp::warning("not null");
    // SRS is not NULL, so explore validation
  //  OGRErr err = SRS->Validate();
    SRS->exportToProj4(&proj);
    outproj[0] = proj;
    info_out[0] = Rcpp::clone(outproj);

    SRS->exportToMICoordSys(&proj);
    outproj[0] = proj;
    info_out[1] = Rcpp::clone(outproj);

    SRS->exportToPrettyWkt(&proj, false);
    outproj[0] = proj;
    info_out[2] = Rcpp::clone(outproj);

    SRS->exportToWkt(&proj);
    outproj[0] = proj;
    info_out[3] = Rcpp::clone(outproj);

    int epsg = SRS->GetEPSGGeogCS();
    info_out[4] = epsg;

    SRS->exportToXML(&proj);
    outproj[0] = proj;
    info_out[5] = Rcpp::clone(outproj);

    CPLFree(proj);
  }

  // clean up if SQL was used https://www.gdal.org/classGDALDataset.html#ab2c2b105b8f76a279e6a53b9b4a182e0
  if (sql[0] != "") {
    poDS->ReleaseResultSet(poLayer);
  }
  GDALClose( poDS );
  return info_out;
}


// [[Rcpp::export]]
List vapour_read_names_cpp(Rcpp::CharacterVector dsource,
                           Rcpp::IntegerVector layer = 0,
                           Rcpp::CharacterVector sql = "",
                           Rcpp::IntegerVector limit_n = 0,
                           Rcpp::IntegerVector skip_n = 0,
                           Rcpp::NumericVector ex = 0)
{
  GDALAllRegister();
  GDALDataset       *poDS;
  poDS = (GDALDataset*) GDALOpenEx(dsource[0], GDAL_OF_VECTOR, NULL, NULL, NULL );
  if( poDS == NULL )
  {
    Rcpp::stop("Open failed.\n");
  }
  OGRLayer  *poLayer;


  OGRPolygon poly;
  OGRLinearRing ring;

  if (ex.length() == 4) {

    ring.addPoint(ex[0], ex[2]); //xmin, ymin
    ring.addPoint(ex[0], ex[3]); //xmin, ymax
    ring.addPoint(ex[1], ex[3]); //xmax, ymax
    ring.addPoint(ex[1], ex[2]); //xmax, ymin

    ring.closeRings();
    poly.addRing(&ring);
  }

  if (sql[0] != "") {
    if (ex.length() == 4) {
      poLayer =  poDS->ExecuteSQL(sql[0],
                                  &poly,
                                  NULL );
    } else {
      poLayer =  poDS->ExecuteSQL(sql[0],
                                  NULL,
                                  NULL );
    }

    if (poLayer == NULL) {
      Rcpp::stop("SQL execution failed.\n");
    }

  } else {
    poLayer =  poDS->GetLayer(layer[0]);
  }
  if (poLayer == NULL) {
    Rcpp::stop("Layer open failed.\n");
  }
  OGRFeature *poFeature;
  poLayer->ResetReading();

  //OGRFeatureDefn *poFDefn = poLayer->GetLayerDefn();
  CollectorList feature_xx;
  int iFeature = 0;
  int lFeature = 0;
  int nFeature = (int)poLayer->GetFeatureCount();
  if (nFeature > MAX_INT)
    Rcpp::stop("Number of features exceeds maximal number able to be read");

  if (nFeature == -1) {
    nFeature = 0;
    // we have to find out first because this driver doesn't support GetFeatureCount
    // https://trac.osgeo.org/gdal/wiki/rfc66_randomlayerreadwrite
    while( (poFeature = poLayer->GetNextFeature()) != NULL )
    {
      nFeature++;
      OGRFeature::DestroyFeature( poFeature );
    }
    poLayer->ResetReading();

  }
  if (limit_n[0] > 0) {
    if (limit_n[0] < nFeature) {
      nFeature = limit_n[0];
    }
  }

  if (nFeature < 1) {
    if (skip_n[0] > 0) {
      Rcpp::stop("no features to be read (is 'skip_n' set too high?");
    }

    Rcpp::stop("no features to be read");
  }

  double aFID;
  Rcpp::NumericVector rFID(1);
  while( (poFeature = poLayer->GetNextFeature()) != NULL )
  {

    if (iFeature >= skip_n[0]) {
      aFID = (double) poFeature->GetFID();
      OGRFeature::DestroyFeature( poFeature );
      rFID[0] = aFID;
      feature_xx.push_back(Rcpp::clone(rFID));
      lFeature++;
    }
    iFeature++;
    if (limit_n[0] > 0 && lFeature >= limit_n[0]) {
      break;  // short-circuit for limit_n
    }
  }
  // clean up if SQL was used https://www.gdal.org/classGDALDataset.html#ab2c2b105b8f76a279e6a53b9b4a182e0
  if (sql[0] != "") {
    poDS->ReleaseResultSet(poLayer);
  }
  GDALClose( poDS );

  if (lFeature < 1) {
    if (skip_n[0] > 0) {
      Rcpp::stop("no features to be read (is 'skip_n' set too high?");
    }

    Rcpp::stop("no features to be read");
  }
  return(feature_xx.vector());
}



// [[Rcpp::export]]
CharacterVector vapour_report_attributes_cpp(Rcpp::CharacterVector dsource,
                                             Rcpp::IntegerVector layer = 0,
                                             Rcpp::CharacterVector sql = "")
{
  GDALAllRegister();
  GDALDataset       *poDS;
  poDS = (GDALDataset*) GDALOpenEx(dsource[0], GDAL_OF_VECTOR, NULL, NULL, NULL );
  if( poDS == NULL )
  {
    Rcpp::stop("Open failed.\n");
  }


  OGRLayer  *poLayer;

  if (sql[0] != "") {
    poLayer =  poDS->ExecuteSQL(sql[0],
                                NULL,
                                NULL );

    if (poLayer == NULL) {
      Rcpp::stop("SQL execution failed.\n");
    }

  } else {
    poLayer =  poDS->GetLayer(layer[0]);
  }
  if (poLayer == NULL) {
    Rcpp::stop("Layer open failed.\n");
  }

  OGRFeature *poFeature;
  poLayer->ResetReading();

  OGRFeatureDefn *poFDefn = poLayer->GetLayerDefn();
  //int iFeature = 0;
  poFeature = poLayer->GetNextFeature();

  int fieldcount = poFDefn->GetFieldCount();
  Rcpp::CharacterVector out(fieldcount);
  Rcpp::CharacterVector fieldnames(fieldcount);
  // as at 2019-08-23 https://gdal.org/ogr__core_8h.html#a787194bea637faf12d61643124a7c9fc
  //   OFTInteger Simple32bitinteger
  //   OFTIntegerList Listof32bitintegers
  //   OFTReal DoublePrecisionfloatingpoint
  //   OFTRealList Listofdoubles
  //   OFTString StringofASCIIchars
  //   OFTStringList Arrayofstrings
  //   OFTWideString deprecated
  //   OFTWideStringList deprecated
  //   OFTBinary RawBinarydata
  //   OFTDate Date
  //   OFTTime Time
  //   OFTDateTime DateandTime
  //   OFTInteger64 Single64bitinteger
  //   OFTInteger64List Listof64bitintegers
  //
  if (poFeature != NULL)
  {

    int iField;
    for( iField = 0; iField < fieldcount; iField++ )
    {
      OGRFieldDefn *poFieldDefn = poFDefn->GetFieldDefn( iField );
      fieldnames[iField] = poFieldDefn->GetNameRef();
      if( poFieldDefn->GetType() == OFTBinary ) {
        out[iField] = "OFTBinary";
      }
      if( poFieldDefn->GetType() == OFTDate ) {
        out[iField] = "OFTDate";
      }
      if( poFieldDefn->GetType() == OFTDateTime ) {
        out[iField] = "OFTDateTime";
      }
      if( poFieldDefn->GetType() == OFTInteger ) {
        out[iField] = "OFTInteger";
      }
      if( poFieldDefn->GetType() == OFTInteger64 ) {
        out[iField] = "OFTInteger64";
      }
      if( poFieldDefn->GetType() == OFTInteger64List ) {
        out[iField] = "OFTInteger64List";
      }
      if( poFieldDefn->GetType() == OFTIntegerList ) {
        out[iField] = "OFTIntegerList";
      }
      if( poFieldDefn->GetType() == OFTReal ) {
        out[iField] = "OFTReal";
      }
      if( poFieldDefn->GetType() == OFTRealList ) {
        out[iField] = "OFTRealList";
      }
      if( poFieldDefn->GetType() == OFTString ) {
        out[iField] = "OFTString";
      }
      if( poFieldDefn->GetType() == OFTStringList ) {
        out[iField] = "OFTStringList";
      }
      if( poFieldDefn->GetType() == OFTTime ) {
        out[iField] = "OFTTime";
      }
      // if( poFieldDefn->GetType() == OFTWideString ) {
      //   out[iField] = "OFTWideString";
      // }
      // if( poFieldDefn->GetType() == OFTWideStringList ) {
      //   out[iField] = "OFTWideStringList";
      // }
    }

    OGRFeature::DestroyFeature( poFeature );

  }
  out.attr("names") = fieldnames;
  // clean up if SQL was used https://www.gdal.org/classGDALDataset.html#ab2c2b105b8f76a279e6a53b9b4a182e0
  if (sql[0] != "") {
    poDS->ReleaseResultSet(poLayer);
  }
  GDALClose( poDS );
  return(out);
}


