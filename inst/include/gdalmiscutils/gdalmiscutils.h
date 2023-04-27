#ifndef GDALMISCUTILS_H
#define GDALMISCUTILS_H

#include <Rcpp.h>

namespace gdalmiscutils {
using namespace Rcpp;

inline NumericVector limit_skip_to_start_end(IntegerVector skip_n, IntegerVector limit_n) {
  R_xlen_t start = 0;
  R_xlen_t end = 0;
  if (skip_n[0] > 0) {  // silently ignore negative values
    start = (R_xlen_t)skip_n[0];
  }
  if (limit_n[0] > 0) { // silently ignore negative values
    end = start + (R_xlen_t)limit_n[0] - 1;
  } else {
    end = -1; // signal to read all available
  }
  NumericVector out(2);
  out[0] = (double)start; out[1] = (double)end;
  return  out;
}

inline NumericVector limit_skip_n_to_start_end_len(IntegerVector skip_n, IntegerVector limit_n, NumericVector n) {
  R_xlen_t start = 0;
  R_xlen_t end = (R_xlen_t)n[0] - 1;
  if (skip_n[0] > 0) {  // silently ignore negative values
    start = (R_xlen_t)skip_n[0];
  }
  if (limit_n[0] > 0) { // silently ignore negative values
    end = start + (R_xlen_t)limit_n[0] - 1;
  }
  if (is_infinite(skip_n)[0]) {
    Rcpp::warning("skip_n not a valid value, assuming 'skip_n = 0'"); 
  }
  if (n[0] > 0 && start >= (R_xlen_t)n[0] ) {
    Rcpp::stop("skip_n skips all available features");
  }
  if (end > ((R_xlen_t)n[0] - 1)) {
    if (start > 0) {
      Rcpp::warning("limit_n is greater than the number of available features (given 'skip_n')");
    } else {
      Rcpp::warning("limit_n is greater than the number of available features");
    }
    end = (R_xlen_t)n[0] - 1;
  }
  R_xlen_t len = end - start + 1;
  NumericVector out(3);
  out[0] = (double)start; out[1] = (double)end; out[2] = (double)len;
  return  out;
}

} // GDALMISCUTILS
#endif
