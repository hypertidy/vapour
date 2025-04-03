#include <vector>


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
