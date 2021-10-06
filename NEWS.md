# vapour 0.8.5

## BREAKING CHANGES

* Raster read now detects type of the source and return raw, integer, or double
as per the native type in the source.

* `wkt` argument for the target projection in warper functions is now replaced
by 'projection'.  If 'wkt' is given it is used to set the value for 'projection'
and a message emitted. No change has been made at the C++ level, which still uses the
original names.
 
 
## BUG FIXES 

* Fixed configure.ac, upgraded syntax causing warnings on CRAN. 

* Fixed huge SDS name wrecking bug. So now works for WMTS sources, for example. 

* Fixed bug in use of scaling, was applied without conversion to floating point type. 

## NEW FEATURES

* `vapour_raster_info()` gains `datatype` field. 

* Warp read functions gain `warp_options` and `transformation_options`. 

* New feature to provide `band_output_type` to raster read functions, to take
Byte, Int32, or Float64.

* New functions `vapour_read_raster_raw()`, `vapour_read_raster_int()`,
`vapour_read_raster_dbl()`, `vapour_read_raster_chr()` and its alias
`vapour_read_raster_hex()` to return specific types of atomic vector. _chr and
_hex convert raw bytes to colours.

* New function `vapour_read_raster_block()` a helper around the internal C++
reader for the simple case of offset/dimension read. There's a matching but
non-exported vapour_write_raster_block(). See issue/PR #123 for examples.

# vapour 0.8.0

* Fix for CRAN errors, with broken configure. 

# vapour 0.7.8

* Fixed errors for earlier GDAL (that were seen on Solaris with GDAL 2.2.4, now 2.4.4), by condition on GDAL version for 
 importFromWkt() as sf does. This means vapour can work on GDAL from version 2.0.1, and so support its use on Ubuntu 18.04. 

# vapour 0.7.7

* Remove unused vapour.cpp which trigger unused warnings: MAX_INT variable and code GetPointsInternal, warnings on CRAN r-devel-linux-x86_64-fedora-clang, 2021-08-03. 

* Fixed dependence on GDAL, require version GDAL >= 2.3.0. Older versions not const correct for OGRGeometry::importFromWkt(), triggering
errors on Solaris CRAN which was GDAL 2.2.4 at 2021-08-03. 

* Fixed `vapour_layer_info()`, now accepts name of layer not just index to be more consistent across other layer functions. Now includes
 more information about the layer. 

* `vapour_read_fields()` will now return binary fields. 

* `vapour_srs_wkt()` now accepts more inputs, not just proj strings. 

# vapour 0.7.6

* Include OFTBinary in supported field types. 

* REALLY fixed the CRAN UBSAN problem. Listed here: https://github.com/hypertidy/vapour/issues/110

* `vapour_raster_info()` gains 'extent' element. 

* `vapour_report_fields()` new function aliased to `vapour_report_attributes()` and `vapour_read_fields()` to `vapour_read_attributes()` as more sensible names for what they do. The '_attributes' versions might be deprecated in future. 

* `vapour_layer_info()` gains driver, layer name/s, fields, and feature count. 

* `vapour_raster_info()` now returns `$projstring`, the proj string version of `$projection` (was called 'proj4' and previously 
 unimplemented). The value may be an empty string. 
 
* File list from `vapour_raster_info()` now returns a missing value (NA) when no files are in the list. 

* Fixed a serious bug caused by 'vapour_raster_info()' attempting to create a character vector from a null pointer
 when no file list exists for a source (such as an image server). 
 
# vapour 0.7.5

* Fixed input type problems found on CRAN, see https://github.com/hypertidy/vapour/issues/110. Thanks to CRAN 
 for explaining. 

* Warper input can now include multiple sources. 


# vapour 0.7.0

* Fixed CRAN windows problem with NetCDF. 

* New *internal* functions `vapour_read_geometry_ia()` and `vapour_read_geometry_ij()` to read by feature id arbitrary
and range - not sure if will make export ... `_fa()` counterpart is problematic. 

* New function `vapour_layer_info()` to return the previous internal projection string info for a vector   
 layer. 

# vapour 0.6.5

* Fix to configure for Fedora thanks to Inaki Ucar (@Enchufa2) https://github.com/hypertidy/vapour/issues/95

