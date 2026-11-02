#ifndef FFGRAPH_TEST_CHECK_H
#define FFGRAPH_TEST_CHECK_H

#include <cstdlib>
#include <iostream>

// Unlike assert(), evaluate test operations and check results under NDEBUG too.
#define FF_CHECK(condition) do { \
  if (!(condition)) { \
    std::cerr << __FILE__ << ':' << __LINE__ \
              << ": check failed: " #condition << '\n'; \
    std::abort(); \
  } \
} while (false)

#endif
