#include "test_check.h"
#include <ffgraph/FFSpring.h>
#include <ffgraph/FFInspector.h>
#include <ffgraph/FFGraphicEdge.h>
#include <cstring>
#include <vector>

namespace {
struct ReferenceForces {
  int dimension;
  std::vector<FSSpringNodes> nodes;
  void compute_forces(FFCoord,FFPoint*,int);
  void compute_forces_edges(FFCoord,FFPoint*,int);
};
int referenceBreak() { FF_CHECK(false); return 0; }
#include "spring_force_reference.inc"
struct Spring : FFSpring {
  void init(const FFGraph& g) override { FFLayoutLabel::init(g); }
  void observer(FFInspector* i) { inspector = i; }
};
struct Compare : FFInspector {
  Spring& spring;
  FFGraphClass* graph;
  bool weighted;
  int dimension, calls = 0;
  Compare(Spring& s, const FFGraph& g, bool w, int dim) : FFInspector(&s), spring(s), graph(g.operator->()), weighted(w), dimension(dim) {}
  void inspect() override {}
  void update() override {
    const int count = graph->getNumNodes();
    spring.write_nodePosition(count);
    ReferenceForces reference{dimension,std::vector<FSSpringNodes>(count+1)};
    const auto& all = graph->allNodes();
    for (Pix p = all.first(); p; all.next(p)) {
      auto node = all(p); auto& n = reference.nodes[node->getValue()];
      n.attrib = 0; n.coord = node->getGraphicNode()->getCoords();
      if (dimension == 2) n.coord.v[2] = 0;
      n.toSucc |= node->transToSucc(); n.fromPred |= node->transFromPred();
      // Match init_FSSpringNodes' insertion order from the original adjacency
      // sets, not the separately copied edge sets (whose iteration may differ).
      const auto& outgoing = node->transToSucc(); const auto& incoming = node->transFromPred();
      for (Pix e = outgoing.first(); e; outgoing.next(e)) n.conNodes.add(outgoing(e)->target()->getValue());
      for (Pix e = incoming.first(); e; incoming.next(e)) n.conNodes.add(incoming(e)->source()->getValue());
    }
    std::vector<FFPoint> actual(count+1), expected(count+1);
    spring.init_forces(actual.data(),count); spring.init_forces(expected.data(),count);
    if (weighted) { spring.compute_forces_edges(100,actual.data(),count); reference.compute_forces_edges(100,expected.data(),count); }
    else { spring.compute_forces(100,actual.data(),count); reference.compute_forces(100,expected.data(),count); }
    for (int i = 1; i <= count; ++i) if (std::memcmp(actual[i].v,expected[i].v,sizeof actual[i].v) != 0) {
      std::cerr << "dimension=" << dimension << " weighted=" << weighted << " frame=" << calls << " node=" << i << '\n';
      for (int k = 0; k < 4; ++k) std::cerr << k << ": " << actual[i].v[k] << " vs " << expected[i].v[k] << '\n';
      FF_CHECK(false);
    }
    if (++calls == 3) spring.stop();
  }
};
void exercise(int seed, int dimension, bool weighted, bool directed) {
  FFGraph graph(YES); graph->_setDirected(directed);
  graph->setNodeIndex(graph->addNodeLabel(new FFGraphicNode));
  graph->setEdgeIndex(graph->addEdgeLabel(new FFGraphicEdge));
  std::vector<FFNode> nodes;
  for (int i = 0; i < 7; ++i) nodes.push_back(graph->addNode(seed%2 ? FFPoint(40,30,0) : FFPoint(i*17,i%3*21,i%2*9)));
  for (int i = 0; i < 13; ++i) {
    auto edge = graph->addEdge(nodes[(i*3+seed)%7],nodes[(i*5+2)%7]);
    edge->setWeight(i%4 == 0 ? 0 : (i%3+1)*0.75f);
  }
  graph->addEdge(nodes[0],nodes[1])->setWeight(2);
  graph->addEdge(nodes[0],nodes[1])->setWeight(1.5f);
  auto* spring = new Spring; graph->addGraphLabel(spring);
  spring->setDimension(dimension); spring->setUseEdgeWeight(weighted); FF_CHECK(spring->setQuality(0));
  auto* compare = new Compare(*spring,graph,weighted,dimension); spring->observer(compare);
  spring->layout(); FF_CHECK(compare->calls == 3);
}
}
int main() {
  for (int seed = 0; seed < 16; ++seed) for (int dimension : {2,3})
    for (bool weighted : {false,true}) for (bool directed : {false,true}) exercise(seed,dimension,weighted,directed);
  std::cout << "128 graph cases / 384 force frames match the frozen kernels bit for bit\n";
}
