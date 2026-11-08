#include "test_check.h"
#include <ffgraph/FFreduce_crossing.h>
#include <algorithm>
#include <cstdint>
#include <vector>

extern FFCoord row_barycenter(int*,int,int,int);
extern FFCoord column_barycenter(int*,int,int,int);
extern void exch_double(FFCoord*,FFCoord*);
extern void exch_rows(int*,int,int,int);
extern void exch_columns(int*,int,int,int);
extern int b_r(int*,int*,int,int,int);
extern int b_c(int*,int*,int,int,int);
extern int number_of_crossings(int*,int,int,int);
namespace {
#include "sugiyama_sort_reference.inc"

std::uint64_t crossingOracle(const std::vector<int>& matrix, int rows, int columns, int stride) {
  std::vector<std::pair<int,int>> edges;
  for (int r = 1; r <= rows; ++r) for (int c = 1; c <= columns; ++c)
    if (matrix[r*stride+c] == 1) edges.emplace_back(r,c);
  std::uint64_t crossings = 0;
  for (std::size_t i = 0; i < edges.size(); ++i)
    for (std::size_t j = i+1; j < edges.size(); ++j)
      if ((edges[i].first < edges[j].first && edges[i].second > edges[j].second) ||
          (edges[i].first > edges[j].first && edges[i].second < edges[j].second)) ++crossings;
  return crossings;
}

void compare(const std::vector<int>& original, const std::vector<int>& paired,
             int rows, int columns, int stride, bool rowWise, int pairing, int passes = 1) {
  auto actual = original, expected = original, partner = paired, referencePartner = paired;
  int* a = pairing == 0 ? nullptr : pairing == 1 ? partner.data() : actual.data();
  int* b = pairing == 0 ? nullptr : pairing == 1 ? referencePartner.data() : expected.data();
  for (int pass = 0; pass < passes; ++pass) {
    const bool byRows = rowWise != bool(pass % 2);
    FF_CHECK((byRows ? b_r : b_c)(actual.data(),a,rows,columns,stride) == 0);
    FF_CHECK((byRows ? reference_b_r : reference_b_c)(expected.data(),b,rows,columns,stride) == 0);
    FF_CHECK(actual == expected && partner == referencePartner);
    FF_CHECK(std::uint64_t(number_of_crossings(actual.data(),rows,columns,stride)) ==
             crossingOracle(expected,rows,columns,stride));
  }
}
}
int main() {
  unsigned cases = 0;
  for (auto [rows,columns] : {std::pair<int,int>{4,3},{3,4},{3,3},{4,2},{2,4},{0,4},{4,0}}) {
    const int stride = std::max(rows,columns)+3;
    for (unsigned bits = 0; bits < (1u << (rows*columns)); ++bits)
      for (bool rowWise : {false,true}) for (int pairing : {0,1,2}) {
        std::vector<int> original(stride*stride), paired(stride*stride);
        for (int i = 0; i < stride*stride; ++i) { original[i] = 50000+i; paired[i] = -10000-i; }
        for (int r = 1; r <= rows; ++r) for (int c = 1; c <= columns; ++c)
          original[r*stride+c] = (bits >> ((r-1)*columns+c-1)) & 1;
        compare(original,paired,rows,columns,stride,rowWise,pairing);
        ++cases;
      }
  }
  // Wider, padded matrices exercise long/disjoint cycles, fixed nonzero slots,
  // ties and zero holes. Repeated alternating sweeps also cover aliased axes.
  std::uint32_t random = 20261104;
  for (auto [rows,columns] : {std::pair<int,int>{12,12},{31,7},{7,31},{17,23}})
    for (int sample = 0; sample < 24; ++sample) {
      const int stride = std::max(rows,columns) + (sample == 0 ? 257 : 5);
      std::vector<int> original(stride*stride), paired(stride*stride);
      for (int i = 0; i < stride*stride; ++i) { original[i] = 50000+i; paired[i] = -10000-i; }
      for (int r = 1; r <= rows; ++r) for (int c = 1; c <= columns; ++c) {
        random = random * 1664525u + 1013904223u;
        original[r*stride+c] = (r % 5 && c % 7) ? int((random >> 16) % 4 == 0) : 0;
      }
      if (rows == 12 && columns == 12 && sample == 0) {
        const int targets[] = {1,3,4,2,5,7,6,11,8,9,10,0};
        for (int r = 1; r <= rows; ++r) for (int c = 1; c <= columns; ++c)
          original[r*stride+c] = c == targets[r-1];
      }
      for (bool rowWise : {false,true}) for (int pairing : {0,1,2}) {
        compare(original,paired,rows,columns,stride,rowWise,pairing,3);
        ++cases;
      }
    }
  std::cout << cases << " full-stride barycenter permutations match the original bubble operations\n";
}
