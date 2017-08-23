#include <R.h>
#include <Rinternals.h>
#include <stdlib.h> // for NULL
#include <R_ext/Rdynload.h>

/* FIXME: 
   Check these declarations against the C/Fortran source code.
*/

/* .Call calls */
extern SEXP _vapour_rasterio(SEXP);
extern SEXP _vapour_sql_vapour(SEXP, SEXP, SEXP);
extern SEXP _vapour_to_bblob(SEXP, SEXP);
extern SEXP _vapour_to_binary(SEXP, SEXP);
extern SEXP _vapour_to_format(SEXP, SEXP, SEXP);
extern SEXP _vapour_vapour(SEXP, SEXP);

static const R_CallMethodDef CallEntries[] = {
    {"_vapour_rasterio",   (DL_FUNC) &_vapour_rasterio,   1},
    {"_vapour_sql_vapour", (DL_FUNC) &_vapour_sql_vapour, 3},
    {"_vapour_to_bblob",   (DL_FUNC) &_vapour_to_bblob,   2},
    {"_vapour_to_binary",  (DL_FUNC) &_vapour_to_binary,  2},
    {"_vapour_to_format",  (DL_FUNC) &_vapour_to_format,  3},
    {"_vapour_vapour",     (DL_FUNC) &_vapour_vapour,     2},
    {NULL, NULL, 0}
};

void R_init_vapour(DllInfo *dll)
{
    R_registerRoutines(dll, NULL, CallEntries, NULL, NULL);
    R_useDynamicSymbols(dll, FALSE);
}
