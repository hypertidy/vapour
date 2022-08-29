#' Tile servers as VRT string
#'
#' first argument is a tile server template such as 'https://tile.openstreetmap.org/${z}/${x}/${y}.png'
#' GDAL expects the '${z}' and so forth pattern. See GDAL doc for 'WMS' driver, the TMS minidriver 
#' specification. This is just a helper function to put the right Mercator extent on. 
#' 
#' You might want to modify the band count, projection, xmin,xmax,ymin,ymax for non global or
#' non Mercator tile servers. 
#' @param x tile server see Details
#' @param user_agent 
#' @param bands_count 
#' @param block_size 
#' @param projection 
#' @param y_origin 
#' @param tile_count 
#' @param tile_level 
#' @param xmin 
#' @param xmax 
#' @param ymin 
#' @param ymax 
#' @param silent 
#'
#' @return GDAL xml text string for a tiled xyz service
#' @noRd
#'
#' @examples
#' osm_src <- vapour:::.vapour_tilexyz()
#' bm_src <- vapour:::.vapour_tilexyz("http://s3.amazonaws.com/com.modestmaps.bluemarble/${z}-r${y}-c${x}.jpg", 
#'                  tile_level = 9L)
#' ## these are tile server sources useable by raster,stars,terra,python rasterio, etc
#' writeLines(bm_src, tfile <- tempfile(fileext = ".vrt"))
vapour_tilexyz <- function(x, 
                           user_agent = getOption("HTTPUserAgent"), 
                           bands_count = 3L,
                           block_size = c(256L, 256L), 
                           projection = "EPSG:3857", 
                           y_origin = "top", 
                           tile_count = c(1L, 1L),
                           tile_level = 18L, 
                           xmin = -20037508.34, xmax = 20037508.34, 
                           ymin = -20037508.34, ymax = 20037508.34, 
                           silent = FALSE) {
  if (missing(x)) {
    if (!silent) {
    message("no tile server provided, using OSM as a default (assuming global mercator, 3 band image, etc. see args to vapour_tilexyz()")
    message("\n\n")
    }
    x <- "https://tile.openstreetmap.org/${z}/${x}/${y}.png"
  }
  sprintf('<GDAL_WMS>
    <Service name="TMS">
        <ServerUrl>%s</ServerUrl>
    </Service>
    <DataWindow>
        <UpperLeftX>%f</UpperLeftX>
        <UpperLeftY>%f</UpperLeftY>
        <LowerRightX>%f</LowerRightX>
        <LowerRightY>%f</LowerRightY>
        <TileLevel>%i</TileLevel>
        <TileCountX>%i</TileCountX>
        <TileCountY>%i</TileCountY>
        <YOrigin>%s</YOrigin>
    </DataWindow>
    <Projection>%s</Projection>
    <BlockSizeX>%i</BlockSizeX>
    <BlockSizeY>%i</BlockSizeY>
    <BandsCount>%i</BandsCount>
    <UserAgent>%s</UserAgent>
</GDAL_WMS>', 
          x, 
          xmin, ymax, xmax, ymin,
          tile_level, tile_count[1L], tile_count[2L], 
          y_origin, 
          projection, 
          block_size[1L], block_size[2L], bands_count, user_agent)
}
