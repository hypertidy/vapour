#include <Rcpp.h>

#include "gdalgeometry/gdalgeometry.h"
//#include "gdalmiscutils/gdalmiscutils.h"
#include "ogr_spatialref.h"

using namespace Rcpp;






List layer_read_transform_geom_ij(OGRLayer *poLayer,   OGRCoordinateTransformation *poCT, NumericVector ij, CharacterVector format) {
  OGRFeature *poFeature;
  R_xlen_t st = (R_xlen_t)ij[0]; 
  R_xlen_t en = (R_xlen_t)ij[1]; 
  
  List out(en - st + 1);
  R_xlen_t ii = 0;
  R_xlen_t cnt = 0;
  
  //if (target_chk != OGRERR_NONE) Rcpp::stop("cannot initialize target projection");
  while(ii <= en &&  (poFeature = poLayer->GetNextFeature()) != NULL ) {
    
    Rprintf("%i\n", ii); 
    if (ii >= st) {
      OGRFeature *poNewFeat = new OGRFeature(poFeature->GetDefnRef());
      poNewFeat->SetFrom(poFeature);
      if(poNewFeat != NULL) {
        //poNewFeat->SetFID(poFeature->GetFID());
        OGRGeometry *poGeometry = poNewFeat->GetGeometryRef();
        if (poGeometry != NULL) {
          poGeometry->transform(poCT); 
          out[cnt] = gdalgeometry::feature_read_geom(poNewFeat, format)[0]; 
        }
           OGRFeature::DestroyFeature(poNewFeat); 
      }
      cnt++;
   
    }
    OGRFeature::DestroyFeature(poFeature);
    ii++;
  }
  if (cnt < out.length()) {
    Rcpp::warning("not as many geoms as requested");
  }
  return out;
}

// [[Rcpp::export]]
List dsn_read_transform_geom_ij(CharacterVector dsn, IntegerVector layer,
                                CharacterVector sql, NumericVector ex,
                                CharacterVector format, NumericVector ij, CharacterVector crs) {
  
  GDALDataset       *poDS;
  poDS = (GDALDataset*) GDALOpenEx(dsn[0], GDAL_OF_VECTOR, NULL, NULL, NULL );
  if( poDS == NULL )
  {
    Rcpp::stop("Open failed.\n");
  }
  OGRLayer *poLayer = gdallibrary::gdal_layer(poDS, layer, sql, ex);
  
  
  OGRSpatialReference *oTargetSRS = nullptr;
  oTargetSRS = new OGRSpatialReference;
  OGRErr target_chk =  oTargetSRS->SetFromUserInput(crs[0]);
  if (target_chk != OGRERR_NONE) Rcpp::stop("cannot initialize target projection");
  
  
  
  OGRSpatialReference *oSourceSRS = nullptr;
  oSourceSRS = new OGRSpatialReference;
  oSourceSRS = poLayer->GetSpatialRef(); 
  if (!oSourceSRS) Rcpp::stop("source does not have a valid projection");
  
  // 
  // 
  OGRCoordinateTransformation *poCT;
  poCT = OGRCreateCoordinateTransformation(oSourceSRS, oTargetSRS);
  if( poCT == NULL )	{
    delete oTargetSRS;
    delete oSourceSRS;
    
    Rcpp::stop( "Transformation to this target CRS not possible from this source dataset, target CRS given: \n\n %s \n\n", 
                (char *)  crs[0] );
    
  }
  delete oTargetSRS;
  delete oSourceSRS;
  
  List out = layer_read_transform_geom_ij(poLayer, poCT, ij, format); 
  
  
  // clean up if SQL was used https://www.gdal.org/classGDALDataset.html#ab2c2b105b8f76a279e6a53b9b4a182e0
  if (sql[0] != "") {
    poDS->ReleaseResultSet(poLayer);
  }
  GDALClose(poDS);
  return out;
}
