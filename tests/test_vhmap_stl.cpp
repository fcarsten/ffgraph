#include <iostream>
#include <cassert>
#include <string>
inline unsigned int HASH(int& x) { return static_cast<unsigned int>(x * 2654435761u); }
inline int EQ(int& a, int& b) { return a == b; }
#include "../include/container/VHMap.h"
#include "../container/MFMap.cc"
int main() {
  VHMap<int, std::string> m(std::string("default"), 2);
  assert(m.length() == 0 && m.OK());
  m[1] = "one"; m[2] = "two"; m[3] = "three";
  assert(m.length() == 3 && m[1] == "one" && m.contains(2));
  Pix p = m.seek(2); assert(p && m.key(p) == 2 && m.contents(p) == "two" && m.owns(p));
  int seen = 0; for (Pix i=m.first(); i; m.next(i)) ++seen; assert(seen == 3);
  m.del(2); assert(!m.contains(2) && m.length() == 2 && m.OK());
  VHMap<int, std::string> copy(m); assert(copy.length() == 2 && copy[1] == "one");
  copy.clear(); assert(copy.empty() && copy.OK());
}
