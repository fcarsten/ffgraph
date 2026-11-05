#ifndef FFGRAPH_PLATFORM_H
#define FFGRAPH_PLATFORM_H

#include <cstdlib>
#include <stdexcept>
#ifdef _WIN32
#include <chrono>
#include <cstdint>
#include <thread>
#else
#include <unistd.h>
#endif

namespace ffgraph_platform {
#ifdef _WIN32
// POSIX's 48-bit generator, so seeded Random layouts retain their sequence.
inline std::uint64_t randomState = 0x1234abcd330eULL;
inline void seedRandom(long seed) {
  randomState = (static_cast<std::uint64_t>(static_cast<std::uint32_t>(seed)) << 16) | 0x330e;
}
inline long randomLong() {
  randomState = (randomState * 0x5deece66dULL + 0xb) & 0xffffffffffffULL;
  return static_cast<long>(randomState >> 17);
}
inline void sleepSeconds(unsigned seconds) {
  std::this_thread::sleep_for(std::chrono::seconds(seconds));
}
inline void setEnvironment(const char* name, const char* value) {
  if (_putenv_s(name, value) != 0) throw std::runtime_error("Cannot set environment variable");
}
#else
inline void seedRandom(long seed) { ::srand48(seed); }
inline long randomLong() { return ::lrand48(); }
inline void sleepSeconds(unsigned seconds) { ::sleep(seconds); }
inline void setEnvironment(const char* name, const char* value) {
  if (::setenv(name, value, 1) != 0) throw std::runtime_error("Cannot set environment variable");
}
#endif
}
#endif
