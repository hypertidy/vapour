#ifndef GDALVECTORSTREAM_H
#define GDALVECTORSTREAM_H

// WIP https://gdal.org/development/rfc/rfc86_column_oriented_api.html
// written entirely by Dewey Dunnington and the GDAL project
// https://github.com/paleolimbot/sf/tree/stream-reading

#include <Rcpp.h>

#include <ogrsf_frmts.h>
#if (GDAL_VERSION_MAJOR >= 3 && GDAL_VERSION_MINOR >= 6)

#include <ogr_recordbatch.h>


class GDALStreamWrapper {
public:
  static void Make(struct ArrowArrayStream* stream, Rcpp::List shelter,
                   struct ArrowArrayStream* stream_out) {
                     stream_out->get_schema = &get_schema_wrap;
    stream_out->get_next = &get_next_wrap;
    stream_out->get_last_error = &get_last_error_wrap;
    stream_out->release = &release_wrap;
    stream_out->private_data = new GDALStreamWrapper(stream, shelter);
  }
  
  ~GDALStreamWrapper() {
    stream_.release(&stream_);
    GDALDataset* poDS = (GDALDataset*)R_ExternalPtrAddr(shelter_[0]);
    GDALClose(poDS);
    R_SetExternalPtrAddr(shelter_[0], nullptr);
  }
  
private:
  // The parent stream as returned from GDAL
  struct ArrowArrayStream stream_;
  Rcpp::List shelter_;
  
  GDALStreamWrapper(struct ArrowArrayStream* stream, Rcpp::List shelter):
    shelter_(shelter) {
    memcpy(&stream_, stream, sizeof(struct ArrowArrayStream));
    stream->release = nullptr;
  }
  
  int get_schema(struct ArrowSchema* out) {
    return stream_.get_schema(&stream_, out);
  }
  
  int get_next(struct ArrowArray* out) {
    return stream_.get_next(&stream_, out);
  }
  
  const char* get_last_error() {
    return stream_.get_last_error(&stream_);
  }
  
  static int get_schema_wrap(struct ArrowArrayStream* stream, struct ArrowSchema* out) {
    return reinterpret_cast<GDALStreamWrapper*>(stream->private_data)->get_schema(out);
  }
  
  static int get_next_wrap(struct ArrowArrayStream* stream, struct ArrowArray* out) {
    return reinterpret_cast<GDALStreamWrapper*>(stream->private_data)->get_next(out);
  }
  
  static const char* get_last_error_wrap(struct ArrowArrayStream* stream) {
    return reinterpret_cast<GDALStreamWrapper*>(stream->private_data)->get_last_error();
  }
  
  static void release_wrap(struct ArrowArrayStream* stream) {
    delete reinterpret_cast<GDALStreamWrapper*>(stream->private_data);
    stream->release = nullptr;
  }
};

#endif

