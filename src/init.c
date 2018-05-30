#include <R.h>
#include <Rinternals.h>
#include <stdlib.h> // for NULL
#include <R_ext/Rdynload.h>

/* FIXME: 
   Check these declarations against the C/Fortran source code.
*/

/* .Call calls */
extern SEXP _vapour_raster_info_cpp(SEXP);
extern SEXP _vapour_raster_io_cpp(SEXP, SEXP, SEXP, SEXP);
extern SEXP _vapour_sds_info_cpp(SEXP);
extern SEXP _vapour_vapour_layer_names_cpp(SEXP, SEXP);
extern SEXP _vapour_vapour_projection_info_cpp(SEXP, SEXP, SEXP);
extern SEXP _vapour_vapour_read_attributes_cpp(SEXP, SEXP, SEXP);
extern SEXP _vapour_vapour_read_geometry_cpp(SEXP, SEXP, SEXP, SEXP, SEXP);

static const R_CallMethodDef CallEntries[] = {
    {"_vapour_raster_info_cpp",            (DL_FUNC) &_vapour_raster_info_cpp,            1},
    {"_vapour_raster_io_cpp",              (DL_FUNC) &_vapour_raster_io_cpp,              4},
    {"_vapour_sds_info_cpp",               (DL_FUNC) &_vapour_sds_info_cpp,               1},
    {"_vapour_vapour_layer_names_cpp",     (DL_FUNC) &_vapour_vapour_layer_names_cpp,     2},
    {"_vapour_vapour_projection_info_cpp", (DL_FUNC) &_vapour_vapour_projection_info_cpp, 3},
    {"_vapour_vapour_read_attributes_cpp", (DL_FUNC) &_vapour_vapour_read_attributes_cpp, 3},
    {"_vapour_vapour_read_geometry_cpp",   (DL_FUNC) &_vapour_vapour_read_geometry_cpp,   5},
    {NULL, NULL, 0}
};

void R_init_vapour(DllInfo *dll)
{
    R_registerRoutines(dll, NULL, CallEntries, NULL, NULL);
    R_useDynamicSymbols(dll, FALSE);
}
