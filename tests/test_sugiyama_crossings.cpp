#include <algorithm>
#include <cstdint>
#include <iostream>
#include <stdexcept>
#include <utility>
#include <vector>
#include <limits>
#include "flayout/FFcrossing_count.h"

extern int number_of_crossings(int*, int, int, int);
extern int number_of_crossings_matrixarray(int*, int, int, int, int, int*);
namespace {
void require(bool okay, const char* message) {
  if (!okay) throw std::runtime_error(message);
}
struct Matrix {
  int rows, cols, stride;
  std::vector<int> cells;
  Matrix(int r, int c) : rows(r), cols(c), stride(std::max(r,c) + 3), cells((r + 2) * stride, 1) {
    for (int i = 1; i <= r; ++i) for (int j = 1; j <= c; ++j) at(i,j) = 0;
  }
  int& at(int i, int j) { return cells[i * stride + j]; }
};
std::uint64_t oracle(Matrix& m) {
  std::vector<std::pair<int,int>> edges;
  for (int i = 1; i <= m.rows; ++i) for (int j = 1; j <= m.cols; ++j)
    if (m.at(i,j) == 1) edges.emplace_back(i,j);
  std::uint64_t count = 0;
  for (std::size_t i = 0; i < edges.size(); ++i) for (std::size_t j = i + 1; j < edges.size(); ++j)
    if ((edges[i].first < edges[j].first && edges[i].second > edges[j].second) ||
        (edges[j].first < edges[i].first && edges[j].second > edges[i].second)) ++count;
  return count;
}
void check(Matrix& m, std::uint64_t expected) {
  const auto before = m.cells;
  const auto actual = number_of_crossings(m.cells.data(),m.rows,m.cols,m.stride);
  require(actual >= 0 && static_cast<std::uint64_t>(actual) == expected, "wrong crossing count");
  require(ffgraph_sugiyama::countCrossings(m.cells.data(),m.rows,m.cols,m.stride) == expected,
          "wide counter differs from oracle");
  require(before == m.cells, "counter changed its matrix");
}
}
int main() {
  try {
    Matrix small(4,4);
    for (std::uint32_t mask = 0; mask < 65536; ++mask) {
      for (int bit = 0; bit < 16; ++bit) small.at(bit / 4 + 1,bit % 4 + 1) = (mask >> bit) & 1;
      check(small,oracle(small));
    }
    std::uint32_t random = 20261102;
    for (int sample = 0; sample < 128; ++sample) {
      Matrix m(sample % 17,(sample * 7) % 19);
      for (int i = 1; i <= m.rows; ++i) for (int j = 1; j <= m.cols; ++j) {
        random = random * 1664525u + 1013904223u;
        m.at(i,j) = static_cast<int>((random >> 16) % 4) - 1;
      }
      check(m,oracle(m));
    }
    for (int size : {8,16,32,64,128,256}) {
      Matrix full(size,size);
      for (int i = 1; i <= size; ++i) for (int j = 1; j <= size; ++j) full.at(i,j) = 1;
      const std::uint64_t pairs = std::uint64_t(size) * (size - 1) / 2;
      check(full,pairs * pairs);
    }
    // The old int counter overflowed here. Layout decisions must retain
    // the exact total while compatibility helpers have a defined ceiling.
    Matrix wide(512,512);
    for (int i = 1; i <= 512; ++i) for (int j = 1; j <= 512; ++j) wide.at(i,j) = 1;
    const std::uint64_t pairs = std::uint64_t(512) * 511 / 2;
    require(ffgraph_sugiyama::countCrossings(wide.cells.data(),512,512,wide.stride) == pairs * pairs,
            "large count lost precision");
    require(number_of_crossings(wide.cells.data(),512,512,wide.stride) == std::numeric_limits<int>::max(),
            "int counter must saturate without overflow");
    // Each 256x256 layer fits int, but their sum exceeds it.
    constexpr int stride = 257, matrixSize = stride * stride, levels = 3;
    std::vector<int> matrices(levels * matrixSize, 0), nodeArray((levels + 1) * stride, 0);
    for (int level = 0; level < levels; ++level)
      for (int i = 1; i <= 256; ++i) for (int j = 1; j <= 256; ++j)
        matrices[level * matrixSize + i * stride + j] = 1;
    for (int level = 0; level <= levels; ++level) nodeArray[level * stride] = 256;
    const std::uint64_t layerPairs = std::uint64_t(256) * 255 / 2;
    const auto total = levels * layerPairs * layerPairs;
    require(ffgraph_sugiyama::countCrossingsArray(matrices.data(),stride,256,matrixSize,levels,
            nodeArray.data()) == total, "cross-layer sum overflowed");
    require(number_of_crossings_matrixarray(matrices.data(),stride,256,matrixSize,levels,
            nodeArray.data()) == std::numeric_limits<int>::max(), "int total must saturate");
    require(ffgraph_sugiyama::countCrossings(nullptr,0,100,101) == 0 &&
            ffgraph_sugiyama::countCrossings(nullptr,100,1,101) == 0,
            "degenerate matrix must not read or allocate");
    std::cout << "crossing oracles: exhaustive 4x4, rectangular/nonbinary and closed-form matrices passed\n";
  } catch (const std::exception& error) {
    std::cerr << "Sugiyama crossings: " << error.what() << '\n';
    return 1;
  }
}
