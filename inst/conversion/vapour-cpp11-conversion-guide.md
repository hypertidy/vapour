# vapour: Rcpp → cpp11 Conversion Guide
## Prepared for the big-bang conversion

### Setup

1. Add to DESCRIPTION:
   ```
   LinkingTo: cpp11, Rcpp    # both during transition, then remove Rcpp
   ```
   (keep Rcpp in Imports until RcppExports.R is replaced)

2. Run `usethis::use_cpp11()` — but DON'T let it remove Rcpp yet

3. Add to Makevars.in:
   ```
   PKG_CPPFLAGS=@PKG_CPPFLAGS@ -I../inst/include/
   ```
   (unchanged — cpp11 is header-only, picked up via LinkingTo)

4. You can vendor cpp11 headers with `cpp11::vendor_cpp11()` if you want
   them in `inst/include/cpp11/` for reproducibility.

---

### Mechanical substitutions (all headers and src files)

#### Includes
```
#include <Rcpp.h>          →  #include <cpp11.hpp>
                              #include <cpp11/R.hpp>       // if using R API directly
```

#### Namespace
```
using namespace Rcpp;      →  using namespace cpp11;
                              namespace writable = cpp11::writable;
```

#### Registration
```
// [[Rcpp::export]]        →  [[cpp11::register]]
```

#### Read-only vector types (function parameters)
```
CharacterVector            →  strings
NumericVector              →  doubles
IntegerVector              →  integers
LogicalVector              →  logicals
RawVector                  →  raws
List                       →  list        (read-only, for input params)
```

#### Writable vector types (local construction, return values)
```
CharacterVector out(n);    →  writable::strings out(n);
NumericVector out(n);      →  writable::doubles out(n);
IntegerVector out(n);      →  writable::integers out(n);
LogicalVector out(n);      →  writable::logicals out(n);
RawVector raw(n);          →  writable::raws raw(n);
List out(n);               →  writable::list out(n);
```

#### Named list creation
```
Rcpp::List::create(        →  writable::list out;
  Rcpp::Named("a") = x,       using namespace cpp11::literals;
  Rcpp::Named("b") = y        out.push_back({"a"_nm = x, "b"_nm = y});
);                             // OR build positionally then set names:
                               writable::list out = {x, y};
                               out.names() = {"a", "b"};
```

#### Static creation
```
NumericVector::create(a, b)  →  writable::doubles({a, b})
  // or:
  writable::doubles out = {a, b};
IntegerVector::create(0)     →  writable::integers({0})
CharacterVector::create(s)   →  writable::strings({s})
```

#### Element access
```
out[0] = x;                →  out[0] = x;          // same for writable
x = vec[0];                →  x = vec[0];           // same for read-only
```

#### String element access
```
(const char*) cv[0]        →  std::string(cv[0])   // or CHAR(STRING_ELT(cv, 0))
cv[0].empty()              →  cv[0] == ""           // cpp11::r_string has no .empty()
                              // OR: std::string(cv[0]).empty()
NA_STRING                  →  NA_STRING             // unchanged (R macro)
```

#### Attributes
```
out.attr("names") = names;  →  out.attr("names") = names;  // same
out.names() = names;        →  out.names() = {"a", "b"};   // same
```

#### Error handling
```
Rcpp::stop("msg %s", x);   →  cpp11::stop("msg %s", x.c_str());
                               // cpp11::stop uses printf-style, needs c_str() for std::string
Rcpp::warning("msg");       →  cpp11::warning("msg");
```

#### Calling R functions from C++
```
Rcpp::Function f("fun");    →  auto f = cpp11::package("base")["fun"];
f(x, y);                      f(x, y);
// OR for known packages:
Environment pkg = Environment::namespace_env("vapour");
Function f = pkg["fun"];    →  auto f = cpp11::package("vapour")["fun"];
```

#### Specific vapour patterns

##### is_infinite (gdalmiscutils.h)
```
is_infinite(skip_n)[0]     →  !R_finite(static_cast<double>(skip_n[0]))
```

