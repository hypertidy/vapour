test_that("tiles works", {
  expect_silent(vapour_tilexyz("https://tile.openstreetmap.org/${z}/${x}/${y}.png"))
})
