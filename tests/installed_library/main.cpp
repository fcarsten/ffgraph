// A real consumer of the installed graph, graphics, layout and parser archives.
#include <ffgraph/FFGraph.h>
#include <ffgraph/FFGraphicNode.h>
#include <ffgraph/FFGraphicEdge.h>
#include <ffgraph/FFSugi.h>
#include <ffgraph/FFSpring.h>
#include <ffgraph/FFTclWindow.h>
#include <ffgraph/FFTcl3DWindow.h>
#include <fstream>
#include <cmath>
#include <iostream>

struct HeadlessSugi : FFSugi {
  void init(const FFGraph& graph) override { FFLayoutLabel::init(graph); }
};

int main() {
  FFGraph graph(YES);
  graph->setNodeIndex(graph->addNodeLabel(new FFGraphicNode));
  graph->setEdgeIndex(graph->addEdgeLabel(new FFGraphicEdge));
  auto a = graph->addNode(FFPoint(0, 0));
  auto b = graph->addNode(FFPoint(100, 100));
  auto edge = graph->addEdge(a, b);
  edge->setName("installed library edge");
  auto* layout = new HeadlessSugi;
  graph->addGraphLabel(layout);
  layout->layout();
  if (graph->getNumNodes() != 2 || graph->getNumEdges() != 1
      || edge->getName() != String("installed library edge")) return 1;
  for (auto node : {a, b})
    for (int axis = 0; axis < 3; ++axis)
      if (!std::isfinite(node->getGraphicNode()->getCoords().v[axis])) return 2;
  // load() references all three generated parser backends at link time.
  std::ofstream("consumer.gml") << "graph [ node [ id 1 ] node [ id 2 ] edge [ source 1 target 2 ] ]\n";
  FFGraph imported(YES);
  imported->load(String("consumer.gml"), YES);
  if (imported->getNumNodes() != 2 || imported->getNumEdges() != 1) return 3;
  std::cout << "Installed graph, layout and GML import passed\n";
}
