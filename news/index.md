# Changelog

## vapour (development version)

This is a robustness release: no function is removed and no return value
changes shape. What changes is how problems are reported, and a number
of inputs that crashed R are now errors. Six functions are added, to
read fields and feature ids for chosen features.

### GDAL messages are R conditions

- Errors and warnings from GDAL are no longer written to stderr, where R
  could not catch them and some front ends did not show them. They are
  collected during each call and then signalled in R:

  - when the call fails, GDAL’s reason is part of the error message, for
    example
    `cannot open 'nofile.gpkg': nofile.gpkg: No such file or directory`;
  - when the call returns, each GDAL warning is an R warning that starts
    `GDAL warning <n>:`, and each GDAL failure that did not stop the
    call is an R warning that starts `GDAL error <n>:`.

  So [`suppressWarnings()`](https://rdrr.io/r/base/warning.html),
  [`tryCatch()`](https://rdrr.io/r/base/conditions.html) and
  `options(warn = 2)` now apply to GDAL’s messages. Code that was quiet
  at the console because the text went to stderr may now show warnings:
  they are the same messages. Output from `CPL_DEBUG` is still printed
  as it happens.

- Messages raised on GDAL’s worker threads (a warp with `-multi`, or
  `GDAL_NUM_THREADS`) are collected as well, with GDAL 3.7 or later.

- The wording of some errors has changed with this. A vector source that
  cannot be opened is `cannot open '<dsn>': <reason>` (it was
  `Open failed.`), and a raster source is
  `cannot open dataset '<dsn>': <reason>` or
  `GDAL was unable to open '<dsn>': <reason>`. A warp source that cannot
  be read is an error that names it, where it was a line of printed text
  followed by an empty error.

### Open options

- Every function that reads a source gains `open_options`, a character
  vector of `"NAME=VALUE"` strings passed to the driver when the source
  is opened, as `-oo` in the GDAL command line tools
  ([\#224](https://github.com/hypertidy/vapour/issues/224)). This covers
  the raster readers
  ([`vapour_raster_info()`](https://hypertidy.github.io/vapour/reference/vapour_raster_info.md),
  [`vapour_raster_gcp()`](https://hypertidy.github.io/vapour/reference/vapour_raster_gcp.md),
  [`vapour_sds()`](https://hypertidy.github.io/vapour/reference/vapour_sds.md),
  [`vapour_sds_names()`](https://hypertidy.github.io/vapour/reference/vapour_sds_names.md),
  [`vapour_geolocation()`](https://hypertidy.github.io/vapour/reference/vapour_geolocation.md),
  [`vapour_read_raster()`](https://hypertidy.github.io/vapour/reference/vapour_read_raster.md)
  and its typed forms,
  [`vapour_read_raster_block()`](https://hypertidy.github.io/vapour/reference/vapour_read_raster_block.md),
  [`vapour_vrt()`](https://hypertidy.github.io/vapour/reference/vapour_vrt.md),
  the `gdal_raster_*()` functions) and the vector readers
  ([`vapour_layer_names()`](https://hypertidy.github.io/vapour/reference/vapour_layer_names.md),
  [`vapour_layer_info()`](https://hypertidy.github.io/vapour/reference/vapour_layer_info.md),
  [`vapour_layer_extent()`](https://hypertidy.github.io/vapour/reference/vapour_layer_extent.md),
  [`vapour_driver()`](https://hypertidy.github.io/vapour/reference/GDAL-library.md),
  [`vapour_geom_name()`](https://hypertidy.github.io/vapour/reference/vapour_geom_name.md),
  [`vapour_geom_summary()`](https://hypertidy.github.io/vapour/reference/vapour_geom_summary.md),
  [`vapour_report_fields()`](https://hypertidy.github.io/vapour/reference/vapour_report_fields.md),
  [`vapour_read_fields()`](https://hypertidy.github.io/vapour/reference/vapour_read_fields.md),
  [`vapour_read_fids()`](https://hypertidy.github.io/vapour/reference/vapour_read_fids.md),
  [`vapour_read_geometry()`](https://hypertidy.github.io/vapour/reference/vapour_read_geometry.md)
  and its variants,
  [`vapour_read_extent()`](https://hypertidy.github.io/vapour/reference/vapour_read_geometry.md),
  [`vapour_feature_extent()`](https://hypertidy.github.io/vapour/reference/vapour_read_geometry.md),
  [`vapour_read_type()`](https://hypertidy.github.io/vapour/reference/vapour_read_geometry.md)).
  [`vapour_vrt()`](https://hypertidy.github.io/vapour/reference/vapour_vrt.md)
  records the options in the VRT, so they apply again when the VRT is
  opened.

- The `open_options` argument of
  [`vapour_warp_raster()`](https://hypertidy.github.io/vapour/reference/vapour_warp_raster.md)
  now has an effect, as do `"-oo"` pairs given in `options` to
  [`vapour_warp_raster()`](https://hypertidy.github.io/vapour/reference/vapour_warp_raster.md)
  and the `gdal_raster_*()` functions. They were accepted and ignored,
  because vapour opens the sources itself.

### Reading chosen features

- Fields and feature ids can now be read for chosen features, as
  geometry already could. The new functions follow
  [`vapour_read_geometry_ia()`](https://hypertidy.github.io/vapour/reference/vapour_read_geometry.md)
  and
  [`vapour_read_geometry_ij()`](https://hypertidy.github.io/vapour/reference/vapour_read_geometry.md):

  - [`vapour_read_fields_ia()`](https://hypertidy.github.io/vapour/reference/vapour_read_fields.md),
    [`vapour_read_fids_ia()`](https://hypertidy.github.io/vapour/reference/vapour_read_fids.md):
    by position, counted from 0, in any order and with repeats;
  - [`vapour_read_fields_ij()`](https://hypertidy.github.io/vapour/reference/vapour_read_fields.md),
    [`vapour_read_fids_ij()`](https://hypertidy.github.io/vapour/reference/vapour_read_fids.md):
    by an inclusive range of positions;
  - [`vapour_read_geometry_fa()`](https://hypertidy.github.io/vapour/reference/vapour_read_geometry.md),
    [`vapour_read_fields_fa()`](https://hypertidy.github.io/vapour/reference/vapour_read_fields.md):
    by feature id, the values
    [`vapour_read_fids()`](https://hypertidy.github.io/vapour/reference/vapour_read_fids.md)
    returns. This is direct access where the driver supports it.

  A position or feature id that is not in the layer gives `NA` (`NULL`
  for geometry).

- [`vapour_read_geometry_ia()`](https://hypertidy.github.io/vapour/reference/vapour_read_geometry.md)
  now reads `ia` in any order and with repeats, as documented. Only
  positions in ascending order without repeats were read, and the rest
  came back empty.

### Crashes fixed

- [`vapour_geolocation()`](https://hypertidy.github.io/vapour/reference/vapour_geolocation.md)
  crashed R for a source that could not be opened, and for a source with
  geolocation metadata in which any item was not set. An item that is
  not set is now `NA`.

- [`vapour_vrt()`](https://hypertidy.github.io/vapour/reference/vapour_vrt.md)
  kept the source open, once for every call that changed anything about
  the source (`extent`, `projection`, `bands`, `geolocation`,
  `overview`, `options`, or a subdataset). A long loop ran out of file
  handles.

- [`vapour_read_fields()`](https://hypertidy.github.io/vapour/reference/vapour_read_fields.md)
  and
  [`vapour_read_attributes()`](https://hypertidy.github.io/vapour/reference/vapour_read_fields.md)
  failed with `Invalid input type, expected 'integer' actual 'logical'`
  for any layer with a boolean field. Boolean fields are read as logical
  vectors.

- A read that failed inside GDAL returned a vector of zeros after
  printing `we have a problem at RasterIO`. It is now an error with
  GDAL’s reason. A block read outside the raster is the common case.

- The size of a raster read is computed without integer overflow. A
  request for more than 2^31 cells could write outside its buffer.

- Internal entry points with no check on the open, the driver, the band
  or the size of the data no longer crash: writing a block with more
  values than cells, a create-copy with an unknown driver, a feature
  read by a FID that is not there, and index reads past the end of the
  index.

- Datasets, features, SQL result sets and option structures are now held
  in owning handles in the C++ code, so an error, a warning under
  `options(warn = 2)` or a failed allocation part way through a call no
  longer leaves them open. All sources are opened in one place.

### Other fixes

- [`vapour_raster_info()`](https://hypertidy.github.io/vapour/reference/vapour_raster_info.md)
  now answers for the subdataset when `sds` is given. `sds` was ignored
  and the container was described.
  [`vapour_read_raster()`](https://hypertidy.github.io/vapour/reference/vapour_read_raster.md)
  checks its `window` against the subdataset it reads for the same
  reason.

- `vapour_raster_info(min_max = TRUE)` computes the minimum and maximum.
  It reported them only when the source already recorded them.

- [`vapour_warp_raster()`](https://hypertidy.github.io/vapour/reference/vapour_warp_raster.md):
  `transformation_options` no longer discards `warp_options` and
  `options`, and `source_extent` is now applied. It was dropped, so the
  source kept its own extent.

- `vapour_vrt(nomd = TRUE)` removes all of the dataset and band metadata
  from the VRT. Some was left, `AREA_OR_POINT` for a GeoTIFF.

- [`vapour_read_fields()`](https://hypertidy.github.io/vapour/reference/vapour_read_fields.md)
  gives `NA` for a feature that was counted and then not read, with the
  existing warning `not as many features as requested`. Those values
  were whatever was in memory. This happens when a driver counts by
  bounding box and then filters by geometry, as GeoPackage does for
  `sql` with `extent`.

- [`vapour_layer_extent()`](https://hypertidy.github.io/vapour/reference/vapour_layer_extent.md)
  and
  [`vapour_layer_info()`](https://hypertidy.github.io/vapour/reference/vapour_layer_info.md)
  give an extent of `NA` for a layer that has none (no geometry column,
  or no features), without printing `problem in get extent`.

- [`buildvrt()`](https://hypertidy.github.io/vapour/reference/buildvrt.md)
  no longer reads its source names and options through pointers to
  strings that had been freed.

## vapour 0.20.0

CRAN release: 2026-11-26

- New
  [`vapour_feature_extent()`](https://hypertidy.github.io/vapour/reference/vapour_read_geometry.md)
  returns the extent of each feature as a matrix with columns `xmin`,
  `xmax`, `ymin`, `ymax`, one row for each feature and a row of `NA` for
  a missing geometry.
  [`vapour_read_extent()`](https://hypertidy.github.io/vapour/reference/vapour_read_geometry.md)
  is unchanged and still returns a list of four-number vectors
  ([\#142](https://github.com/hypertidy/vapour/issues/142)).

- Integer reads no longer turn every value at or below the nodata value
  into `NA`, only the nodata value itself. This affected
  [`vapour_read_raster()`](https://hypertidy.github.io/vapour/reference/vapour_read_raster.md)
  and
  [`vapour_warp_raster()`](https://hypertidy.github.io/vapour/reference/vapour_warp_raster.md)
  on integer sources: with a nodata value of 0, all negative values were
  lost ([\#219](https://github.com/hypertidy/vapour/issues/219)).

- [`gdal_raster_data()`](https://hypertidy.github.io/vapour/reference/gdal_raster_data.md)
  and
  [`vapour_warp_raster()`](https://hypertidy.github.io/vapour/reference/vapour_warp_raster.md)
  gain `unscale = TRUE`, as
  [`vapour_read_raster()`](https://hypertidy.github.io/vapour/reference/vapour_read_raster.md)
  and
  [`vapour_read_raster_block()`](https://hypertidy.github.io/vapour/reference/vapour_read_raster_block.md)
  already have. Set it to `FALSE` to get the stored values rather than
  values converted by the band’s scale and offset
  ([\#226](https://github.com/hypertidy/vapour/issues/226)).

- An integer `band_output_type` now gives the stored values in that
  type. It was ignored for a band with a scale, which came back as
  scaled doubles, so
  [`vapour_read_raster_int()`](https://hypertidy.github.io/vapour/reference/vapour_read_raster_raw.md)
  and
  [`vapour_warp_raster_int()`](https://hypertidy.github.io/vapour/reference/vapour_warp_raster_raw.md)
  did not return integers for such a source
  ([\#226](https://github.com/hypertidy/vapour/issues/226)).

- A band offset is now applied when the band has no scale. It was
  ignored.

- Band arguments follow one rule
  ([\#150](https://github.com/hypertidy/vapour/issues/150)): `NULL`
  means all bands, otherwise band numbers counted from 1, which may be
  repeated and in any order. A value less than 1, a missing value or a
  fraction is an error.

  - [`vapour_read_raster()`](https://hypertidy.github.io/vapour/reference/vapour_read_raster.md)
    accepts `band = NULL`, and reads several bands with one call to GDAL
    rather than opening the source for each.
  - [`vapour_warp_raster()`](https://hypertidy.github.io/vapour/reference/vapour_warp_raster.md)
    with the default `bands = NULL` returns all bands, as documented. It
    returned only the first.
  - [`gdal_raster_data()`](https://hypertidy.github.io/vapour/reference/gdal_raster_data.md)
    with a `bands` value less than 1 still returns all bands, with a
    warning to use `bands = NULL`.

- New
  [`vapour_sds()`](https://hypertidy.github.io/vapour/reference/vapour_sds.md)
  returns a data frame of the subdatasets of one or more sources, with
  GDAL’s description of each. It is the vectorised form of
  [`vapour_sds_names()`](https://hypertidy.github.io/vapour/reference/vapour_sds_names.md)
  ([\#133](https://github.com/hypertidy/vapour/issues/133)).

- [`vapour_layer_info()`](https://hypertidy.github.io/vapour/reference/vapour_layer_info.md)
  now reports the EPSG code in `projection$EPSG` when the projection
  carries one or GDAL can identify one for it, and `NA` otherwise. It
  was always 0
  ([\#234](https://github.com/hypertidy/vapour/issues/234)).

- [`vapour_layer_info()`](https://hypertidy.github.io/vapour/reference/vapour_layer_info.md)
  gains `filelist`, the files that make up the data source
  ([\#120](https://github.com/hypertidy/vapour/issues/120)).

- [`vapour_all_drivers()`](https://hypertidy.github.io/vapour/reference/GDAL-library.md)
  gains `sql_dialects`, the SQL dialects each vector driver supports
  ([\#175](https://github.com/hypertidy/vapour/issues/175)).

- [`vapour_raster_info()`](https://hypertidy.github.io/vapour/reference/vapour_raster_info.md)
  gains `image_structure`, the dataset’s IMAGE_STRUCTURE metadata such
  as compression and interleaving
  ([\#113](https://github.com/hypertidy/vapour/issues/113)).

- [`vapour_raster_info()`](https://hypertidy.github.io/vapour/reference/vapour_raster_info.md)
  on a source with no geotransform now reports GDAL’s default
  `c(0, 1, 0, 0, 0, 1)`. It reported a transform with the y axis
  flipped, which the source does not have
  ([\#225](https://github.com/hypertidy/vapour/issues/225)).

- An invalid `extent` filter now warns once rather than once for each
  time the layer is opened internally
  ([\#178](https://github.com/hypertidy/vapour/issues/178)).

- [`vapour_layer_info()`](https://hypertidy.github.io/vapour/reference/vapour_layer_info.md)
  no longer warns “unable export SRS to XML”, or lets GDAL print MapInfo
  projection errors, for a projection those formats cannot express.
  Neither format is part of what it returns.

- Arguments that GDAL cannot parse are now an R error rather than a
  crash. An unknown `resample` or a malformed `options` value in
  [`gdal_raster_data()`](https://hypertidy.github.io/vapour/reference/gdal_raster_data.md),
  [`gdal_raster_dsn()`](https://hypertidy.github.io/vapour/reference/gdal_raster_data.md),
  [`gdal_raster_image()`](https://hypertidy.github.io/vapour/reference/gdal_raster_data.md),
  [`gdal_raster_nara()`](https://hypertidy.github.io/vapour/reference/gdal_raster_data.md)
  or
  [`vapour_warp_raster()`](https://hypertidy.github.io/vapour/reference/vapour_warp_raster.md)
  took down the R session, because the warp options were used without
  checking that GDAL had created them
  ([\#231](https://github.com/hypertidy/vapour/issues/231)).

- [`buildvrt()`](https://hypertidy.github.io/vapour/reference/buildvrt.md)
  on a source that cannot be opened, and
  [`vapour_vrt()`](https://hypertidy.github.io/vapour/reference/vapour_vrt.md)
  with `geolocation` on a source that cannot be opened, no longer crash.
  The first is now an error and the second returns `NA` like the other
  [`vapour_vrt()`](https://hypertidy.github.io/vapour/reference/vapour_vrt.md)
  failures.

- [`vapour_read_raster()`](https://hypertidy.github.io/vapour/reference/vapour_read_raster.md)
  reads more than one band again. A `band` vector longer than one was an
  error under R 4.3.0 and later
  ([\#203](https://github.com/hypertidy/vapour/issues/203)).

- Fixed the PROTECT imbalance reported by rchk in the nativeRaster
  conversion ([\#239](https://github.com/hypertidy/vapour/issues/239)).

- The Int8, Int64 and UInt64 band types are now gated on the GDAL
  version number rather than on separate major and minor tests, which
  would have dropped them under GDAL 4.0.

- configure now passes `LDFLAGS` when linking its GDAL and PROJ test
  programs, so builds against a library prefix off the default runtime
  search path (such as conda-forge, which supplies the rpath in
  `LDFLAGS`) no longer fail with “PROJ not available?”. Reported by
  Matthew Feickert while packaging vapour for conda-forge.

- Removed a global `using namespace std;` from the gdalwarpgeneral
  header, which made `list` ambiguous between `cpp11::list` and
  `std::list` under the MSVC STL (Matthew Feickert).

- The bundled copy of PROJ data (inst/proj, present on Windows and macOS
  CRAN binaries and on Linux only with
  `--configure-args='--with-data-copy=yes'`) is now validated at load.
  PROJ checks proj.db’s database layout version lazily, so a copy taken
  from one PROJ version silently breaks when the linked libproj is a
  different version. vapour now points GDAL at the bundled copy, forces
  a database lookup, and if that fails reverts to the PROJ defaults with
  a startup message naming the PROJ that built the bundled proj.db and
  its layout version. Prompted by CRAN’s note on the ICESat2VegR failure
  with PROJ 9.9.0.

- The PROJ search path is now set with `OSRSetPROJSearchPaths()` rather
  than the `PROJ_LIB` environment variable, so it is scoped to GDAL’s
  PROJ contexts and no longer leaks into other packages’ libproj.

- The bundled copy of GDAL data (inst/gdal, present under the same
  conditions as the PROJ copy) is now activated with the `GDAL_DATA`
  configuration option rather than the environment variable, for the
  same reason. It is set before the drivers are registered, because GDAL
  builds its data file search path once during registration and ignores
  `GDAL_DATA` after that. On Windows the bundled copy is the only one
  GDAL can find, and drivers that need a data file (OSM and its
  osmconf.ini) fail without it.

- The gate for restoring `PROJ_LIB` on unload was stale: `proj/alaska`
  was removed in PROJ 7, so the variable was never restored. The PROJ
  gate is now `proj/proj.db`.

- New
  [`vapour_proj_data()`](https://hypertidy.github.io/vapour/reference/vapour_proj_data.md)
  reports the PROJ search paths in use, whether the bundled copy is
  active, and the metadata of the proj.db that resolves.

## vapour 0.16.0

CRAN release: 2026-03-14

- Converted entire C++ backend from Rcpp to cpp11. vapour is now
  header-only with no ABI dependency on Rcpp. This is a major internal
  refactor with no changes to the R API.

- Introduced `with_ogr_layer()` template helper in gdallibrary.h,
  eliminating duplicated open/layer/cleanup/close boilerplate across
  gdalgeometry.h (22 instances removed) and gdallibrary.h.

- Replaced `CollectorList` (Rcpp-based growing list from fs) with a thin
  wrapper around cpp11’s efficiently-growing `writable::list`.

- Fixed brace-matching bug in `gdal_projection_info()` where SQL result
  release and dataset close were inside the `hSRS != nullptr` branch,
  leaking the dataset handle for sources without a CRS.

- Fixed dangling pointer in `gdal_vrt_text()` which returned
  `const char*` from a local `CharacterVector` — now returns
  `std::string`.

- Tightened `validate_limit_n()` to accept 0 as equivalent to NULL (no
  limit), and ensure integer type for all arguments passed to C++ (cpp11
  does not perform implicit type coercion unlike Rcpp).

- Added [`as.integer()`](https://rdrr.io/r/base/integer.html) and
  [`as.numeric()`](https://rdrr.io/r/base/numeric.html) coercion at the
  R level for all arguments passed to registered C++ functions, required
  by cpp11’s strict type checking.

- Comprehensive test coverage added for previously untested exports:
  `vapour_read_fids`, `vapour_read_fields`, `vapour_report_fields`,
  `vapour_report_attributes`, `vapour_geom_name`, `vapour_geolocation`,
  `vapour_get_config`, `vapour_proj_version`, `vapour_create_options`,
  `buildvrt`, `vector_vrt`, `gdal_raster_data`, `gdal_raster_dsn`,
  `gdal_raster_image`, `gdal_raster_nara`. Restored `vapour_vrt` tests
  that were previously commented out.

- Removed stale scratchpad files from inst/ (benchmarks, shiny,
  warpsandbox, pbf, readwrite, stars, cmd).

- Improve nativeRaster implementation.

- Fix CSLConstList changes in GDAL \>=3.13.

- Split gdallibrary.h (784 lines) into four focused headers:
  gdal_layer_utils.h (layer open/close helpers, with_ogr_layer
  templates), gdal_crs.h (CRS/projection functions), gdal_drivers.h
  (driver listing, version, config, VSI), and gdal_field_alloc.h (field
  allocation, feature count, field reporting). gdallibrary.h is now a
  thin umbrella include. Removed 281 lines of dead code
  (gdal_read_fields, gdal_read_geometry) that were superseded by
  gdalgeometry.h equivalents.

- Added as_cstr() helper in common_vapour.h for extracting const char\*
  from cpp11::r_string without allocating a temporary std::string.
  Replaced 66 instances of std::string(x\[0\]).c_str() across all
  headers.

- Added include guard to common_vapour.h.

- Extracted write_feature_fields() helper in gdalgeometry.h, shared
  across layer_read_fields_ij, layer_read_fields_ia, and
  layer_read_fields_fa. The \_ia and \_fa variants now have full field
  type support (DateTime, Binary, NA handling) matching \_ij, fixing
  silent data loss for these types when reading by index or FID.

- Updated GitHub Actions check-standard workflow: dropped pinned
  Rtools42/43 Windows configs in favour of current defaults, updated
  actions/checkout to v4.

## vapour 0.15.0

CRAN release: 2025-12-21

- Fix C++ standard (don’t specify).

## vapour 0.14.0

- Explicit conversion of unsigned long to int for GDALBuildVRT(), thanks
  to CRAN.

## vapour 0.13.0

CRAN release: 2025-11-18

- Fixed a change in upcoming GDAL for OGRSpatialReference, thanks to
  CRAN.

## vapour 0.12.0

CRAN release: 2025-08-20

- Fixed a number of unused variable warnings, found from mac_release
  test on CRAN.

- Removed test that guarded for 0x0 output from GDAL, thanks to CRAN.

- Stream reading now restored.

## vapour 0.11.0

CRAN release: 2025-04-04

- Removed bashisms, thanks CRAN.

## vapour 0.10.0

CRAN release: 2024-06-14

- Fixed leaks from valgrind, thanks CRAN and rhub.

- Cleaned up a number of address sanitize issues and segfaults, thanks
  to CRAN.

- Old warper code is removed, now uses ‘gdal_raster\_’ from warpgeneral.

- Fixed putting bad options in (empty strings).

- Fixed type problem in internal get projection strings.

- Urgent release to fix segfault on CRAN, instances of int instead of
  IntegerVector for R devel.

- Fix incorrect format string for “-te”.

- Use pkg-config on Windows when available, thanks Tomas Kalibera and
  CRAN.

- New function
  [`gdal_raster_nara()`](https://hypertidy.github.io/vapour/reference/gdal_raster_data.md)
  to return nativeRaster in a list understood by `ximage()`.

- New function
  [`vector_vrt()`](https://hypertidy.github.io/vapour/reference/vector_vrt.md)
  to generate VRT for SQL and/or reprojection.

- Fix cross-compilation for ARM on universe, thanks to Jeroen Ooms.

- New function
  [`buildvrt()`](https://hypertidy.github.io/vapour/reference/buildvrt.md)
  as a special-case for -separate from gdalbuildvrt app.

- [`vapour_vrt()`](https://hypertidy.github.io/vapour/reference/vapour_vrt.md)
  gains ‘options’ argument, so we can in particular do
  `options = c("-expand", "rgb", "-ot", "Byte")` to warp 16-bit integer
  colour palettes from GTiff to PNG. :)

- [`gdal_raster_dsn()`](https://hypertidy.github.io/vapour/reference/gdal_raster_data.md)
  can now write to non-GTiff/COG via `options = c("-of", DRIVER)`.

- New function
  [`vapour_geolocation()`](https://hypertidy.github.io/vapour/reference/vapour_geolocation.md)
  retrieves any existing geolocation information.

- [`vapour_vrt()`](https://hypertidy.github.io/vapour/reference/vapour_vrt.md)
  called with geolocation arrays now scrubs the geotransform from the
  output (which ensures the warper uses the arrays without setting
  ‘-geoloc’).

- [`vapour_create()`](https://hypertidy.github.io/vapour/reference/vapour_create.md)
  gets creation options, data type options, and driver options.

- Fixed failure to close file created by
  [`vapour_create()`](https://hypertidy.github.io/vapour/reference/vapour_create.md)
  fixes [\#202](https://github.com/hypertidy/vapour/issues/202).

## vapour 0.9.5

CRAN release: 2023-04-11

- [`vapour_vrt()`](https://hypertidy.github.io/vapour/reference/vapour_vrt.md)
  gains an ‘overview’ argument. Wish of
  <https://github.com/hypertidy/vapour/issues/186>

- [`gdal_raster_data()`](https://hypertidy.github.io/vapour/reference/gdal_raster_data.md),
  [`gdal_raster_dsn()`](https://hypertidy.github.io/vapour/reference/gdal_raster_data.md),
  and
  [`gdal_raster_image()`](https://hypertidy.github.io/vapour/reference/gdal_raster_data.md)
  now allow a source with geolocation arrays to be warped to a
  non-longlat projection.

- The extent reported by ‘vapour_raster_info()’ is now correct for the
  general case including non-zero skew geotransforms.

- ‘vapour_raster_info()’ gains a ‘corners’ element.

- All functions that read or query data source/s now check for tilde “~”
  at the beginning of the string/s, and normalize the path as needed.
  Reported by [@Sibada](https://github.com/Sibada) in
  [\#193](https://github.com/hypertidy/vapour/issues/193).

- vapour now imports nanoarrow and includes internal experimental
  support for GDAL (\>= 3.6) stream reading (RFC 86). Nothing is exposed
  for general use yet.

- New Makevars.ucrt with patch contributed by CRAN, Tomas Kalibera.

- raster read gains “unscale” argument, `TRUE` by default which means
  offset and scale values are applied and return value is of type
  Float64, set to `FALSE` to avoid applying scale/offset to band values
  (this was always implicitly false (no offset scale applied) if the
  output type wasn’t suitable). .

- Fix inefficiency in gdal_raster_data() which was warping all bands in
  MEM lol.

- Fix for
  [`vapour_vrt()`](https://hypertidy.github.io/vapour/reference/vapour_vrt.md)
  where ‘geolocation’ was not being included. Fixes
  [\#192](https://github.com/hypertidy/vapour/issues/192).

- New capability to drop dataset and band metadata, used by functions
  [`vapour_vrt()`](https://hypertidy.github.io/vapour/reference/vapour_vrt.md)
  and
  [`vapour_warp_raster()`](https://hypertidy.github.io/vapour/reference/vapour_warp_raster.md)
  which gain a new argument ‘nomd’, which is FALSE by default. If TRUE,
  the dataset and band metadata are removed from an open dataset before
  it is converted to VRT.

This particularly makes VRT DSN strings a lot smaller, for use by
whatarelief and raadtools for example. There should be no change to
current default uses, the argument ‘nomd’ must be specified for the
change.

- New function
  [`vapour_crs_is_lonlat()`](https://hypertidy.github.io/vapour/reference/vapour_crs_is_lonlat.md)
  to test crs string.

- Removed C++11 requirement.

- remove LDFLAGS from configure, as per sf
  [\#1369](https://github.com/hypertidy/vapour/issues/1369) and vapour
  [\#188](https://github.com/hypertidy/vapour/issues/188), thanks to
  [@gremms1](https://github.com/gremms1) and
  [@sgoslee](https://github.com/sgoslee) for report on Fedora.

- Removed FromHandle (introduced GDAL 2.3) and undeclared use of
  down_cast, and now tested on GDAL 2.2.3.

- New Makevars.ucrt with patch contributed by CRAN, and removed outdated
  C++11 requirement.

## vapour 0.9.3

CRAN release: 2023-01-06

- Update to CRAN Makevars.ucrt.

## vapour 0.9.2

CRAN release: 2022-11-16

- Resubmit after 0.9.1 was rejected, fixed configure.ac and checked on
  on M1MAC/macbuilder.

- The SQL dialect in use for the ‘sql’ argument can now be controlled,
  it is set to “” by default. See `?vapour-package` for details.

- ‘vapour_layer_info’ now supports spatial filter, the ‘extent’ argument
  in ‘vapour_layer_info’ is now for limiting the spatial extent of a
  layer query (was for controlling whether extent calculated or not).

- ‘gdal_read_names’ removed from gdallibrary, and ‘read_names_gdal_cpp’
  renamed to ‘read_fids_gdal_cpp’ and now returns a numeric vector
  rather than a list.

- Fixed another stray cast (int), now checking more robustly for nodata
  value for integer bands. We have made some inroads into better
  consistency of use of long integer values in the code base, but vapour
  does not yet support long vectors.

## vapour 0.9.0

CRAN release: 2022-11-07

- Big cleanup.

- Resubmit after 0.8.82 was rejected.

- Fix examples and documentation for ‘sst_c’, ‘vapour_read_geometry’,
  ‘vapour_read_fields’, ‘vapour_create()’, ‘vapour_read_raster_block()’
  and ‘vapour_write_raster_block()’ from CRAN submission.

- Clarified authorship for Jim Hester (listed as copyright holder in
  DESCRIPTION), removed comment from code in CollectorList.h.

- Resubmit after 0.8.8.81 was removed.

- Remove obscure NetCDF/HDF5 tests that don’t work on CRAN macos.

## vapour 0.8.81

CRAN release: 2022-08-30

### BREAKING CHANGES

- ‘vapour_sds_names()’ now returns a character vector not a list.

- ‘gdalraster::gdal_list_subdatasets’ now returns unprocessed subdataset
  strings, of the form ’SUBDATASET\_{i}\_NAME={DRIVER}:{URI}:layer’, and
  the strings are cleaned in R (but, can use `vapour_raster_info`,
  [`vapour_sds_names()`](https://hypertidy.github.io/vapour/reference/vapour_sds_names.md)
  for a separate pathway to this.)

------------------------------------------------------------------------

- Fixed CRAN warnings due to incorrect use of boolean operator,
  [\#158](https://github.com/hypertidy/vapour/issues/158).

- [`vapour_raster_info()`](https://hypertidy.github.io/vapour/reference/vapour_raster_info.md)
  gains a subdatasets vector, this is just the source input if
  subdatasets are not present.

- Subdatasets can now be named by variable name or by index in
  ‘vapour_vrt()’. Note this can’t always work as some services don’t
  have names in the sense that classic subdataset sources (like netcdf)
  do.

- ‘warp_options’ and ‘transformation_options’ has been removed from the
  ‘gdalwarpmem’ namespace, and from the R and Rcpp wrappers. All options
  are now assumed to be bundled into one string list (CharacterVector).

- Setting options for the ‘vapour_warp_raster()’ has been reconfigured.
  Arguments ‘warp_options’, ‘transformation_options’, ‘open_options’,
  and ‘dataset_output_options’ correspond to the ‘NAME=VALUE’
  configuration provided by gdalwarp as ‘-wo’, ‘-to’, ‘-oo’, and ‘-doo’.
  All of these should be entered in bare form (without the ‘-wo’, ‘-to’
  etc) but for ‘options’ these must be entered as per standard gdalwarp
  options. Some are disallowed and are checked for, triggering an error
  because we either set them via other arguments, or they can’t be
  supported atm.

- New options setting support means we can support all gdalwarp options,
  notably ‘-cutline’, ‘-csql’, ‘-cwhere’, and ‘-crop_to_cutline’ which
  allow masking by a a polygon layer from a vector data source. Thanks
  to Hugh Graham for the motivation for this. :)

- New internal function `raster_has_geolocation_gdal_cpp()`, early begin
  of does this datasource have geolocation arrays (because if it does we
  can just push it through the warper, otherwise nominate them with
  `vapour_vrt(geolocation = )`).

- [`vapour_vrt()`](https://hypertidy.github.io/vapour/reference/vapour_vrt.md)
  gains geolocation argument, which can be named dsns for each of
  longitude,latitude. Currently assumed to be ‘OGC:CRS84’.

- Now depend on GDAL 2.2.3 as a minimum, because ubuntu 18.04 is still
  in wide use. GDALInfo() lib needs GDAL 2.1 as minimum. Some info
  features need later, but do not fail (AFAIK).

- [`vapour_raster_info()`](https://hypertidy.github.io/vapour/reference/vapour_raster_info.md)
  now has `dimension` element, intended for use instead of `dimXY` (for
  standard idiom like {gdalio} uses with ‘extent’, ‘dimension’,
  ‘projection’ as the basis of what a raster is). `dimXY` will be
  removed in future.

- New namespace ‘gdalapplib’ with one utility for GDALInfo(), this is
  vectorized at the cpp level and includes subdataset handling. It does
  not include listing of TAR and ZIP archives.
  [`vapour_raster_info()`](https://hypertidy.github.io/vapour/reference/vapour_raster_info.md)
  now uses this.

- New functions
  [`vapour_create()`](https://hypertidy.github.io/vapour/reference/vapour_create.md)
  [`vapour_read_raster_block()`](https://hypertidy.github.io/vapour/reference/vapour_read_raster_block.md)
  and
  [`vapour_read_raster_block()`](https://hypertidy.github.io/vapour/reference/vapour_read_raster_block.md)
  for creating and writing to files.

- Moved C++ functions `gdal_sds_list()`, `gdal_extent_only()`
  `gdal_raster_info()`, `gdal_raster_gcp()`, `init_resample_alg()`,
  `gdal_read_band_values()`, `gdal_read_dataset_values()`,
  `gdal_raster_dataset_io()`, `gdal_raster_io()` to *gdalraster*
  namespace from *gdallibrary* namespace. (Some of these will get
  refactored out).

- New function
  [`vapour_vrt()`](https://hypertidy.github.io/vapour/reference/vapour_vrt.md).

- [`vapour_warp_raster()`](https://hypertidy.github.io/vapour/reference/vapour_warp_raster.md)
  default has changed for `bands`, it’s now NULL which means “all
  bands”, not `1L`.

- New functions
  [`vapour_set_config()`](https://hypertidy.github.io/vapour/reference/vapour_set_config.md)
  and
  [`vapour_get_config()`](https://hypertidy.github.io/vapour/reference/vapour_set_config.md)
  to control configuration options for GDAL.

- Flushed out memory bugs with valgrind.

- Refactored read from raster and warp raster. The warping uses the same
  band reader as the RasterIO reader, and more cases of ‘GDALOpen()’ for
  rasters go via the underlying ‘gdalraster’ namespace pathway.

- Faster layer extent determination in
  [`vapour_layer_info()`](https://hypertidy.github.io/vapour/reference/vapour_layer_info.md),
  new function
  [`vapour_layer_extent()`](https://hypertidy.github.io/vapour/reference/vapour_layer_extent.md).

- Better escaping for layer name, needed quotes around layer ‘SELECT’
  when counting features with SQL for FlatGeoBuf.

## vapour 0.8.8

(This never made it to CRAN.)

- Patch release for R UCRT changes. Applied patch provided by CRAN.

- Removed invalid missing value setting for type raw and int in the
  warper, causing representable range errors on cran.

## vapour 0.8.5

CRAN release: 2021-10-07

### BREAKING CHANGES

- Raster read now detects type of the source and return raw, integer, or
  double as per the native type in the source.

- `wkt` argument for the target projection in warper functions is now
  replaced by ‘projection’. If ‘wkt’ is given it is used to set the
  value for ‘projection’ and a message emitted. No change has been made
  at the C++ level, which still uses the original names.

### BUG FIXES

- Fixed configure.ac, upgraded syntax causing warnings on CRAN.

- Fixed huge SDS name wrecking bug. So now works for WMTS sources, for
  example.

- Fixed bug in use of scaling, was applied without conversion to
  floating point type.

### NEW FEATURES

- [`vapour_raster_info()`](https://hypertidy.github.io/vapour/reference/vapour_raster_info.md)
  gains `datatype` field.

- Warp read functions gain `warp_options` and `transformation_options`.

- New feature to provide `band_output_type` to raster read functions, to
  take Byte, Int32, or Float64.

- New functions
  [`vapour_read_raster_raw()`](https://hypertidy.github.io/vapour/reference/vapour_read_raster_raw.md),
  [`vapour_read_raster_int()`](https://hypertidy.github.io/vapour/reference/vapour_read_raster_raw.md),
  [`vapour_read_raster_dbl()`](https://hypertidy.github.io/vapour/reference/vapour_read_raster_raw.md),
  [`vapour_read_raster_chr()`](https://hypertidy.github.io/vapour/reference/vapour_read_raster_raw.md)
  and its alias
  [`vapour_read_raster_hex()`](https://hypertidy.github.io/vapour/reference/vapour_read_raster_raw.md)
  to return specific types of atomic vector. \_chr and \_hex convert raw
  bytes to colours.

- New function
  [`vapour_read_raster_block()`](https://hypertidy.github.io/vapour/reference/vapour_read_raster_block.md)
  a helper around the internal C++ reader for the simple case of
  offset/dimension read. There’s a matching but non-exported
  vapour_write_raster_block(). See issue/PR
  [\#123](https://github.com/hypertidy/vapour/issues/123) for examples.

## vapour 0.8.0

CRAN release: 2021-08-25

- Fix for CRAN errors, with broken configure.

## vapour 0.7.8

CRAN release: 2021-08-14

- Fixed errors for earlier GDAL (that were seen on Solaris with GDAL
  2.2.4, now 2.4.4), by condition on GDAL version for importFromWkt() as
  sf does. This means vapour can work on GDAL from version 2.0.1, and so
  support its use on Ubuntu 18.04.

## vapour 0.7.7

CRAN release: 2021-08-05

- Remove unused vapour.cpp which trigger unused warnings: MAX_INT
  variable and code GetPointsInternal, warnings on CRAN
  r-devel-linux-x86_64-fedora-clang, 2021-08-03.

- Fixed dependence on GDAL, require version GDAL \>= 2.3.0. Older
  versions not const correct for OGRGeometry::importFromWkt(),
  triggering errors on Solaris CRAN which was GDAL 2.2.4 at 2021-08-03.

- Fixed
  [`vapour_layer_info()`](https://hypertidy.github.io/vapour/reference/vapour_layer_info.md),
  now accepts name of layer not just index to be more consistent across
  other layer functions. Now includes more information about the layer.

- [`vapour_read_fields()`](https://hypertidy.github.io/vapour/reference/vapour_read_fields.md)
  will now return binary fields.

- [`vapour_srs_wkt()`](https://hypertidy.github.io/vapour/reference/vapour_srs_wkt.md)
  now accepts more inputs, not just proj strings.

## vapour 0.7.6

CRAN release: 2021-08-02

- Include OFTBinary in supported field types.

- REALLY fixed the CRAN UBSAN problem. Listed here:
  <https://github.com/hypertidy/vapour/issues/110>

- [`vapour_raster_info()`](https://hypertidy.github.io/vapour/reference/vapour_raster_info.md)
  gains ‘extent’ element.

- [`vapour_report_fields()`](https://hypertidy.github.io/vapour/reference/vapour_report_fields.md)
  new function aliased to
  [`vapour_report_attributes()`](https://hypertidy.github.io/vapour/reference/vapour_report_fields.md)
  and
  [`vapour_read_fields()`](https://hypertidy.github.io/vapour/reference/vapour_read_fields.md)
  to
  [`vapour_read_attributes()`](https://hypertidy.github.io/vapour/reference/vapour_read_fields.md)
  as more sensible names for what they do. The ’\_attributes’ versions
  might be deprecated in future.

- [`vapour_layer_info()`](https://hypertidy.github.io/vapour/reference/vapour_layer_info.md)
  gains driver, layer name/s, fields, and feature count.

- [`vapour_raster_info()`](https://hypertidy.github.io/vapour/reference/vapour_raster_info.md)
  now returns `$projstring`, the proj string version of `$projection`
  (was called ‘proj4’ and previously unimplemented). The value may be an
  empty string.

- File list from
  [`vapour_raster_info()`](https://hypertidy.github.io/vapour/reference/vapour_raster_info.md)
  now returns a missing value (NA) when no files are in the list.

- Fixed a serious bug caused by ‘vapour_raster_info()’ attempting to
  create a character vector from a null pointer when no file list exists
  for a source (such as an image server).

## vapour 0.7.5

CRAN release: 2021-07-21

- Fixed input type problems found on CRAN, see
  <https://github.com/hypertidy/vapour/issues/110>. Thanks to CRAN for
  explaining.

- Warper input can now include multiple sources.

## vapour 0.7.0

CRAN release: 2021-07-04

- Fixed CRAN windows problem with NetCDF.

- New *internal* functions
  [`vapour_read_geometry_ia()`](https://hypertidy.github.io/vapour/reference/vapour_read_geometry.md)
  and
  [`vapour_read_geometry_ij()`](https://hypertidy.github.io/vapour/reference/vapour_read_geometry.md)
  to read by feature id arbitrary and range - not sure if will make
  export … `_fa()` counterpart is problematic.

- New function
  [`vapour_layer_info()`](https://hypertidy.github.io/vapour/reference/vapour_layer_info.md)
  to return the previous internal projection string info for a vector  
  layer.

## vapour 0.6.5

CRAN release: 2021-06-28

- Fix to configure for Fedora thanks to Inaki Ucar
  ([@Enchufa2](https://github.com/Enchufa2))
  <https://github.com/hypertidy/vapour/issues/95>

- [`vapour_warp_raster()`](https://hypertidy.github.io/vapour/reference/vapour_warp_raster.md)
  now accepts an extent for the extraction with giving a dimension, it
  returns native pixels found within that extent. This is a special case
  for when you have control over the input window extent and now the
  dimensions implicitly. (We might formalize around storing the
  dimensions, extent, and crs as an attribute but that’s been
  unnecessary so far.)

- Input projection strings for `wkt` or `source_wkt` in
  `vapour_warp_raster` may be anything acceptable by GDAL itself. These
  include WKT variants, PROJ strings, EPSG strings, and file names.

- The function
  [`vapour_warp_raster()`](https://hypertidy.github.io/vapour/reference/vapour_warp_raster.md)
  now uses argument ‘bands’ rather than ‘band’ at the R level. The
  warper function correctly handles repeated band numbers in `bands` and
  can be set to `NULL`, to return all bands. (Note that
  [`vapour_read_raster()`](https://hypertidy.github.io/vapour/reference/vapour_read_raster.md)
  still uses ‘band’).

- User interface for \`vapour_warp_raster()’ now changed to use ‘extent’
  and ‘source_extent’ in place of ‘geotransform’ and
  ‘source_geotransform’, which are now deprecated.

- Function
  [`vapour_warp_raster()`](https://hypertidy.github.io/vapour/reference/vapour_warp_raster.md)
  now exported, properly handles input-replacement geotransform or
  projection for sources that have insufficient metadata. This works
  with a variety of input data source names (files, URLs, image servers,
  etc.) Overviews are automatically dealt with (by choosing the right
  level of detail) by using the gdalwarp app library functionality.
  (This warp function may be refactored to use extent rather than
  geotransform).

- Added vsi example with zipped shapefile.

- Fixed missing stdlib.h declaration for exit() in configure tests,
  thanks to CRAN. [\#88](https://github.com/hypertidy/vapour/issues/88)

- New function
  [`vapour_read_type()`](https://hypertidy.github.io/vapour/reference/vapour_read_geometry.md)
  to return the integer geometry type (GDAL’s wkb enum code).

- `vapour_read_geometry(what = "point")` is now defunct

- Major refactor to use C++ headers, so other packages can access the
  API without using R functions. There’s a more comprehensive set of
  identifier, names, fields, extent, and geometry readers, and an
  addition of the GDALwarp() facility which is a generalization of
  RasterIO. This will now work on a much greater range of sources than
  the older RasterIO (still WIP).

- New function
  [`vapour_geom_name()`](https://hypertidy.github.io/vapour/reference/vapour_geom_name.md)
  to get a data source geometry “column name”. Non-database sources
  return an empty string *unless* `ExecuteSQL()` was called, and then it
  is “*ogr_geometry*”.

Typical names are ‘geom’ in GPKG, ‘SHAPE’ in GDB.

## vapour 0.5.5

CRAN release: 2020-06-23

- Deleted compile tests from configure, we can add proper tests if this
  is needed.

- Now set env vars GDAL_DATA and PROJ_LIB to vapour/gdal/ and
  vapour/proj/ when present (i.e. Windows generally, MacOS on CRAN).

- Update to configure to ensure data copy of proj/ and gdal/ on MacOS on
  CRAN, thanks to CRAN, Roger Bivand, and to James Balamuta for enormous
  help on github actions and installing the CRAN mac binaries.

- Now using GDAL 3 on Windows.

- Get available overview sizes, wish of
  [\#78](https://github.com/hypertidy/vapour/issues/78).

## vapour 0.5.0

CRAN release: 2020-01-30

- Improve tests to be more robust, thanks to CRAN.
  [\#77](https://github.com/hypertidy/vapour/issues/77)

- Removed pesky message and failing tests thanks to Joseph Stachelek
  ([@jsta](https://github.com/jsta))
  [\#76](https://github.com/hypertidy/vapour/issues/76).

## vapour 0.4.0

CRAN release: 2019-09-27

### BREAKING CHANGES

- The function
  [`vapour_read_raster()`](https://hypertidy.github.io/vapour/reference/vapour_read_raster.md)
  now returns a list of (one) numeric vector. This change is so a future
  version can return more than one band for this and related functions.

### NEW FEATURES

- New data set `tas_wkt` a Well-Known-Text string to simplify an example
  in the documentation.

- The function
  [`vapour_read_raster()`](https://hypertidy.github.io/vapour/reference/vapour_read_raster.md)
  now returns a list with a single vector of values from the requested
  band. In future this may return values from multiple bands. The new
  function
  [`vapour_warp_raster()`](https://hypertidy.github.io/vapour/reference/vapour_warp_raster.md)
  uses the same scheme.

- Fixed static linking for MacOS build, thanks(!!) to Jeroen Ooms for
  guidance and thanks to CRAN for reporting the issue.

- New function
  [`vapour_srs_wkt()`](https://hypertidy.github.io/vapour/reference/vapour_srs_wkt.md)
  to convert PROJ4 strings to Well-Known Text.

- New *internal-only* function
  [`vapour_warp_raster()`](https://hypertidy.github.io/vapour/reference/vapour_warp_raster.md)
  to return a warped version from a raster source. Please use with
  caution, it’s still flaky for certain cases and will crash your
  session (setting projection on NetCDF 0,360 is one example but not
  fleshed out yet - see hypertidy/lazyraster for in-dev usage examples.)

- New internal function `warp_memory_cpp()` to warp small in memory
  rasters.

## vapour 0.3.0

CRAN release: 2019-09-18

- Updates to configure to remove deprecated R CMD config CPP (thanks to
  CRAN).

- Updates to enable use of the deprecated PROJ API.

- New function
  [`vapour_vsi_list()`](https://hypertidy.github.io/vapour/reference/vapour_vsi_list.md)
  to list contents of virtual files. See
  <https://twitter.com/mdsumner/status/1158335706483179521> and
  <https://github.com/hypertidy/vapour/issues/55> for inspirations.

- Added a hex sticker! Thanks to
  [@Maschette](https://github.com/Maschette).

- Added ‘proj4’ string as well as ‘projection’ (WKT) to the output of
  [`vapour_raster_info()`](https://hypertidy.github.io/vapour/reference/vapour_raster_info.md)  
  (these aren’t always available, especially not for longlat NetCDF).

- Fixed failure in `limit_n`, `skip_n` behaviour for reading names
  (FIDs) and attributes,
  ([\#60](https://github.com/hypertidy/vapour/issues/60)).

## vapour 0.2.0

CRAN release: 2019-05-06

- Fixed incorrect use of C-level `printf()` and unnecessary tests thanks
  to CRAN feedback.

- Add ‘type’ to output of
  [`vapour_geom_summary()`](https://hypertidy.github.io/vapour/reference/vapour_geom_summary.md),
  the integer code for each geometry type.

- Restructured conversion to text geometries to properly free up memory,
  was causing memory leaks (found with valgrind).

- Use of `sql` now correctly uses ‘GDALDataset::ReleaseResultsSet()’ in
  each applicable function. Query with the ‘sql’ argument was not being
  executed in
  [`vapour_layer_names()`](https://hypertidy.github.io/vapour/reference/vapour_layer_names.md)
  but now is - only has utility for insert and drop queries, so will be
  rarely used and probably never had any impact before.

- Raster read gains a new argument `native = FALSE` to control use of
  the native window without specifying it. If `native = TRUE` then the
  native dimensions are used and read in full.

- Vector read functions gain new `extent` argument to apply a spatial
  filter in conjunction with the `sql` argument, per discussion
  ([\#34](https://github.com/hypertidy/vapour/issues/34)). The extent
  can be of type sp, sf, raster, or generic vector
  `c(xmin, xmax, ymin, ymax)`. The extent is ignored if the `sql`
  argument is not specified, with a warning. Applies to
  [`vapour_geom_summary()`](https://hypertidy.github.io/vapour/reference/vapour_geom_summary.md),
  [`vapour_read_attributes()`](https://hypertidy.github.io/vapour/reference/vapour_read_fields.md),
  [`vapour_read_extent()`](https://hypertidy.github.io/vapour/reference/vapour_read_geometry.md),
  [`vapour_read_geometry()`](https://hypertidy.github.io/vapour/reference/vapour_read_geometry.md),
  [`vapour_read_geometry_text()`](https://hypertidy.github.io/vapour/reference/vapour_read_geometry.md),
  and
  [`vapour_read_names()`](https://hypertidy.github.io/vapour/reference/vapour_read_fids.md).
  Thanks to Joseph Stachelek for the suggestion.

- Vector read functions gain new `limit_n` and `skip_n` arguments to
  specify a sequential set of features to be read, added to
  [`vapour_geom_summary()`](https://hypertidy.github.io/vapour/reference/vapour_geom_summary.md),  
  [`vapour_read_attributes()`](https://hypertidy.github.io/vapour/reference/vapour_read_fields.md),
  [`vapour_read_extent()`](https://hypertidy.github.io/vapour/reference/vapour_read_geometry.md),
  [`vapour_read_geometry()`](https://hypertidy.github.io/vapour/reference/vapour_read_geometry.md),
  [`vapour_read_geometry_text()`](https://hypertidy.github.io/vapour/reference/vapour_read_geometry.md),
  and
  [`vapour_read_names()`](https://hypertidy.github.io/vapour/reference/vapour_read_fids.md).
  Their effect occurs after that of the `sql` argument.

- New function
  [`vapour_driver()`](https://hypertidy.github.io/vapour/reference/GDAL-library.md)
  to determine chosen driver (vector, short name) of the data source.

- New argument `min_max` to allow fast default use of
  [`vapour_raster_info()`](https://hypertidy.github.io/vapour/reference/vapour_raster_info.md),
  as per ([\#50](https://github.com/hypertidy/vapour/issues/50)).

- New function
  [`vapour_raster_gcp()`](https://hypertidy.github.io/vapour/reference/vapour_raster_gcp.md)
  to return GCP (ground control points) if present.

- New function
  [`vapour_report_attributes()`](https://hypertidy.github.io/vapour/reference/vapour_report_fields.md)
  to return a string description of the internal GDAL type for data
  attributes wish of Grant Williamson.

- Attribute types ‘OFTInteger64’ are now returned as doubles, ‘Date’ and
  ‘DateTime’ are returned as strings.

- New functions
  [`vapour_gdal_version()`](https://hypertidy.github.io/vapour/reference/GDAL-library.md)
  and
  [`vapour_all_drivers()`](https://hypertidy.github.io/vapour/reference/GDAL-library.md)
  to return information about the GDAL library in use.

- Function
  [`vapour_read_names()`](https://hypertidy.github.io/vapour/reference/vapour_read_fids.md)
  is now implemented using the library API, rather than via OGRSQL. All
  trace of the OGRSQL FID workaround and warning has been removed.

- New function
  [`vapour_geom_summary()`](https://hypertidy.github.io/vapour/reference/vapour_geom_summary.md)
  to return basic identity, validity, and extent of feature geometries.

- Values for the extent (bounding box) of a feature are now set to NA
  values for an empty or missing geometry.

- Fix drastic memory leak in
  [`vapour_read_attributes()`](https://hypertidy.github.io/vapour/reference/vapour_read_fields.md),
  thanks to Grant Williamson.

## vapour 0.1.0

CRAN release: 2018-08-14

- Functions that include a `layer` index now accept a layer name which
  is used to find the (0-based) layer index.

- This is a breaking release, all old functions have been made defunct
  or removed entirely.

## vapour 0.0.1

- All C++ functions now have explicit R wrappers instead of Rcpp
  exports.

- New naming convention uses `vapour_` for vector sources, `raster_` for
  raster sources to make function names a little more consistent.

- New wrapper `vapour_sds_names` to replace `sds_info` and
  `raster_sds_info` which are now deprecated.

- New wrapper `vapour_read_geometry_what` to replace
  `vapour_read_feature_what` which i is now deprecated.

- New function `vapour_read_names` to return the vector of FID values.

- The read for raster data now returns numeric or integer appropriately.

- `vapour_raster_info` now includes `bands` as the count of available
  bands.

- The IO read now allows a 4-element `window` to return data at native
  resolution, by copying the third and fourth values (source dimension)
  to the fifth and sixth values (output dimension) respectively.

- Subdatasets are now supported.

- Added sanity check behaviour to `vapour_raster_read` to avoid out of
  bounds read attempts.

- Resampling options added to raster data read.

- Upgraded to rwinlib gdal 2.2.3.

- added function `vapour_layer_names` to return character layer names,
  and implicitly provide a layer count

- feature iteration now avoids “GetFeatureCount” and collects each
  element in a growing list, thanks to Jim Hester

- renamed `format` argument to `textformat`, this is ignored unless
  `what = "text"`

- Created a single C++ feature read to remove repeated logic,
  `vapour_read_geometry`, `vapour_read_geometry_text`, and
  `vapour_read_extent` all call the same function.
