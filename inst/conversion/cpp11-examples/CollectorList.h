#ifndef COLLECTORLIST_H
#define COLLECTORLIST_H

#include <cpp11.hpp>

// Growing list, adapted from Jim Hester's fs package CollectorList.
// cpp11::writable::list grows efficiently (unlike Rcpp::List::push_back),
// so this is now a thin wrapper.
class CollectorList {
  cpp11::writable::list data_;

public:
  CollectorList() {}

  void push_back(SEXP x) {
    data_.push_back(x);
  }

  cpp11::list vector() {
    return data_;
  }
};

#endif
