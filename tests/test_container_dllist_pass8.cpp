#include <DLList.h>
#include <cassert>
#include <stdexcept>
#include <vector>

struct Item {
  int value;
  static int copies_until_throw;
  explicit Item(int v) : value(v) {}
  Item(const Item& other) : value(other.value) {
    if (copies_until_throw == 0) throw std::runtime_error("copy failed");
    if (copies_until_throw > 0) --copies_until_throw;
  }
  Item(Item&& other) noexcept : value(other.value) {}
};
int Item::copies_until_throw = -1;

int main() {
  DLList<Item> a, b;
  a.append(Item(1)); a.append(Item(2));
  b.append(Item(3)); b.append(Item(4)); b.append(Item(5));
  Item::copies_until_throw = 1;
  bool failed = false;
  try { a = b; } catch (const std::runtime_error&) { failed = true; }
  assert(failed);
  assert(a.length() == 2 && a.front().value == 1 && a.rear().value == 2);
  Item::copies_until_throw = -1;
  a = b;
  assert(a.length() == 3 && a.front().value == 3 && a.rear().value == 5);
  a = a;
  assert(a.length() == 3 && a.front().value == 3 && a.rear().value == 5);
  Pix p = a.first(); a.next(p); assert(a(p).value == 4);
  a.del(p);
  assert(a.length() == 2 && p && a(p).value == 5);
}
