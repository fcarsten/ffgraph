#include <cstddef>
inline unsigned int HASH(int& x) { return static_cast<unsigned int>(x); }
inline int EQ(int& a, int& b) { return a == b; }
#include <container/VHSet.h>
#include "../container/MFSet.cc"
#include <cassert>

int main() {
  VHSet<int> a;
  Pix p1 = a.add(10);
  a.add(20);
  a.add(10); // duplicate must not increase set size
  assert(a.length() == 2 && a.contains(10) && a.contains(20));
  assert(a.owns(p1) && a(p1) == 10);

  Pix p = a.first();
  int visited = 0;
  while (p) { ++visited; a.next(p); }
  assert(visited == 2);

  VHSet<int> b;
  b.add(20); b.add(30);
  a |= b;
  assert(a.length() == 3 && a.contains(30));
  a -= b;
  assert(a.length() == 1 && a.contains(10));
  assert(a.OK());
  a.clear();
  assert(a.empty() && a.OK());
}