* `vapour_warp_raster()` now accepts an extent for the extraction with giving a dimension, it returns native 
 pixels found within that extent. This is a special case for when you have control over the input window extent 
 and now the dimensions implicitly. (We might formalize around storing the dimensions, extent, and crs as an 
 attribute but that's been unnecessary so far.)
 
* Input projection strings for `wkt` or `source_wkt` in `vapour_warp_raster` may be anything acceptable by GDAL itself. These 
 include WKT variants, PROJ strings, EPSG strings, and file names. 

* The function `vapour_warp_raster()` now uses argument 'bands' rather than 'band' at the R level. The warper function 
 correctly handles repeated band numbers in `bands` and can be set to `NULL`, to return all bands. (Note that `vapour_read_raster()`
 still uses 'band'). 

* User interface for `vapour_warp_raster()' now changed to use 'extent' and 'source_extent' in place of
 'geotransform' and 'source_geotransform', which are now deprecated. 
 
* Function `vapour_warp_raster()` now exported, properly handles input-replacement geotransform or projection
 for sources that have insufficient metadata. This works with a variety of input data source names (files, URLs, 
 image servers, etc.) Overviews are automatically dealt with (by choosing the right level of detail) by using
 the gdalwarp app library functionality. (This warp function may be refactored to use extent rather than geotransform). 

* Added vsi example with zipped shapefile. 

* Fixed missing stdlib.h declaration for exit() in configure tests, thanks to CRAN. #88

* New function `vapour_read_type()` to return the integer geometry type (GDAL's
wkb enum code).

* `vapour_read_geometry(what = "point")` is now defunct

* Major refactor to use C++ headers, so other packages can access the API
without using R functions. There's a more comprehensive set of identifier,
names, fields, extent, and geometry readers, and an addition of the GDALwarp()
facility which is a generalization of RasterIO. This will now work on a much
greater range of sources than the older RasterIO (still WIP).
 
* New function `vapour_geom_name()` to get a data source geometry "column name".
Non-database sources return an empty string *unless* `ExecuteSQL()` was called,
and then it is "_ogr_geometry_".

Typical names are 'geom' in GPKG, 'SHAPE' in GDB. 


# vapour 0.5.5

* Deleted compile tests from configure, we can add proper tests if this is needed. 

* Now set env vars GDAL_DATA and PROJ_LIB to vapour/gdal/ and vapour/proj/ when present (i.e. Windows generally, MacOS on CRAN). 
* Update to configure to ensure data copy of proj/ and gdal/ on MacOS on CRAN, thanks to CRAN, Roger Bivand, and to James 
 Balamuta for enormous help on github actions and installing the CRAN mac binaries. 

* Now using GDAL 3 on Windows. 

* Get available overview sizes, wish of #78. 

# vapour 0.5.0

* Improve tests to be more robust, thanks to CRAN. #77

* Removed pesky message and failing tests thanks to Joseph Stachelek (@jsta)
 #76. 

# vapour 0.4.0

## BREAKING CHANGES

* The function `vapour_read_raster()` now returns a list of (one) numeric vector. This change 
 is so a future version can return more than one band for this and related functions. 
 
## NEW FEATURES

* New data set `tas_wkt` a Well-Known-Text string to simplify an example
 in the documentation. 
 
* The function `vapour_read_raster()` now returns a list with a single vector of
 values from the requested band. In future this may return values from multiple
 bands. The new function `vapour_warp_raster()` uses the same scheme.

* Fixed static linking for MacOS build, thanks(!!) to Jeroen Ooms for guidance 
 and thanks to CRAN for reporting the issue. 
 
* New function `vapour_srs_wkt()` to convert PROJ4 strings to Well-Known Text. 

* New *internal-only* function `vapour_warp_raster()` to return a warped version from
 a raster source. Please use with caution, it's still flaky for certain cases and will
 crash your session (setting projection on NetCDF 0,360 is one example but not fleshed
 out yet - see hypertidy/lazyraster for in-dev usage examples.)

* New internal function `warp_memory_cpp()` to warp small in memory rasters. 

# vapour 0.3.0

* Updates to configure to remove deprecated R CMD config CPP (thanks to CRAN). 

* Updates to enable use of the deprecated PROJ API. 

* New function `vapour_vsi_list()` to list contents of virtual files.  See https://twitter.com/mdsumner/status/1158335706483179521 and https://github.com/hypertidy/vapour/issues/55 for inspirations. 

* Added a hex sticker!  Thanks to @Maschette. 

* Added 'proj4' string as well as 'projection' (WKT) to the output of `vapour_raster_info()`  
 (these aren't always available, especially not for longlat NetCDF). 

* Fixed failure in `limit_n`, `skip_n` behaviour for reading names (FIDs) and attributes, 
 (#60).
 
# vapour 0.2.0

* Fixed incorrect use of C-level `printf()` and unnecessary tests thanks to CRAN 
 feedback. 
* Add 'type' to output of `vapour_geom_summary()`, the integer code for each 
 geometry type. 

* Restructured conversion to text geometries to properly free up memory, was 
 causing memory leaks (found with valgrind). 
 
* Use of `sql` now correctly uses 'GDALDataset::ReleaseResultsSet()' in each 
 applicable function. Query with the 'sql' argument was not being executed in 
 `vapour_layer_names()` but now is - only has utility for insert and drop queries, 
 so will be rarely used and probably never had any impact before. 

* Raster read gains a new argument `native = FALSE` to control use of the native 
 window without specifying it. If `native = TRUE` then the native dimensions 
 are used and read in full. 

* Vector read functions gain new `extent` argument to apply a spatial filter in 
 conjunction with the `sql` argument, per discussion (#34). The extent can be of 
 type sp, sf, raster, or generic vector `c(xmin, xmax, ymin, ymax)`.  The extent 
 is ignored if the `sql` argument is not specified, with a warning.  Applies to 
 `vapour_geom_summary()`,  `vapour_read_attributes()`, `vapour_read_extent()`, 
 `vapour_read_geometry()`, `vapour_read_geometry_text()`, and `vapour_read_names()`.
  Thanks to Joseph Stachelek for the suggestion. 

* Vector read functions gain new `limit_n` and `skip_n` arguments to specify a 
 sequential set of features to be read, added to `vapour_geom_summary()`,  
 `vapour_read_attributes()`, `vapour_read_extent()`, `vapour_read_geometry()`, 
 `vapour_read_geometry_text()`, and `vapour_read_names()`. Their effect occurs 
 after that of the `sql` argument. 
 
* New function `vapour_driver()` to determine chosen driver (vector, short name) 
 of the data source. 

* New argument `min_max` to allow fast default use of `vapour_raster_info()`, as 
 per (#50). 

* New function `vapour_raster_gcp()` to return GCP (ground control points) if 
 present. 

* New function `vapour_report_attributes()` to return a string description of 
 the internal GDAL type for data attributes wish of Grant Williamson. 
 
* Attribute types 'OFTInteger64' are now returned as doubles, 'Date' and 
 'DateTime' are  returned as strings. 

* New functions `vapour_gdal_version()` and `vapour_all_drivers()` to return 
 information about the GDAL library in use. 

* Function `vapour_read_names()` is now implemented using the library API, rather 
 than via OGRSQL.  All trace of the OGRSQL FID workaround and warning has been 
 removed. 

* New function `vapour_geom_summary()` to return basic identity, validity, and 
 extent of feature geometries. 

* Values for the extent (bounding box) of a feature are now set to NA values for 
 an empty or missing geometry. 

* Fix drastic memory leak in `vapour_read_attributes()`, thanks to Grant Williamson. 

# vapour 0.1.0

* Functions that include a `layer` index now accept a layer name which is used to find the (0-based) layer index. 

* This is a breaking release, all old functions have been made defunct or removed entirely. 

# vapour 0.0.1

* All C++ functions now have explicit R wrappers instead of Rcpp exports. 

* New naming convention uses `vapour_` for vector sources, `raster_` for raster sources 
 to make function names a little more consistent. 

* New wrapper `vapour_sds_names` to replace `sds_info` and `raster_sds_info` which are now deprecated. 

* New wrapper `vapour_read_geometry_what` to replace `vapour_read_feature_what` which i  is now deprecated. 

* New function  `vapour_read_names` to return the vector of FID values. 

* The read for raster data now returns numeric or integer appropriately. 

* `vapour_raster_info` now includes `bands` as the count of available bands. 

* The IO read now allows a 4-element `window` to return data at native resolution, by
 copying the third and fourth values (source dimension) to the fifth and sixth values
 (output dimension) respectively. 
 
* Subdatasets are now supported. 

* Added sanity check behaviour to `vapour_raster_read` to avoid out of bounds read attempts. 

* Resampling options added to raster data read. 

* Upgraded to rwinlib gdal 2.2.3. 

* added function `vapour_layer_names` to return character layer names, and
 implicitly provide a layer count

* feature iteration now avoids "GetFeatureCount" and collects each element
 in a growing list, thanks to Jim Hester

* renamed `format` argument to `textformat`, this is ignored unless `what = "text"`

* Created a single C++ feature read to remove repeated logic, `vapour_read_geometry`, `vapour_read_geometry_text`,  and `vapour_read_extent` all call the same function. 
