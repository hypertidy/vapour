#f <- tail(raadtools::sstfiles()$fullname, 1)
#f <- "/rdsi/PUBLIC/raad/data/eclipse.ncdc.noaa.gov/pub/OI-daily-v2/NetCDF/2017/AVHRR/avhrr-only-v2.20170910_preliminary.nc"
f <- 'NETCDF:"/rdsi/PUBLIC/raad/data/eclipse.ncdc.noaa.gov/pub/OI-daily-v2/NetCDF/2017/AVHRR/avhrr-only-v2.20170910_preliminary.nc":sst'
#f <-  "/rdsi/PUBLIC/raad/data/www.ngdc.noaa.gov/mgg/global/relief/ETOPO2/ETOPO2v2-2006/ETOPO2v2c/netCDF/ETOPO2v2c_f4.nc"

# required components
## TILING
## -[ ] line is a fall back, might have internal tiling
## -[ ] or impose an arbitrary one
## -[ ] needs index-alignment, tiling done in index space, converted to sf

## POLYGONS
## -[x] non-raster conversion to sf from extent/dims
## -[ ]
## EXTENT / BBOX
## -[x] a "discrete" class which is xmin, ymin, xmax, ymax, nx, ny
## -[x] conversion of GDAL 6-figure transform to "discrete"
## -[ ] tile index conversion to cell edge, so c(offsetX, offsetY, windowX, windowY) <-> c(xmin, ymin, xmax, ymax)
## -[ ] spatial extent <-> index extent, based on "discrete"
#' @param dim is the size of the grid
#' @param tile is the size of the tiles
tile_dim <- function(dim, tile = NULL) {
  tile <- if (is.null(tile)) c(dim[1], rep(1L, length(dim) -1)) else stopifnot(length(dim) == length(tile))
  tile
}

#' 6 figure transform to discrete axes
#' @param x 6 figure geotransform as returned by raster_info
#' @param dim 2 figure dimXY as returned by raster_info
transform6 <- function(x, dim) {
  disc_rete(mins = c(x[1], x[4] + dim[2] * x[6]),
            maxs = c(x[1] + dim[1] * x[2], x[4]),
            dims = dim)
}
dummy_discrete <- function() {
  data.frame(name = c("x", "y"), min = c(0, 0), max = c(3, 4), dim = c(3, 4))
}
#' table of axes
#' @param mins minimum coordinates
#' @param maxs maximum coordinates
#' @param dims axis length
#' @param name optional name for each axis
disc_rete <- function(mins, maxs, dims, name = NULL) {
  if (missing(mins)) return(dummy_discrete())
  stopifnot(length(mins) == length(maxs))
  stopifnot(length(mins) == length(dims))
  walphabet <- c(letters[-20], letters[20])
  if (is.null(name)) name <- walphabet[(seq_along(mins) + 21) %% 26 + 1]
  data.frame(name = name, min = mins, max = maxs, dim = dims)
}
#' @param x (ordered) row of axes
edge_coord <- function(x) {
 setNames(do.call(expand.grid, lapply(split(x, seq_len(nrow(x))), function(axisrow) {
   axis <- as.list(axisrow)
   seq(axis$min, axis$max, length = axis$dim + 1)
 }
 )), x$name)
}

pair_segs <- function (x) {
  cbind(head(x, -1), tail(x, -1))
}
pair_four <- function (xp, nc) {
  (xp + c(0, 0, rep(nc, 2)))[c(1, 2, 4, 3)]
}
#' build quad mesh from first two axes
#' @param x a discrete set of axes
#'
quad_mesh <- function(x, ...) {
  exy <- edge_coord(x[1:2, ])
  ncolumns <- x$dim[1]
  nrows <- x$dim[2]
  ind <- apply(pair_segs(seq(ncolumns + 1)), 1, pair_four, nc = ncolumns +
                 1)
  ind0 <- as.vector(ind) + rep(seq(0, length = nrows, by = ncolumns +
                                     1), each = 4 * ncolumns)
  list(vb = t(cbind(exy, 0, 1)), ib =  matrix(ind0, nrow = 4))

}
#' fast creation of simple features tiles from abstract
#' axis specification
tile_gon <- function(x, ...) {
  qm <- quad_mesh(x)
  ## spex:::polygonize.RasterLayer
  l <- lapply(split(t(qm$vb[1:2, qm$ib]), rep(seq_len(ncol(qm$ib)),
                                              each = 4)), function(x) structure(list(matrix(x, ncol = 2)[c(1,
                                                                                                           2, 3, 4, 1), ]), class = c("XY", "POLYGON", "sfg")))
  st_sf(geometry = st_sfc(l))
}

## let's assume we did this without raster
# library(raster)
# r <- raster(f)
# projection(r) <- "+init=epsg:4326"
# tiles <- spex::polygonize(raster(raster::extent(r), crs = projection(r),
#                                  nrow = nrow(r) / 240, ncol = ncol(r)/240))
ri <- raster_info(f)
tiles <- tile_gon(transform6(ri$geotransform, ri$dimXY/240))


populate_raster <- function(n, tile = NULL, data = NULL) {
  out <- replicate(n, NULL)
  ##, tibble::as_tibble(list(data = numeric())))
  if (!is.null(tile)) out[[tile]] <- tibble::tibble(data = data)
  out
}
tiles$raster_data <- populate_raster(nrow(tiles))
library(sf)
library(spex)
extent <- function(x, ...) UseMethod("extent")
extent.sfc_POLYGON <- function(x, ...) raster::extent(attr(x, "bbox")[c("xmin", "xmax", "ymin", "ymax")])

gdal_source <- function(obj, geom) {
  ex <- tabularaster:::index_extent(obj, extent(geom))
  offset_source_x <- c(xmin(ex), xmax(ex))
  offset_source_y <- c(nrow(obj) - c(ymax(ex), ymin(ex)))
  as.vector(rbind(offset_source_x, offset_source_y))
}

itile <- 1
library(sf)
gs <- gdal_source(r, st_geometry(tiles[itile, ]))
library(vapour)
tiles$raster_data <- populate_raster(nrow(tiles), a$tile, raster_io(f, c(gs, c(diff(gs[c(1, 3)])+1, diff(gs[c(2, 3)])+1))))
tiles$tile <- seq_len(nrow(tiles))
par(mfrow = c(2, 1))
plot(crop(r, tiles[itile, ]), asp = "")
plot(tiles$raster_data[[itile]]$data, pch = ".")


library(mapedit)
a <- selectFeatures(tiles)
gs <- gdal_source(r, st_geometry(a))
tiles$raster_data <- populate_raster(nrow(tiles), a$tile, raster_io(f, c(gs, c(diff(gs[c(1, 3)])+1, diff(gs[c(2, 3)])+1))))
par(mfrow = c(2, 1))
plot(crop(r, tiles[itile, ]), asp = "")
plot(tiles$raster_data[[a$tile]]$data, pch = ".")


