#include "test_check.h"
#include <ffgraph/FFPlatform.h>
#include <ffgraph/FFGraphClass.h>
#include <string>

int main() {
  ffgraph_platform::seedRandom(12345);
  // Frozen glibc/POSIX lrand48 sequence, shared by Windows Random layouts.
  for (long expected : {483889296L, 1973930609L, 444188209L, 1556452597L,
                        1572385691L, 1946656043L})
    FF_CHECK(ffgraph_platform::randomLong() == expected);
  ffgraph_platform::setEnvironment("FFGRAPH_PLATFORM_TEST", "value with spaces");
  FF_CHECK(std::string(std::getenv("FFGRAPH_PLATFORM_TEST")) == "value with spaces");
  FF_CHECK(expand_path("plain/file.g") == "plain/file.g");
#ifdef _WIN32
  ffgraph_platform::setEnvironment("HOME", "C:/test profile");
  FF_CHECK(expand_path("~/file.g") == "C:/test profile/file.g");
  FF_CHECK(expand_path("~other/file.g") == "~other/file.g");
#endif
}
