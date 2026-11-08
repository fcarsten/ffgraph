// Private Sugiyama implementation interface; not an installed/public API.
#ifndef FFGRAPH_SUGIYAMA_CROSSING_COUNT_H
#define FFGRAPH_SUGIYAMA_CROSSING_COUNT_H
#include <cstdint>
namespace ffgraph_sugiyama {
using CrossingCount = std::uint64_t;
CrossingCount countCrossings(const int* matrix, int rows, int columns, int stride);
// nodeArray uses a pitch of maxNodeIndex + 1 between levels.
CrossingCount countCrossingsArray(const int* matrices, int stride, int maxNodeIndex,
                                int matrixSize, int levels, const int* nodeArray);
}
#endif
