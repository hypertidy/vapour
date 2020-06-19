context("rasterio-resampling")
skip_on_cran()
f <- system.file("extdata", "sst.tif", package = "vapour")
## writeLines(paste(as.character(vapour_read_raster(f, window = c(0, 0, 10, 10, 5, 5))), collapse = "\", \""))

l <- list(
  average = c("285.728515625", "285.713256835938", "285.903015136719", "286.009765625", "285.931243896484", "285.712249755859", "285.695007324219", "285.798492431641", "286.095001220703", "286.252014160156", "286.017761230469", "285.896484375", "285.907257080078", "286.305755615234", "286.400024414062", "286.205261230469", "286.117980957031", "286.002990722656", "286.320251464844", "286.643005371094", "286.016754150391", "285.974731445312", "286.084014892578", "286.392242431641", "286.851257324219"),
  bilinear = c("285.715454101562", "285.726593017578", "285.881713867188", "286.005065917969", "285.970733642578", "285.743774414062", "285.734710693359", "285.828369140625", "286.099853515625", "286.193969726562", "285.988464355469", "285.904418945312", "285.943695068359", "286.277465820312", "286.372406005859", "286.178802490234", "286.087890625", "286.035186767578", "286.323547363281", "286.600769042969", "286.011291503906", "285.991668701172", "286.09326171875", "286.394104003906", "286.810546875"),
  cubic = c("285.711334228516", "285.712432861328", "285.892425537109", "286.010803222656", "285.956298828125", "285.724029541016", "285.710632324219", "285.796112060547", "286.107299804688", "286.227142333984", "286.002593994141", "285.891815185547", "285.917694091797", "286.314727783203", "286.399078369141", "286.22216796875", "286.113189697266", "286.004669189453", "286.321380615234", "286.630737304688", "286.026580810547", "285.972930908203", "286.075042724609", "286.382568359375", "286.858764648438"),
  cubicspline = c("285.713073730469", "285.737335205078", "285.871551513672", "286.002838134766", "285.988037109375", "285.762329101562", "285.759338378906", "285.857299804688", "286.093475341797", "286.158905029297", "285.972930908203", "285.912841796875", "285.966583251953", "286.249298095703", "286.349060058594", "286.141876220703", "286.066925048828", "286.060699462891", "286.324279785156", "286.570434570312", "286.003326416016", "286.005706787109", "286.108245849609", "286.404083251953", "286.768859863281"),
 # gauss = c("285.714050292969", "285.755676269531", "285.926361083984", "286.050750732422", "285.911529541016", "285.778381347656", "285.738555908203", "285.862548828125", "286.222137451172", "286.084442138672", "286.045196533203", "285.934112548828", "286.066009521484", "286.410064697266", "286.282135009766", "286.205322265625", "286.063385009766", "286.037506103516", "286.39599609375", "286.641082763672", "285.914123535156", "285.957885742188", "286.205993652344", "286.546569824219", "286.204986572266"),
  lanczos = c("285.711212158203", "285.708770751953", "285.900024414062", "286.015258789062", "285.958587646484", "285.721984863281", "285.710632324219", "285.773315429688", "286.109130859375", "286.242248535156", "286.003601074219", "285.882934570312", "285.907470703125", "286.340270996094", "286.412628173828", "286.244323730469", "286.134582519531", "285.989410400391", "286.317565917969", "286.63525390625", "286.022735595703", "285.955749511719", "286.0615234375", "286.357391357422", "286.859344482422"),
  mode = c("285.720001220703", "285.631011962891", "285.825012207031", "285.933990478516", "285.903991699219", "285.648986816406", "285.670989990234", "285.768005371094", "285.934997558594", "286.308990478516", "285.911987304688", "285.894012451172", "285.760009765625", "286.10400390625", "286.380004882812", "286.135009765625", "286.140014648438", "286.014007568359", "286.361999511719", "286.606994628906", "286.079010009766", "286.121002197266", "285.975006103516", "286.226989746094", "286.600006103516"),
  nearestneighbour = c("285.743011474609", "285.786010742188", "285.998992919922", "286.063995361328", "286.037994384766", "285.763000488281", "285.697998046875", "285.830993652344", "286.265014648438", "286.134002685547", "286.070007324219", "285.930999755859", "286.092010498047", "286.450012207031", "286.501007080078", "286.264007568359", "286.075988769531", "285.915008544922", "286.334991455078", "286.701995849609", "285.869995117188", "285.940002441406", "286.252014160156", "286.549011230469", "287.141998291016")
)


