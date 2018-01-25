# vapour 0.0.1

* added function `vapour_layer_names` to return character layer names, and
 implicitly provide a layer count

* feature iteration now avoids "GetFeatureCount" and collects each element
 in a growing list, thanks to Jim Hester

* renamed `format` argument to `textformat`, this is ignored unless `what = "text"`

* Created a single C++ feature read to remove repeated logic, `vapour_read_geometry`, `vapour_read_geometry_text`, 
 and `vapour_read_extent` all call `vapour_read_geometry_what`. 
