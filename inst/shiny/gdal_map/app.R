library(vapour)
#srcfile <- "D:\\data\\topography\\etopo2\\subset.tif"
#srcfile <- raadtools::topofile("etopo2")
srcfile <- "/rdsi/PRIVATE/raad/data_local/www.bodc.ac.uk/gebco/GRIDONE_2D.nc"
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

  fluidRow(
    column(width = 12,
           # In a plotOutput, passing values for click, dblclick, hover, or brush
           # will enable those interactions.
           plotOutput("plot1", height = 750,
                      brush = brushOpts(
                        id = "plot_brush"
                      ))))
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
