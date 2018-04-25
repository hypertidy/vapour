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
extern SEXP _vapour_vapour_layer_names(SEXP, SEXP);
extern SEXP _vapour_vapour_read_attributes(SEXP, SEXP, SEXP);
extern SEXP _vapour_vapour_read_feature_what(SEXP, SEXP, SEXP, SEXP, SEXP);

static const R_CallMethodDef CallEntries[] = {
    {"_vapour_raster_info_cpp",          (DL_FUNC) &_vapour_raster_info_cpp,          1},
    {"_vapour_raster_io_cpp",            (DL_FUNC) &_vapour_raster_io_cpp,            4},
    {"_vapour_vapour_layer_names",       (DL_FUNC) &_vapour_vapour_layer_names,       2},
    {"_vapour_vapour_read_attributes",   (DL_FUNC) &_vapour_vapour_read_attributes,   3},
    {"_vapour_vapour_read_feature_what", (DL_FUNC) &_vapour_vapour_read_feature_what, 5},
    {NULL, NULL, 0}
};

void R_init_vapour(DllInfo *dll)
{
    R_registerRoutines(dll, NULL, CallEntries, NULL, NULL);
    R_useDynamicSymbols(dll, FALSE);
}
