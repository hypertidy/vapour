d <- tibble::tibble(index = 1:5, name = c("one", "two", "three", "four", "five"), 
                    WKT = c("MULTIPOINT ((1 1))", "MULTIPOINT ((2 1), (2 2))", "MULTIPOINT ((3 1), (3 2), (3 3))", 
                             "MULTIPOINT ((4 1), (4 2), (4 3), (4 4))", "MULTIPOINT ((5 1), (5 2), (5 3), (5 4), (5 5))"
                    ))
readr::write_csv(d, "inst/extdata/index_point.csv")


icsv <- system.file("extdata/index_point.csv", package = "vapour", mustWork = TRUE)
