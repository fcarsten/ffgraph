#include <container/AVLSet.h>
#include "../container/MFSet.cc"
#include "../container/AVL.cc"
#include "../container/AVLSet.cc"
#include "test_check.h"
#include <set>
int CMP(int& a, int& b) { return (a > b) - (a < b); }
int EQ(int& a, int& b) { return a == b; }

int main() {
  AVLSet<int> a, b;
  for (int x : {1, 3, 5}) a.add(x);
  for (int x : {1, 2, 3, 4, 5, 6}) b.add(x);
  FF_CHECK(a.OK() && b.OK());
  FF_CHECK(a <= b); // ordered subset, with interspersed missing elements
  FF_CHECK(!(b <= a));
  AVLSet<int> copy(a);
  FF_CHECK(copy.OK() && copy == a);
  copy |= b; FF_CHECK(copy.OK() && copy == b);
  copy &= a; FF_CHECK(copy.OK() && copy == a);
  copy -= a; FF_CHECK(copy.empty() && copy.OK());
  b -= b; FF_CHECK(b.empty() && b.OK());
}
