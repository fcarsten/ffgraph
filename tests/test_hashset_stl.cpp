#include <cstddef>
inline unsigned int HASH(int& x) { return static_cast<unsigned int>(x); }
inline int EQ(int& a, int& b) { return a == b; }
#include <container/VHSet.h>
#include "../container/MFSet.cc"
#include "test_check.h"

int main() {
  VHSet<int> a;
  Pix p1 = a.add(10);
  a.add(20);
  a.add(10); // duplicate must not increase set size
  FF_CHECK(a.length() == 2 && a.contains(10) && a.contains(20));
  FF_CHECK(a.owns(p1) && a(p1) == 10);

  Pix p = a.first();
  int visited = 0;
  while (p) { ++visited; a.next(p); }
  FF_CHECK(visited == 2);

  VHSet<int> b;
  b.add(20); b.add(30);
  a |= b;
  FF_CHECK(a.length() == 3 && a.contains(30));
  a -= b;
  FF_CHECK(a.length() == 1 && a.contains(10));
  FF_CHECK(a.OK());
  a.clear();
  FF_CHECK(a.empty() && a.OK());
}
