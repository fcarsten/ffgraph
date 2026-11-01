// A failed *semantic* GML import must not leave a partially updated graph.
// Construct parser events directly to exercise the adapter without a scanner.
#include "gml_graph_adapter.h"
#include "../graph_corpus/graph_snapshot.h"

#include <ffgraph/FFGraph.h>
#include <ffgraph/FFGraphicEdge.h>
#include <ffgraph/FFGraphicNode.h>
#include <ffgraph/FFNodeClass.h>
#include <ffgraph/FFEdgeClass.h>

#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>

namespace {
using ffgraph_gml::Event;
using ffgraph_gml::Result;

void require(bool condition, const std::string& why) {
  if (!condition) throw std::runtime_error(why);
}

Event begin(const char* key) { return {Event::Kind::Begin, key, {}}; }
Event end() { return {Event::Kind::End, {}, {}}; }
Event number(const char* key, int value) {
  Event event{Event::Kind::Integer, key, {}};
  event.integer = value;
  return event;
}
Event label(const char* key, const char* value) {
  Event event{Event::Kind::String, key, value};
  return event;
}

void initialise(FFGraph& graph) {
  graph->setNodeIndex(graph->addNodeLabel(new FFGraphicNode));
  graph->setEdgeIndex(graph->addEdgeLabel(new FFGraphicEdge));
  graph->setName("original");
  auto* first = graph->addNode();
  first->setValue(11);
  first->setName("existing first");
  auto* second = graph->addNode();
  second->setValue(22);
  second->setName("existing second");
  auto* edge = graph->addEdge(first, second);
  require(edge != nullptr, "cannot prepare original edge");
  edge->setValue(9);
  edge->setName("existing edge");
  graph->setStartNode(first);
}

Result parsed(std::vector<Event> events) {
  Result result;
  result.ok = true;
  result.events.values = std::move(events);
  return result;
}

void rejectWithoutMutation(const std::string& case_name, const Result& input,
                           const std::string& expected_error) {
  FFGraph graph(YES);
  initialise(graph);
  const auto baseline = ffgraph_test::capture(*graph.operator->());
  FFNodeClass* const first = graph->getStartNode();
  const auto initial_edges = graph->getNumEdges();
  for (int repeat = 0; repeat < 15; ++repeat) {
    const auto response = ffgraph_gml::applyToGraph(input, *graph.operator->());
    require(!response.ok, case_name + ": unexpectedly accepted");
    require(response.error.find(expected_error) != std::string::npos,
            case_name + ": unexpected diagnostic: " + response.error);
    require(ffgraph_test::difference(baseline, ffgraph_test::capture(*graph.operator->())).empty(),
            case_name + ": changed graph on failure");
    require(graph->getStartNode() == first && graph->getNumEdges() == initial_edges,
            case_name + ": damaged ownership or start-node identity");
  }
}

void acceptedAppend() {
  FFGraph graph(YES);
  initialise(graph);
  const FFNodeClass* initial_start = graph->getStartNode();
  auto input = parsed({begin("graph"), label("label", "new graph name"),
                       begin("node"), number("id", 101), label("label", "new node"), end(),
                       begin("edge"), number("id", 44), number("source", 101),
                       number("target", 22), label("label", "new edge"), end(), end()});
  const auto response = ffgraph_gml::applyToGraph(input, *graph.operator->());
  require(response.ok, "valid append rejected: " + response.error);
  require(graph->getStartNode() == initial_start, "valid append replaced existing nodes");
  require(graph->getNumNodes() == 3 && graph->getNumEdges() == 2,
          "valid append has incorrect cardinality");
  auto* added = graph->nodeWithValue(101);
  require(added && std::string(added->getName().c_str()) == "new node", "node not imported");
  bool edge_found = false;
  for (Pix p = graph->allEdges().first(); p; graph->allEdges().next(p)) {
    auto* edge = graph->allEdges()(p);
    if (edge->getValue() == 44) {
      require(edge->source() == added && edge->target()->getValue() == 22,
              "valid append edge endpoints incorrect");
      edge_found = true;
    }
  }
  require(edge_found && std::string(graph->getName().c_str()) == "new graph name",
          "valid append lost attributes");

  // Keep a historical quirk: by_id preserves aliases when a node receives
  // multiple `id` attributes. Preflight must not reject such an import.
  const auto aliases = parsed({begin("graph"), begin("node"), number("id", 31),
                               number("id", 32), end(), begin("edge"),
                               number("source", 31), number("target", 22), end(), end()});
  const auto alias_response = ffgraph_gml::applyToGraph(aliases, *graph.operator->());
  require(alias_response.ok, "reassigned node ID alias unexpectedly rejected");
  require(graph->getNumNodes() == 4 && graph->getNumEdges() == 3,
          "reassigned ID import has incorrect cardinality");
  auto* aliased = graph->nodeWithValue(32);
  bool alias_edge = false;
  for (Pix p = graph->allEdges().first(); p; graph->allEdges().next(p)) {
    auto* edge = graph->allEdges()(p);
    alias_edge |= edge->source() == aliased && edge->target()->getValue() == 22;
  }
  require(alias_edge, "preflight changed source alias resolution");

  const auto before_empty = ffgraph_test::capture(*graph.operator->());
  require(ffgraph_gml::applyToGraph(parsed({}), *graph.operator->()).ok,
          "valid empty event stream rejected");
  require(ffgraph_test::difference(before_empty, ffgraph_test::capture(*graph.operator->())).empty(),
          "empty event stream changed graph");
}
}  // namespace

int main() {
  try {
    rejectWithoutMutation("unknown target after other changes",
      parsed({begin("graph"), label("label", "changed name"),
              begin("node"), number("id", 101), end(),
              begin("edge"), number("source", 101), number("target", 999999), end(), end()}),
      "Unknown GML edge target ID");
    rejectWithoutMutation("unknown source after other changes",
      parsed({begin("graph"), begin("node"), number("id", 101), end(),
              begin("edge"), number("source", 999999), end(), end()}),
      "Unknown GML edge source ID");
    rejectWithoutMutation("edge before node after graph rename",
      parsed({begin("graph"), label("label", "changed name"),
              begin("edge"), number("target", 11), end(), end()}),
      "GML edge without a preceding node");
    rejectWithoutMutation("later invalid edge",
      parsed({begin("graph"), begin("node"), number("id", 101), end(),
              begin("edge"), number("source", 101), number("target", 22), end(),
              begin("edge"), number("source", 101), number("target", 999999), end(), end()}),
      "Unknown GML edge target ID");
    rejectWithoutMutation("stale ID map is accepted by legacy semantics",
      parsed({begin("graph"), begin("node"), number("id", 31), number("id", 32), end(),
              begin("edge"), number("source", 31), number("target", 999999), end(), end()}),
      "Unknown GML edge target ID");
    auto invalid_syntax = parsed({begin("graph"), begin("node")});
    invalid_syntax.ok = false;
    invalid_syntax.events.error = "intentionally truncated";
    rejectWithoutMutation("syntax error", invalid_syntax, "intentionally truncated");
    acceptedAppend();
    std::cout << "Pass 11 semantic atomicity: 6 rejection cases x15, append, alias and empty stream passed\n";
  } catch (const std::exception& error) {
    std::cerr << "Pass 11 semantic atomicity: " << error.what() << '\n';
    return 1;
  }
}
