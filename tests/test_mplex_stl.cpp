#include <container/MPlex.h>
// The historical build explicitly instantiates this template implementation.
#include "../container/MFPlex.cc"
#include "test_check.h"

int main() {
  MPlex<int> p;
  FF_CHECK(p.length() == 0 && p.empty() && p.OK());

  FF_CHECK(p.add_high(10) == 0);
  FF_CHECK(p.add_high(20) == 1);
  FF_CHECK(p.add_low(5) == -1);
  FF_CHECK(p.length() == 3 && p.count() == 3 && p.OK());
  FF_CHECK(p[-1] == 5 && p[0] == 10 && p[1] == 20);

  Pix it = p.first();
  FF_CHECK(it && p(it) == 5);
  p.next(it); FF_CHECK(it && p(it) == 10);
  p.next(it); FF_CHECK(it && p(it) == 20);
  p.next(it); FF_CHECK(!it);

  FF_CHECK(p.del_index(0) == 1);
  FF_CHECK(!p.valid(0) && p.available() == 1 && p.count() == 2 && p.OK());
  const int hole = p.unused_index();
  FF_CHECK(hole == 0);
  p.undel_index(hole);
  FF_CHECK(p.valid(0) && p[0] == 10 && p.OK());

  p.del_high();
  FF_CHECK(p.length() == 2 && p.high() == 0 && p.OK());
  p.adjust_bounds();
  FF_CHECK(p.low() == -1 && p.high() == 0 && p.length() == 2 && p.OK());

  MPlex<int> copy(p);
  FF_CHECK(copy.length() == 2 && copy[-1] == 5 && copy[0] == 10 && copy.OK());
  copy.reverse();
  FF_CHECK(copy[-1] == 10 && copy[0] == 5 && copy.OK());
  copy.reset_low(10);
  FF_CHECK(copy.low() == 10 && copy.high() == 11 && copy[10] == 10 && copy[11] == 5 && copy.OK());

  MPlex<int> tail;
  tail.add_high(30);
  tail.add_high(40);
  copy.append(tail);
  FF_CHECK(copy[12] == 30 && copy[13] == 40 && copy.OK());
  copy.clear();
  FF_CHECK(copy.empty() && copy.length() == 0 && copy.OK());

  MPlex<int> left(-7,4),right(20,9);
  left.add_high(11);left.add_high(22);left.del_index(-7);
  right.add_high(33);right.add_high(44);
  const Pix lp=left.first(),rp=right.first();
  int* la=&left(lp);int* ra=&right(rp);
  static_assert(noexcept(left.swap(right)),"rollback exchange must not allocate or throw");
  left.swap(right);
  FF_CHECK(left.low()==20 && right.low()==-6 && left.OK() && right.OK());
  FF_CHECK(&left(rp)==ra && &right(lp)==la && left(rp)==33 && right(lp)==22);
  FF_CHECK(right.available()==1 && !right.valid(-7));
  left.swap(right);
  FF_CHECK(&left(lp)==la && &right(rp)==ra && left.available()==1);
  left.swap(left);FF_CHECK(left.OK() && &left(lp)==la);
}
