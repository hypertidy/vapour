# Read feature field data

Read features fields (attributes), optionally after SQL execution.

## Usage

``` r
vapour_read_fields_ia(
  dsource,
  layer = 0L,
  sql = "",
  extent = NA,
  ia = NULL,
  open_options = NULL
)

vapour_read_fields_ij(
  dsource,
  layer = 0L,
  sql = "",
  extent = NA,
  ij = NULL,
  open_options = NULL
)

vapour_read_fields_fa(
  dsource,
  layer = 0L,
  sql = "",
  extent = NA,
  fa = NULL,
  open_options = NULL
)

vapour_read_fields(
  dsource,
  layer = 0L,
  sql = "",
  limit_n = NULL,
  skip_n = 0,
  extent = NA,
  open_options = NULL
)

vapour_read_attributes(
  dsource,
  layer = 0L,
  sql = "",
  limit_n = NULL,
  skip_n = 0,
  extent = NA,
  open_options = NULL
)
```

## Arguments

- dsource:

  data source name (path to file, connection string, URL)

- layer:

  integer of layer to work with, defaults to the first (0) or the name
  of the layer

- sql:

  if not empty this is executed against the data source (layer will be
  ignored)

- extent:

  apply an arbitrary extent, only when 'sql' used (must be 'ex = c(xmin,
  xmax, ymin, ymax)' but sp bbox, sf bbox, and raster extent also
  accepted)

- ia:

  an arbitrary index, integer vector with values between 0 and one less
  the number of features, duplicates allowed and arbitrary order is ok

- open_options:

  open options for the driver, a character vector of `"NAME=VALUE"`
  strings (as `-oo` in the GDAL command line tools), `NULL` for none.
  The options a driver accepts are listed on its page at
  <https://gdal.org/en/stable/drivers/vector/index.html>

- ij:

  a range index, integer vector of length two with values between 0 and
  one less the number of features, this range of geometries is returned

