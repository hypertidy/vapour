# vapour

A lightweight GDAL API package for R.

## Details

Provides low-level access to 'GDAL' functionality for R packages. The
aim is to minimize the level of interpretation put on the 'GDAL'
facilities, to enable direct use of it for a variety of purposes. 'GDAL'
is the 'Geospatial Data Abstraction Library' a translator for raster and
vector geospatial data formats that presents a single raster abstract
data model and single vector abstract data model to the calling
application for all supported formats <https://gdal.org/>.

Lightweight means we access parts of the GDAL API as near as possible to
their native usage. GDAL is not a lightweight library, but provides a
very nice abstraction over format details for a very large number of
different formats.

Functions for raster and vector sources are included.

|  |  |
|----|----|
| [`vapour_all_drivers`](https://hypertidy.github.io/vapour/reference/GDAL-library.md) | list of all available drivers, with type and features |
| [`vapour_driver`](https://hypertidy.github.io/vapour/reference/GDAL-library.md) | report short name of driver that will be used for a data source |
| [`vapour_gdal_version`](https://hypertidy.github.io/vapour/reference/GDAL-library.md) | report version of GDAL in use |
| [`vapour_srs_wkt`](https://hypertidy.github.io/vapour/reference/vapour_srs_wkt.md) | produce WKT projection string from various projection string inputs |
| [`vapour_vsi_list`](https://hypertidy.github.io/vapour/reference/vapour_vsi_list.md) | report contents of VSI sources |

|  |  |
|----|----|
| [`vapour_raster_gcp`](https://hypertidy.github.io/vapour/reference/vapour_raster_gcp.md) | return internal ground control points, if present |
| [`vapour_raster_info`](https://hypertidy.github.io/vapour/reference/vapour_raster_info.md) | structural metadata of a source |
| [`vapour_read_raster`](https://hypertidy.github.io/vapour/reference/vapour_read_raster.md) | read data direct from a window of a raster band source |
| [`vapour_sds_names`](https://hypertidy.github.io/vapour/reference/vapour_sds_names.md) | list individual raster sources in a source containing subdatasets |
| [`vapour_warp_raster`](https://hypertidy.github.io/vapour/reference/vapour_warp_raster.md) | read data direct from a raster source into a specific window |

|  |  |
|----|----|
| [`vapour_driver`](https://hypertidy.github.io/vapour/reference/GDAL-library.md) | report name of the driver used for a given source |
| [`vapour_geom_name`](https://hypertidy.github.io/vapour/reference/vapour_geom_name.md) | report attribute name of geometry |
| [`vapour_geom_summary`](https://hypertidy.github.io/vapour/reference/vapour_geom_summary.md) | report simple properties of each feature geometry |
| [`vapour_layer_names`](https://hypertidy.github.io/vapour/reference/vapour_layer_names.md) | list names of vector layers in a data source |
| [`vapour_layer_info`](https://hypertidy.github.io/vapour/reference/vapour_layer_info.md) | list of data source, driver, layer name/s, fields, feature count, projection |
| [`vapour_read_extent`](https://hypertidy.github.io/vapour/reference/vapour_read_geometry.md) | read the extent, or bounding box, of geometries in a layer |
| [`vapour_feature_extent`](https://hypertidy.github.io/vapour/reference/vapour_read_geometry.md) | the same extents as a matrix, one row for each feature |
| [`vapour_read_fields`](https://hypertidy.github.io/vapour/reference/vapour_read_fields.md) | read attributes of features in a layer, the columnar data associated with each geometry |
| [`vapour_read_geometry`](https://hypertidy.github.io/vapour/reference/vapour_read_geometry.md) | read geometry in binary (blob, WKB) form |
| [`vapour_read_geometry_fa`](https://hypertidy.github.io/vapour/reference/vapour_read_geometry.md) | read geometry by feature id, with [`vapour_read_fields_fa`](https://hypertidy.github.io/vapour/reference/vapour_read_fields.md) for fields. The `_ia` and `_ij` forms are also there for fields and for feature ids |
| [`vapour_read_geometry_ia`](https://hypertidy.github.io/vapour/reference/vapour_read_geometry.md) | read geometry by index, arbitrary |
| [`vapour_read_geometry_ij`](https://hypertidy.github.io/vapour/reference/vapour_read_geometry.md) | read geometry by sequential index, i to j |
| [`vapour_read_geometry_text`](https://hypertidy.github.io/vapour/reference/vapour_read_geometry.md) | read geometry in text form, various formats |
| [`vapour_read_names`](https://hypertidy.github.io/vapour/reference/vapour_read_fids.md) | read the 'names' of features in a layer, the 'FID' |
| [`vapour_read_type`](https://hypertidy.github.io/vapour/reference/vapour_read_geometry.md) | read the GDAL types of attributes |
| [`vapour_report_fields`](https://hypertidy.github.io/vapour/reference/vapour_report_fields.md) | report internal type of each attribute by name |

As far as possible vapour aims to minimize the level of interpretation
provided for the functions, so that developers can choose how things are
implemented. Functions return raw lists or vectors rather than data
frames or classed types.

## GDAL messages

Errors and warnings from GDAL are signalled as R conditions. When a call
fails, GDAL's reason is part of the error message. When a call returns,
each message GDAL raised along the way is an R warning that starts with
`GDAL warning` or `GDAL error`, so
[`suppressWarnings()`](https://rdrr.io/r/base/warning.html),
[`tryCatch()`](https://rdrr.io/r/base/conditions.html) and
`options(warn = 2)` work on them as on any other warning.

## Open options

Functions that read a source take `open_options`, a character vector of
`"NAME=VALUE"` strings for the driver, as `-oo` in the GDAL command line
tools. `NULL`, [`character()`](https://rdrr.io/r/base/character.html)
and `""` all mean none. The options each driver accepts are listed on
its page at <https://gdal.org/en/stable/drivers/>.

## options

The following options can be set to control global behaviour.

|                                    |                                |
|------------------------------------|--------------------------------|
| `Sys.getenv("vapour.sql.dialect")` | the current SQL dialect in use |

## SQL dialect

The SQL dialect can be set to "" (empty string), "OGRSQL", or "SQLITE".

The empty string indicates that the native dialect will be used, see
[OGRSQL and SQLITE for
GDAL](https://gdal.org/en/stable/user/ogr_sql_sqlite_dialect.html) and
the [GDAL_DMD_SUPPORTED_SQL_DIALECTS development
documentation](https://gdal.org/en/stable/api/raster_c_api.html).

Setting "NATIVE" as an alias for "" is quite recent and has not been
tested with vapour, similarly no testing has been done with non
OGRSQL-native or SQLITE-native drivers yet.

## See also

Useful links:

- <https://github.com/hypertidy/vapour>

- <https://hypertidy.github.io/vapour/>

- Report bugs at <https://github.com/hypertidy/vapour/issues>

## Author

**Maintainer**: Michael Sumner <mdsumner@gmail.com>
([ORCID](https://orcid.org/0000-0002-2471-7511))

Authors:

- Michael Sumner <mdsumner@gmail.com>
  ([ORCID](https://orcid.org/0000-0002-2471-7511))

Other contributors:

- Simon Wotherspoon (RasterIO configuration for resampling options)
  \[contributor\]

- Mark Padgham (helped get started :)) \[contributor\]

- Edzer Pebesma (wrote the field-read handling, adapted here from sf)
  \[contributor\]

- Roger Bivand (wrote configure.ac, adapted here from rgdal)
  \[contributor\]

- Jim Hester (wrote CollectorList.h, copied here from fs package)
  \[contributor, copyright holder\]

- Timothy Keitt (wrote GetPointsInternal copied here from rgdal2
  package) \[contributor\]

- Jeroen Ooms (tweaked build process, provided Windows build tools)
  \[contributor\]

- Dale Maschette (created the hex logo) \[contributor\]

- Even Rouault (primary author of the COG format and its use of the
  GDALwarp app-library, example code used by the warper function here)
  \[contributor\]

- Robert Hijmans (code in terra package used as example/inspiration)
  \[contributor\]

- Dewey Dunnington (wrote the columnar-access mode streaming Arrow
  support) \[contributor\]

- Tomas Kalibera \[contributor\]
