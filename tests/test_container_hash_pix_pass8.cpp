extern void (*lib_error_handler)(const char*, const char*);
#include <container/VOHSet.h>
#include <container/VHMap.h>
#include <container/VHBag.h>
#include "../container/MFSet.cc"
#include "../container/MFMap.cc"
#include "../container/MFBag.cc"
#include "test_check.h"
#include <set>
#include <random>

int EQ(int& lhs, int& rhs) { return lhs == rhs; }
void (*lib_error_handler)(const char*, const char*) = nullptr;

int main() {
  VOHSet<int> set;
  Pix erased = set.add(11);
  Pix retained = set.add(22);
  set.del(11);
  FF_CHECK(!set.owns(erased)); // never dereference an invalid Pix
  Pix stale = erased;
  set.next(stale);
  FF_CHECK(!stale);
  FF_CHECK(set.owns(retained) && set(retained) == 22);
  set.resize(1000);
  FF_CHECK(set.owns(retained) && set(retained) == 22);
  set.clear();
  FF_CHECK(!set.owns(retained));
  bool threw = false;
  try { (void)set(retained); } catch (const std::out_of_range&) { threw = true; }
  FF_CHECK(threw);
  FF_CHECK(set.OK());

  // Pointer-to-element Pix are stable across rehashes (unordered containers).
  VHMap<int, int> map(-1);
  map[10] = 42;
  Pix mp = map.seek(10);
  map.resize(1000);
  FF_CHECK(map.owns(mp) && map.key(mp) == 10 && map.contents(mp) == 42);
  map.del(10);
  FF_CHECK(!map.owns(mp));
  VHBag<int> bag;
  Pix bp = bag.add(10);
  bag.add(10);
  bag.resize(1000);
  FF_CHECK(bag.owns(bp) && bag(bp) == 10);
  bag.remove(10);
  FF_CHECK(!bag.owns(bp));

  // Keep the address-membership index synchronised through set algebra,
  // assignment and deterministic random insert/erase/rehash operations.
  std::mt19937 rng(0x825B);
  VOHSet<int> random_set;
  std::set<int> reference;
  for (int step = 0; step < 1200; ++step) {
    const int item = static_cast<int>(rng() % 150);
    const int operation = static_cast<int>(rng() % 9);
    if (operation <= 2) {
      random_set.add(item);
      reference.insert(item);
    } else if (operation <= 4) {
      random_set.del(item);
      reference.erase(item);
    } else if (operation == 5) {
      random_set.resize(static_cast<int>(rng() % 400 + 1));
    } else if (operation == 6) {
      VOHSet<int> copy(random_set);
      FF_CHECK(copy.OK() && copy.length() == random_set.length());
      random_set = copy;
    } else if (operation == 7) {
      VOHSet<int> subset;
      for (int x = 0; x < 150; x += 3) subset.add(x);
      random_set &= subset;
      for (auto it = reference.begin(); it != reference.end();) {
        if (*it % 3 != 0) it = reference.erase(it);
        else ++it;
      }
    } else {
      VOHSet<int> additions;
      additions.add(item); additions.add(149);
      random_set |= additions;
      reference.insert(item); reference.insert(149);
    }
    FF_CHECK(random_set.OK() && random_set.length() == static_cast<int>(reference.size()));
    int visited = 0;
    for (Pix cur = random_set.first(); cur; random_set.next(cur)) {
      FF_CHECK(random_set.owns(cur));
      FF_CHECK(reference.count(random_set(cur)));
      ++visited;
    }
    FF_CHECK(visited == random_set.length());
  }
  VOHSet<int> self;
  self.add(1); self.add(2);
  self |= self;
  FF_CHECK(self.length() == 2 && self.OK());
  self &= self;
  FF_CHECK(self.length() == 2 && self.OK());
  self -= self;
  FF_CHECK(self.empty() && self.OK());
}
