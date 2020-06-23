library(vapour)
#srcfile <- "D:\\data\\topography\\etopo2\\subset.tif"
#srcfile <- raadtools::topofile("etopo1")
srcfile <- "/rdsi/PRIVATE/raad/data_local/www.bodc.ac.uk/gebco/GRIDONE_2D.nc"
#srcfile <- "/rdsi/PUBLIC/raad/data/www.ngdc.noaa.gov/mgg/global/relief/ETOPO1/data/ice_surface/grid_registered/netcdf/ETOPO1_Ice_g_gdal.grd"
#srcfile <- "/rdsi/PUBLIC/raad/data/www.ngdc.noaa.gov/mgg/global/relief/ETOPO2/ETOPO2v2-2006/ETOPO2v2c/netCDF/ETOPO2v2c_f4.nc"
library(gladr)
if (FALSE) plot_raster(srcfile)

ui <- fluidPage(
  # Some custom CSS for a smaller font for preformatted text
  tags$head(
    tags$style(HTML("
                    pre, table.table {
                    font-size: smaller;
                    }
                    "))
    ),

  fluidRow(   column(width = 4, wellPanel(
    radioButtons("plot_type", "Plot type",
                 c("base", "ggplot2")
    )
  )),
    column(width = 12,
           # In a plotOutput, passing values for click, dblclick, hover, or brush
           # will enable those interactions.
           plotOutput("plot1", height = 750,
                      # Equivalent to: click = clickOpts(id = "plot_click")
                      click = "plot_click",
                      dblclick = dblclickOpts(
                        id = "plot_dblclick"
                      ),
                      hover = hoverOpts(
                        id = "plot_hover"
                      ),
                      ## see here for reset https://stackoverflow.com/questions/30588472/is-it-possible-to-clear-the-brushed-area-of-a-plot-in-shiny
                      brush = brushOpts(
                        id = "plot_brush"
                      )
           )

           ))
)


server <- function(input, output) {
  get_extent <- reactive({
    if (is.null(input$plot_brush)) NULL else
      raster::extent(input$plot_brush$xmin,
                     input$plot_brush$xmax,
                     input$plot_brush$ymin,
                     input$plot_brush$ymax)
  })
  output$plot1 <- renderPlot({
      plot_raster(srcfile, ext = get_extent())

  })

}


shinyApp(ui, server)
