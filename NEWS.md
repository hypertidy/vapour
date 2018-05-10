# vapour 0.0.1

* Subdatasets are now supported. 

* Added sanity check behaviour to `raster_io` to avoid out of bounds read attempts. 

* Resampling options added to raster data read. 

* Upgraded to rwinlib gdal 2.2.3. 

* added function `vapour_layer_names` to return character layer names, and
 implicitly provide a layer count

* feature iteration now avoids "GetFeatureCount" and collects each element
 in a growing list, thanks to Jim Hester

* renamed `format` argument to `textformat`, this is ignored unless `what = "text"`

* Created a single C++ feature read to remove repeated logic, `vapour_read_geometry`, `vapour_read_geometry_text`, 
 and `vapour_read_extent` all call `vapour_read_geometry_what`. 
