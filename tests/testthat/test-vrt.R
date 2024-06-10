tif <- system.file("extdata", "sst.tif", package = "vapour", mustWork = TRUE)
hdf <- system.file("extdata/gdal/complex.h5", package = "vapour", mustWork = TRUE)
vrttif <- vapour_vrt(tif)
#vrthdf <- vapour_vrt(hdf)
## can't do this because file is absolute atm
#vrttif <- "<VRTDataset rasterXSize=\"143\" rasterYSize=\"286\">\n  <SRS dataAxisToSRSAxisMapping=\"2,1\">GEOGCS[\"WGS 84\",DATUM[\"WGS_1984\",SPHEROID[\"WGS 84\",6378137,298.257223563,AUTHORITY[\"EPSG\",\"7030\"]],AUTHORITY[\"EPSG\",\"6326\"]],PRIMEM[\"Greenwich\",0],UNIT[\"degree\",0.0174532925199433,AUTHORITY[\"EPSG\",\"9122\"]],AXIS[\"Latitude\",NORTH],AXIS[\"Longitude\",EAST],AUTHORITY[\"EPSG\",\"4326\"]]</SRS>\n  <GeoTransform>  1.4000000000000000e+02,  6.9999999999999937e-02,  0.0000000000000000e+00, -3.9997222067892665e+01,  0.0000000000000000e+00, -7.0003889104950298e-02</GeoTransform>\n  <Metadata>\n    <MDI key=\"AREA_OR_POINT\">Area</MDI>\n  </Metadata>\n  <Metadata domain=\"IMAGE_STRUCTURE\">\n    <MDI key=\"COMPRESSION\">LZW</MDI>\n    <MDI key=\"INTERLEAVE\">BAND</MDI>\n  </Metadata>\n  <VRTRasterBand dataType=\"Float32\" band=\"1\" blockYSize=\"14\">\n    <Metadata>\n      <MDI key=\"STATISTICS_MAXIMUM\">289.859</MDI>\n      <MDI key=\"STATISTICS_MEAN\">279.8360951673</MDI>\n      <MDI key=\"STATISTICS_MINIMUM\">271.35</MDI>\n      <MDI key=\"STATISTICS_STDDEV\">4.9558170498586</MDI>\n    </Metadata>\n    <NoDataValue>-3.399999952144364e+38</NoDataValue>\n    <ColorInterp>Gray</ColorInterp>\n    <SimpleSource>\n      <SourceFilename relativeToVRT=\"0\">/perm_storage/home/gdalbuilder/R/x86_64-pc-linux-gnu-library/4.1/vapour/extdata/sst.tif</SourceFilename>\n      <SourceBand>1</SourceBand>\n      <SourceProperties RasterXSize=\"143\" RasterYSize=\"286\" DataType=\"Float32\" BlockXSize=\"143\" BlockYSize=\"14\" />\n      <SrcRect xOff=\"0\" yOff=\"0\" xSize=\"143\" ySize=\"286\" />\n      <DstRect xOff=\"0\" yOff=\"0\" xSize=\"143\" ySize=\"286\" />\n    </SimpleSource>\n  </VRTRasterBand>\n</VRTDataset>\n"
#vrthdf <- "<VRTDataset rasterXSize=\"5\" rasterYSize=\"5\">\n  <VRTRasterBand dataType=\"CFloat32\" band=\"1\" blockYSize=\"1\">\n    <SimpleSource>\n      <SourceFilename relativeToVRT=\"0\">HDF5:\"/perm_storage/home/gdalbuilder/vapour/inst/extdata/gdal/complex.h5\"://f16</SourceFilename>\n      <SourceBand>1</SourceBand>\n      <SourceProperties RasterXSize=\"5\" RasterYSize=\"5\" DataType=\"CFloat32\" BlockXSize=\"5\" BlockYSize=\"1\" />\n      <SrcRect xOff=\"0\" yOff=\"0\" xSize=\"5\" ySize=\"5\" />\n      <DstRect xOff=\"0\" yOff=\"0\" xSize=\"5\" ySize=\"5\" />\n    </SimpleSource>\n  </VRTRasterBand>\n</VRTDataset>\n"

# test_that("vrt works", {
#   expect_equal(vapour_driver(vrttif), "VRT")
#   # expect_equal(vapour_vrt(hdf, sds = 1), vrthdf)
#   expect_silent(vapour_vrt(hdf, bands = 2))
#   expect_equal(vapour_vrt(hdf, bands = 2), NA_character_)
#   expect_length(vapour_read_raster(vapour_vrt(tif, bands = rep(1, 3)), band = 1:3, window = c(0, 0, 2, 3)), 3L)
#   
#   ## type is Complex so weird message 
#   expect_error(vapour_read_raster(vapour_vrt(hdf, bands = rep(1, 3)), band = 1:3, window = c(0, 0, 2, 3)), "is it Complex")
#   ex <- c(19, 20, -30, -50)
#   expect_equal(vapour_raster_info(vapour_vrt(tif, extent = ex))$extent, ex)
#   prj <- "EPSG:3031"            
#   expect_equal(vapour_raster_info(vapour_vrt(tif, projection = prj))$projection, vapour_srs_wkt(prj))
#   
# })