##### CollectorList.h
Replace with cpp11 growing list:
```cpp
// OLD: CollectorList feature_xx; feature_xx.push_back(val);
// NEW:
writable::list feature_xx;
feature_xx.push_back(val);
// cpp11 writable::list already grows efficiently
```

##### RObject / XPtr (gdalarrowstream)
```
Rcpp::RObject              →  cpp11::sexp
R_MakeExternalPtr(...)     →  unchanged (raw R API, works in cpp11)
```

##### PROTECT/UNPROTECT (gdalraster.h nativeRaster, gdallibrary.h crs_is_lonlat)
These use raw R API which works unchanged under cpp11.
BUT: consider replacing with cpp11 types where possible:
```cpp
// OLD:
SEXP out = PROTECT(Rf_allocVector(LGLSXP, 1));
SET_LOGICAL_ELT(out, 0, false);
UNPROTECT(1);
return out;
// NEW:
writable::logicals out(1);
out[0] = FALSE;
return out;
```

##### Rcpp::Named for list element setting
```
// OLD (gdallibrary.h gdal_list_drivers):
Rcpp::List out = Rcpp::List::create(
  Rcpp::Named("driver") = sname,
  Rcpp::Named("name") = lname, ...);

// NEW:
using namespace cpp11::literals;
writable::list out = {
  "driver"_nm = sname,
  "name"_nm = lname, ...
};
```

##### Rcpp::Function ISOdatetime/ISOdate (gdalgeometry.h)
```
// OLD:
Rcpp::Function ISOdatetime("ISOdatetime");
Rcpp::NumericVector ret = ISOdatetime(Year, Month, Day, Hour, Minute, Second, tzone);

// NEW:
auto ISOdatetime = cpp11::package("base")["ISOdatetime"];
cpp11::sexp ret = ISOdatetime(Year, Month, Day, Hour, Minute, Second, tzone);
double val = cpp11::doubles(ret)[0];
```

##### gdal_layer() calling back to R (gdallibrary.h line ~153)
```
// OLD:
Environment vapour = Environment::namespace_env("vapour");
Function vapour_getenv_sql_dialect = vapour["vapour_getenv_sql_dialect"];
CharacterVector R_dialect = vapour_getenv_sql_dialect();
const char *sql_dialect = (const char *)R_dialect[0];

// NEW:
auto vapour_getenv_sql_dialect = cpp11::package("vapour")["vapour_getenv_sql_dialect"];
cpp11::strings R_dialect(vapour_getenv_sql_dialect());
const char *sql_dialect = CHAR(STRING_ELT(R_dialect, 0));
```

---

### Conversion order (recommended)

1. **gdalmiscutils.h** (55 lines, no GDAL deps)
2. **common_vapour.h** (13 lines, no Rcpp at all — already done)
3. **CollectorList.h** (26 lines)
4. **gdalapplib.h** (215 lines)
5. **gdalreadwrite.h** (269 lines)
6. **gdalraster.h** (1350 lines — the big one, has nativeRaster PROTECT)
7. **gdallibrary.h** (988 lines — calls R functions, has CollectorList)
8. **gdalgeometry.h** (659 lines — ISOdatetime/ISOdate calls)
9. **gdalwarpgeneral.h** (277 lines)
10. **gdalarrowstream/gdalvectorstream.h** (298 lines — RObject/XPtr)
11. **All src/*.cpp files** (once headers compile)
12. Delete `src/RcppExports.cpp` and `R/RcppExports.R`
13. Run `cpp11::cpp_register()` to generate new bindings
14. Remove Rcpp from LinkingTo and Imports

### After conversion

- Run `devtools::document()` to regenerate NAMESPACE
- Run tests from step 1
- `R CMD check`

### Key differences to watch

- cpp11 does NOT do implicit type coercion (double→int etc) — add explicit casts
- cpp11 `stop()`/`warning()` are printf-style, NOT C++ stream style — use `%s` with `.c_str()`
- writable vectors don't have `push_back` with preallocated capacity like Rcpp — but they DO grow efficiently (unlike Rcpp which copies on every push_back)
- No `Rcpp::RNGScope` needed — cpp11 doesn't wrap this
- `[[cpp11::register]]` is a C++11 attribute, not a comment — must be on its own line before the function
