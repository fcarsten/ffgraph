#include <container/AVLMap.h>
#include "../container/MFMap.cc"
#include "../container/AVL.cc"
#include "../container/AVLMap.cc"
#include <cassert>
#include <map>
#include <random>
int CMP(int& a, int& b) { return (a > b) - (a < b); }
int EQ(int& a, int& b) { return a == b; }

int main() {
  AVLMap<int, int> map(-1);
  std::map<int, int> reference;
  std::mt19937 rng(8107);
  for (int i = 0; i < 1000; ++i) {
    int key = static_cast<int>(rng() % 100);
    if (rng() % 3 == 0) { map.del(key); reference.erase(key); }
    else { int value = i + 1; map[key] = value; reference[key] = value; }
    assert(map.OK() && map.length() == static_cast<int>(reference.size()));
    for (Pix p = map.first(); p; map.next(p)) {
      assert(reference.at(map.key(p)) == map.contents(p));
      assert(map.owns(p));
    }
    for (const auto& kv : reference) {
      Pix p = map.seek(kv.first);
      assert(p && map.contents(p) == kv.second);
    }
  }
  AVLMap<int, int> copy(map);
  assert(copy.OK() && copy.length() == map.length());
  copy.clear(); assert(copy.empty() && copy.OK());
  assert(map.OK());
}
