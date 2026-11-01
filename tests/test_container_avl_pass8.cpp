#include <container/AVLSet.h>
#include "../container/MFSet.cc"
#include "../container/AVL.cc"
#include "../container/AVLSet.cc"
#include <cassert>
#include <set>
int CMP(int& a, int& b) { return (a > b) - (a < b); }
int EQ(int& a, int& b) { return a == b; }

int main() {
  AVLSet<int> a, b;
  for (int x : {1, 3, 5}) a.add(x);
  for (int x : {1, 2, 3, 4, 5, 6}) b.add(x);
  assert(a.OK() && b.OK());
  assert(a <= b); // ordered subset, with interspersed missing elements
  assert(!(b <= a));
  AVLSet<int> copy(a);
  assert(copy.OK() && copy == a);
  copy |= b; assert(copy.OK() && copy == b);
  copy &= a; assert(copy.OK() && copy == a);
  copy -= a; assert(copy.empty() && copy.OK());
  b -= b; assert(b.empty() && b.OK());
}
