// Test-only replacement for the archive's reduction object. Compile the actual
// production code, checking every permutation on its real incoming matrices.
#include "sugiyama_corpus_verification.h"
#include <ffgraph/FFreduce_crossing.h>
#include <stdexcept>
#include <vector>

#define FFGRAPH_VERIFY_SUGIYAMA_PERMUTATIONS
#define reduce_crossings checked_reduce_crossings
#include "../flayout/FFreduce_crossing.cc"
#undef reduce_crossings
#undef FFGRAPH_VERIFY_SUGIYAMA_PERMUTATIONS

namespace {
#include "sugiyama_sort_reference.inc"
ffgraph_test::SugiVerification stats;
bool referenceOnly = false;

// Independent inversion enumeration: for each edge, inspect earlier rows'
// column counts strictly to its right. Deliberately use O(edges*columns), not
// the production counter's running-right recurrence.
std::uint64_t crossingOracle(const int* matrix, int rows, int columns, int stride) {
  std::vector<std::uint64_t> previous(columns+1,0);
  std::uint64_t count = 0;
  for (int row = 1; row <= rows; ++row) {
    for (int col = 1; col <= columns; ++col)
      if (matrix[std::size_t(row)*stride+col] == 1)
        for (int right = col+1; right <= columns; ++right) count += previous[right];
    for (int col = 1; col <= columns; ++col)
      if (matrix[std::size_t(row)*stride+col] == 1) ++previous[col];
  }
  return count;
}

std::uint64_t layerCrossings(SNode* nodes, int levels, int stride, int pitch, int* order) {
  std::vector<int> matrix(std::size_t(stride)*stride);
  std::uint64_t total = 0;
  for (int level = 0; level < levels; ++level) {
    make_matrix(nodes,level,matrix.data(),stride,pitch-1,order);
    const int rows = order[pitch*level], columns = order[pitch*(level+1)];
    const auto expected = crossingOracle(matrix.data(),rows,columns,stride);
    if (ffgraph_sugiyama::countCrossings(matrix.data(),rows,columns,stride) != expected)
      throw std::runtime_error("corpus layer crossing counter disagrees with independent oracle");
    total += expected;
  }
  return total;
}
}

namespace ffgraph_test {
void resetSugiVerification(bool reference) { stats = {}; referenceOnly = reference; }
SugiVerification sugiVerification() { return stats; }

int checkedPermutation(int* matrix, int* partner, int rows, int columns, int stride,
                       bool byRows, Permutation implementation) {
  const auto reference = byRows ? reference_b_r : reference_b_c;
  if (referenceOnly) return reference(matrix,partner,rows,columns,stride);
  const auto size = std::size_t(stride)*stride;
  std::vector<int> expected(matrix,matrix+size), paired;
  if (partner && partner != matrix) paired.assign(partner,partner+size);
  int* expectedPartner = !partner ? nullptr : partner == matrix ? expected.data() : paired.data();
  if (reference(expected.data(),expectedPartner,rows,columns,stride) != 0)
    throw std::runtime_error("corpus reference permutation failed");
  const int result = implementation(matrix,partner,rows,columns,stride,byRows);
  if (result != 0 || !std::equal(expected.begin(),expected.end(),matrix) ||
      (!paired.empty() && !std::equal(paired.begin(),paired.end(),partner)))
    throw std::runtime_error("corpus production permutation differs from frozen reference");
  if (ffgraph_sugiyama::countCrossings(matrix,rows,columns,stride) !=
      crossingOracle(expected.data(),rows,columns,stride))
    throw std::runtime_error("corpus permutation crossing count differs from reference");
  ++stats.permutations;
  stats.checkedMatrices += partner && partner != matrix ? 2 : 1;
  return result;
}
}

// Observe actual final layer order, including dummy vertices, outside the
// production algorithm. Both reference and optimized runs use this wrapper.
int reduce_crossings(SNode* nodes, int levels, int width, int maxNode, int count,
                     int algorithm, int* order, int iterations1, int iterations2) {
  ++stats.reductions;
  stats.levels = std::max(stats.levels,levels+1);
  for (int level = 0; level <= levels; ++level)
    stats.width = std::max(stats.width,order[(maxNode+1)*level]);
  stats.initialCrossings = layerCrossings(nodes,levels,width+1,maxNode+1,order);
  const int result = checked_reduce_crossings(nodes,levels,width,maxNode,count,
                                               algorithm,order,iterations1,iterations2);
  stats.finalCrossings = layerCrossings(nodes,levels,width+1,maxNode+1,order);
  return result;
}
