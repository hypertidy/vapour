# vapour 0.0.1

* feature iteration now avoids "GetFeatureCount" and 
* renamed `format` argument to `textformat`, this is ignored unless `what = "text"`

* Created a single C++ feature read to remove repeated logic, `vapour_read_geometry`, `vapour_read_geometry_text`, 
 and `vapour_read_extent` all call `vapour_read_geometry_what`. 
