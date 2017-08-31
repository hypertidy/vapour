<!-- README.md is generated from README.Rmd. Please edit that file -->
[![Travis-CI Build Status](https://travis-ci.org/hypertidy/vapour.svg?branch=master)](https://travis-ci.org/hypertidy/vapour) [![AppVeyor Build Status](https://ci.appveyor.com/api/projects/status/github/hypertidy/vapour?branch=master&svg=true)](https://ci.appveyor.com/project/hypertidy/vapour) [![Coverage Status](https://img.shields.io/codecov/c/github/hypertidy/vapour/master.svg)](https://codecov.io/github/hypertidy/vapour?branch=master) [![CRAN\_Status\_Badge](http://www.r-pkg.org/badges/version/vapour)](https://cran.r-project.org/package=vapour)

vapour
======

The goal of vapour is to provide a basic **GDAL API** package for R. Ideally, this could become a common foundation for other packages to specialize. A parallel goal is to be freed from the powerful but sometimes limiting high-level data models of GDAL itself, specifically these are *simple features* and *affine-based regular rasters composed of 2D slices*. (GDAL will possibly remove these limitations over time but still there will always be value in having modularity in an ecosystem of tools. )

This is inspired by and draws heavily on work done [the sf package](https://github.com/r-spatial/sf) and rgdal and rgdal2.

Purpose
=======

Current we have control to do the following:

-   read attributes only
-   read geometry only
-   read geometry as raw binary, or various text forms
-   read geometry bounding box only
-   (optionally) apply OGRSQL to a layer prior to any of the above <http://www.gdal.org/ogr_sql.html>

Limitations, work-in-progress and other discussion are active here: <https://github.com/hypertidy/vapour/issues/4>

Examples
--------

There's a function `vapour_read_attributes` that returns the attributes as list of vectors.

``` r
pfile <- system.file("extdata", "point.shp", package = "vapour")
library(vapour)
vapour_read_attributes(pfile)
#> $a
#>  [1]  1  2  3  4  5  6  7  8  9 10
```

A higher level function *somewhere else* could wrap that function to return a data frame, but we don't want that in `vapour` because it's not aligned with the goals of being lightweight and reducing the level of interpretation applied. The `data.frame` function in R is actually a very primitive implememtation for data frames, so we avoid putting that interpretation on the data and leave that up to the developer / user.

``` r
mvfile <- system.file("extdata/tab/list_locality_postcode_meander_valley.tab", package="vapour")

as.data.frame(vapour_read_attributes(mvfile),  stringsAsFactors = FALSE)
#>    LOCAL_ID               NAME POSTCODE PLAN_REF   GAZ_DATE NOM_REG_NO
#> 1    100422           Caveside     7304  CPR5322 1970-01-01       947L
#> 2    100366            Weegena     7304  CPR5327 1970-01-01      1300M
#> 3    100337          Kimberley     7304  CPR5361 1970-01-01      1063T
#> 4    100308            Parkham     7304  CPR5327 1970-01-01      1179Y
#> 5    100263          Frankford     7275  CPR5142 1970-01-01      1003Q
#> 6    100311        Bridgenorth     7277  CPR5140 1970-01-01       925X
#> 7    100377         Summerhill     7250  CPR5150 1970-01-01      4806G
#> 8    100407           Mayberry     7304  CPR5321 1970-01-01      1116M
#> 9    100429      Western Creek     7304  CPR5322 1970-01-01      1304X
#> 10   100423            Meander     7304  CPR5322 1970-01-01      1117P
#> 11   100387          Deloraine     7304  CPR5327 1970-01-01       268G
#> 12   100397            Carrick     7291  CPR5329 1970-01-01       943C
#> 13   100328           Rosevale     7292  CPR5329 1970-01-01      1219E
#> 14   100269          Sassafras     7307  CPR5238 1970-01-01      1231T
#> 15   100416       Dairy Plains     7304  CPR5322 1970-01-01     38109M
#> 16   100437             Liffey     7301  CPR5323 1970-01-01      1090C
#> 17   100357           Westbury     7303  CPR5328 1970-01-01      1303T
#> 18   100331          Selbourne     7292  CPR5329 1970-01-01      1630F
#> 19   100370 Blackstone Heights     7250  CPR5326 1970-01-01     38108A
#> 20   100421    Cradle Mountain     7306  CPR5363 1970-01-01     38113D
#> 21   100460      Lake St Clair     7140  CPR5570 1970-01-01     39197M
#> 22   100401          Chudleigh     7304  CPR5322 1970-01-01       951C
#> 23   100398          Red Hills     7304  CPR5322 1970-01-01      2399W
#> 24   100425      Golden Valley     7304  CPR5323 1970-01-01      1018H
#> 25   100325     Elizabeth Town     7304  CPR5327 1970-01-01       280X
#> 26   100412          Whitemore     7303  CPR5323 1970-01-01      1311T
#> 27   100383             Hagley     7292  CPR5329 1970-01-01      1030Y
#> 28   100324           Birralee     7303  CPR5328 1970-01-01       909X
#> 29   100378        Quamby Bend     7292  CPR5329 1970-01-01      7650E
#> 30   100355          Trevallyn     7250  CPR5155 1970-01-01      4807J
#> 31   100373       Mount Roland     7306  CPR5359 1970-01-01     38115E
#> 32   100351            Moltema     7304  CPR5327 1970-01-01      1124M
#> 33   100326        Reedy Marsh     7304  CPR5328 1970-01-01      1628T
#> 34   100419              Cluan     7303  CPR5323 1970-01-01      2828H
#> 35   100360           Westwood     7292  CPR5329 1970-01-01      1265S
#> 36   100364          Middlesex     7306  CPR5362 1970-01-01     38114Q
#> 37   100400              Liena     7304  CPR5321 1970-01-01      1088Q
#> 38   100420            Montana     7304  CPR5322 1970-01-01      1126R
#> 39   100415       Quamby Brook     7304  CPR5323 1970-01-01      1196B
#> 40   100363             Weetah     7304  CPR5327 1970-01-01       599C
#> 41   100428          Bracknell     7302  CPR5323 1970-01-01       919C
#> 42   100335          Riverside     7250  CPR5140 1970-01-01        25L
#> 43   100418      Mersey Forest     7304  CPR5321 1970-01-01     38110N
#> 44   100396         Mole Creek     7304  CPR5321 1970-01-01      1123K
#> 45   100346       Lower Beulah     7306  CPR5361 1970-01-01       907R
#> 46   100403            Needles     7304  CPR5322 1970-01-01      1150R
#> 47   100388              Exton     7303  CPR5328 1970-01-01       281A
#> 48   100411           Osmaston     7303  CPR5323 1970-01-01      2839P
#> 49   100424      Bishopsbourne     7301  CPR5044 1970-01-01       910F
#> 50   100446 Walls of Jerusalem     7304  CPR5571 1970-01-01     39206M
#> 51   100430    Central Plateau     7304  CPR5569 1970-01-01     38974G
#> 52   100440      Jackeys Marsh     7304  CPR5322 1970-01-01      1053P
#> 53   100385           Dunorlan     7304  CPR5327 1970-01-01       983W
#> 54   100417               Oaks     7303  CPR5323 1970-01-01      1162C
#> 55   100393           Longford     7301  CPR5044 1970-01-01       348G
#> 56   100394            Hadspen     7290  CPR5329 1970-01-01      1029P
#> 57   100389    Travellers Rest     7250  CPR5326 1970-01-01     38111C
#> 58   100381      Prospect Vale     7250  CPR5325 1970-01-01        58H
#>                                       UFI          CREATED_ON
#> 1  {4a5db4da-ca19-41a0-8dd4-c28a14bbee18} 2016-03-04 10:42:37
#> 2  {253b676e-2791-469c-ac5e-9cb3a95cc158} 2015-06-19 13:46:50
#> 3  {75f60a99-4c58-4d3e-911d-bbaa9a04164c} 2016-09-16 10:54:56
#> 4  {b008d456-4e80-4237-80f6-a26c03817e3c} 2014-06-06 16:50:22
#> 5  {953f4006-6397-4d03-af97-507eab170862} 2014-12-08 09:07:12
#> 6  {5cf8e2a3-631c-4d52-a79c-0ce475f76848} 2015-05-11 10:49:09
#> 7  {03a9def3-2f58-43da-b4ec-bc8f2e9a2eab} 2017-01-12 13:04:10
#> 8  {d71e82ff-7416-4a0b-b803-8764b5bcb451} 2014-02-26 14:44:53
#> 9  {a3f54d77-bcb7-4809-940d-040f14d8640f} 2014-06-06 16:50:24
#> 10 {128ae910-e6d0-4420-a3d6-daf031f44e67} 2016-03-03 16:18:06
#> 11 {b1b54795-ded8-49ee-8567-cf286545dc8f} 2017-05-10 14:26:15
#> 12 {566f5fdb-7f84-4ff6-8dd6-63e4454e1fd3} 2017-01-12 13:04:10
#> 13 {2eeb3ff9-a32b-4715-87c9-8da102fe60b3} 2016-11-15 12:54:45
#> 14 {4872b9bc-bc96-49a4-a765-88774bdfe098} 2017-05-10 14:26:15
#> 15 {88725292-d307-405d-8de3-12d3c0a77ab6} 2014-06-06 16:50:26
#> 16 {73c660ab-fdc7-4b31-b22d-78cc71770cbc} 2016-03-03 16:18:06
#> 17 {86c19cc3-cd18-44e3-a5b5-ecaf79780571} 2017-04-03 12:24:18
#> 18 {2a0859b4-4656-4f6a-aa2b-cb933bd108de} 2016-11-15 12:54:45
#> 19 {922d7c01-85d5-4a4f-ab3c-da7b0382861b} 2017-05-10 14:26:15
#> 20 {e17d940e-0240-4a12-acef-d2377386439d} 2014-02-26 14:44:53
#> 21 {a87b5dc0-98c5-44fa-9fe5-50701096785e} 2014-02-26 14:45:03
#> 22 {184bddd2-e6c2-4e2a-99be-2c690d31c97b} 2017-04-03 12:24:18
#> 23 {b392f80d-2922-4d91-8a77-a5278ca7c787} 2014-06-06 16:50:25
#> 24 {c33654f5-41be-43a8-b506-80416a2bda25} 2015-12-14 08:46:42
#> 25 {f421c569-e274-488b-a195-6c487648053c} 2016-11-15 12:54:45
#> 26 {2349bffa-a979-49ae-8a43-b69aa0f24370} 2017-04-03 12:24:18
#> 27 {69c3c915-fc8d-4001-8b15-40042f3e7117} 2016-11-15 12:54:45
#> 28 {fda6b877-69b3-42c8-abc5-d92cf1a98b1e} 2014-12-08 09:07:14
#> 29 {69eb3d26-614d-464b-b59b-aa9bc36d8a54} 2015-10-28 11:28:12
#> 30 {887d42bf-4f02-42f1-9135-2f5b8d78e8cc} 2015-07-31 12:13:03
#> 31 {a00cfee1-a76c-47c6-bdb0-61408c34037b} 2014-12-08 09:07:15
#> 32 {9e283e9b-eebb-430b-b4b5-0f129ded465d} 2015-06-19 13:46:51
#> 33 {7f74079d-0b95-40f1-a53d-94902d5e3e09} 2014-06-06 16:50:23
#> 34 {9353c745-9bd5-4182-8cf2-01791a6b2d64} 2017-04-03 12:24:18
#> 35 {86abde4b-6458-4d12-8d3c-1af5f1f123a7} 2017-01-12 13:04:10
#> 36 {4347282e-7616-451b-b421-e53c36c7e064} 2016-03-04 10:42:37
#> 37 {60767eb6-79ad-490a-bcf4-7f6f532c13c5} 2014-12-08 09:07:18
#> 38 {cee33f2c-c615-4b4a-bf07-9e4578e6a5b9} 2014-06-06 16:50:24
#> 39 {e74609a2-b8af-47b6-b5c1-480668f3192f} 2017-05-10 14:26:15
#> 40 {7b954d94-6f77-45e8-8693-dfdca8d47d52} 2014-06-06 16:50:22
#> 41 {1effaa60-8abe-4cba-8c26-249aad2046d6} 2016-03-03 16:18:05
#> 42 {79cec965-7882-406a-83d0-13787ce055c7} 2015-12-14 08:46:42
#> 43 {0f06fa94-73cf-42ae-a2d2-144ee7c07a68} 2016-03-02 12:05:14
#> 44 {a5cf8425-3496-492f-9878-22ccb4af7267} 2017-04-03 12:58:01
#> 45 {50845792-6f5d-4d0d-b85c-ec95993e7097} 2017-04-03 12:58:01
#> 46 {6f6b41b7-555f-4f31-a37f-4e424167d9f1} 2017-04-03 12:24:18
#> 47 {a3aa7385-0fbb-4dc2-b13b-9910b2507e64} 2012-03-08 14:02:08
#> 48 {ef412a61-9aae-48ca-8257-df0af892956d} 2012-02-03 11:37:53
#> 49 {39413160-812c-463d-820d-d8f4c1aa82c8} 2017-05-22 15:05:14
#> 50 {5a0bd757-9a8b-45c2-b497-6edaaec7052d} 2015-10-28 11:28:09
#> 51 {24b22df7-c905-406c-a19e-cc77fd294507} 2017-05-10 14:26:15
#> 52 {5c4c3d24-ca65-40cd-a11e-e501248f9f65} 2016-03-03 16:18:05
#> 53 {a1990658-8a33-4175-aace-9a5b6a5e5962} 2017-01-12 13:04:10
#> 54 {f671bc53-71cb-4d47-9b86-2a5afa7bbf99} 2014-06-17 14:08:36
#> 55 {84d3f931-7ef9-4d7e-9bd9-f47e095511a0} 2017-05-22 15:05:14
#> 56 {4820bc13-ab68-4dd9-a1c5-2856e7af44b3} 2017-01-12 13:04:10
#> 57 {a3960c6a-0b2b-43a7-9a6d-dbfc1a189966} 2015-04-08 11:29:27
#> 58 {6af4252a-c213-4052-9baf-f4315240075f} 2017-05-10 14:26:15
#>                                 LIST_GUID SHAPE_AREA  SHAPE_LEN
#> 1  {839edd46-01a7-4a45-9d97-499962fa952b}      -9999  39785.880
#> 2  {de35ebd4-0ac0-4299-947d-87d07c69426a}      -9999  31587.543
#> 3  {73ced9ad-ee9a-41d5-a5bc-c95c4ab948d9}      -9999  35693.318
#> 4  {37f17d1f-d2a0-4b78-ba0d-e5f62f216658}      -9999  67614.515
#> 5  {47f3a313-913f-4f83-8dc1-021907f9ee80}      -9999  70140.783
#> 6  {425d01cc-223b-4348-b965-384a98fd7999}      -9999  38156.704
#> 7  {b67a3c2b-08cb-42d4-825b-e40fcab2fb5e}    2302643   9517.766
#> 8  {60ba6d69-2021-462e-936f-544ee7c2595b}      -9999  38379.277
#> 9  {fe452b60-7ef7-44f2-8741-71126b4fb0a3}      -9999  42948.737
#> 10 {876e5283-2a5d-46aa-92da-4aad5e02cb8e}      -9999  94270.024
#> 11 {f29cc705-df86-4627-9725-f022c7a7665a}      -9999  68696.703
#> 12 {b4dfcc34-6125-419a-9219-eb2563696623}      -9999  49516.829
#> 13 {0371b2e7-c72f-447a-991e-451ec78dfb62}      -9999  37643.704
#> 14 {6495b253-047d-40be-a842-694907a4700e}      -9999  83539.932
#> 15 {3d221f2f-2af7-4150-8e11-89e612c9620a}      -9999  28668.151
#> 16 {90808a7b-5134-4352-a0a0-9eb941c1c6fc}      -9999  69478.598
#> 17 {28538e58-ea0a-4876-ba28-77fb91fb4c56}      -9999  65351.499
#> 18 {9f9bf307-bb1d-4a44-9997-f4ef0ccb771a}      -9999  48579.511
#> 19 {53d25ef5-ee06-4040-93e4-f40adc31452d}    7832529  12135.978
#> 20 {d53dd2a5-ff7a-4c6f-98b4-a8bc463a83ea}      -9999 200297.904
#> 21 {d508dd6c-c05a-4085-9043-52342bec9b42}      -9999 209224.675
#> 22 {1fe385b9-753d-4abb-a485-5dc8811c47a0}      -9999  38611.683
#> 23 {ec7a8d40-af0f-431e-8f42-313f68b59546}      -9999  34268.901
#> 24 {24f934f6-c9e2-4856-82dc-6794e4c3e2a8}      -9999  56009.364
#> 25 {fca48545-c9e9-4971-8146-52a5e625e38e}      -9999  55266.161
#> 26 {3c7aea1e-2c10-4e0d-a437-2af95b984aac}      -9999  43353.208
#> 27 {69d4b471-8414-4ee0-b8ee-37a54df00d26}      -9999  48065.647
#> 28 {51b7f399-0e8f-4d16-91d9-b64e89e06e5c}      -9999  40268.776
#> 29 {e61f737f-2531-4beb-b13e-e929c56c0302}    8486736  19106.809
#> 30 {6b478510-78fa-4fba-afa2-3ef85190cee4}    9852836  16050.466
#> 31 {c72f1ca0-11f8-4481-94c4-4ec11cb7fa88}      -9999  49329.223
#> 32 {14b518dd-04b4-4edc-a566-a0712e4f81ab}      -9999  28920.182
#> 33 {a91a9168-5623-4c27-9bf5-921d3b2fad55}      -9999  71731.265
#> 34 {91390c0f-8706-41b7-b85c-d776be118f8c}      -9999  36651.119
#> 35 {f5402dd6-8236-4165-aad2-df6510f5b45b}      -9999  54107.685
#> 36 {e6e6878b-d4bd-470d-85d3-3a0266644bb0}      -9999 114901.874
#> 37 {172a69bd-6ab6-4125-8db7-b28cf447e58a}      -9999  40650.548
#> 38 {c517afd0-d363-4ee6-9daf-b0b92859754e}      -9999  26196.383
#> 39 {5222cf87-76e7-42d5-885d-05adfd833113}      -9999  27696.797
#> 40 {632db980-62f5-4676-bb10-ca6c8636b2ca}      -9999  26183.768
#> 41 {036a25e8-5ae6-4758-b047-8e2a48f7078a}      -9999  53843.912
#> 42 {7aab64fc-9092-430d-adf5-a6187ccac639}      -9999  44108.040
#> 43 {d8b29014-d1f0-4537-90b0-2fc69409fa8f}      -9999 152351.454
#> 44 {28abf88d-446c-4396-a0d1-5a8f487d626d}      -9999  75018.037
#> 45 {630d7192-57a9-4655-8ff2-732c5d617bdf}      -9999  41961.578
#> 46 {48a6c36b-7cd7-44b3-a051-8331a10469bd}      -9999  24157.077
#> 47 {d6a63e2d-6c78-45f1-8f78-c97a7a0f9798}      -9999  28578.244
#> 48 {c256e5de-6bb6-49c2-a9b7-eb1cc4a4358a}      -9999  22549.288
#> 49 {36d9a9de-c587-4d06-8cc5-a40d735ea929}      -9999  31002.635
#> 50 {028ad03f-fd0f-41e7-88ff-288a65388203}      -9999 166213.126
#> 51 {725932a9-aec5-4aad-b083-589b5650cc31}      -9999 474887.643
#> 52 {f8ee3df0-d971-424e-8620-d7dc1b664dbd}      -9999  43304.587
#> 53 {4281ae3b-465d-4a74-9ffe-f3255e3d0c25}      -9999  29551.180
#> 54 {428423a1-6297-44f6-824c-938c1a7d7877}      -9999  20044.890
#> 55 {7c0633e1-2e04-4d20-8b8c-1c5dc688ec12}      -9999  94995.922
#> 56 {f65f7d94-39aa-46a4-8a39-019f0b6cd468}      -9999  14480.093
#> 57 {e8392030-0ef6-4617-8506-74ee4e19b36b}    6136801  12276.269
#> 58 {0e788275-bc03-4b2d-b9b0-2b9d13793154}      -9999  16297.430
```

There are many useful higher level operations that can be used with this. The simplest is the ability to use GDAL as a database-like connection to attribute tables.

A low-level function will return a character vector of JSON, GML, KML or WKT.

``` r
vapour_read_geometry(pfile)[5:6]  ## format = "WKB"
#> [[1]]
#>  [1] 01 01 00 00 00 00 00 60 08 18 ad ec 3f 00 00 e0 9a ec 77 e2 3f
#> 
#> [[2]]
#>  [1] 01 01 00 00 00 00 00 c0 40 3c bb d0 3f 00 00 80 0e 30 25 d5 3f

vapour_read_geometry_text(pfile)[5:6]  ## format = "json"
#> [1] "{ \"type\": \"Point\", \"coordinates\": [ 0.89612962375395, 0.577139189234003 ] }" 
#> [2] "{ \"type\": \"Point\", \"coordinates\": [ 0.261427939636633, 0.330394758377224 ] }"

cfile <- system.file("extdata/sst_c.gpkg", package = "vapour")
vapour_read_geometry_text(pfile, format = "gml")[2]
#> [1] "<gml:Point><gml:coordinates>0.145755324047059,0.395469118840992</gml:coordinates></gml:Point>"

## don't do this with a non-longlat data set like cfile
vapour_read_geometry_text(pfile, format = "kml")[1:2]
#> [1] "<Point><coordinates>0.623376188334078,0.380098037654534</coordinates></Point>"
#> [2] "<Point><coordinates>0.145755324047059,0.395469118840992</coordinates></Point>"

str(vapour_read_geometry_text(cfile, format = "wkt")[1:2])
#>  chr [1:2] "MULTILINESTRING ((-16254.4210476553 -3269904.98849485,-48956.5880244328 -3282652.40200143,-82133.8545994558 -33"| __truncated__ ...
```

We can combine these together to get a custom data set.

``` r
library(dplyr)
dat <- as.data.frame(vapour_read_attributes(cfile),  stringsAsFactors = FALSE) %>% dplyr::mutate(wkt = vapour_read_geometry_text(cfile, format = "wkt"))
glimpse(dat)
#> Observations: 7
#> Variables: 3
#> $ level <chr> "275", "280", "285", "290", "295", "300", "305"
#> $ sst   <dbl> 1.85, 6.85, 11.85, 16.85, 21.85, 26.85, 31.85
#> $ wkt   <chr> "MULTILINESTRING ((-16254.4210476553 -3269904.98849485,-...
```

Fast summary
------------

There is a basic function `vapour_read_extent` to return a straightforward bounding box vector for every feature, so that we can flexibly build an index of a data set for later use.

``` r
mvfile <- system.file("extdata/tab/list_locality_postcode_meander_valley.tab", package="vapour")
str(vapour_read_extent(mvfile))
#> List of 58
#>  $ : num [1:4] 448353 457706 5386606 5397352
#>  $ : num [1:4] 453544 459318 5403972 5412505
#>  $ : num [1:4] 454840 461042 5411562 5417892
#>  $ : num [1:4] 461505 476213 5410911 5424854
#>  $ : num [1:4] 471573 483157 5417110 5424645
#>  $ : num [1:4] 491638 494048 5417262 5419331
#>  $ : num [1:4] 508512 510057 5408709 5409648
#>  $ : num [1:4] 435993 444517 5392900 5402213
#>  $ : num [1:4] 453298 462671 5383592 5393869
#>  $ : num [1:4] 457592 473357 5377059 5397051
#>  $ : num [1:4] 465137 477418 5394645 5408531
#>  $ : num [1:4] 495773 507915 5396427 5405789
#>  $ : num [1:4] 489939 499712 5410677 5418328
#>  $ : num [1:4] 459056 471905 5418849 5425154
#>  $ : num [1:4] 455980 463979 5392870 5399259
#>  $ : num [1:4] 475889 493826 5379077 5389494
#>  $ : num [1:4] 480223 490958 5397052 5414286
#>  $ : num [1:4] 485858 494029 5407194 5418267
#>  $ : num [1:4] 505330 508582 5407832 5411813
#>  $ : num [1:4] 412719 433694 5364102 5392659
#>  $ : num [1:4] 418130 431437 5352613 5368181
#>  $ : num [1:4] 452420 460772 5395775 5404447
#>  $ : num [1:4] 459779 470225 5398391 5405474
#>  $ : num [1:4] 471219 485530 5383849 5395087
#>  $ : num [1:4] 457719 469430 5407328 5420616
#>  $ : num [1:4] 486794 497211 5391896 5400794
#>  $ : num [1:4] 488190 498092 5396973 5409378
#>  $ : num [1:4] 479268 487581 5412326 5419668
#>  $ : num [1:4] 487082 491200 5405846 5410168
#>  $ : num [1:4] 507192 508682 5409572 5411550
#>  $ : num [1:4] 432998 441413 5402891 5406595
#>  $ : num [1:4] 457813 463400 5408117 5415279
#>  $ : num [1:4] 470867 483434 5404320 5419938
#>  $ : num [1:4] 481800 490524 5388711 5398922
#>  $ : num [1:4] 493483 506413 5403349 5412391
#>  $ : num [1:4] 416790 424281 5390142 5396738
#>  $ : num [1:4] 432186 439291 5396541 5404521
#>  $ : num [1:4] 462417 469412 5391797 5398604
#>  $ : num [1:4] 473323 481800 5392390 5399469
#>  $ : num [1:4] 466298 472556 5407297 5412735
#>  $ : num [1:4] 488006 497239 5385526 5393899
#>  $ : num [1:4] 500476 505427 5409888 5413864
#>  $ : num [1:4] 422718 443693 5364752 5398969
#>  $ : num [1:4] 436393 455345 5390197 5405569
#>  $ : num [1:4] 448464 450390 5404212 5404797
#>  $ : num [1:4] 459377 465847 5397255 5404093
#>  $ : num [1:4] 475502 482058 5400269 5408109
#>  $ : num [1:4] 476793 484115 5394878 5400905
#>  $ : num [1:4] 495936 499733 5389589 5396934
#>  $ : num [1:4] 425270 449448 5353987 5383046
#>  $ : num [1:4] 439927 462874 5358223 5393865
#>  $ : num [1:4] 468039 477380 5378281 5388344
#>  $ : num [1:4] 457541 466788 5403285 5409104
#>  $ : num [1:4] 495086 500372 5393786 5399069
#>  $ : num [1:4] 503880 507861 5396801 5404336
#>  $ : num [1:4] 502595 507481 5402752 5406382
#>  $ : num [1:4] 506075 509426 5404305 5408099
#>  $ : num [1:4] 506637 511687 5405460 5409603
```

This makes for a very lightweight summary data set that will scale to hundreds of large inputs.

``` r
dat <- as.data.frame(vapour_read_attributes(mvfile), 
                     stringsAsFactors = FALSE)
library(raster)
#> Loading required package: sp
#> 
#> Attaching package: 'raster'
#> The following object is masked from 'package:dplyr':
#> 
#>     select
dat$bbox <- vapour_read_extent(mvfile)

plot(purrr::reduce(lapply(dat$bbox, raster::extent), raster::union))
purrr::walk(lapply(dat$bbox, raster::extent), plot, add = TRUE)
```

![](README-unnamed-chunk-7-1.png)

An example is this set of *some number of* property boundary shapefiles, read into a few hundred Mb of simple features.

``` r
library(dplyr)
files <- raadfiles::thelist_files(format = "") %>% filter(grepl("parcel", fullname), grepl("shp$", fullname)) %>% 
  slice(1:8)
#> Warning in raadfiles::thelist_files(format = ""): datadir and file root
#> don't match?
library(vapour)
system.time(purrr::map(files$fullname, sf::read_sf))
#>    user  system elapsed 
#>   9.005   0.175   9.255
library(blob)

## our timing is competitive, and we get to choose what is read
## and when
read_table <- function(file) as.data.frame(vapour_read_attributes(file),  stringsAsFactors = FALSE)
system.time({
d <- purrr::map(files$fullname, read_table)
d <- dplyr::bind_rows(d)
g <- purrr::map(files$fullname, vapour_read_geometry)
d[["wkb"]] <- new_blob(unlist(g, recursive = FALSE))
})
#>    user  system elapsed 
#>   3.305   0.408   3.780
```

We can read that in this simpler way for a quick data set to act as an index.

``` r
system.time({
  d <- purrr::map_df(files$fullname, read_table)
  d$bbox <- unlist(purrr::map(files$fullname, vapour_read_extent), recursive = FALSE)
})
#>    user  system elapsed 
#>   3.051   0.373   3.463

pryr::object_size(d)
#> 46.7 MB
glimpse(d)
#> Observations: 107,854
#> Variables: 20
#> $ CID        <chr> "", "", "", "", "", "", "", "", "", "", "", "", "",...
#> $ VOLUME     <chr> "169864", "", "", "136703", "", "", "212990", "2449...
#> $ FOLIO      <int> 2, 0, 0, 1, 0, 0, 1, 1, 0, 1, 0, 0, 9, 0, 1, 0, 8, ...
#> $ PID        <chr> "", "", "", "", "", "", "", "", "", "", "", "", "",...
#> $ POT_PID    <chr> "", "", "", "", "", "", "", "", "", "", "", "", "",...
#> $ LPI        <chr> "FEV10", "KKL85", "", "HSY23", "GES42", "", "480244...
#> $ CAD_TYPE1  <chr> "Private Parcel", "Authority Land", "Authority Land...
#> $ CAD_TYPE2  <chr> "Private Parcel", "Forestry Tasmania", "Forestry Ta...
#> $ TENURE_TY  <chr> "Freehold Title", "Crown Land", "Crown Land", "Crow...
#> $ FEAT_NAME  <chr> "", "", "", "", "", "", "", "", "", "", "", "", "",...
#> $ STRATA_LEV <chr> "Not Applicable", "Not Applicable", "Not Applicable...
#> $ COMP_AREA  <dbl> 1200513.096, 23096.094, 148147.128, 4603209.423, 48...
#> $ MEAS_AREA  <dbl> 1207000, 0, 0, 0, 0, 0, 0, 136700, 0, 0, 0, 18260, ...
#> $ UFI        <chr> "cad013844403", "cad013933348", "cad013933302", "ca...
#> $ FMP        <chr> "cad000029000", "cad000029000", "cad000029000", "ca...
#> $ CREATED_ON <chr> "2015-08-25 14:31:57", "2016-04-07 10:22:12", "2016...
#> $ LIST_GUID  <chr> "{d1b80f74-2873-46d1-a6ed-d1d27a45bd6e}", "{3cb147d...
#> $ SHAPE_AREA <dbl> 1200513.096, 23096.094, 148147.128, 4603209.423, 48...
#> $ SHAPE_LEN  <dbl> 4382.7258, 2365.0896, 14870.8324, 16288.4164, 14990...
#> $ bbox       <list> [<551456.8, 552738.1, 5413518.9, 5414799.6>, <5544...
```

Set up
------

I've kept a record of a minimal GDAL wrapper package here:

<https://github.com/mdsumner/gdalmin>

This must be run when your function definitions change:

``` r
tools::package_native_routine_registration_skeleton("../vapour", "src/init.c",character_only = FALSE)
```

Context
-------

My first real attempt at DBI abstraction is here, this is still an aspect that is desperately needed in R to help bring tidyverse attention to spatial:

<https://github.com/mdsumner/RGDALSQL>

Before that I had worked on getting sp and dplyr to at least work together <https://github.com/dis-organization/sp_dplyrexpt> and recently rgdal was updated to allow tibbles to be used, something that spbabel and spdplyr really needed to avoid friction.

Early exploration of allow non-geometry read with rgdal was tried here: <https://github.com/r-gris/gladr>

Big thanks to Edzer Pebesma and Roger Bivand and Tim Keitt for prior art that I crib and copy from. Jeroen Ooms helped the R community hugely by providing an automatable build process for libraries on Windows. Mark Padgham helped kick me over a huge obstacle in using C++ libraries with R. Simon Wotherspoon and Ben Raymond have endured my ravings about wanting this level of control for many years.

Code of conduct
===============

Please note that this project is released with a [Contributor Code of Conduct](CONDUCT.md). By participating in this project you agree to abide by its terms.
