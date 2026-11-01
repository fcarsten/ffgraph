#include "rome_records.h"
#include "rome_graph_adapter.h"
#include "../test_check.h"
#include <ffgraph/FFGraph.h>
#include <ffgraph/FFGraphClass.h>
#include <ffgraph/FFNodeClass.h>
#include <ffgraph/FFEdgeClass.h>
#include <ffgraph/FFGraphLabel.h>
#include <ffgraph/FFNodeLabel.h>
#include <ffgraph/FFEdgeLabel.h>
#include <ffgraph/FFGraphicNode.h>
#include <ffgraph/FFGraphicEdge.h>
#include <container/AVLMap.h>
#include "../../fgraph/RomeParser.h"
#include "../../fgraph/RomeScanner.h"
#include <algorithm>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <random>
#include <sstream>
#include <string>
#include <tuple>
#include <vector>
#include <unistd.h>

extern charErrorHandler* userCharErrorHandler;
namespace {
std::vector<std::string> errors;
void error(errGrade grade, const char* message, const char*, int) {
  FF_CHECK(grade == USER_ERROR);
  errors.emplace_back(message);
}
template<class Base> struct Payload final : Base {
  String value = "prototype payload";
  Payload() { this->_setType("RomePayload"); }
  Payload* proto() const override { return new Payload; }
  BOOL parse(const String& text) override { value = text; return YES; }
  String unparse() override { return value; }
};
struct Watch final : FFGraphLabel {
  std::vector<labelEvent> events;
  Watch() { _setType("RomeWatch"); }
  Watch* proto() const override { return new Watch; }
  void event(labelEvent kind, labelEventStruct*) override {
    events.push_back(kind);
    if (kind == ADD_NODE) {
      // Nested modern parses from real application label callbacks are isolated.
      const auto nested = ffgraph_rome::parse("999 0\n");
      FF_CHECK(nested.ok && nested.records.size() == 1 && nested.records[0].id == 999);
    }
  }
};
struct Relabel final : FFGraphLabel {
  int nodes = 0;
  Relabel() { _setType("RomeRelabel"); }
  Relabel* proto() const override { return new Relabel; }
  void event(labelEvent kind, labelEventStruct*) override {
    if (kind == ADD_NODE && ++nodes == 3) {
      auto* old = getGraph()->nodeWithValue(1);
      FF_CHECK(old); old->setValue(11);
    }
  }
};
struct Capture {
  std::ostringstream output;
  std::streambuf* previous = std::cerr.rdbuf(output.rdbuf());
  ~Capture() { std::cerr.rdbuf(previous); }
};
struct File {
  std::string path = (std::filesystem::temp_directory_path() / "ff-rome-XXXXXX.rome").string();
  explicit File(const std::string& contents) {
    const int fd = mkstemps(path.data(), 5);
    FF_CHECK(fd >= 0); close(fd);
    std::ofstream out(path, std::ios::binary); out << contents; out.close(); FF_CHECK(out.good());
  }
  ~File() { std::error_code error; std::filesystem::remove(path, error); }
};
std::string read(const std::string& folder, const std::string& name) {
  std::ifstream in(std::string(FFGRAPH_TEST_SOURCE_DIR) + "/tests/" + folder + "/" + name + ".rome",
                   std::ios::binary);
  FF_CHECK(in.good());
  return {std::istreambuf_iterator<char>(in), {}};
}
Watch* setup(FFGraph& graph, bool graphics, bool merge) {
  graph->setName("Rome oracle"); graph->_setDirected(YES);
  if (graphics) {
    graph->setNodeIndex(graph->addNodeLabel(new FFGraphicNode));
    graph->setEdgeIndex(graph->addEdgeLabel(new FFGraphicEdge));
  }
  graph->addNodeLabel(new Payload<FFNodeLabel>);
  graph->addEdgeLabel(new Payload<FFEdgeLabel>);
  if (merge) {
    auto* old = graph->addNode(FFPoint(7, 8, 9)); old->setValue(41); old->setName("existing");
    auto* edge = graph->addEdge(old, old); edge->setValue(900);
    edge->setName("existing loop"); edge->setWeight(2.5);
    old->getLabel(old->getLabelIndex("RomePayload"))->parse("existing node payload");
    edge->getLabel(edge->getLabelIndex("RomePayload"))->parse("existing edge payload");
    graph->setStartNode(old); graph->addEndNode(old);
    if (auto* graphic = old->getGraphicNode()) graphic->setColor("blue");
  }
  auto* watch = new Watch; graph->addGraphLabel(watch); watch->events.clear();
  return watch;
}
std::string snapshot(FFGraph& graph) {
  std::ostringstream out;
  out << std::setprecision(17) << graph->getDirected() << ' ' << std::quoted(std::string(graph->getName()));
  if (graph->getStartNode()) out << " start " << graph->getStartNode()->getValue();
  std::vector<int> ends;
  for (Pix p = graph->getEndNodes().first(); p; graph->getEndNodes().next(p))
    ends.push_back(graph->getEndNodes()(p)->getValue());
  std::sort(ends.begin(), ends.end()); for (int id : ends) out << " end " << id;
  out << '\n';
  std::vector<std::string> nodes, edges;
  for (Pix p = graph->allNodes().first(); p; graph->allNodes().next(p)) {
    auto* node = graph->allNodes()(p);
    std::ostringstream row; row << std::setprecision(17);
    row << node->getValue() << ' ' << std::quoted(std::string(node->getName()))
        << ' ' << node->indeg() << ' ' << node->outdeg() << ' '
        << std::quoted(std::string(node->getLabel(node->getLabelIndex("RomePayload"))->unparse()));
    if (auto* graphic = node->getGraphicNode()) {
      for (int d = 0; d < 3; ++d) row << ' ' << graphic->getCoords().v[d];
      for (int d = 0; d < 3; ++d) row << ' ' << graphic->getRange().v[d];
      row << ' ' << std::quoted(std::string(graphic->getColor()))
          << ' ' << std::quoted(std::string(graphic->getIcon()))
          << ' ' << std::quoted(std::string(graphic->getSelIcon()))
          << ' ' << std::quoted(std::string(graphic->getMask()))
          << ' ' << std::quoted(std::string(graphic->getSelMask()))
          << ' ' << graphic->getShape() << ' ' << graphic->getFigure();
    }
    nodes.push_back(row.str());
  }
  for (Pix p = graph->allEdges().first(); p; graph->allEdges().next(p)) {
    auto* edge = graph->allEdges()(p);
    FF_CHECK(graph->allNodes().contains(edge->source()) && graph->allNodes().contains(edge->target()));
    std::ostringstream row; row << std::setprecision(17);
    row << edge->getValue() << ' ' << edge->source()->getValue() << ' ' << edge->target()->getValue()
        << ' ' << edge->getWeight() << ' ' << std::quoted(std::string(edge->getName())) << ' '
        << std::quoted(std::string(edge->getLabel(edge->getLabelIndex("RomePayload"))->unparse()));
    if (auto* graphic = edge->getGraphicEdge()) {
      row << ' ' << std::quoted(std::string(graphic->getColor())) << ' ' << graphic->anzPoints();
      for (int i = 0; i < graphic->anzPoints(); ++i)
        for (int d = 0; d < 3; ++d) row << ' ' << graphic->getPointAt(i).v[d];
    }
    edges.push_back(row.str());
  }
  std::sort(nodes.begin(), nodes.end()); std::sort(edges.begin(), edges.end());
  for (const auto& row : nodes) out << "node " << row << '\n';
  for (const auto& row : edges) out << "edge " << row << '\n';
  FF_CHECK(graph->getNumNodes() == static_cast<int>(nodes.size()));
  FF_CHECK(graph->getNumEdges() == static_cast<int>(edges.size()));
  return out.str();
}
using Edge = std::tuple<int, int, int>; // id, source, target
void oracle(FFGraph& graph, std::vector<int> nodes, std::vector<Edge> edges) {
  std::vector<int> actualNodes; std::vector<Edge> actualEdges;
  for (Pix p = graph->allNodes().first(); p; graph->allNodes().next(p))
    actualNodes.push_back(graph->allNodes()(p)->getValue());
  for (Pix p = graph->allEdges().first(); p; graph->allEdges().next(p)) {
    auto* edge = graph->allEdges()(p);
    actualEdges.emplace_back(edge->getValue(), edge->source()->getValue(), edge->target()->getValue());
  }
  std::sort(nodes.begin(), nodes.end()); std::sort(edges.begin(), edges.end());
  std::sort(actualNodes.begin(), actualNodes.end()); std::sort(actualEdges.begin(), actualEdges.end());
  FF_CHECK(actualNodes == nodes && actualEdges == edges);
}
int legacy(FFGraph& graph, const std::string& input, int mode) {
  errors.clear(); Capture capture;
  if (mode == 0) { RomeControl parser; std::istringstream in(input); parser.Parse(in, graph.operator->()); }
  else if (mode == 1) { RomeControl parser; parser.Parse(String(input), graph.operator->()); }
  else { File file(input); graph->load(String(file.path), YES); }
  const std::string warning = "Couldn't find nodes for edge", text = capture.output.str();
  int missing = 0;
  for (std::size_t pos = 0; (pos = text.find(warning, pos)) != std::string::npos; pos += warning.size())
    ++missing;
  return missing;
}
void compare(const std::string& input, bool graphics, bool merge, int mode,
             const std::vector<int>& expectedNodes, const std::vector<Edge>& expectedEdges, int missing) {
  FFGraph old(YES), modern(YES);
  auto* oldWatch = setup(old, graphics, merge); auto* modernWatch = setup(modern, graphics, merge);
  FF_CHECK(legacy(old, input, mode) == missing && errors.empty());
  const auto parsed = ffgraph_rome::parse(input); FF_CHECK(parsed.ok);
  const auto applied = ffgraph_rome::applyToGraph(parsed, *modern.operator->(), mode == 2);
  FF_CHECK(applied.ok && applied.missing.size() == static_cast<std::size_t>(missing));
  if (missing == 2) {
    FF_CHECK(applied.missing[0].edge == 80 && applied.missing[0].source == 1 &&
      applied.missing[0].target == 2 && applied.missing[0].line == 1);
    FF_CHECK(applied.missing[1].edge == 82 && applied.missing[1].source == 2 &&
      applied.missing[1].target == 99 && applied.missing[1].line == 5);
  }
  const bool sentinelAlias = merge && mode == 2 &&
    std::find(expectedNodes.begin(), expectedNodes.end(), -1) != expectedNodes.end();
  if (sentinelAlias) {
    // Old IDs reset to -1 can alias imported -1. Allocation-dependent container
    // order can select different physical duplicates in the two graphs.
    // Check each graph's live lookup and preserved seed identity explicitly.
    for (auto* graph : {old.operator->(), modern.operator->()}) {
      auto* resolved = graph->nodeWithValue(-1);
      FFNodeClass* original = nullptr;
      for (Pix p = graph->allNodes().first(); p; graph->allNodes().next(p))
        if (graph->allNodes()(p)->getName() == "existing") original = graph->allNodes()(p);
      FF_CHECK(original && original->getValue() == -1 && graph->getStartNode() == original);
      FF_CHECK(original->getLabel(original->getLabelIndex("RomePayload"))->unparse() == "existing node payload");
      FF_CHECK(graph->getEndNodes().contains(original));
      if (auto* graphic = original->getGraphicNode())
        FF_CHECK(graphic->getx() == 7 && graphic->gety() == 8 && graphic->getz() == 9 && graphic->getColor() == "blue");
      for (Pix p = graph->allEdges().first(); p; graph->allEdges().next(p)) {
        auto* edge = graph->allEdges()(p);
        if (edge->getValue() == 8) FF_CHECK(edge->source() == resolved && edge->target() == graph->nodeWithValue(0));
        if (edge->getValue() == 9) FF_CHECK(edge->source() == resolved && edge->target() == resolved);
        if (edge->getValue() == 900)
          FF_CHECK(edge->source() == original && edge->target() == original && edge->getWeight() == 2.5 && edge->getName() == "existing loop");
      }
    }
  } else FF_CHECK(snapshot(old) == snapshot(modern));
  std::vector<labelEvent> events = oldWatch->events;
  if (mode == 2) {
    FF_CHECK(!events.empty() && events.front() == START_LOAD && events.back() == END_LOAD);
    events.erase(events.begin()); events.pop_back();
  }
  FF_CHECK(events == modernWatch->events);
  auto nodes = expectedNodes; auto edges = expectedEdges;
  if (merge) { const int id = mode == 2 ? -1 : 41; nodes.push_back(id); edges.emplace_back(900, id, id); }
  oracle(old, nodes, edges); oracle(modern, nodes, edges);
}
void fixtures() {
  struct Fixture { const char* folder; const char* name; std::vector<int> nodes; std::vector<Edge> edges; int missing = 0; };
  const Fixture cases[] = {
    {"rome_fixtures", "structure", {1,2,9}, {{100,1,2},{101,1,2},{102,1,1}}},
    {"rome_fixtures", "duplicates", {-5,-5,7}, {{20,-5,7},{21,7,7}}},
    {"rome_fixtures", "forward-missing", {1,2}, {{81,2,1}}, 2},
    {"rome_fixtures", "punctuation", {1,2}, {{3,1,2}}},
    {"rome_fixtures", "empty", {}, {}},
    {"modern_rome/fixtures", "integer-limits", {-2147483647-1,2147483647},
      {{-2147483647-1,-2147483647-1,2147483647},{2147483647,2147483647,2147483647}}},
    {"modern_rome/fixtures", "lexical-quirks", {1,2,3}, {{10,1,2},{11,3,1}}},
    {"modern_rome/fixtures", "negative-sentinel", {-1,0}, {{8,-1,0},{9,-1,-1}}}
  };
  for (const auto& fixture : cases) {
    const auto input = read(fixture.folder, fixture.name);
    for (bool graphics : {false,true}) for (bool merge : {false,true})
      for (int mode = 0; mode < 3; ++mode) for (int ending = 0; ending < 3; ++ending) {
        std::string text = input;
        if (ending == 1) for (std::size_t pos = 0; (pos = text.find('\n',pos)) != std::string::npos; pos += 2)
          text.insert(pos,1,'\r');
        if (ending == 2) while (!text.empty() && text.back() == '\n') text.pop_back();
        compare(text,graphics,merge,mode,fixture.nodes,fixture.edges,fixture.missing);
      }
  }
  // Resolve duplicate existing IDs using each graph's actual container order.
  for (bool reset : {false,true}) {
    FFGraph graph(YES); auto* a = graph->addNode(); auto* b = graph->addNode();
    a->setValue(5); b->setValue(5); a->setName("a"); b->setName("b");
    auto* expected = graph->nodeWithValue(5);
    const int id = reset ? -1 : 5;
    if (reset) { a->setValue(-1); b->setValue(-1); expected = graph->nodeWithValue(-1); }
    const auto applied = ffgraph_rome::applyToGraph(ffgraph_rome::parse("88 0 " +
      std::to_string(id) + " " + std::to_string(id) + "\n"), *graph.operator->(), reset);
    FF_CHECK(applied.ok && graph->getNumNodes() == 2 && graph->getNumEdges() == 1);
    auto* edge = graph->allEdges()(graph->allEdges().first());
    FF_CHECK(edge->source() == expected && edge->target() == expected);
  }
  // Resolve endpoints from current graph values, including real label callback
  // mutations between rows. A cached ID map would give the wrong graph here.
  for (int mode = 0; mode < 3; ++mode) {
    FFGraph old(YES), modern(YES); setup(old,true,false); setup(modern,true,false);
    old->addGraphLabel(new Relabel); modern->addGraphLabel(new Relabel);
    const std::string input = "1 0\n2 0\n80 0 1 2\n3 0\n81 0 1 2\n82 0 11 2\n";
    FF_CHECK(legacy(old,input,mode) == 1 && errors.empty());
    const auto applied = ffgraph_rome::applyToGraph(ffgraph_rome::parse(input),*modern.operator->(),mode==2);
    FF_CHECK(applied.ok && applied.missing.size() == 1 && applied.missing[0].edge == 81);
    FF_CHECK(snapshot(old) == snapshot(modern));
    oracle(modern,{2,3,11},{{80,11,2},{82,11,2}});
  }
  // An edge can refer to existing merge nodes through direct Parse, but public
  // file loading resets their IDs before resolving the same row.
  for (bool graphics : {false,true}) for (int mode = 0; mode < 3; ++mode) {
    const std::vector<Edge> imported = mode == 2 ? std::vector<Edge>{} : std::vector<Edge>{{42,41,41}};
    compare("42 0 41 41\n1 0\n",graphics,true,mode,{1},imported,mode == 2 ? 1 : 0);
  }
  std::cout << "Rome fixtures: 288 matrix imports plus duplicate lookup and callback cases\n";
}
void generated() {
  std::size_t imports = 0;
  for (unsigned seed = 0; seed < 32; ++seed) for (bool graphics : {false,true}) {
    FFGraph source(YES); setup(source,graphics,false); source->_setDirected(NO);
    const int count = seed == 0 ? 0 : static_cast<int>(seed * 2 + 1);
    std::vector<FFNodeClass*> nodes;
    for (int i = 0; i < count; ++i) {
      auto* node = source->addNode(); node->setValue((i % 2 ? -1 : 1) * (1000 + i)); nodes.push_back(node);
    }
    std::mt19937 random(seed);
    for (int i = 0; i < count * 4; ++i) {
      auto* from = nodes[random()%nodes.size()]; auto* to = nodes[random()%nodes.size()];
      auto* edge = source->addEdge(from,to);
      edge->setValue(i - count);
    }
    std::ostringstream text;
    std::vector<int> expectedNodes; std::vector<Edge> expectedEdges;
    for (auto* node : nodes) { text << node->getValue() << " 2147483647\n"; expectedNodes.push_back(node->getValue()); }
    text << "#\n";
    for (Pix p = source->allEdges().first(); p; source->allEdges().next(p)) {
      auto* edge = source->allEdges()(p);
      text << edge->getValue() << " -2147483648 " << edge->source()->getValue() << ' ' << edge->target()->getValue() << '\n';
      expectedEdges.emplace_back(edge->getValue(),edge->source()->getValue(),edge->target()->getValue());
    }
    const auto input = text.str();
    for (int mode = 0; mode < 3; ++mode) {
      compare(input,graphics,false,mode,expectedNodes,expectedEdges,0);
      FFGraph modern(YES); setup(modern,graphics,false);
      FF_CHECK(ffgraph_rome::applyToGraph(ffgraph_rome::parse(input),*modern.operator->()).ok);
      FF_CHECK(snapshot(source) == snapshot(modern)); // Source graph is an independent oracle.
      ++imports;
    }
  }
  std::cout << "Generated Rome oracle: " << imports << " differential imports\n";
}
void failures() {
  const char* paths[][2] = {
    {"rome_fixtures","partial-error"}, {"rome_fixtures","keyword-error"}, {"rome_fixtures","string-error"},
    {"modern_rome/fixtures","extra-field-error"}, {"modern_rome/fixtures","indented-comment-error"},
    {"modern_rome/fixtures","real-error"}
  };
  for (const auto& path : paths) for (int mode = 0; mode < 3; ++mode) {
    const auto input = read(path[0],path[1]);
    FFGraph old(YES), modern(YES); setup(old,true,true); auto* watch = setup(modern,true,true);
    const auto before = snapshot(modern);
    legacy(old,input,mode); FF_CHECK(errors.size() == 1);
    FF_CHECK(errors.front().find("in line 1") != std::string::npos);
    FF_CHECK(old->getNumNodes() > 1 && !old->getDirected()); // Legacy prefix mutation.
    const auto parsed = ffgraph_rome::parse(input); FF_CHECK(!parsed.ok && parsed.records.empty());
    FF_CHECK(!ffgraph_rome::applyToGraph(parsed,*modern.operator->(),mode==2).ok);
    FF_CHECK(snapshot(modern) == before && watch->events.empty());
    FF_CHECK(parsed.error.find("line 2") != std::string::npos || parsed.error.find("line 3") != std::string::npos ||
             parsed.error.find("line 4") != std::string::npos);
  }
  for (const char* name : {"id-overflow","metadata-overflow"}) {
    const auto parsed = ffgraph_rome::parse(read("modern_rome/fixtures",name));
    FF_CHECK(!parsed.ok && parsed.records.empty() && parsed.error.find("out of range in line 2") != std::string::npos);
    // Do not invoke legacy atoi on out-of-range values as a compatibility oracle.
  }
  FFGraph graph(YES); auto* watch = setup(graph,true,true); const auto before = snapshot(graph);
  std::string nul = "1 0\n"; nul.push_back('\0'); nul += "2 0\n";
  FF_CHECK(!ffgraph_rome::applyToGraph(ffgraph_rome::parse(nul),*graph.operator->(),true).ok);
  FF_CHECK(snapshot(graph) == before && watch->events.empty());
  // The legacy line feeder truncates a sufficiently long comment and stops
  // reading after failbit. The modern scanner consumes the complete input.
  const std::string longLine = "#" + std::string(65536,'x') + "\n1 0\n2 0\n";
  FFGraph old(YES), modern(YES); legacy(old,longLine,0); FF_CHECK(errors.empty());
  FF_CHECK(old->getNumNodes() == 0);
  FF_CHECK(ffgraph_rome::applyToGraph(ffgraph_rome::parse(longLine),*modern.operator->()).ok);
  oracle(modern,{1,2},{});
}
}
int main(int argc, char** argv) {
  const auto previous = userCharErrorHandler; userCharErrorHandler = error;
  if (argc == 1) fixtures();
  else if (argc == 2 && std::string(argv[1]) == "--generated") generated();
  else if (argc == 2 && std::string(argv[1]) == "--failures") failures();
  else FF_CHECK(false);
  userCharErrorHandler = previous;
}
