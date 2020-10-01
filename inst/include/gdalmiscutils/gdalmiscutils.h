#ifndef GDALMISCUTILS_H
#define GDALMISCUTILS_H

#include <Rcpp.h>

namespace gdalmiscutils {
using namespace Rcpp;

inline IntegerVector limit_skip_n_to_start_end_len(IntegerVector skip_n, IntegerVector limit_n, IntegerVector n) {
  int start = 0;
  int end = n[0] - 1;
  if (skip_n[0] > 0) {  // silently ignore negative values
    start = skip_n[0];
  }
  if (limit_n[0] > 0) { // silently ignore negative values
    end = start + limit_n[0] - 1;
  }
  if (start >= n[0]) {
    Rcpp::stop("skip_n skips all available features");
  }
  if (end > n[0]) {
    if (start > 0) {
      Rcpp::warning("limit_n is greater than the number of available features (given 'skip_n')");
    } else {
      Rcpp::warning("limit_n is greater than the number of available features");
    }
    end = n[0] - 1;
  }
  int len = end - start + 1;
  IntegerVector out(3);
  out[0] = start; out[1] = end; out[2] = len;
  return  out;
}

} // GDALMISCUTILS
#endif
