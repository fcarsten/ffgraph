#include <ffgraph/FFSugi.h>
#include <ffgraph/FFGraphicNode.h>
#include <ffgraph/FFGraphicEdge.h>
#include <cmath>
#include <iostream>
#include <stdexcept>
#include <vector>

namespace {
void require(bool condition, const char* message) {
  if (!condition) throw std::runtime_error(message);
}
struct HeadlessSugi : FFSugi {
  void init(const FFGraph& graph) override { FFLayoutLabel::init(graph); }
};
void exercise(int count, bool reverse, int orientation, bool edgeGraphics) {
  FFGraph graph(YES);
  graph->_setDirected(YES);
  graph->setNodeIndex(graph->addNodeLabel(new FFGraphicNode));
  if (edgeGraphics) graph->setEdgeIndex(graph->addEdgeLabel(new FFGraphicEdge));
  std::vector<FFNode> nodes;
  std::vector<FFEdge> edges;
  for (int i = 0; i < count; ++i) {
    auto node = graph->addNode(FFPoint(i * 13, i * 7, 0));
    node->setValue(i + 1); nodes.push_back(node);
  }
  for (int i = 1; i < count; ++i) edges.push_back(graph->addEdge(nodes[i-1], nodes[i]));
  auto routed = reverse ? graph->addEdge(nodes.back(), nodes.front())
                        : graph->addEdge(nodes.front(), nodes.back());
  edges.push_back(routed);
  for (std::size_t i = 0; i < edges.size(); ++i) {
    edges[i]->setValue(i + 1); edges[i]->setWeight(1.5f);
    edges[i]->setName("preserve-edge");
  }
  graph->setStartNode(nodes.front());
  auto* sugi = new HeadlessSugi; graph->addGraphLabel(sugi);
  sugi->setOrient(orientation);
  for (int repeat = 0; repeat < 2; ++repeat) {
    sugi->layout(); // CTest's existing 30-second bound catches nontermination.
    require(graph->getNumNodes() == count && graph->getNumEdges() == count && graph->getDirected(),
            "layout changed graph size/direction");
    require(graph->getStartNode() == nodes.front(), "layout changed start node");
    for (auto node : nodes) {
      require(graph->allNodes().contains(node), "layout replaced a node");
      for (auto value : node->getGraphicNode()->getCoords().v)
        require(std::isfinite(value), "nonfinite node position");
    }
    for (int i = 0; i < count; ++i) {
      auto edge = edges[i];
      auto source = i == count - 1 ? (reverse ? nodes.back() : nodes.front()) : nodes[i];
      auto target = i == count - 1 ? (reverse ? nodes.front() : nodes.back()) : nodes[i+1];
      require(graph->allEdges().contains(edge) && edge->source() == source && edge->target() == target,
              "layout changed edge identity/direction");
      require(edge->getWeight() == 1.5f && edge->getName() == "preserve-edge", "layout changed edge attributes");
      auto* graphic = edge->getGraphicEdge();
      require(bool(graphic) == edgeGraphics, "layout changed graphics availability");
      if (!graphic) continue;
      const int expected = edge != routed ? 2 : (count == 1 ? 4 : count);
      require(graphic->anzPoints() == expected, "wrong single-/two-dummy route");
      for (int j = 0; j < graphic->anzPoints(); ++j)
        for (auto value : graphic->getPointAt(j).v)
          require(std::isfinite(value), "nonfinite edge route");
    }
  }
}
}
int main() {
  try {
    for (int orientation : {1, 2}) for (bool graphics : {false, true}) {
      exercise(1, false, orientation, graphics); // Self-loop, one dummy.
      exercise(3, false, orientation, graphics); // Long edge, one dummy.
      exercise(4, false, orientation, graphics); // Two-dummy forward control.
      exercise(4, true, orientation, graphics);  // Two-dummy reversed control.
    }
    std::cout << "verified 32 Sugiyama layouts with/without edge graphics\n";
  } catch (const std::exception& error) {
    std::cerr << error.what() << '\n'; return 1;
  }
}
