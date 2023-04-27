#include <ogrsf_frmts.h>
#include<iostream>
#include <ogr_recordbatch.h>

int main(int argc, char **argv) {
  GDALAllRegister();
  // dsn, layer
  // std::cout << argv[1] << std::endl;
  // std::cout << argv[2] << std::endl;
  GDALDataset * poDS = (GDALDataset*) GDALOpenEx( argv[1], GDAL_OF_VECTOR, NULL, NULL, NULL );
  if( poDS == NULL )
  {
    printf( "Open failed %s.\n", argv[1] );
    exit( 1 );
  }
  OGRLayer  *poLayer;
  
  poLayer = poDS->GetLayerByName( argv[2] );
  
  if (poLayer) {
  struct ArrowArrayStream stream;
  if( !poLayer->GetArrowStream(&stream, nullptr))
  {
    fprintf(stderr, "GetArrowStream() failed\n");
    exit(1);
  }
  struct ArrowSchema schema;
  if( stream.get_schema(&stream, &schema) == 0 )
  {
    // Do something useful
    schema.release(&schema);
  }
  while( true )
  {
    struct ArrowArray array;
    // Look for an error (get_next() returning a non-zero code), or
    // end of iteration (array.release == nullptr)
    //
    if( stream.get_next(&stream, &array) != 0 ||
        array.release == nullptr )
    {
      break;
    }
    // Do something useful
    array.release(&array);
  }
  stream.release(&stream);
  } else {
    std::cout << "getting layer '" << argv[2] << "' from dsn '" << argv[1] << "' failed" << std::endl;
  }
  
  GDALClose( poDS );
 return 1;   
}