## todo reinstate a test with gauss
test_that("resampling works", {
  for (i in seq_along(l)) {
    ## just check their length
    expect_equal(length(as.numeric(l[[i]])),
                 length(vapour_read_raster(f, window = c(0, 0, 10, 10, 5, 5), resample = names(l)[i])[[1]]))
  }

  expect_warning(vapour_read_raster(f, window = c(0, 0, 10, 10, 10, 10), resample = "idontexist"), "resample mode 'idontexist' is unknown")
})
test_that("window handling is sane", {

  expect_silent(vapour_read_raster(f, window = c(0, 0, 10, 10)))
  expect_error(vapour_read_raster(f, window = c(-1, 0, 10, 10)), "window cannot index lower than 0")
  expect_error(vapour_read_raster(f, window = c(0, -1, 10, 10)), "window cannot index lower than 0")
  expect_error(vapour_read_raster(f, window = c(0, 0, -1, 10)), "window size cannot be less than 1")
  expect_error(vapour_read_raster(f, window = c(0, 0, 10, -1)), "window size cannot be less than 1")

  expect_error(vapour_read_raster(f, window = c(142, 0, 10, 10)), "window size cannot exceed grid dimension")


  expect_error(vapour_read_raster(f, window = c(140, 0, 10, 10)), "window size cannot exceed grid dimension")
  expect_error(vapour_read_raster(f, window = c(0, 10, 10, 1000)), "window size cannot exceed grid dimension")

  expect_error(vapour_read_raster(f, window = c(0, 0, 10, 10, -2, 5)), "requested output dimension cannot be less than 1")
  expect_error(vapour_read_raster(f, window = c(0, 0, 10, 10, 5, -2)), "requested output dimension cannot be less than 1")

  })


## test gcps
test_that("gcps work", {
  expect_output(gcp_null <- vapour_raster_gcp(f), "No GCP \\(ground control points\\) found.")
  expect_that(length(gcp_null), equals(6L))
  expect_that(length(unlist(gcp_null)), equals(0L))
  expect_named(gcp_null, c("Pixel", "Line", "X", "Y", "Z", "CRS"))
  gcpfile <- system.file("extdata/gcps/volcano_gcp.tif", package = "vapour",mustWork = TRUE )
  gcp_real <- vapour_raster_gcp(gcpfile)

  expect_that(length(gcp_real), equals(6L))
  expect_that(length(unlist(gcp_real)), equals(16L))
  expect_named(gcp_real, c("Pixel", "Line", "X", "Y", "Z", "CRS"))

  expect_equal(unlist(gcp_real[-6L]), c(Pixel1 = 0, Pixel2 = 5, Pixel3 = 20, Line1 = 0, Line2 = 5,
                                   Line3 = 15, X1 = 100, X2 = 200, X3 = 300, Y1 = 100, Y2 = 200,
                                   Y3 = 300, Z1 = 0, Z2 = 0, Z3 = 0))
})


test_that("band sanity prevails", {
  f1 <- system.file("extdata/gdal/geos_rad.nc", package = "vapour", mustWork = TRUE)
  ## doesn't work on CRAN mac GDAL 2.4.2 and PROJ 5.2.0
  tst <- try(vapour_sds_names(f1), silent = TRUE)
  canopen <- !inherits(tst, "try-error")
  if (!canopen) skip("unable to open netcdf file on this platform")
  expect_equivalent(unique(vapour_read_raster(f1, native = TRUE, band = 1)[[1]]), 129.0)


  expect_error(vapour_read_raster(f1))
  expect_silent(vapour_read_raster(f1, native = TRUE, band = 1))

  expect_error(vapour_read_raster(f1, native = TRUE, band = NA))
  expect_error(vapour_read_raster(f1, native = TRUE, band = 0))
  expect_error(vapour_read_raster(f1, native = TRUE, band = 2))
  expect_error(vapour_read_raster(f1, native = TRUE, band = ""))
  expect_warning(vapour_read_raster(f1, window = c(0, 0, 5, 5, 8, 8), native = TRUE))
})



