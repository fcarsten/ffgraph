#include "test_check.h"
#include <vector>
#include "DLList.h"

static std::vector<int> values(const DLList<int>& list) {
  std::vector<int> result;
  for (Pix p = list.first(); p; list.next(p)) result.push_back(list(p));
  return result;
}

int main() {
  DLList<int> list;
  list.append(1);
  list.append(3);
  Pix first = list.first();
  Pix last = first;
  list.next(last);

  list.ins_after(first, 2);
  FF_CHECK((values(list) == std::vector<int>{1, 2, 3}));
  list.ins_before(last, 4);
  FF_CHECK((values(list) == std::vector<int>{1, 2, 4, 3}));

  // Existing handles remain valid across insertion.
  FF_CHECK(list(first) == 1);
  FF_CHECK(list(last) == 3);

  list.ins_before(nullptr, 5);
  FF_CHECK((values(list) == std::vector<int>{1, 2, 4, 3, 5}));
  list.ins_after(nullptr, 6);
  FF_CHECK((values(list) == std::vector<int>{1, 2, 4, 3, 5, 6}));
}
