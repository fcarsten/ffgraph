#include <iostream>
#include "test_check.h"
#include <string>
inline unsigned int HASH(int& x) { return static_cast<unsigned int>(x * 2654435761u); }
inline int EQ(int& a, int& b) { return a == b; }
#include "../include/container/VHMap.h"
#include "../container/MFMap.cc"
int main() {
  VHMap<int, std::string> m(std::string("default"), 2);
  FF_CHECK(m.length() == 0 && m.OK());
  m[1] = "one"; m[2] = "two"; m[3] = "three";
  FF_CHECK(m.length() == 3 && m[1] == "one" && m.contains(2));
  Pix p = m.seek(2); FF_CHECK(p && m.key(p) == 2 && m.contents(p) == "two" && m.owns(p));
  int seen = 0; for (Pix i=m.first(); i; m.next(i)) ++seen; FF_CHECK(seen == 3);
  m.del(2); FF_CHECK(!m.contains(2) && m.length() == 2 && m.OK());
  VHMap<int, std::string> copy(m); FF_CHECK(copy.length() == 2 && copy[1] == "one");
  copy.clear(); FF_CHECK(copy.empty() && copy.OK());
}