namespace gdalvectorstream {
using namespace Rcpp; 

#if (GDAL_VERSION_MAJOR >= 3 && GDAL_VERSION_MINOR >= 6)






static void finalize_dataset_xptr(SEXP dataset_xptr) {
  GDALDataset *poDS = (GDALDataset*)R_ExternalPtrAddr(dataset_xptr);
  if (poDS != nullptr) {
    GDALClose(poDS);
  }
}

inline Rcpp::List ogr_layer_setup(Rcpp::CharacterVector datasource, Rcpp::CharacterVector layer,
                               Rcpp::CharacterVector query,
                               //Rcpp::CharacterVector options, 
                               bool quiet, 
                               //Rcpp::CharacterVector drivers,
                               Rcpp::CharacterVector wkt_filter,
                               bool dsn_exists,
                               bool dsn_isdb,
                               int width) {
  // adapted from the OGR tutorial @ www.gdal.org
  //std::vector <char *> open_options = create_options(options, quiet);
  //std::vector <char *> drivers_v = create_options(drivers, quiet);
  GDALDataset *poDS;
  poDS = (GDALDataset *) GDALOpenEx( datasource[0], GDAL_OF_VECTOR | GDAL_OF_READONLY,
          // drivers.size() ? drivers_v.data() : NULL, open_options.data(), NULL );
          NULL, NULL, NULL );
  if( poDS == NULL ) {
    // could not open dsn
    if( dsn_isdb ) {
      Rcpp::stop("Cannot open %s; Check connection parameters.", datasource);
    }
    if( dsn_exists ) {
      Rcpp::stop("Cannot open %s; %s %s",
                 datasource,
                 "The source could be corrupt or not supported.",
                 "See `st_drivers()` for a list of supported formats.");
    }
    Rcpp::stop("Cannot open %s; The file doesn't seem to exist.", datasource);
  }
  
  // Will close the dataset if some early return/exception prevents GDALClose() from being
  // called/allows the result to be accessed by the caller.
  Rcpp::RObject dataset_xptr = R_MakeExternalPtr(poDS, R_NilValue, R_NilValue);
  R_RegisterCFinalizer(dataset_xptr, &finalize_dataset_xptr);
  
  if (layer.size() == 0 && Rcpp::CharacterVector::is_na(query[0])) { // no layer specified
    switch (poDS->GetLayerCount()) {
    case 0: { // error:
    Rcpp::stop("No layers in datasource.");
  }
    case 1: { // silent:
      OGRLayer *poLayer = poDS->GetLayer(0);
      layer = Rcpp::CharacterVector::create(poLayer->GetName());
      break;
    }
    default: { // select first layer: message + warning:
      OGRLayer *poLayer = poDS->GetLayer(0);
      layer = Rcpp::CharacterVector::create(poLayer->GetName());
      if (! quiet) { // #nocov start
        Rcpp::Rcout << "Multiple layers are present in data source " << datasource[0] << ", ";
        Rcpp::Rcout << "reading layer `" << layer[0] << "'." << std::endl;
        Rcpp::Rcout << "Use `st_layers' to list all layer names and their type in a data source." << std::endl;
        Rcpp::Rcout << "Set the `layer' argument in `st_read' to read a particular layer." << std::endl;
      } // #nocov end
      Rcpp::Function warning("warning");
      warning("automatically selected the first layer in a data source containing more than one.");
    }
    }
  }
  
  OGRLayer *poLayer;
  if (! Rcpp::CharacterVector::is_na(query[0])) {
    poLayer = poDS->ExecuteSQL(query[0], NULL, NULL);
    if (poLayer == NULL)
      Rcpp::stop("Query execution failed, cannot open layer.\n"); // #nocov
    if (layer.size())
      Rcpp::warning("argument layer is ignored when query is specified\n"); // #nocov
  } else
    poLayer = 	poDS->GetLayerByName(layer[0]);
  if (poLayer == NULL) {
    Rcpp::Rcout << "Cannot open layer " << layer[0] << std::endl;
    Rcpp::stop("Opening layer failed.\n");
  }
  
  // set spatial filter?
  if (wkt_filter.size()) {
    char *wkt = wkt_filter[0];
    OGRGeometry *new_geom;
#if GDAL_VERSION_MAJOR <= 2 && GDAL_VERSION_MINOR <= 2
    OGRErr err = OGRGeometryFactory::createFromWkt(&wkt, poLayer->GetSpatialRef(), &new_geom);
#else
    OGRErr err = OGRGeometryFactory::createFromWkt((const char *) wkt, poLayer->GetSpatialRef(), &new_geom);
#endif
    if (err != OGRERR_NONE) {
      Rcpp::Rcout << "Cannot create geometry from: " << wkt_filter[0] << std::endl;
      Rcpp::stop("wkt parse error.\n");
    }
    poLayer->SetSpatialFilter(new_geom);
    OGRGeometryFactory::destroyGeometry(new_geom);
  }
  
  if (! quiet) {
    if (! Rcpp::CharacterVector::is_na(query[0]))
      Rcpp::Rcout << "Reading query `" << query[0] << "'" << std::endl << "from data source ";
    else
      Rcpp::Rcout << "Reading layer `" << layer[0] << "' from data source ";
    // if (LENGTH(datasource[0]) > (width - (34 + LENGTH(layer[0]))))
    Rcpp::String ds(datasource(0));
    if (layer.size()) {
      Rcpp::String la(layer(0));
      if (strlen(ds.get_cstring()) > (width - (34 + strlen(la.get_cstring()))))
        Rcpp::Rcout << std::endl << "  ";
    }
    Rcpp::Rcout << "`" << datasource[0] << "' ";
    if (((int) strlen(ds.get_cstring())) > (width - 25))
      Rcpp::Rcout << std::endl << "  ";
    Rcpp::Rcout << "using driver `" << poDS->GetDriverName() << "'" << std::endl;                       // #nocov
  }
  
  // Keeps the dataset external pointer alive as long as the layer external pointer is alive
  Rcpp::RObject layer_xptr = R_MakeExternalPtr(poLayer, R_NilValue, dataset_xptr);
  
  return Rcpp::List::create(dataset_xptr, layer_xptr);
}


// inline Rcpp::List read_ogr(Rcpp::CharacterVector datasource, Rcpp::CharacterVector layer,
//                         Rcpp::CharacterVector query,
//                         Rcpp::CharacterVector options, bool quiet, Rcpp::NumericVector toTypeUser,
//                         Rcpp::CharacterVector fid_column_name, Rcpp::CharacterVector drivers,
//                         Rcpp::CharacterVector wkt_filter,
//                         bool promote_to_multi = true, bool int64_as_string = false,
//                         bool dsn_exists = true,
//                         bool dsn_isdb = false,
//                         int width = 80) {
//   Rcpp::List prep = ogr_layer_setup(datasource, layer, query, 
//                                         //options,
//                                         quiet,  
//                                         //drivers,
//                                         wkt_filter,
//                                         dsn_exists, dsn_isdb, width);
//   OGRDataSource* poDS = (OGRDataSource*)(R_ExternalPtrAddr(prep[0]));
//   OGRLayer* poLayer = (OGRLayer*)R_ExternalPtrAddr(prep[1]);
//   
//   Rcpp::List out; // = sf_from_ogrlayer(poLayer, quiet, int64_as_string, toTypeUser, fid_column_name,
//   //                promote_to_multi);
//   
//   // clean up if SQL was used https://www.gdal.org/classGDALDataset.html#ab2c2b105b8f76a279e6a53b9b4a182e0
//   if (! Rcpp::CharacterVector::is_na(query[0]))
//     poDS->ReleaseResultSet(poLayer);
//   
//   GDALClose(poDS);
//   R_SetExternalPtrAddr(prep[0], nullptr);
//   return out;
// }

inline Rcpp::List read_gdal_stream(
    Rcpp::RObject stream_xptr,
    Rcpp::CharacterVector datasource, 
    Rcpp::CharacterVector layer,
    Rcpp::CharacterVector query,
    Rcpp::CharacterVector options, 
    bool quiet, 
    Rcpp::CharacterVector drivers,
    Rcpp::CharacterVector wkt_filter,
    bool dsn_exists,
    bool dsn_isdb,
    Rcpp::CharacterVector fid_column,
    int width) {
  
  const char* array_stream_options[] = {"INCLUDE_FID=NO", nullptr};
  if (fid_column.size() == 1) {
    array_stream_options[0] = "INCLUDE_FID=YES";
  }
  
  Rcpp::List prep = ogr_layer_setup(datasource, layer, query, 
                                        //options,
                                        quiet,  
                                        //drivers,
                                        wkt_filter,
                                        dsn_exists, dsn_isdb, width);
  OGRDataSource* poDS = (OGRDataSource*)(R_ExternalPtrAddr(prep[0]));
  OGRLayer* poLayer = (OGRLayer*)R_ExternalPtrAddr(prep[1]);
  auto stream_out = reinterpret_cast<struct ArrowArrayStream*>(
    R_ExternalPtrAddr(stream_xptr));
  
  OGRSpatialReference* crs = poLayer->GetSpatialRef();
  char* wkt_out;
  std::string wkt_str; 
  if (crs) {
    crs->exportToWkt(&wkt_out);
    wkt_str = "yaba"; 
  } else {
    wkt_str = ""; 
  }
  CPLFree(wkt_out);
  
  struct ArrowArrayStream stream_temp;
  if (!poLayer->GetArrowStream(&stream_temp, array_stream_options)) {
    Rcpp::stop("Failed to open ArrayStream from Layer");
  }
  
  GDALStreamWrapper::Make(&stream_temp, prep, stream_out);
  
    // clean up if SQL was used https://www.gdal.org/classGDALDataset.html#ab2c2b105b8f76a279e6a53b9b4a182e0
    if (! Rcpp::CharacterVector::is_na(query[0]))
      poDS->ReleaseResultSet(poLayer);

    GDALClose(poDS);
    R_SetExternalPtrAddr(prep[0], nullptr);

  // The reported feature count is incorrect if there is a query
  double num_features;
  if (query.size() == 0) {
    num_features = (double) poLayer->GetFeatureCount(false);
  } else {
    num_features = -1;
  }
  
  return Rcpp::List::create(wkt_str, Rcpp::NumericVector::create(num_features));
}

#else

inline Rcpp::List read_gdal_stream(
    Rcpp::RObject stream_xptr,
    Rcpp::CharacterVector datasource, 
    Rcpp::CharacterVector layer,
    Rcpp::CharacterVector query,
    Rcpp::CharacterVector options, 
    bool quiet, 
    Rcpp::CharacterVector drivers,
    Rcpp::CharacterVector wkt_filter,
    bool dsn_exists,
    bool dsn_isdb,
    Rcpp::CharacterVector fid_column,
    int width) {
  Rcpp::stop("read_stream() requires GDAL >= 3.6");
}

#endif

} // gdalvectorstream

#endif
