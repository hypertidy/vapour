dp <- "/rdsi/PUBLIC/raad/data/listdata.thelist.tas.gov.au/opendata/data"
#"/rdsi/PUBLIC/raad/data/listdata.thelist.tas.gov.au/opendata/data/list_locality_postcode_meander_valley.tab"
fs <- file.path(dp, paste0("list_locality_postcode_meander_valley.", c("tab", "DAT", "ID", "MAP")))
file.copy(fs, "inst/extdata/tab/")


