#include "/usr/include/gdal/ogrsf_frmts.h"

#include <Rcpp.h>
using namespace Rcpp;

//' Vapourous cloud like substances
//'
//' Microprocessors, databases, servers.
//' @export
// [[Rcpp::export]]
int vapour()
{
  GDALAllRegister();
  GDALDataset       *poDS;
  poDS = (GDALDataset*) GDALOpenEx( "point.shp", GDAL_OF_VECTOR, NULL, NULL, NULL );
  if( poDS == NULL )
  {
    printf( "Open failed.\n" );
    exit( 1 );
  }
  OGRLayer  *poLayer;
  poLayer = poDS->GetLayerByName( "point" );
  OGRFeature *poFeature;
  poLayer->ResetReading();
  while( (poFeature = poLayer->GetNextFeature()) != NULL )
  {
    OGRFeatureDefn *poFDefn = poLayer->GetLayerDefn();
    int iField;
    for( iField = 0; iField < poFDefn->GetFieldCount(); iField++ )
    {
      OGRFieldDefn *poFieldDefn = poFDefn->GetFieldDefn( iField );
      if( poFieldDefn->GetType() == OFTInteger )
        printf( "%d,", poFeature->GetFieldAsInteger( iField ) );
      else if( poFieldDefn->GetType() == OFTInteger64 )
        printf( CPL_FRMT_GIB ",", poFeature->GetFieldAsInteger64( iField ) );
      else if( poFieldDefn->GetType() == OFTReal )
        printf( "%.3f,", poFeature->GetFieldAsDouble(iField) );
      else if( poFieldDefn->GetType() == OFTString )
        printf( "%s,", poFeature->GetFieldAsString(iField) );
      else
        printf( "%s,", poFeature->GetFieldAsString(iField) );
    }
    OGRGeometry *poGeometry;
    poGeometry = poFeature->GetGeometryRef();
    if( poGeometry != NULL
          && wkbFlatten(poGeometry->getGeometryType()) == wkbPoint )
    {
      OGRPoint *poPoint = (OGRPoint *) poGeometry;
      printf( "%.3f,%3.f\n", poPoint->getX(), poPoint->getY() );
    }
    else
    {
      printf( "no point geometry\n" );
    }
    OGRFeature::DestroyFeature( poFeature );
  }
  GDALClose( poDS );
  return(1);
}
