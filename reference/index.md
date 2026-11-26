# Package index

## Package overview

- [`vapour-package`](https://hypertidy.github.io/vapour/reference/vapour-package.md)
  [`vapour`](https://hypertidy.github.io/vapour/reference/vapour-package.md)
  : vapour

## Raster information

Structural metadata, subdatasets, control points and geolocation.

- [`vapour_raster_info()`](https://hypertidy.github.io/vapour/reference/vapour_raster_info.md)
  : Raster information
- [`vapour_sds_names()`](https://hypertidy.github.io/vapour/reference/vapour_sds_names.md)
  : GDAL raster subdatasets (variables)
- [`vapour_sds()`](https://hypertidy.github.io/vapour/reference/vapour_sds.md)
  : Subdatasets of raster sources
- [`vapour_raster_gcp()`](https://hypertidy.github.io/vapour/reference/vapour_raster_gcp.md)
  : Raster ground control points
- [`vapour_geolocation()`](https://hypertidy.github.io/vapour/reference/vapour_geolocation.md)
  : Retrieve geolocation information for a dataset

## Raster data

Read, warp and create raster data.

- [`vapour_read_raster()`](https://hypertidy.github.io/vapour/reference/vapour_read_raster.md)
  : Raster IO (read)

- [`vapour_read_raster_raw()`](https://hypertidy.github.io/vapour/reference/vapour_read_raster_raw.md)
  [`vapour_read_raster_int()`](https://hypertidy.github.io/vapour/reference/vapour_read_raster_raw.md)
  [`vapour_read_raster_dbl()`](https://hypertidy.github.io/vapour/reference/vapour_read_raster_raw.md)
  [`vapour_read_raster_chr()`](https://hypertidy.github.io/vapour/reference/vapour_read_raster_raw.md)
  [`vapour_read_raster_hex()`](https://hypertidy.github.io/vapour/reference/vapour_read_raster_raw.md)
  : type safe(r) raster read

- [`vapour_read_raster_block()`](https://hypertidy.github.io/vapour/reference/vapour_read_raster_block.md)
  : Read or write raster block

- [`vapour_write_raster_block()`](https://hypertidy.github.io/vapour/reference/vapour_write_raster_block.md)
  :

  Write data to a block *in an existing file*.

- [`vapour_warp_raster()`](https://hypertidy.github.io/vapour/reference/vapour_warp_raster.md)
  : Raster warper (reprojection)

- [`vapour_warp_raster_raw()`](https://hypertidy.github.io/vapour/reference/vapour_warp_raster_raw.md)
  [`vapour_warp_raster_int()`](https://hypertidy.github.io/vapour/reference/vapour_warp_raster_raw.md)
  [`vapour_warp_raster_dbl()`](https://hypertidy.github.io/vapour/reference/vapour_warp_raster_raw.md)
  [`vapour_warp_raster_chr()`](https://hypertidy.github.io/vapour/reference/vapour_warp_raster_raw.md)
  [`vapour_warp_raster_hex()`](https://hypertidy.github.io/vapour/reference/vapour_warp_raster_raw.md)
  : type safe(r) raster warp

- [`vapour_create_options()`](https://hypertidy.github.io/vapour/reference/vapour_create.md)
  [`vapour_create()`](https://hypertidy.github.io/vapour/reference/vapour_create.md)
  : Create raster file

- [`gdal_raster_data()`](https://hypertidy.github.io/vapour/reference/gdal_raster_data.md)
  [`gdal_raster_dsn()`](https://hypertidy.github.io/vapour/reference/gdal_raster_data.md)
  [`gdal_raster_image()`](https://hypertidy.github.io/vapour/reference/gdal_raster_data.md)
  [`gdal_raster_nara()`](https://hypertidy.github.io/vapour/reference/gdal_raster_data.md)
  : General raster read and convert

## Vector layers

Inspect layers, geometry columns and summaries.

- [`vapour_layer_names()`](https://hypertidy.github.io/vapour/reference/vapour_layer_names.md)
  : Read GDAL layer names
- [`vapour_layer_info()`](https://hypertidy.github.io/vapour/reference/vapour_layer_info.md)
  : Read GDAL layer info
- [`vapour_layer_extent()`](https://hypertidy.github.io/vapour/reference/vapour_layer_extent.md)
  : Read layer extent
- [`vapour_geom_name()`](https://hypertidy.github.io/vapour/reference/vapour_geom_name.md)
  : Read geometry column name
- [`vapour_geom_summary()`](https://hypertidy.github.io/vapour/reference/vapour_geom_summary.md)
  : Summary of available geometry

## Vector geometry and fields

Read geometry, feature identifiers and attribute fields.

- [`vapour_read_geometry_ia()`](https://hypertidy.github.io/vapour/reference/vapour_read_geometry.md)
  [`vapour_read_geometry_ij()`](https://hypertidy.github.io/vapour/reference/vapour_read_geometry.md)
  [`vapour_read_geometry()`](https://hypertidy.github.io/vapour/reference/vapour_read_geometry.md)
  [`vapour_read_geometry_text()`](https://hypertidy.github.io/vapour/reference/vapour_read_geometry.md)
  [`vapour_read_extent()`](https://hypertidy.github.io/vapour/reference/vapour_read_geometry.md)
  [`vapour_read_type()`](https://hypertidy.github.io/vapour/reference/vapour_read_geometry.md)
  : Read GDAL feature geometry
- [`vapour_read_fids()`](https://hypertidy.github.io/vapour/reference/vapour_read_fids.md)
  [`vapour_read_names()`](https://hypertidy.github.io/vapour/reference/vapour_read_fids.md)
  : Read feature names
- [`vapour_read_fields()`](https://hypertidy.github.io/vapour/reference/vapour_read_fields.md)
  [`vapour_read_attributes()`](https://hypertidy.github.io/vapour/reference/vapour_read_fields.md)
  : Read feature field data
- [`vapour_report_fields()`](https://hypertidy.github.io/vapour/reference/vapour_report_fields.md)
  [`vapour_report_attributes()`](https://hypertidy.github.io/vapour/reference/vapour_report_fields.md)
  : Read feature field types.

## CRS, GDAL and configuration

Projection strings, versions, drivers and GDAL config.

- [`vapour_srs_wkt()`](https://hypertidy.github.io/vapour/reference/vapour_srs_wkt.md)
  : PROJ4 string to WKT
- [`vapour_crs_is_lonlat()`](https://hypertidy.github.io/vapour/reference/vapour_crs_is_lonlat.md)
  : Is the CRS string representative of angular coordinates
- [`vapour_proj_data()`](https://hypertidy.github.io/vapour/reference/vapour_proj_data.md)
  : Report on the PROJ data directory in use
- [`vapour_set_config()`](https://hypertidy.github.io/vapour/reference/vapour_set_config.md)
  [`vapour_get_config()`](https://hypertidy.github.io/vapour/reference/vapour_set_config.md)
  : Set and query GDAL configuration options
- [`vapour_gdal_version()`](https://hypertidy.github.io/vapour/reference/GDAL-library.md)
  [`vapour_proj_version()`](https://hypertidy.github.io/vapour/reference/GDAL-library.md)
  [`vapour_all_drivers()`](https://hypertidy.github.io/vapour/reference/GDAL-library.md)
  [`vapour_driver()`](https://hypertidy.github.io/vapour/reference/GDAL-library.md)
  : GDAL version and drivers.

## Virtual sources

VRT construction and virtual file system listings.

- [`vapour_vrt()`](https://hypertidy.github.io/vapour/reference/vapour_vrt.md)
  : Virtual raster
- [`vector_vrt()`](https://hypertidy.github.io/vapour/reference/vector_vrt.md)
  : Vector VRT
- [`buildvrt()`](https://hypertidy.github.io/vapour/reference/buildvrt.md)
  : Build vrt, special case "-separate"
- [`vapour_vsi_list()`](https://hypertidy.github.io/vapour/reference/vapour_vsi_list.md)
  : Read GDAL virtual source contents

## Example data

- [`sst_c`](https://hypertidy.github.io/vapour/reference/sst_c.md) : SST
  contours
- [`tas_wkt`](https://hypertidy.github.io/vapour/reference/tas_wkt.md) :
  Example WKT coordinate reference system
