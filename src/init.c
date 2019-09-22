#include <R.h>
#include <Rinternals.h>
#include <stdlib.h> // for NULL
#include <R_ext/Rdynload.h>

/* FIXME: 
   Check these declarations against the C/Fortran source code.
*/

/* .Call calls */
extern SEXP _vapour_find_feature_count_cpp(SEXP, SEXP, SEXP);
extern SEXP _vapour_proj_to_wkt_cpp(SEXP);
extern SEXP _vapour_raster_gcp_cpp(SEXP);
extern SEXP _vapour_raster_info_cpp(SEXP, SEXP);
extern SEXP _vapour_raster_io_cpp(SEXP, SEXP, SEXP, SEXP);
extern SEXP _vapour_sds_info_cpp(SEXP);
extern SEXP _vapour_vapour_all_drivers_cpp();
extern SEXP _vapour_vapour_driver_cpp(SEXP);
extern SEXP _vapour_vapour_gdal_version_cpp();
extern SEXP _vapour_vapour_layer_names_cpp(SEXP, SEXP);
extern SEXP _vapour_vapour_projection_info_cpp(SEXP, SEXP, SEXP);
extern SEXP _vapour_vapour_read_attributes_cpp(SEXP, SEXP, SEXP, SEXP, SEXP, SEXP);
extern SEXP _vapour_vapour_read_geometry_cpp(SEXP, SEXP, SEXP, SEXP, SEXP, SEXP, SEXP, SEXP);
extern SEXP _vapour_vapour_read_names_cpp(SEXP, SEXP, SEXP, SEXP, SEXP, SEXP);
extern SEXP _vapour_vapour_report_attributes_cpp(SEXP, SEXP, SEXP);
extern SEXP _vapour_VSI_list(SEXP);
extern SEXP _vapour_warp_memory_cpp(SEXP, SEXP, SEXP, SEXP, SEXP, SEXP);

static const R_CallMethodDef CallEntries[] = {
    {"_vapour_find_feature_count_cpp",       (DL_FUNC) &_vapour_find_feature_count_cpp,       3},
    {"_vapour_proj_to_wkt_cpp",              (DL_FUNC) &_vapour_proj_to_wkt_cpp,              1},
    {"_vapour_raster_gcp_cpp",               (DL_FUNC) &_vapour_raster_gcp_cpp,               1},
    {"_vapour_raster_info_cpp",              (DL_FUNC) &_vapour_raster_info_cpp,              2},
    {"_vapour_raster_io_cpp",                (DL_FUNC) &_vapour_raster_io_cpp,                4},
    {"_vapour_sds_info_cpp",                 (DL_FUNC) &_vapour_sds_info_cpp,                 1},
    {"_vapour_vapour_all_drivers_cpp",       (DL_FUNC) &_vapour_vapour_all_drivers_cpp,       0},
    {"_vapour_vapour_driver_cpp",            (DL_FUNC) &_vapour_vapour_driver_cpp,            1},
    {"_vapour_vapour_gdal_version_cpp",      (DL_FUNC) &_vapour_vapour_gdal_version_cpp,      0},
    {"_vapour_vapour_layer_names_cpp",       (DL_FUNC) &_vapour_vapour_layer_names_cpp,       2},
    {"_vapour_vapour_projection_info_cpp",   (DL_FUNC) &_vapour_vapour_projection_info_cpp,   3},
    {"_vapour_vapour_read_attributes_cpp",   (DL_FUNC) &_vapour_vapour_read_attributes_cpp,   6},
    {"_vapour_vapour_read_geometry_cpp",     (DL_FUNC) &_vapour_vapour_read_geometry_cpp,     8},
    {"_vapour_vapour_read_names_cpp",        (DL_FUNC) &_vapour_vapour_read_names_cpp,        6},
    {"_vapour_vapour_report_attributes_cpp", (DL_FUNC) &_vapour_vapour_report_attributes_cpp, 3},
    {"_vapour_VSI_list",                     (DL_FUNC) &_vapour_VSI_list,                     1},
    {"_vapour_warp_memory_cpp",              (DL_FUNC) &_vapour_warp_memory_cpp,              6},
    {NULL, NULL, 0}
};

void R_init_vapour(DllInfo *dll)
{
    R_registerRoutines(dll, NULL, CallEntries, NULL, NULL);
    R_useDynamicSymbols(dll, FALSE);
}
