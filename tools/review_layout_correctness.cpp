// Manual diagnostics for the 2026-11 correctness review, not CTest regressions.
// Compile this file only with GCC/Clang -fno-access-control to inspect Spring's
// existing workspace. Production sources/headers require no instrumentation.
// Exit 0 means the named observation reproduced; it does not mean a bug is fixed.
#include <ffgraph/FFSpring.h>
#include <ffgraph/FFSugi.h>
#include <ffgraph/FFGraphicEdge.h>
#include <cmath>
#include <cfenv>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <string>

extern int r_c(int*, int*, int, int);
extern BOOL sorted(FFCoord*, int);

namespace {
void require(bool condition, const char* message) {
  if (!condition) throw std::runtime_error(message);
}
struct HeadlessSpring : FFSpring {
  void init(const FFGraph& graph) override { FFLayoutLabel::init(graph); }
};
struct HeadlessSugi : FFSugi {
  void init(const FFGraph& graph) override { FFLayoutLabel::init(graph); }
};
void graphics(FFGraph& graph, bool edges = true) {
  graph->setNodeIndex(graph->addNodeLabel(new FFGraphicNode));
  if (edges) graph->setEdgeIndex(graph->addEdgeLabel(new FFGraphicEdge));
}
void weightedForce() {
  FFGraph graph(YES);
  auto a = graph->addNode(); auto b = graph->addNode();
  a->setValue(1); b->setValue(2);
  auto edge = graph->addEdge(a, b); edge->setWeight(2);
  HeadlessSpring spring;
  auto workspace = std::make_unique<FSSpringNodes[]>(3);
  spring.nodes = workspace.get();
  workspace[1].coord = FFPoint(0, 0, 0);
  workspace[2].coord = FFPoint(100, 0, 0);
  workspace[1].toSucc.add(edge);
  workspace[2].fromPred.add(edge);
  FFCoord observed[2];
  for (int dimension : {2, 3}) {
    spring.setDimension(dimension);
    workspace[1].attrib = workspace[2].attrib = 0;
    FFPoint forces[3]; spring.init_forces(forces, 2);
    spring.compute_forces_edges(100, forces, 2);
    observed[dimension - 2] = forces[1].v[0];
    require(forces[2].v[0] == -forces[1].v[0], "pair forces lost symmetry");
    std::cout << dimension << "D weight=2 distance=100 k=100: force.x="
              << forces[1].v[0] << '\n';
  }
  spring.nodes = nullptr;
  require(observed[0] == 150 && observed[1] == 350, "weighted discrepancy did not reproduce");
}
void zeroForce() {
  HeadlessSpring spring;
  auto workspace = std::make_unique<FSSpringNodes[]>(2);
  spring.nodes = workspace.get(); workspace[1].move = YES;
  for (int dimension : {2, 3}) {
    spring.setDimension(dimension); workspace[1].coord = FFPoint(10, 20, 30);
    FFPoint forces[2]; FFCoord lengths[2] = {0, 0};
    spring.compute_nodePosition(forces, 1, 0, lengths);
    std::cout << dimension << "D zero force and temperature: x=" << workspace[1].coord.v[0] << '\n';
    require(std::isnan(workspace[1].coord.v[0]), "zero-force NaN did not reproduce");
  }
  spring.nodes = nullptr;
}
void unweightedDistance() {
  FFGraph graph(YES);
  auto a = graph->addNode(); auto b = graph->addNode();
  graph->addEdge(a, b)->setWeight(100);
  HeadlessSpring spring; spring.init(graph); spring.setUseEdgeWeight(NO);
  FFCoord distance = 100;
  spring.change_optDist(distance);
  std::cout << "edge weighting disabled, weight=100: effective k=" << distance << '\n';
  require(distance == 500, "unweighted distance adjustment did not reproduce");
}
void pinnedSingleton() {
  FFGraph graph(YES); graphics(graph);
  auto node = graph->addNode(FFPoint(321, 456, 17));
  auto* spring = new HeadlessSpring; graph->addGraphLabel(spring);
  FFSet<FFNode> fixed; fixed.add(node);
  spring->layout(fixed);
  const auto point = node->getGraphicNode()->getCoords();
  std::cout << "fixed singleton: (321,456,17) -> (" << point.v[0] << ',' << point.v[1] << ',' << point.v[2] << ")\n";
  require(point.v[0] == 100 && point.v[1] == 100 && point.v[2] == 100,
          "fixed singleton displacement did not reproduce");
}
void localSelection() {
  FFGraph graph(YES); graphics(graph);
  auto a = graph->addNode(FFPoint(0, 0, 0)); a->setValue(1);
  auto b = graph->addNode(FFPoint(1000, 0, 0)); b->setValue(2);
  graph->addEdge(a, b);
  auto* spring = new HeadlessSpring; graph->addGraphLabel(spring);
  FFSet<FFNode> selected; selected.add(a);
  spring->layout(selected);
  const auto pa = a->getGraphicNode()->getCoords();
  const auto pb = b->getGraphicNode()->getCoords();
  std::cout << "local selection contains a: a.x=" << pa.v[0] << ", b.x=" << pb.v[0] << '\n';
  require(pa.v[0] == 0 && pb.v[0] != 1000, "local-selection inversion did not reproduce");
}
void publicZeroForce() {
  FFGraph graph(YES); graphics(graph);
  auto a = graph->addNode(FFPoint(0, 0, 0)); a->setValue(1);
  auto b = graph->addNode(FFPoint(100, 0, 0)); b->setValue(2);
  graph->addEdge(a, b);
  auto* spring = new HeadlessSpring; graph->addGraphLabel(spring);
  FFSet<FFNode> fixed;
  FFNode last = nullptr;
  const auto& all = graph->allNodes();
  for (Pix p = all.first(); p; all.next(p)) last = all(p);
  fixed.add(last); // Suppress the startup perturbation; the pair is at equilibrium.
  require(spring->setQuality(0), "continuous mode rejected");
  std::cout << "entering continuous public layout at exact equilibrium, FP traps enabled" << std::endl;
  // Linux diagnostic only: demonstrate the actual public call reaches 0/0.
  feenableexcept(FE_INVALID | FE_DIVBYZERO);
  spring->layout(fixed);
  throw std::runtime_error("public equilibrium invalid operation did not reproduce");
}
void rectangularTies() {
  constexpr int stride = 5;
  int matrix[stride * stride] = {};
  matrix[1] = 101; matrix[2] = 102;
  // True barycenters: col1=(1+3)/2=2, col2=(1+4)/2=2.5.
  // r_c considers only rows 1..2, incorrectly treating both as barycenter 1.
  matrix[1 * stride + 1] = matrix[3 * stride + 1] = 1;
  matrix[1 * stride + 2] = matrix[4 * stride + 2] = 1;
  require(r_c(matrix, nullptr, 2, stride) == 0, "r_c failed");
  std::cout << "4x2 unequal barycenters (2,2.5): column identities " << matrix[1] << ',' << matrix[2] << '\n';
  require(matrix[1] == 102 && matrix[2] == 101, "rectangular false tie did not reproduce");
}
void zeroHole() {
  FFCoord values[] = {0, 3, 0, 2};
  const bool result = sorted(values, 3);
  std::cout << "sorted([3,0,2]), ignoring zero: " << result << '\n';
  require(result, "zero-hole sorted discrepancy did not reproduce");
}
void noGraphics(int orientation) {
  FFGraph graph(YES); graphics(graph, false);
  auto node = graph->addNode(); graph->addEdge(node, node);
  auto* sugi = new HeadlessSugi; graph->addGraphLabel(sugi);
  sugi->setOrient(orientation);
  std::cout << "entering Sugiyama: one self-loop without edge graphics, orientation=" << orientation << std::endl;
  sugi->layout();
  throw std::runtime_error("missing-graphics hang did not reproduce");
}
void controls() {
  HeadlessSugi sugi; HeadlessSpring spring;
  const bool noReduction = sugi.setCrossingAlg(2);
  const bool manualQuality = spring.setQuality(-1);
  std::cout << "Sugi noneRed selector 2 accepted=" << noReduction
            << "; Spring manual-quality -1 accepted=" << manualQuality << '\n';
  require(!noReduction && sugi.getCrossingAlg() == 0 && !manualQuality,
          "setting discrepancies did not reproduce");
}
}
int main(int argc, char** argv) {
  try {
    if (argc != 2) throw std::runtime_error("specify one review case");
    const std::string name = argv[1];
    if (name == "weighted-force") weightedForce();
    else if (name == "unweighted-distance") unweightedDistance();
    else if (name == "zero-force") zeroForce();
    else if (name == "fixed-singleton") pinnedSingleton();
    else if (name == "local-selection") localSelection();
    else if (name == "public-zero-force") publicZeroForce();
    else if (name == "rectangular-ties") rectangularTies();
    else if (name == "zero-hole") zeroHole();
    else if (name == "controls") controls();
    else if (name == "no-edge-graphics-1") noGraphics(1);
    else if (name == "no-edge-graphics-2") noGraphics(2);
    else throw std::runtime_error("unknown review case");
  } catch (const std::exception& error) {
    std::cerr << error.what() << '\n'; return 1;
  }
}
