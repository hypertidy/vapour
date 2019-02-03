system('R -d  "valgrind --track-origins=yes --leak-check=full" -f tests/grindy/test.R 2>&1 | tee tests/grindy/valgrind')

vg <- readLines("tests/grindy/valgrind")
as.integer(gsub("\\)", "", unlist(lapply(strsplit(grep("rasterio\\.cpp:", vg, value = TRUE), "\\.cpp:"), "[", 2)))) -> idx
tx <- readLines("src/rasterio.cpp")
tx[idx]

## from exportToJson:
## The returned string should be freed with CPLFree() when no longer required.


## no more ExecuteSQL leaks:
## no more exportTo[Text] leaks:

##Only one problem at line 220 in vapour_read_attributes_cpp (see block below)
##  while((poFeature = poLayer->GetNextFeature()) != NULL)



# OGRFeatureDefn *poFDefn = poLayer->GetLayerDefn();
# bool int64_as_string = false;
# Rcpp::List out = allocate_attribute(poFDefn, nFeature, int64_as_string);
# int iFeature = 0;  // always increment iFeature, it is position through the loop
# int lFeature = 0; // keep a count of the features we actually send out
# LINE 220: while((poFeature = poLayer->GetNextFeature()) != NULL)
# {
#

