#include "test_check.h"
#include <ffgraph/FFSpring.h>
#include <ffgraph/FFInspector.h>
#include <ffgraph/FFGraphicEdge.h>
#include <array>
#include <cmath>
#include <vector>

namespace {
struct HeadlessSpring : FFSpring {
  void init(const FFGraph& g) override { FFLayoutLabel::init(g); }
  void controls(FFInspector* value) { inspector = value; }
};
using Frame = std::vector<std::array<FFCoord,4>>;
struct Recorder : FFInspector {
  HeadlessSpring& spring;
  const std::vector<FFNode>& nodes;
  std::vector<Frame> frames;
  Recorder(HeadlessSpring& s, const std::vector<FFNode>& n) : FFInspector(&s), spring(s), nodes(n) {}
  void inspect() override {}
  void update() override {
    spring.write_nodePosition(nodes.size());
    Frame frame;
    for (auto node : nodes) {
      const auto p = node->getGraphicNode()->getCoords();
      for (auto v : p.v) FF_CHECK(std::isfinite(v));
      frame.push_back({p.v[0],p.v[1],p.v[2],p.v[3]});
    }
    frames.push_back(std::move(frame));
    if (frames.size() == 8) spring.stop();
  }
};
void exercise(int dimension, bool directed) {
  FFGraph graph(YES); graph->_setDirected(directed);
  graph->setNodeIndex(graph->addNodeLabel(new FFGraphicNode));
  graph->setEdgeIndex(graph->addEdgeLabel(new FFGraphicEdge));
  const std::vector<FFPoint> initial = {{0,0,0},{170,35,10},{60,180,-20}};
  std::vector<FFNode> nodes;
  for (auto p : initial) nodes.push_back(graph->addNode(p));
  const std::vector<FFEdge> edges = {graph->addEdge(nodes[0],nodes[1]),graph->addEdge(nodes[1],nodes[2]),
                                    graph->addEdge(nodes[2],nodes[0]),graph->addEdge(nodes[0],nodes[1])};
  auto* spring = new HeadlessSpring; graph->addGraphLabel(spring);
  auto* recorder = new Recorder(*spring,nodes); spring->controls(recorder);
  spring->setDimension(dimension); FF_CHECK(spring->setQuality(0));
  auto run = [&](bool weighted, const std::array<FFCoord,4>& weights) {
    for (std::size_t i = 0; i < nodes.size(); ++i) nodes[i]->getGraphicNode()->moveTo(initial[i]);
    for (std::size_t i = 0; i < edges.size(); ++i) edges[i]->setWeight(weights[i]);
    spring->setUseEdgeWeight(weighted); recorder->frames.clear(); spring->layout();
    FF_CHECK(recorder->frames.size() == 8 && spring->getOptDistance() == 100);
    FF_CHECK(graph->getNumNodes() == 3 && graph->getNumEdges() == 4 && bool(graph->getDirected()) == directed);
    for (std::size_t i = 0; i < edges.size(); ++i) {
      FF_CHECK(graph->allEdges().contains(edges[i]) && edges[i]->getWeight() == weights[i]);
      const int a[] = {0,1,2,0}, b[] = {1,2,0,1};
      FF_CHECK(edges[i]->source() == nodes[a[i]] && edges[i]->target() == nodes[b[i]]);
      auto* g = edges[i]->getGraphicEdge(); FF_CHECK(g && g->anzPoints() == 2);
      for (int j = 0; j < g->anzPoints(); ++j) for (auto v : g->getPointAt(j).v) FF_CHECK(std::isfinite(v));
    }
    return recorder->frames;
  };
  const auto reference = run(false,{1,1,1,1});
  const auto varied = run(false,{100,30,0,-2});
  FF_CHECK(reference == varied);
  FF_CHECK(reference == run(false,{0,0,0,0}));
  FF_CHECK(run(true,{1,1,1,1}) != run(true,{100,100,100,100}));
  FF_CHECK(reference == run(false,{100,30,0,-2})); // Toggle back after weighted runs.
  FF_CHECK(spring->setxoff(999) && spring->setyoff(777) && spring->setzoff(333));
  FF_CHECK(reference == run(false,{1,1,1,1})); // Preserve legacy multi-node offset behavior.
  edges[0]->setWeight(100);
  FFCoord distance = 100; spring->change_optDist(distance); FF_CHECK(distance == 100);
  spring->setUseEdgeWeight(YES); distance = 100; spring->change_optDist(distance); FF_CHECK(distance == 500);
}
}
int main() {
  for (int dimension : {2,3}) for (bool directed : {false,true}) exercise(dimension,directed);
  std::cout << "unweighted trajectories ignore weights; weighted mode remains responsive in 2D/3D\n";
}
