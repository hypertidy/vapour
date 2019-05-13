context("test-limit_skip_n")

file <- "list_locality_postcode_meander_valley.tab"
f <- system.file(file.path("extdata/tab", file), package="vapour")

test_that("limit_n is robust", {
  expect_length(vapour_read_names(f,  limit_n = 4, skip_n = 9), 4L)  ## BUG!!!
  expect_length(vapour_read_geometry(f,  limit_n = 4, skip_n = 9), 4L)
  expect_length(vapour_read_geometry_text(f,  limit_n = 4, skip_n = 9), 4L)
  expect_length(vapour_geom_summary(f,  limit_n = 4, skip_n = 9)[[1]], 4L)
  expect_length(vapour_read_attributes(f,  limit_n = 4, skip_n = 9)[[1]], 4L)
  expect_length(vapour_read_extent(f,  limit_n = 4, skip_n = 9), 4L)

  ## there are 58 features
  expect_length(vapour_read_names(f,  limit_n = 4, skip_n = 56), 2L)
  expect_length(vapour_read_geometry(f,  limit_n = 4, skip_n = 56), 2L)
  expect_length(vapour_read_geometry_text(f,  limit_n = 4, skip_n = 56), 2L)
  expect_length(vapour_read_attributes(f,  limit_n = 4, skip_n = 56)[[1]], 2L)
  expect_length(vapour_read_extent(f,  limit_n = 4, skip_n = 56), 2L)

  expect_equivalent(vapour_read_names(f), 1:58)
  expect_equivalent(vapour_read_names(f, skip_n = 56), 57:58)
  expect_equivalent(vapour_read_names(f, skip_n = 56, limit_n = 4), 57:58)
  expect_equivalent(vapour_read_names(f, skip_n = 56, limit_n = 1), 57)
  expect_equivalent(vapour_read_names(f, sql = "SELECT FID FROM list_locality_postcode_meander_valley WHERE FID < 10"),
                    1:9)

  expect_equivalent(vapour_read_names(f, sql = "SELECT FID FROM list_locality_postcode_meander_valley WHERE FID < 10",
                                      limit_n = 4, skip_n = 3),
                    4:7)
  expect_equivalent(lengths(vapour_geom_summary(f,  limit_n = 4, skip_n = 9)), rep(4L, 7))

  expect_equal(lengths(vapour_geom_summary(f,  limit_n = 4, skip_n = 9)),
               setNames(rep(4L, 7),  c("FID", "valid_geometry", "type", "xmin", "xmax", "ymin", "ymax")) )
})
