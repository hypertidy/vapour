#ifndef GDALMISCUTILS_H
#define GDALMISCUTILS_H

#include <cpp11.hpp>

namespace gdalmiscutils {
using namespace cpp11;
namespace writable = cpp11::writable;

inline doubles limit_skip_to_start_end(integers skip_n, integers limit_n) {
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
  writable::doubles out(2);
  out[0] = (double)start; out[1] = (double)end;
  return out;
}

inline doubles limit_skip_n_to_start_end_len(integers skip_n, integers limit_n, doubles n) {
  R_xlen_t start = 0;
  R_xlen_t end = (R_xlen_t)n[0] - 1;
  if (skip_n[0] > 0) {  // silently ignore negative values
    start = (R_xlen_t)skip_n[0];
  }
  if (limit_n[0] > 0) { // silently ignore negative values
    end = start + (R_xlen_t)limit_n[0] - 1;
  }
  if (!R_finite(static_cast<double>(skip_n[0]))) {
    cpp11::warning("skip_n not a valid value, assuming 'skip_n = 0'");
  }
  if (n[0] > 0 && start >= (R_xlen_t)n[0] ) {
    cpp11::stop("skip_n skips all available features");
  }
  if (end > ((R_xlen_t)n[0] - 1)) {
    if (start > 0) {
      cpp11::warning("limit_n is greater than the number of available features (given 'skip_n')");
    } else {
      cpp11::warning("limit_n is greater than the number of available features");
    }
    end = (R_xlen_t)n[0] - 1;
  }
  R_xlen_t len = end - start + 1;
  writable::doubles out(3);
  out[0] = (double)start; out[1] = (double)end; out[2] = (double)len;
  return out;
}

} // GDALMISCUTILS
#endif
