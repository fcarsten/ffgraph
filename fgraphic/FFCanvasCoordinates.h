// Private conversion boundary for Tk's int-indexed canvas geometry.
#ifndef FFGRAPH_CANVAS_COORDINATES_H
#define FFGRAPH_CANVAS_COORDINATES_H
#include <ffgraph/FFPoint.h>
#include <cmath>
#include <limits>

namespace ffgraph_canvas {
inline bool toInt(double value,int& result) noexcept {
  const double truncated=std::trunc(value);
  if(!std::isfinite(value) || truncated<std::numeric_limits<int>::min() ||
     truncated>std::numeric_limits<int>::max()) return false;
  result=static_cast<int>(truncated);
  return true;
}
inline int integer(double value) noexcept {
  int result=0;toInt(value,result);return result;
}
inline bool supported(const FFPoint& point,bool radius=false) noexcept {
  int x,y,r;
  if(!std::isfinite(point.v[2])) return false;
  if(!toInt(point.v[0],x) || !toInt(point.v[1],y)) return false;
  if(!radius) return true;
  if(!toInt(point.v[2],r)) return false;
  // The Tcl node procedures form x/y +/- radius. Check that footprint too.
  const double extent=std::abs(double(r));
  return double(x)-extent>=std::numeric_limits<int>::min() &&
         double(x)+extent<=std::numeric_limits<int>::max() &&
         double(y)-extent>=std::numeric_limits<int>::min() &&
         double(y)+extent<=std::numeric_limits<int>::max();
}
}
#endif
