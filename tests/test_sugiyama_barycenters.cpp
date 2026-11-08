#include <ffgraph/FFreduce_crossing.h>
#include <ffgraph/FFSugi.h>
#include <ffgraph/FFGraphicNode.h>
#include <ffgraph/FFGraphicEdge.h>
#include <algorithm>
#include <cmath>
#include <cstdint>
#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>

extern BOOL sorted(FFCoord*, int);
extern int phase2_down(int*, int, int*, int, int, int);

namespace {
void require(bool condition, const char* message) {
  if (!condition) throw std::runtime_error(message);
}
void sortedness() {
  // Enumerate all short sequences over zero and three positive barycenters.
  // The independent oracle erases zero slots before checking ordering.
  for (int size = 0; size <= 7; ++size) {
    const unsigned limit = 1u << (2 * size);
    for (unsigned code = 0; code < limit; ++code) {
      std::vector<FFCoord> values(size + 2, -99), filtered;
      for (int i = 1; i <= size; ++i) {
        values[i] = (code >> (2 * (i-1))) & 3;
        if (values[i] != 0) filtered.push_back(values[i]);
      }
      const auto before = values;
      const bool expected = std::is_sorted(filtered.begin(), filtered.end());
      require(bool(sorted(values.data(), size)) == expected, "sortedness differs from nonzero subsequence");
      require(values == before, "sortedness changed input");
    }
  }
}
struct Barycenter {
  int sum = 0, count = 0;
};
bool equal(Barycenter a, Barycenter b) {
  if (!a.count || !b.count) return !a.count && !b.count;
  return a.sum * b.count == b.sum * a.count;
}
void columnPhase(int rows, int cols, unsigned bits, bool paired) {
  const int stride = std::max(rows, cols) + 3, matrixSize = stride * stride;
  std::vector<int> matrices(2 * matrixSize), levels(3 * stride, 0);
  // Distinct metadata/padding and sentinel ones outside the active rectangle.
  for (int i = 0; i < 2 * matrixSize; ++i) matrices[i] = 1000 + i;
  for (int i = 1; i < stride; ++i) for (int j = 1; j < stride; ++j)
    matrices[i * stride + j] = (i > rows || j > cols) ? (i+j) % 2 : 0;
  std::vector<Barycenter> bary(cols + 1);
  for (int i = 1; i <= rows; ++i) for (int j = 1; j <= cols; ++j) {
    const bool edge = (bits >> ((i-1) * cols + j-1)) & 1;
    matrices[i * stride + j] = edge;
    if (edge) { bary[j].sum += i; ++bary[j].count; }
  }
  levels[0] = rows; levels[stride] = cols; levels[2 * stride] = 1;
  // The neighboring level has just one column, so its final column phase is
  // a no-op. Row metadata shares identities with the preceding column headers.
  for (int j = 1; j <= cols; ++j) {
    matrices[j] = 300 + j;
    matrices[matrixSize + j * stride] = 300 + j;
    matrices[matrixSize + j * stride + 1] = 1;
  }
  const auto before = matrices;
  std::vector<int> permutation(stride);
  for (int i = 0; i < stride; ++i) permutation[i] = i;
  // Compare adjacent nonempty barycenters using integer fractions, independent
  // of production float arithmetic and its sorted()/column_barycenter helpers.
  bool ordered = true;
  Barycenter previous;
  for (int j = 1; j <= cols; ++j) if (bary[j].count) {
    if (previous.count && previous.sum * bary[j].count > bary[j].sum * previous.count)
      ordered = false;
    previous = bary[j];
  }
  if (ordered) for (int begin = 1; begin <= cols;) {
    int end = begin + 1;
    while (end <= cols && equal(bary[begin], bary[end])) ++end;
    std::reverse(permutation.begin() + begin, permutation.begin() + end);
    begin = end;
  }
  require(phase2_down(matrices.data(), matrixSize, levels.data(), paired ? 2 : 1, stride, stride-1) == 0,
          "column phase failed");
  for (int i = 0; i < stride; ++i) for (int j = 0; j < stride; ++j) {
    require(matrices[i * stride + j] == before[i * stride + permutation[j]],
            "column reversal used incorrect barycenters or moved wrong cells");
    require(matrices[matrixSize + i * stride + j] == before[matrixSize + (paired ? permutation[i] : i) * stride + j],
            "neighboring row permutation is inconsistent");
  }
}
void columnPhases() {
  // Exhaustive binary rectangles (including tall, wide and square), with zero
  // rows/columns, distinct/tied barycenters and paired metadata/padding checks.
  for (const auto dimensions : {std::pair<int,int>{4,2}, {2,4}, {3,3}, {0,4}, {4,0}}) {
    const auto [rows, cols] = dimensions;
    for (unsigned mask = 0; mask < (1u << (rows * cols)); ++mask)
      for (bool paired : {false, true}) columnPhase(rows, cols, mask, paired);
  }
}
const std::vector<std::pair<int,int>> stoppingEdges = {{1,6},{2,5},{2,7},{3,7},{4,5}};
int pairCrossings(const std::vector<FFCoord>& order,
                  const std::vector<std::pair<int,int>>& edges = stoppingEdges) {
  int result = 0;
  for (std::size_t i = 0; i < edges.size(); ++i)
    for (std::size_t j = i+1; j < edges.size(); ++j) {
      const auto [a,b] = edges[i]; const auto [c,d] = edges[j];
      if ((order[a] - order[c]) * (order[b] - order[d]) < 0) ++result;
    }
  return result;
}
void stoppingKernel() {
  // A 4x3 two-level graph. The old OR accepts a sorted three-row prefix even
  // though the fourth row is out of order, retaining one avoidable crossing.
  std::vector<SNode> nodes(8);
  for (auto [a,b] : stoppingEdges) nodes[a].succ.append(b);
  std::vector<int> levels = {4,1,2,3,4, 3,5,6,7,0};
  require(reduce_crossings(nodes.data(), 1, 4, 4, 7, 0, levels.data(), 8, 3) == 0,
          "two-level reduction failed");
  const auto beforeRows = std::vector<int>{1,2,3,4}, beforeCols = std::vector<int>{5,6,7};
  auto afterRows = std::vector<int>(levels.begin()+1, levels.begin()+5);
  auto afterCols = std::vector<int>(levels.begin()+6, levels.begin()+9);
  std::sort(afterRows.begin(), afterRows.end()); std::sort(afterCols.begin(), afterCols.end());
  require(afterRows == beforeRows && afterCols == beforeCols && levels[0] == 4 && levels[5] == 3,
          "two-level reduction changed level membership");
  std::vector<FFCoord> rank(8);
  for (int i = 1; i <= 4; ++i) rank[levels[i]] = i;
  for (int i = 1; i <= 3; ++i) rank[levels[5+i]] = i;
  require(pairCrossings(rank) == 0, "two-level reduction stopped with an avoidable crossing");
}
void twoLevelSweep() {
  int cases = 0;
  for (const auto [rows, cols] : {std::pair<int,int>{4,3},{3,4},{3,3},{4,2},{2,4}}) {
    const int stride = std::max(rows,cols)+1;
    for (unsigned bits = 0; bits < (1u << (rows*cols)); ++bits) {
      std::vector<SNode> nodes(rows+cols+1);
      std::vector<std::pair<int,int>> edges;
      std::vector<bool> hasIncoming(cols, false);
      for (int i = 0; i < rows; ++i) for (int j = 0; j < cols; ++j)
        if ((bits >> (i*cols+j)) & 1) {
          nodes[i+1].succ.append(rows+j+1);
          edges.emplace_back(i+1,rows+j+1); hasIncoming[j] = true;
        }
      // A target with no incoming edge belongs to level zero, not level one.
      if (std::find(hasIncoming.begin(),hasIncoming.end(),false) != hasIncoming.end()) continue;
      std::vector<int> levels(2*stride,0);
      std::vector<FFCoord> rank(rows+cols+1);
      levels[0] = rows; levels[stride] = cols;
      for (int i = 1; i <= rows; ++i) { levels[i] = i; rank[i] = i; }
      for (int j = 1; j <= cols; ++j) { levels[stride+j] = rows+j; rank[rows+j] = j; }
      const int initial = pairCrossings(rank,edges);
      require(reduce_crossings(nodes.data(), 1, stride-1, stride-1, rows+cols,
                               0, levels.data(), 8, 3) == 0, "two-level sweep failed");
      require(levels[0] == rows && levels[stride] == cols, "sweep changed level size");
      std::vector<int> afterRows(levels.begin()+1,levels.begin()+rows+1);
      std::vector<int> afterCols(levels.begin()+stride+1,levels.begin()+stride+cols+1);
      std::sort(afterRows.begin(),afterRows.end()); std::sort(afterCols.begin(),afterCols.end());
      for (int i = 1; i <= rows; ++i) require(afterRows[i-1] == i, "sweep lost source identity");
      for (int j = 1; j <= cols; ++j) require(afterCols[j-1] == rows+j, "sweep lost target identity");
      for (int i = 1; i <= rows; ++i) rank[levels[i]] = i;
      for (int j = 1; j <= cols; ++j) rank[levels[stride+j]] = j;
      require(pairCrossings(rank,edges) <= initial, "sweep worsened initial crossing count");
      ++cases;
    }
  }
  require(cases == 6425, "unexpected two-level sweep coverage");
}
struct HeadlessSugi : FFSugi {
  void init(const FFGraph& graph) override { FFLayoutLabel::init(graph); }
};
void stoppingLayout() {
  for (int orientation : {1,2}) {
    FFGraph graph(YES); graph->_setDirected(YES);
    graph->setNodeIndex(graph->addNodeLabel(new FFGraphicNode));
    graph->setEdgeIndex(graph->addEdgeLabel(new FFGraphicEdge));
    std::vector<FFNode> nodes(1);
    for (int id = 1; id <= 7; ++id) {
      auto node = graph->addNode(); node->setValue(id); nodes.push_back(node);
    }
    std::vector<FFEdge> edges;
    for (auto [a,b] : stoppingEdges) {
      auto edge = graph->addEdge(nodes[a],nodes[b]); edge->setValue(edges.size()+1);
      edge->setWeight(1.5f); edge->setName("two-level-edge"); edges.push_back(edge);
    }
    auto* sugi = new HeadlessSugi; graph->addGraphLabel(sugi); sugi->setOrient(orientation);
    sugi->layout();
    require(graph->getNumNodes() == 7 && graph->getNumEdges() == 5 && graph->getDirected(),
            "two-level layout changed graph size/direction");
    std::vector<FFCoord> order(8);
    for (int id = 1; id <= 7; ++id) {
      require(graph->allNodes().contains(nodes[id]), "two-level layout replaced a node");
      const auto point = nodes[id]->getGraphicNode()->getCoords();
      for (auto v : point.v) require(std::isfinite(v), "nonfinite two-level coordinate");
      order[id] = point.v[orientation == 1 ? 0 : 1];
      if (id > 1 && id != 5)
        require(point.v[orientation == 1 ? 1 : 0] ==
                nodes[id-1]->getGraphicNode()->getCoords().v[orientation == 1 ? 1 : 0],
                "vertices in a level have different level coordinates");
    }
    require(nodes[1]->getGraphicNode()->getCoords().v[orientation == 1 ? 1 : 0] !=
            nodes[5]->getGraphicNode()->getCoords().v[orientation == 1 ? 1 : 0],
            "two levels collapsed to one");
    for (int a = 1; a <= 7; ++a) for (int b = a+1; b <= 7; ++b)
      if ((a <= 4) == (b <= 4)) require(order[a] != order[b], "same-level vertices overlap");
    require(pairCrossings(order) == 0, "complete two-level layout retains an avoidable crossing");
    for (std::size_t i = 0; i < edges.size(); ++i) {
      auto edge = edges[i]; const auto [a,b] = stoppingEdges[i];
      require(graph->allEdges().contains(edge) && edge->source() == nodes[a] && edge->target() == nodes[b] &&
              edge->getWeight() == 1.5f && edge->getName() == "two-level-edge",
              "two-level layout changed edge identity/incidence/attributes");
      require(edge->getGraphicEdge()->anzPoints() == 2, "unexpected bend in two-level edge");
      for (int p = 0; p < 2; ++p) for (auto v : edge->getGraphicEdge()->getPointAt(p).v)
        require(std::isfinite(v), "nonfinite two-level edge route");
    }
  }
}
}
int main(int argc, char** argv) {
  try {
    const std::string mode = argc == 2 ? argv[1] : "all";
    if (mode == "all" || mode == "sorted") sortedness();
    if (mode == "all" || mode == "columns") columnPhases();
    if (mode == "all" || mode == "stopping-kernel") stoppingKernel();
    if (mode == "all" || mode == "stopping-layout") stoppingLayout();
    if (mode == "all" || mode == "sweep") twoLevelSweep();
    require(mode == "all" || mode == "sorted" || mode == "columns" ||
            mode == "stopping-kernel" || mode == "stopping-layout" || mode == "sweep", "unknown test mode");
    std::cout << "verified barycenters, paired rectangular permutations and two-level stopping/layout\n";
  } catch (const std::exception& error) {
    std::cerr << error.what() << '\n'; return 1;
  }
}
