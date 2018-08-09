# vapour 0.1.0

* Functions that include a `layer` index now accept a layer name which is used to find the (0-based) layer index. 

* This is a breaking release, all old functions have been made defunct or removed entirely. 

# vapour 0.0.1

* All C++ functions now have explicit R wrappers instead of Rcpp exports. 

* New naming convention uses `vapour_` for vector sources, `raster_` for raster sources 
 to make function names a little more consistent. 

* New wrapper `vapour_sds_names` to replace `sds_info` and `raster_sds_info` which are now deprecated. 

* New wrapper `vapour_read_geometry_what` to replace `vapour_read_feature_what` which i  is now deprecated. 

* New function  `vapour_read_names` to return the vector of FID values. 

* The read for raster data now returns numeric or integer appropriately. 

* `vapour_raster_info` now includes `bands` as the count of available bands. 

* The IO read now allows a 4-element `window` to return data at native resolution, by
 copying the third and fourth values (source dimension) to the fifth and sixth values
 (output dimension) respectively. 
 
* Subdatasets are now supported. 

* Added sanity check behaviour to `vapour_raster_read` to avoid out of bounds read attempts. 

* Resampling options added to raster data read. 

* Upgraded to rwinlib gdal 2.2.3. 

* added function `vapour_layer_names` to return character layer names, and
 implicitly provide a layer count

* feature iteration now avoids "GetFeatureCount" and collects each element
 in a growing list, thanks to Jim Hester

* renamed `format` argument to `textformat`, this is ignored unless `what = "text"`

* Created a single C++ feature read to remove repeated logic, `vapour_read_geometry`, `vapour_read_geometry_text`,  and `vapour_read_extent` all call the same function. 
