#include <cassert>
#include <cstring>
extern void (*lib_error_handler)(const char*, const char*);
#include <container/VOHSet.h>
#include <container/VHMap.h>
#include <container/VHBag.h>
#include "../container/MFSet.cc"
#include "../container/MFMap.cc"
#include "../container/MFBag.cc"

struct Item { int value; };
inline int EQ(Item*& a, Item*& b) { return a == b; }
int EQ(int& a, int& b) { return a == b; }
void (*lib_error_handler)(const char*, const char*) = nullptr;

int main() {
  Item a{1}, b{2};

  // Arithmetic keys use standard hash/equality rather than unresolved
  // legacy HASH<int>/EQ<int> template declarations.
  VOHSet<int> integers;
  integers.add(17);
  integers.add(23);
  integers.add(17);
  assert(integers.length() == 2 && integers.contains(17) && !integers.contains(99));

  VHMap<int, const char*> intMap(nullptr);
  intMap[17] = "seventeen";
  assert(std::strcmp(intMap[17], "seventeen") == 0);

  VHBag<int> intBag;
  intBag.add(4);
  intBag.add(4);
  intBag.add(9);
  assert(intBag.length() == 3 && intBag.nof(4) == 2 && intBag.contains(9));

  VOHSet<Item*> set;
  Pix sa = set.add(&a);
  set.add(&b);
  set.add(&a);
  assert(set.length() == 2 && set.contains(&a) && set.contains(&b));
  assert(set.owns(sa) && set(sa) == &a);

  VHMap<Item*, int> map(0);
  map[&a] = 10;
  map[&b] = 20;
  assert(map.length() == 2 && map[&a] == 10 && map[&b] == 20);

  VHBag<Item*> bag;
  bag.add(&a);
  bag.add(&a);
  bag.add(&b);
  assert(bag.length() == 3 && bag.nof(&a) == 2 && bag.contains(&b));
  Pix bp = bag.seek(&a);
  assert(bp && bag(bp) == &a);
  Pix bp2 = bag.seek(&a, bp);
  assert(bp2 && bp2 != bp && bag(bp2) == &a);
}
