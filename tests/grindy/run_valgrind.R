system('R -d  "valgrind --track-origins=yes --leak-check=full" -f tests/grindy/test.R 2>&1 | tee tests/grindy/valgrind')

vg <- readLines("tests/grindy/valgrind")
as.integer(gsub("\\)", "", unlist(lapply(strsplit(grep("vapour\\.cpp:", vg, value = TRUE), "\\.cpp:"), "[", 2)))) -> idx
tx <- readLines("src/vapour.cpp")
tx[idx]

## from exportToJson:
## The returned string should be freed with CPLFree() when no longer required.


## no more ExecuteSQL leaks:
## no more exportTo[Text] leaks:

##Only one problem
## [1] "  while( (poFeature = poLayer->GetNextFeature()) != NULL && lFeature < nFeature)"



