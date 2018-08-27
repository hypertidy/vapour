context("test-names")
file <- "list_locality_postcode_meander_valley.tab"
layer <- "list_locality_postcode_meander_valley"
mvfile <- system.file(file.path("extdata/tab", file), package="vapour")


test_that("multiplication works", {
  x <- vapour:::vapour_read_names(mvfile)

  expect_length(x, 58)
})
