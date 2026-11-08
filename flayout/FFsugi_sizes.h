// Private bounds for Sugiyama's legacy int-indexed scratch arrays.
#ifndef FFGRAPH_SUGI_SIZES_H
#define FFGRAPH_SUGI_SIZES_H
#include <cstddef>
#include <limits>
#include <stdexcept>

namespace ffgraph_sugiyama {
struct SizeError : std::length_error { SizeError() : std::length_error("Sugiyama scratch dimensions exceed int indexing") {} };
inline int checkedAdd(int a, int b) {
  if (a < 0 || b < 0 || a > std::numeric_limits<int>::max()-b) throw SizeError();
  return a+b;
}
inline int checkedProduct(int a, int b) {
  if (a < 0 || b < 0 || (b && a > std::numeric_limits<int>::max()/b)) throw SizeError();
  return a*b;
}
template<class T> inline int arrayCount(int count) {
  if (count < 0 || std::size_t(count) > std::numeric_limits<std::size_t>::max()/sizeof(T)) throw SizeError();
  return count;
}
inline int scratchNodes(int nodes, int edges) {
  return checkedAdd(checkedAdd(nodes,checkedProduct(3,edges)),1);
}
inline int levelEntries(int levels, int width) {
  return checkedProduct(checkedAdd(levels,2),checkedAdd(width,1));
}
inline int matrixEntries(int stride) { return checkedProduct(stride,stride); }
inline int matrixArrayEntries(int levels, int stride) {
  return checkedProduct(checkedAdd(levels,1),matrixEntries(stride));
}
inline int reductionEntries(int levels, int stride) {
  // Two-level reduction uses separately indexed squares; multi-level reduction
  // indexes contiguous slabs with int offsets. Check the storage actually used.
  return levels == 1 ? matrixEntries(stride) : matrixArrayEntries(levels,stride);
}
}
#endif
