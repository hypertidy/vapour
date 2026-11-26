
## vapour 0.20.0

This release fixes two build issues found while packaging vapour for
conda-forge: `configure` now passes `LDFLAGS` to its GDAL/PROJ test-program
links, and a global `using namespace std;` was removed from an internal header
(it made `list` ambiguous under the MSVC STL). It also validates the bundled
PROJ data at load and scopes the PROJ/GDAL search paths to GDAL's contexts
rather than leaking environment variables into other packages.

It also fixes the rchk PROTECT issue listed under Additional issues, and
several cases where arguments that GDAL could not parse crashed the R session
instead of raising an error.

## Test environments

- local (Linux), R-release
- mac-builder (R-devel)
- win-builder (R-devel)

## R CMD check results

0 errors | 0 warnings | 0 notes

## Notes

The installed size (~10 Mb) comes from the compiled library and the sample
geospatial data in inst/extdata used by examples, tests and vignettes.

## Reverse dependencies

Both reverse dependencies (ceramic, ursa) were checked against this version
with `R CMD check`: no errors or warnings.

Thank you!
