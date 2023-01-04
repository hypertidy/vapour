#' Create vector VRT 
#' 
#' Vector ViRTual data source with options for layer, output projection, select query, SQL dialect, limit
#' on number of output objects. 
#' 
#' When 'crs' is used the name of the 'geom' is required, and will be determined as the source
#' default name. Because we want to have dynamic SQL for geometry functions we must also indicate
#' this name. The best way is to use 'select' subqueries like 'ConvexHull(geom) AS geom', this way the
#' geom name hasn't changed - but for complete control set in the select subquery AND in the geom arg. 
#'
#' @param dsn data source
#' @param layer layer (name or number)  FIXME 0- OR 1- based?
#' @param crs optional targect projection
#' @param select select subquery e.g. 'val1, val2, ConvexHull(geom) AS geom'
#' @param dialect  sql dialect (SQLITE or OGRSQL most important ones)
#' @param limit_n optional limit on number of output objects
#' @param geom must be provided when crs is used, and must match syntax in 'select' of geometry SQL (use 'AS')
#' 
#' @return character string vector data source name for GDAL 
#' @export
#'
#' @examples
#' dsn <- system.file("extdata/sst_c.gpkg", package = "vapour", mustWork = TRUE)
#' vrt <- vapour_vector_vrt(dsn, layer = "sst_c", select = "sst, ConvexHull(geom) as chull", 
#'    limit_n = 4, dialect = "sqlite", crs = "+proj=laea +lat_0=-90", debug = T, out_field = "chull")
vapour_vector_vrt <- function(dsn, layer = NULL,crs = NULL, select = "*", geom = NULL, dialect = "sqlite", limit_n = 0) {
  if (is.null(layer)) {
    layers <- sf::st_layers(dsn)$name
    layer <- layers[1L]
  }
  if (is.numeric(layer)) {
    layers <- sf::st_layers(dsn)$name
    layer <- layers[layer]
  }
  if (is.null(geom) && !is.null(crs))  {
    geom <- vapour::vapour_geom_name(dsn, layer)[1L]
  }

  src_sql <- NULL
  if (!select == "*") {
    if (limit_n > 0) {
      limit_n <- sprintf(" LIMIT %i", limit_n)
    } else {
      limit_n <- ""
    }
    src_sql <- sprintf('<SrcSQL dialect="%s">SELECT %s from %s %s</SrcSQL>', 
                       dialect, select, layer, limit_n)
    
  }
  
  if (!is.null(crs)) {
   out <- sprintf('<OGRVRTDataSource>
    <OGRVRTWarpedLayer>
        <OGRVRTLayer name="%s">
            <SrcDataSource>%s</SrcDataSource>
            %s
        </OGRVRTLayer>
        <WarpedGeomFieldName>%s</WarpedGeomFieldName>
        <TargetSRS>%s</TargetSRS>
    </OGRVRTWarpedLayer>
</OGRVRTDataSource>', layer, dsn, src_sql, geom, crs)
  } else {
 out <-       sprintf('<OGRVRTDataSource>
    <OGRVRTLayer name="%s">
        <SrcDataSource>%s</SrcDataSource>
          %s
    </OGRVRTLayer>
</OGRVRTDataSource>', layer, dsn, src_sql)

  }
  
#   if (debug) {
#         print(dsn)
#   print(layer)
#   print(geom)
#   print(crs)
# print(src_sql)
# 
#   }
out
}