- fa:

  feature id (FID) values, as returned by
  [`vapour_read_fids()`](https://hypertidy.github.io/vapour/reference/vapour_read_fids.md),
  in any order and with repeats

- limit_n:

  an arbitrary limit to the number of features scanned

- skip_n:

  an arbitrary number of features to skip

## Value

list of vectors one for each field in the source, each will be the same
length which will depend on the values of 'skip_n', 'limit_n', 'sql',
and the available records in the source, or on the number of features
asked for with 'ia', 'ij' or 'fa'. The types will be raw, numeric,
integer, character, logical depending on the available mapping to the
types in the source for the data there to R's native vectors.

## Details

Internal types are not fully supported, there are straightforward
conversions for numeric, integer (32-bit) and string types. Date, Time,
DateTime are returned as character, and Integer64 is returned as
numeric.

Three more functions read the fields of chosen features, as
[`vapour_read_geometry_ia()`](https://hypertidy.github.io/vapour/reference/vapour_read_geometry.md)
and its companions do for geometry: `vapour_read_fields_ia` by position
(`ia`, counted from 0, in any order and with repeats),
`vapour_read_fields_ij` by an inclusive range of positions (`ij`), and
`vapour_read_fields_fa` by feature id (`fa`, the values that
[`vapour_read_fids()`](https://hypertidy.github.io/vapour/reference/vapour_read_fids.md)
returns). A position or feature id that is not in the layer gives `NA`
in every field, with a warning.

## Examples

``` r
file <- "list_locality_postcode_meander_valley.tab"
mvfile <- system.file(file.path("extdata/tab", file), package="vapour")
att <- vapour_read_fields(mvfile)
str(att)
#> List of 11
#>  $ LOCAL_ID  : num [1:58] 1e+05 1e+05 1e+05 1e+05 1e+05 ...
#>  $ NAME      : chr [1:58] "Caveside" "Weegena" "Kimberley" "Parkham" ...
#>  $ POSTCODE  : int [1:58] 7304 7304 7304 7304 7275 7277 7250 7304 7304 7304 ...
#>  $ PLAN_REF  : chr [1:58] "CPR5322" "CPR5327" "CPR5361" "CPR5327" ...
#>  $ GAZ_DATE  : Date[1:58], format: "2624775-05-26" "2624775-05-26" ...
#>  $ NOM_REG_NO: chr [1:58] "947L" "1300M" "1063T" "1179Y" ...
#>  $ UFI       : chr [1:58] "{4a5db4da-ca19-41a0-8dd4-c28a14bbee18}" "{253b676e-2791-469c-ac5e-9cb3a95cc158}" "{75f60a99-4c58-4d3e-911d-bbaa9a04164c}" "{b008d456-4e80-4237-80f6-a26c03817e3c}" ...
#>  $ CREATED_ON: chr [1:58] "2016-03-04 10:42:37" "2015-06-19 13:46:50" "2016-09-16 10:54:56" "2014-06-06 16:50:22" ...
#>  $ LIST_GUID : chr [1:58] "{839edd46-01a7-4a45-9d97-499962fa952b}" "{de35ebd4-0ac0-4299-947d-87d07c69426a}" "{73ced9ad-ee9a-41d5-a5bc-c95c4ab948d9}" "{37f17d1f-d2a0-4b78-ba0d-e5f62f216658}" ...
#>  $ SHAPE_AREA: num [1:58] -9999 -9999 -9999 -9999 -9999 ...
#>  $ SHAPE_LEN : num [1:58] 39786 31588 35693 67615 70141 ...
sq <- "SELECT * FROM list_locality_postcode_meander_valley WHERE FID < 5"
(att <- vapour_read_fields(mvfile, sql = sq))
#> $LOCAL_ID
#> [1] 100422 100366 100337 100308
#> 
#> $NAME
#> [1] "Caveside"  "Weegena"   "Kimberley" "Parkham"  
#> 
#> $POSTCODE
#> [1] 7304 7304 7304 7304
#> 
#> $PLAN_REF
#> [1] "CPR5322" "CPR5327" "CPR5361" "CPR5327"
#> 
#> $GAZ_DATE
#> [1] "2624775-05-26" "2624775-05-26" "2638022-06-27" "2624775-05-26"
#> 
#> $NOM_REG_NO
#> [1] "947L"  "1300M" "1063T" "1179Y"
#> 
#> $UFI
#> [1] "{4a5db4da-ca19-41a0-8dd4-c28a14bbee18}"
#> [2] "{253b676e-2791-469c-ac5e-9cb3a95cc158}"
#> [3] "{75f60a99-4c58-4d3e-911d-bbaa9a04164c}"
#> [4] "{b008d456-4e80-4237-80f6-a26c03817e3c}"
#> 
#> $CREATED_ON
#> [1] "2016-03-04 10:42:37" "2015-06-19 13:46:50" "2016-09-16 10:54:56"
#> [4] "2014-06-06 16:50:22"
#> 
#> $LIST_GUID
#> [1] "{839edd46-01a7-4a45-9d97-499962fa952b}"
#> [2] "{de35ebd4-0ac0-4299-947d-87d07c69426a}"
#> [3] "{73ced9ad-ee9a-41d5-a5bc-c95c4ab948d9}"
#> [4] "{37f17d1f-d2a0-4b78-ba0d-e5f62f216658}"
#> 
#> $SHAPE_AREA
#> [1] -9999 -9999 -9999 -9999
#> 
#> $SHAPE_LEN
#> [1] 39785.88 31587.54 35693.32 67614.51
#> 
pfile <- "list_locality_postcode_meander_valley.tab"
dsource <- system.file(file.path("extdata/tab", pfile), package="vapour")
SQL <- "SELECT NAME FROM list_locality_postcode_meander_valley WHERE POSTCODE < 7300"
vapour_read_fields(dsource, sql = SQL)
#> $NAME
#>  [1] "Frankford"          "Bridgenorth"        "Summerhill"        
#>  [4] "Carrick"            "Rosevale"           "Selbourne"         
#>  [7] "Blackstone Heights" "Lake St Clair"      "Hagley"            
#> [10] "Quamby Bend"        "Trevallyn"          "Westwood"          
#> [13] "Riverside"          "Hadspen"            "Travellers Rest"   
#> [16] "Prospect Vale"     
#> 

## chosen features: by position, by range of positions, and by feature id
vapour_read_fields_ia(mvfile, ia = c(0, 5, 2))$NAME
#> [1] "Caveside"    "Bridgenorth" "Kimberley"  
vapour_read_fields_ij(mvfile, ij = c(0, 2))$NAME
#> [1] "Caveside"  "Weegena"   "Kimberley"
fids <- vapour_read_fids(mvfile)
vapour_read_fields_fa(mvfile, fa = fids[c(1, 6, 3)])$NAME
#> [1] "Caveside"    "Bridgenorth" "Kimberley"  
```
