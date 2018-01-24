#include <R.h>
#include <Rinternals.h>
#include <stdlib.h> // for NULL
#include <R_ext/Rdynload.h>

/* FIXME: 
   Check these declarations against the C/Fortran source code.
*/

/* .Call calls */
extern SEXP _vapour_gdal_info(SEXP);
extern SEXP _vapour_raster_info(SEXP);
extern SEXP _vapour_raster_io(SEXP, SEXP, SEXP);
extern SEXP _vapour_vapour_read_attributes(SEXP, SEXP, SEXP);
extern SEXP _vapour_vapour_read_extent(SEXP, SEXP, SEXP);
extern SEXP _vapour_vapour_read_geometry(SEXP, SEXP, SEXP);
extern SEXP _vapour_vapour_read_geometry_text(SEXP, SEXP, SEXP, SEXP);

static const R_CallMethodDef CallEntries[] = {
    {"_vapour_gdal_info",                 (DL_FUNC) &_vapour_gdal_info,                 1},
    {"_vapour_raster_info",               (DL_FUNC) &_vapour_raster_info,               1},
    {"_vapour_raster_io",                 (DL_FUNC) &_vapour_raster_io,                 3},
    {"_vapour_vapour_read_attributes",    (DL_FUNC) &_vapour_vapour_read_attributes,    3},
    {"_vapour_vapour_read_extent",        (DL_FUNC) &_vapour_vapour_read_extent,        3},
    {"_vapour_vapour_read_geometry",      (DL_FUNC) &_vapour_vapour_read_geometry,      3},
    {"_vapour_vapour_read_geometry_text", (DL_FUNC) &_vapour_vapour_read_geometry_text, 4},
    {NULL, NULL, 0}
};

void R_init_vapour(DllInfo *dll)
{
    R_registerRoutines(dll, NULL, CallEntries, NULL, NULL);
    R_useDynamicSymbols(dll, FALSE);
}
