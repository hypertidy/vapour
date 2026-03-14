#ifndef COMMON_VAPOUR_H
#define COMMON_VAPOUR_H


#include <vector>
#include <cpp11.hpp>

// written by R Hijmans in terra
inline std::vector<char *> string_to_charptr(std::vector<std::string> s) {
	size_t n = s.size();
	std::vector<char *> out(n + 1);
	for (size_t i = 0; i < n; i++) {
		out[i] = (char *) (s[i].c_str());
	}
	out[n] = NULL;
	return out;
}

// Extract const char* from a cpp11 r_string (e.g. from strings[0])
// without allocating a temporary std::string.
inline const char* as_cstr(cpp11::r_string x) {
  return CHAR(static_cast<SEXP>(x));
}

#endif
