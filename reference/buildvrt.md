# Build vrt, special case "-separate"

Build vrt, special case "-separate"

## Usage

``` r
buildvrt(dsn)
```

## Arguments

- dsn:

  one or more raster sources

## Value

a character string of the built vrt, multiple sources treated as bands

## Examples

``` r
f <- system.file("extdata/sst.tif", package = "vapour", mustWork = TRUE)
vrt <- buildvrt(c(f, vapour_vrt(f)))
writeLines(vrt)
#> <VRTDataset rasterXSize="143" rasterYSize="286">
#>   <SRS dataAxisToSRSAxisMapping="2,1">GEOGCS["WGS 84",DATUM["WGS_1984",SPHEROID["WGS 84",6378137,298.257223563,AUTHORITY["EPSG","7030"]],AUTHORITY["EPSG","6326"]],PRIMEM["Greenwich",0,AUTHORITY["EPSG","8901"]],UNIT["degree",0.0174532925199433,AUTHORITY["EPSG","9122"]],AXIS["Latitude",NORTH],AXIS["Longitude",EAST],AUTHORITY["EPSG","4326"]]</SRS>
#>   <GeoTransform>  1.4000000000000000e+02,  6.9999999999999937e-02,  0.0000000000000000e+00, -3.9997222067892665e+01,  0.0000000000000000e+00, -7.0003889104950298e-02</GeoTransform>
#>   <VRTRasterBand dataType="Float32" band="1">
#>     <NoDataValue>-3.39999995214436425e+38</NoDataValue>
#>     <ComplexSource>
#>       <SourceFilename relativeToVRT="0">/home/runner/work/_temp/Library/vapour/extdata/sst.tif</SourceFilename>
#>       <SourceBand>1</SourceBand>
#>       <SourceProperties RasterXSize="143" RasterYSize="286" DataType="Float32" BlockXSize="143" BlockYSize="14" />
#>       <SrcRect xOff="0" yOff="0" xSize="143" ySize="286" />
#>       <DstRect xOff="0" yOff="0" xSize="143" ySize="286" />
#>       <NODATA>-3.39999995214436425e+38</NODATA>
#>     </ComplexSource>
#>   </VRTRasterBand>
#>   <VRTRasterBand dataType="Float32" band="2">
#>     <NoDataValue>-3.39999995214436425e+38</NoDataValue>
#>     <ComplexSource>
#>       <SourceFilename relativeToVRT="0">&lt;VRTDataset rasterXSize="143" rasterYSize="286"&gt;
#>   &lt;SRS dataAxisToSRSAxisMapping="2,1"&gt;GEOGCS["WGS 84",DATUM["WGS_1984",SPHEROID["WGS 84",6378137,298.257223563,AUTHORITY["EPSG","7030"]],AUTHORITY["EPSG","6326"]],PRIMEM["Greenwich",0,AUTHORITY["EPSG","8901"]],UNIT["degree",0.0174532925199433,AUTHORITY["EPSG","9122"]],AXIS["Latitude",NORTH],AXIS["Longitude",EAST],AUTHORITY["EPSG","4326"]]&lt;/SRS&gt;
#>   &lt;GeoTransform&gt;  1.4000000000000000e+02,  6.9999999999999937e-02,  0.0000000000000000e+00, -3.9997222067892665e+01,  0.0000000000000000e+00, -7.0003889104950298e-02&lt;/GeoTransform&gt;
#>   &lt;Metadata&gt;
#>     &lt;MDI key="AREA_OR_POINT"&gt;Area&lt;/MDI&gt;
#>   &lt;/Metadata&gt;
#>   &lt;Metadata domain="IMAGE_STRUCTURE"&gt;
#>     &lt;MDI key="INTERLEAVE"&gt;BAND&lt;/MDI&gt;
#>     &lt;MDI key="COMPRESSION"&gt;LZW&lt;/MDI&gt;
#>   &lt;/Metadata&gt;
#>   &lt;VRTRasterBand dataType="Float32" band="1" blockXSize="143" blockYSize="14"&gt;
#>     &lt;Metadata&gt;
#>       &lt;MDI key="STATISTICS_MAXIMUM"&gt;289.859&lt;/MDI&gt;
#>       &lt;MDI key="STATISTICS_MEAN"&gt;279.8360951673&lt;/MDI&gt;
#>       &lt;MDI key="STATISTICS_MINIMUM"&gt;271.35&lt;/MDI&gt;
#>       &lt;MDI key="STATISTICS_STDDEV"&gt;4.9558170498586&lt;/MDI&gt;
#>     &lt;/Metadata&gt;
#>     &lt;NoDataValue&gt;-3.39999995214436425e+38&lt;/NoDataValue&gt;
#>     &lt;ColorInterp&gt;Gray&lt;/ColorInterp&gt;
#>     &lt;SimpleSource&gt;
#>       &lt;SourceFilename relativeToVRT="0"&gt;/home/runner/work/_temp/Library/vapour/extdata/sst.tif&lt;/SourceFilename&gt;
#>       &lt;SourceBand&gt;1&lt;/SourceBand&gt;
#>       &lt;SourceProperties RasterXSize="143" RasterYSize="286" DataType="Float32" BlockXSize="143" BlockYSize="14" /&gt;
#>       &lt;SrcRect xOff="0" yOff="0" xSize="143" ySize="286" /&gt;
#>       &lt;DstRect xOff="0" yOff="0" xSize="143" ySize="286" /&gt;
#>     &lt;/SimpleSource&gt;
#>   &lt;/VRTRasterBand&gt;
#> &lt;/VRTDataset&gt;
#> </SourceFilename>
#>       <SourceBand>1</SourceBand>
#>       <SourceProperties RasterXSize="143" RasterYSize="286" DataType="Float32" BlockXSize="143" BlockYSize="128" />
#>       <SrcRect xOff="0" yOff="0" xSize="143" ySize="286" />
#>       <DstRect xOff="0" yOff="0" xSize="143" ySize="286" />
#>       <NODATA>-3.39999995214436425e+38</NODATA>
#>     </ComplexSource>
#>   </VRTRasterBand>
#> </VRTDataset>
#> 
```
