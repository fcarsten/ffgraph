#include "test_check.h"
#include <cstring>

#ifndef NDEBUG
#error This regression must exercise checks with NDEBUG defined.
#endif

int main(int argc, char** argv) {
  int operations = 0;
  FF_CHECK(++operations == 1);
  FF_CHECK(operations == 1);
  if (argc == 2 && std::strcmp(argv[1], "--intentional-failure") == 0)
    FF_CHECK(false);
  return 0;
}
