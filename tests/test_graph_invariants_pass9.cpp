// Pass 9: graph incidence, ownership and copy regression tests, no Tcl/Tk display.
#include <ffgraph/FFGraph.h>
#include <ffgraph/FFGraphClass.h>
#include <ffgraph/FFNodeClass.h>
#include <ffgraph/FFEdgeClass.h>

#include <cstdlib>
#include <cstdint>
#include <vector>
#include <algorithm>
#include <iostream>
#include <map>
#include <set>
#include <utility>

namespace {
void check(bool ok, const char* message) {
  if (!ok) {
    std::cerr << "graph invariant: " << message << '\n';
    std::abort();
  }
}

void verify(const FFGraph& owner) {
  FFGraphClass* g = owner.operator->();
  const auto& nodes = g->allNodes();
  const auto& edges = g->allEdges();
  check(static_cast<int>(nodes.length()) == g->getNumNodes(), "node count");
  check(static_cast<int>(edges.length()) == g->getNumEdges(), "edge count");
  for (Pix i = nodes.first(); i != nullptr; nodes.next(i)) {
    FFNode n = nodes(i);
    check(n != nullptr && n->getGraph() == g, "node belongs to graph");
    for (Pix j = n->transToSucc().first(); j != nullptr; n->transToSucc().next(j)) {
      FFEdge e = n->transToSucc()(j);
      check(edges.contains(e) && e->source() == n, "outgoing edge incidence");
    }
    for (Pix j = n->transFromPred().first(); j != nullptr; n->transFromPred().next(j)) {
      FFEdge e = n->transFromPred()(j);
      check(edges.contains(e) && e->target() == n, "incoming edge incidence");
    }
  }
  for (Pix i = edges.first(); i != nullptr; edges.next(i)) {
    FFEdge e = edges(i);
    check(e && nodes.contains(e->source()) && nodes.contains(e->target()), "endpoints live");
    check(e->source()->transToSucc().contains(e), "edge in outgoing set");
    check(e->target()->transFromPred().contains(e), "edge in incoming set");
  }
  FFNode start = g->getStartNode();
  check(!start || nodes.contains(start), "start is live");
  const auto& ends = g->getEndNodes();
  for (Pix i = ends.first(); i != nullptr; ends.next(i))
    check(nodes.contains(ends(i)), "end is live");
}

using EndpointValues = std::multiset<std::pair<int, int>>;
EndpointValues edgesByValues(FFGraphClass* g) {
  EndpointValues values;
  for (Pix i = g->allEdges().first(); i; g->allEdges().next(i)) {
    FFEdge e = g->allEdges()(i);
    values.emplace(e->source()->getValue(), e->target()->getValue());
  }
  return values;
}

void copyDoesNotMutateSource() {
  FFGraph g(YES);
  FFNode a = g->addNode();
  FFNode b = g->addNode();
  FFNode c = g->addNode();
  FFNode isolated = g->addNode();
  a->setValue(99); b->setValue(-17); c->setValue(2048); isolated->setValue(7);
  FFEdge ab1 = g->addEdge(a, b);
  FFEdge ab2 = g->addEdge(a, b); // multigraph
  FFEdge bb = g->addEdge(b, b); // loop
  FFEdge bc = g->addEdge(b, c);
  ab1->setValue(1001); ab2->setValue(-11); bb->setValue(345); bc->setValue(94);
  check(g->setStartNode(a), "set start");
  check(g->addEndNode(c), "set end");
  verify(g);
  const EndpointValues before = edgesByValues(g.operator->());
  FFGraph clone(g->copy(nullptr, nullptr, YES));
  verify(clone);
  check(a->getValue() == 99 && b->getValue() == -17 && c->getValue() == 2048 &&
        isolated->getValue() == 7, "copy must not renumber source nodes");
  check(ab1->getValue() == 1001 && ab2->getValue() == -11 &&
        bb->getValue() == 345 && bc->getValue() == 94,
        "copy must not renumber source edges");
  check(edgesByValues(g.operator->()) == before, "source topology values unchanged");
  check(edgesByValues(clone.operator->()) == before, "copy preserves topology/value pairs");
  check(clone->getStartNode() && clone->getStartNode()->getValue() == 99, "copied start");
  check(clone->getEndNodes().length() == 1, "copied end count");
  check(clone->getNumNodes() == 4 && clone->getNumEdges() == 4, "copy counts");

  // A subset copy should include loops/multiedges wholly within the subset.
  FFSet<FFNode> subset;
  subset.add(a); subset.add(b);
  FFGraph partial(g->copy(nullptr, &subset, YES));
  verify(partial);
  check(partial->getNumNodes() == 2 && partial->getNumEdges() == 3, "subset counts");
  check(a->getValue() == 99 && b->getValue() == -17,
        "subset copy must not renumber source");

  // A copy is independent of subsequent source mutation.
  g->removeNode(b);
  verify(g);
  verify(clone);
  check(g->getNumNodes() == 3 && g->getNumEdges() == 0, "removal of high-incidence node");
  check(clone->getNumEdges() == 4, "source deletion does not modify copy");
  g->clear();
  verify(g);
  check(g->getNumNodes() == 0 && g->getNumEdges() == 0, "clear");
  verify(partial);
}

// Deterministic state-machine sequence to catch incidence/Pix regressions
// across interleaved node/edge addition, removal and endpoint mutations.
void randomizedMutations() {
  FFGraph g(YES);
  std::vector<FFNode> nodes;
  std::uint32_t seed = 0x9e3779b9u;
  const auto next = [&seed]() {
    seed = seed * 1664525u + 1013904223u;
    return seed;
  };
  for (int step = 0; step != 500; ++step) {
    const int op = next() % 7;
    if ((op == 0 || nodes.empty()) && nodes.size() < 16) {
      nodes.push_back(g->addNode());
    } else if (op == 1 && nodes.size() > 1) {
      FFNode removed = nodes[next() % nodes.size()];
      g->removeNode(removed);
      nodes.erase(std::remove(nodes.begin(), nodes.end(), removed), nodes.end());
      check(!g->contains(removed), "removed node no longer belongs to graph");
    } else if (op == 2 && nodes.size() >= 1) {
      g->addEdge(nodes[next() % nodes.size()], nodes[next() % nodes.size()]);
    } else if (op == 3 && g->getNumEdges() > 0) {
      FFEdge e = g->allEdges()(g->allEdges().first());
      g->removeEdge(e);
      check(!g->contains(e), "removed edge no longer belongs to graph");
    } else if (op == 4 && g->getNumEdges() > 0) {
      FFEdge e = g->allEdges()(g->allEdges().first());
      e->revert();
    } else if (op == 5 && !nodes.empty()) {
      FFNode n = nodes[next() % nodes.size()];
      g->setStartNode(n);
      g->addEndNode(n);
    } else if (op == 6 && g->getNumEdges() > 0 && !nodes.empty()) {
      FFEdge e = g->allEdges()(g->allEdges().first());
      e->setSource(nodes[next() % nodes.size()]);
      e->setTarget(nodes[next() % nodes.size()]);
    }
    verify(g);
  }
  g->clear();
  verify(g);
}

void endpointMutationAndInvalidInputs() {
  FFGraph g(YES), foreign(YES);
  FFNode a = g->addNode(), b = g->addNode(), c = g->addNode();
  FFNode alien = foreign->addNode();
  FFEdge e = g->addEdge(a,b), loop = g->addEdge(a,a);
  verify(g);
  e->setTarget(c); verify(g);
  e->setSource(b); verify(g);
  e->revert(); verify(g);
  loop->revert(); verify(g);
  check(g->addEdge(nullptr, a) == nullptr, "null source rejected");
  check(g->addEdge(a, nullptr) == nullptr, "null target rejected");
  check(g->addEdge(a, alien) == nullptr, "foreign endpoint rejected");
  g->removeNode(nullptr);
  g->removeEdge(nullptr);
  g->removeNode(alien);
  verify(g);
  check(!g->addEndNode(nullptr), "null end node rejected");
  FFSet<FFNode> invalidEnds;
  invalidEnds.add(nullptr);
  check(!g->setEndNodes(invalidEnds), "null end set rejected");
  verify(g);
  g->removeNode(a);
  verify(g);
  g->clear();
  verify(g);
}
}

int main() {
  copyDoesNotMutateSource();
  endpointMutationAndInvalidInputs();
  randomizedMutations();
}
