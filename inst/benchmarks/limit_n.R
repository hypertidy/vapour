benchmark(sf = read_sf(f),
          v = {d <- vapour_read_attributes(f)
          d$geometry <- st_as_sfc(vapour_read_geometry(f), crs = vapour:::vapour_projection_info_cpp(f))
          st_as_sf(as_tibble(d))}, replications = 200)
# test replications elapsed relative user.self sys.self user.child sys.child
# 1   sf          200   4.122    1.000     3.744    0.380          0         0
# 2    v          200   4.921    1.194     4.840    0.084          0         0



benchmark(sf = read_sf(f)[1:10, ],
          v = {d <- vapour_read_attributes(f, limit_n = 10)
          d$geometry <- st_as_sfc(vapour_read_geometry(f, limit_n = 10), crs = vapour:::vapour_projection_info_cpp(f))
          st_as_sf(as_tibble(d))}, replications = 200)
# test replications elapsed relative user.self sys.self user.child sys.child
# 1   sf          200   3.263     3.34     3.204    0.056          0         0
# 2    v          200   0.977     1.00     0.956    0.020          0         0

