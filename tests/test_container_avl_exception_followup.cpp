#include <container/AVLSet.h>
#include <container/AVLMap.h>
#include "../container/MFSet.cc"
#include "../container/MFMap.cc"
#include "../container/AVL.cc"
#include "../container/AVLSet.cc"
#include "../container/AVLMap.cc"
#include <cassert>
#include <stdexcept>

struct Tracked {
  int value = 0;
  static int alive;
  static int copies_before_throw;
  explicit Tracked(int v = 0) : value(v) { ++alive; }
  Tracked(const Tracked& src) : value(src.value) {
    if (copies_before_throw == 0) throw std::runtime_error("injected copy failure");
    if (copies_before_throw > 0) --copies_before_throw;
    ++alive;
  }
  Tracked& operator=(const Tracked& src) { value = src.value; return *this; }
  ~Tracked() { --alive; }
};
int Tracked::alive = 0;
int Tracked::copies_before_throw = -1;
int CMP(Tracked& a, Tracked& b) { return (a.value > b.value) - (a.value < b.value); }
int EQ(Tracked& a, Tracked& b) { return a.value == b.value; }
int CMP(int& a, int& b) { return (a > b) - (a < b); }
int EQ(int& a, int& b) { return a == b; }

int main() {
  {
    AVLSet<Tracked> src;
    for (int x : {2, 4, 6, 8}) src.add(Tracked(x));
    AVLSet<Tracked> dst;
    for (int x : {1, 3}) dst.add(Tracked(x));
    assert(src.OK() && dst.OK());
    const int baseline = Tracked::alive;
    Tracked::copies_before_throw = 2;
    bool failed = false;
    try { AVLSet<Tracked> copy(src); } catch (const std::runtime_error&) { failed = true; }
    Tracked::copies_before_throw = -1;
    assert(failed && Tracked::alive == baseline); // no nodes leaked by failed copy ctor
    Tracked::copies_before_throw = 2;
    failed = false;
    try { dst = src; } catch (const std::runtime_error&) { failed = true; }
    Tracked::copies_before_throw = -1;
    assert(failed && Tracked::alive == baseline);
    assert(dst.OK() && dst.length() == 2 && dst.contains(Tracked(1)) && dst.contains(Tracked(3)));
    // Root and child-node insertion failures must preserve the count.
    AVLSet<Tracked> empty;
    Tracked::copies_before_throw = 1; // value parameter succeeds; node copy fails
    failed = false;
    try { empty.add(Tracked(10)); } catch (const std::runtime_error&) { failed = true; }
    Tracked::copies_before_throw = -1;
    assert(failed && empty.OK() && empty.empty());
    Tracked::copies_before_throw = 1;
    failed = false;
    try { src.add(Tracked(10)); } catch (const std::runtime_error&) { failed = true; }
    Tracked::copies_before_throw = -1;
    assert(failed && src.OK() && src.length() == 4);
    dst = src;
    assert(dst.OK() && dst == src);
    dst = dst;
    assert(dst.OK() && dst == src);
  }
  assert(Tracked::alive == 0);
  {
    AVLMap<int, Tracked> src(Tracked(99));
    src[2] = Tracked(22);
    src[4] = Tracked(44);
    src[6] = Tracked(66);
    const int baseline = Tracked::alive;
    Tracked::copies_before_throw = 3;
    bool failed = false;
    try { AVLMap<int, Tracked> copy(src); } catch (const std::runtime_error&) { failed = true; }
    Tracked::copies_before_throw = -1;
    assert(failed && Tracked::alive == baseline); // no partially copied map nodes
    assert(src.OK() && src.length() == 3);
    // A failing node allocation must not corrupt count or the threaded tree.
    Tracked::copies_before_throw = 1;
    failed = false;
    try { src[8]; } catch (const std::runtime_error&) { failed = true; }
    Tracked::copies_before_throw = -1;
    assert(failed && src.OK() && src.length() == 3 && !src.contains(8));
    AVLMap<int, Tracked> empty(Tracked(-1));
    Tracked::copies_before_throw = 0;
    failed = false;
    try { empty[1]; } catch (const std::runtime_error&) { failed = true; }
    Tracked::copies_before_throw = -1;
    assert(failed && empty.OK() && empty.empty());
  }
  assert(Tracked::alive == 0);
}
