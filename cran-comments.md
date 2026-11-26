## vapour 0.20.0

This release fixes two build issues found while packaging vapour for
conda-forge.  It also fixes the rchk PROTECT issue listed under Additional issues,
and several cases where arguments that GDAL could not parse crashed the R session
instead of raising an error.

## Test environments

- local (Linux), R-release
- mac-builder (R-devel)
- win-builder (R-devel)

## Reverse dependencies

Both reverse dependencies (ceramic, ursa) were checked against this version
with `R CMD check`: no errors or warnings.

Thank you!
