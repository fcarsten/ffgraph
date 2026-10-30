#include <container/MPlex.h>
// The historical build explicitly instantiates this template implementation.
#include "../container/MFPlex.cc"
#include <cassert>

int main() {
  MPlex<int> p;
  assert(p.length() == 0 && p.empty() && p.OK());

  assert(p.add_high(10) == 0);
  assert(p.add_high(20) == 1);
  assert(p.add_low(5) == -1);
  assert(p.length() == 3 && p.count() == 3 && p.OK());
  assert(p[-1] == 5 && p[0] == 10 && p[1] == 20);

  Pix it = p.first();
  assert(it && p(it) == 5);
  p.next(it); assert(it && p(it) == 10);
  p.next(it); assert(it && p(it) == 20);
  p.next(it); assert(!it);

  assert(p.del_index(0) == 1);
  assert(!p.valid(0) && p.available() == 1 && p.count() == 2 && p.OK());
  const int hole = p.unused_index();
  assert(hole == 0);
  p.undel_index(hole);
  assert(p.valid(0) && p[0] == 10 && p.OK());

  p.del_high();
  assert(p.length() == 2 && p.high() == 0 && p.OK());
  p.adjust_bounds();
  assert(p.low() == -1 && p.high() == 0 && p.length() == 2 && p.OK());

  MPlex<int> copy(p);
  assert(copy.length() == 2 && copy[-1] == 5 && copy[0] == 10 && copy.OK());
  copy.reverse();
  assert(copy[-1] == 10 && copy[0] == 5 && copy.OK());
  copy.reset_low(10);
  assert(copy.low() == 10 && copy.high() == 11 && copy[10] == 10 && copy[11] == 5 && copy.OK());

  MPlex<int> tail;
  tail.add_high(30);
  tail.add_high(40);
  copy.append(tail);
  assert(copy[12] == 30 && copy[13] == 40 && copy.OK());
  copy.clear();
  assert(copy.empty() && copy.length() == 0 && copy.OK());
}
