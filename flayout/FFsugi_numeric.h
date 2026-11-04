// Private checks for the existing integer-based coordinate algorithm.
#ifndef FFGRAPH_SUGI_NUMERIC_H
#define FFGRAPH_SUGI_NUMERIC_H
#include <cmath>
#include <cstdint>
#include <limits>
#include <stdexcept>
#include <ffgraph/FFPoint.h>
namespace ffgraph_sugiyama {
struct NumericError : std::range_error {
  NumericError() : std::range_error("Sugiyama coordinates exceed the supported finite integer range") {}
};
inline int coordinateInt(double value) {
  const double truncated = std::trunc(value);
  if (!std::isfinite(value) || truncated < std::numeric_limits<int>::min() ||
      truncated > std::numeric_limits<int>::max()) throw NumericError();
  return static_cast<int>(value);
}
inline int signedInt(std::int64_t value) {
  if (value < std::numeric_limits<int>::min() || value > std::numeric_limits<int>::max()) throw NumericError();
  return static_cast<int>(value);
}
inline void finitePoint(const FFPoint& point) {
  for (auto value : point.v) if (!std::isfinite(value)) throw NumericError();
  coordinateInt(point.v[0]); coordinateInt(point.v[1]); coordinateInt(point.v[2]);
}
inline FFCoord advanceCoordinate(FFCoord value, int step) {
  const FFCoord next = value+step;
  if (step <= 0 || !std::isfinite(next) || !(next > value)) throw NumericError();
  return next;
}
}
#endif
