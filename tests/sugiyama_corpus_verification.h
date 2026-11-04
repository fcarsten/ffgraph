#ifndef FFGRAPH_TEST_SUGIYAMA_CORPUS_VERIFICATION_H
#define FFGRAPH_TEST_SUGIYAMA_CORPUS_VERIFICATION_H
#include <cstdint>

namespace ffgraph_test {
struct SugiVerification {
  std::uint64_t permutations = 0, checkedMatrices = 0;
  std::uint64_t initialCrossings = 0, finalCrossings = 0;
  int reductions = 0, levels = 0, width = 0;
};
void resetSugiVerification(bool referenceOnly);
SugiVerification sugiVerification();
using Permutation = int (*)(int*,int*,int,int,int,bool);
int checkedPermutation(int*,int*,int,int,int,bool,Permutation);
}
#endif
