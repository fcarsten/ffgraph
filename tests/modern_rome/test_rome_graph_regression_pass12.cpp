#include "rome_records.h"
#include "rome_graph_adapter.h"
#include "../test_check.h"
#include <ffgraph/FFGraph.h>
#include <ffgraph/FFGraphClass.h>
#include <ffgraph/FFNodeClass.h>
#include <ffgraph/FFEdgeClass.h>
#include <ffgraph/FFGraphLabel.h>
#include <ffgraph/ffError.h>
#include <ffgraph/FFNodeLabel.h>
#include <ffgraph/FFEdgeLabel.h>
#include <ffgraph/FFGraphicNode.h>
#include <ffgraph/FFGraphicEdge.h>
#include <algorithm>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <limits>
#include <map>
#include <random>
#include <set>
#include <sstream>
#include <stdexcept>
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
  bool checkNestedParse = true;
  Watch() { _setType("RomeWatch"); }
  Watch* proto() const override { return new Watch; }
  void event(labelEvent kind, labelEventStruct*) override {
    events.push_back(kind);
    if (kind == ADD_NODE && checkNestedParse) {
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
      auto* api = getGraph()->nodeWithValue(1);
      FF_CHECK(api); api->setValue(11);
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
Watch* setup(FFGraph& graph, bool graphics, bool merge, int seedId = 41) {
  graph->setName("Rome oracle"); graph->_setDirected(YES);
  if (graphics) {
    graph->setNodeIndex(graph->addNodeLabel(new FFGraphicNode));
    graph->setEdgeIndex(graph->addEdgeLabel(new FFGraphicEdge));
  }
  graph->addNodeLabel(new Payload<FFNodeLabel>);
  graph->addEdgeLabel(new Payload<FFEdgeLabel>);
  if (merge) {
    auto* api = graph->addNode(FFPoint(7, 8, 9)); api->setValue(seedId); api->setName("existing");
    auto* edge = graph->addEdge(api, api); edge->setValue(900);
    edge->setName("existing loop"); edge->setWeight(2.5);
    api->getLabel(api->getLabelIndex("RomePayload"))->parse("existing node payload");
    edge->getLabel(edge->getLabelIndex("RomePayload"))->parse("existing edge payload");
    graph->setStartNode(api); graph->addEndNode(api);
    if (auto* graphic = api->getGraphicNode()) graphic->setColor("blue");
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
// Exercise direct replay and the actual production filename loader. These are
// API consistency checks; frozen legacy data and independent graph oracles
// supply the compatibility expectations after retirement.
int loadGraph(FFGraph& graph, const std::string& input, int mode) {
  errors.clear(); Capture capture;
  if (mode == 0) {
    const auto applied = ffgraph_rome::applyToGraph(ffgraph_rome::parse(input),*graph.operator->());
    if (!applied.ok) errors.push_back(applied.error);
    else for (std::size_t i = 0; i < applied.missing.size(); ++i)
      std::cerr << "Couldn't find nodes for edge\n";
  } else { File file(input); graph->load(String(file.path), YES); }
  const std::string warning = "Couldn't find nodes for edge", text = capture.output.str();
  int missing = 0;
  for (std::size_t pos = 0; (pos = text.find(warning, pos)) != std::string::npos; pos += warning.size())
    ++missing;
  return missing;
}
void compare(const std::string& input, bool graphics, bool merge, int mode,
             const std::vector<int>& expectedNodes, const std::vector<Edge>& expectedEdges, int missing,
             const std::string& frozenName = {}) {
  FFGraph api(YES), modern(YES);
  auto* apiWatch = setup(api, graphics, merge); auto* directWatch = setup(modern, graphics, merge);
  FF_CHECK(loadGraph(api, input, mode) == missing && errors.empty());
  const auto parsed = ffgraph_rome::parse(input); FF_CHECK(parsed.ok);
  const auto applied = ffgraph_rome::applyToGraph(parsed, *modern.operator->(), mode == 1);
  FF_CHECK(applied.ok && applied.missing.size() == static_cast<std::size_t>(missing));
  if (missing == 2) {
    FF_CHECK(applied.missing[0].edge == 80 && applied.missing[0].source == 1 &&
      applied.missing[0].target == 2 && applied.missing[0].line == 1);
    FF_CHECK(applied.missing[1].edge == 82 && applied.missing[1].source == 2 &&
      applied.missing[1].target == 99 && applied.missing[1].line == 5);
  }
  const bool sentinelAlias = merge && mode == 1 &&
    std::find(expectedNodes.begin(), expectedNodes.end(), -1) != expectedNodes.end();
  if (sentinelAlias) {
    // Old IDs reset to -1 can alias imported -1. Allocation-dependent container
    // order can select different physical duplicates in the two graphs.
    // Check each graph's live lookup and preserved seed identity explicitly.
    for (auto* graph : {api.operator->(), modern.operator->()}) {
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
  } else FF_CHECK(snapshot(api) == snapshot(modern));
  if (!merge && !frozenName.empty()) {
    const auto path = std::filesystem::path(FFGRAPH_TEST_SOURCE_DIR)/"tests/modern_rome/oracles"/
      (frozenName + (graphics ? ".graphics.snapshot" : ".bare.snapshot"));
    std::ifstream in(path,std::ios::binary); FF_CHECK(in.good());
    const std::string expected{std::istreambuf_iterator<char>(in),{}};
    if (snapshot(api) != expected) {
      std::cerr << "Frozen legacy graph differs: " << path << '\n'; FF_CHECK(false);
    }
  }
  std::vector<labelEvent> events = apiWatch->events;
  if (mode == 1) {
    FF_CHECK(!events.empty() && events.front() == START_LOAD && events.back() == END_LOAD);
    events.erase(events.begin()); events.pop_back();
  }
  FF_CHECK(events == directWatch->events);
  auto nodes = expectedNodes; auto edges = expectedEdges;
  if (merge) { const int id = mode == 1 ? -1 : 41; nodes.push_back(id); edges.emplace_back(900, id, id); }
  oracle(api, nodes, edges); oracle(modern, nodes, edges);
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
      for (int mode = 0; mode < 2; ++mode) for (int ending = 0; ending < 3; ++ending) {
        std::string text = input;
        if (ending == 1) for (std::size_t pos = 0; (pos = text.find('\n',pos)) != std::string::npos; pos += 2)
          text.insert(pos,1,'\r');
        if (ending == 2) while (!text.empty() && text.back() == '\n') text.pop_back();
        compare(text,graphics,merge,mode,fixture.nodes,fixture.edges,fixture.missing,fixture.name);
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
  for (int mode = 0; mode < 2; ++mode) {
    FFGraph api(YES), modern(YES); setup(api,true,false); setup(modern,true,false);
    api->addGraphLabel(new Relabel); modern->addGraphLabel(new Relabel);
    const std::string input = "1 0\n2 0\n80 0 1 2\n3 0\n81 0 1 2\n82 0 11 2\n";
    FF_CHECK(loadGraph(api,input,mode) == 1 && errors.empty());
    const auto applied = ffgraph_rome::applyToGraph(ffgraph_rome::parse(input),*modern.operator->(),mode==1);
    FF_CHECK(applied.ok && applied.missing.size() == 1 && applied.missing[0].edge == 81);
    FF_CHECK(snapshot(api) == snapshot(modern));
    oracle(modern,{2,3,11},{{80,11,2},{82,11,2}});
  }
  // An edge can refer to existing merge nodes through direct Parse, but public
  // file loading resets their IDs before resolving the same row.
  for (bool graphics : {false,true}) for (int mode = 0; mode < 2; ++mode) {
    const std::vector<Edge> imported = mode == 1 ? std::vector<Edge>{} : std::vector<Edge>{{42,41,41}};
    compare("42 0 41 41\n1 0\n",graphics,true,mode,{1},imported,mode == 1 ? 1 : 0);
  }
  std::cout << "Rome fixtures: 192 matrix imports plus duplicate lookup and callback cases\n";
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
    for (int mode = 0; mode < 2; ++mode) {
      compare(input,graphics,false,mode,expectedNodes,expectedEdges,0);
      FFGraph modern(YES); setup(modern,graphics,false);
      FF_CHECK(ffgraph_rome::applyToGraph(ffgraph_rome::parse(input),*modern.operator->()).ok);
      FF_CHECK(snapshot(source) == snapshot(modern)); // Source graph is an independent oracle.
      ++imports;
    }
  }
  std::cout << "Generated Rome oracle: " << imports << " API/oracle imports\n";
}
void failures() {
  const char* paths[][2] = {
    {"rome_fixtures","partial-error"}, {"rome_fixtures","keyword-error"}, {"rome_fixtures","string-error"},
    {"modern_rome/fixtures","extra-field-error"}, {"modern_rome/fixtures","indented-comment-error"},
    {"modern_rome/fixtures","real-error"}, {"modern_rome/fixtures","id-overflow"},
    {"modern_rome/fixtures","metadata-overflow"}
  };
  for (const auto& path : paths) for (int mode = 0; mode < 2; ++mode) {
    const auto input = read(path[0],path[1]);
    FFGraph api(YES), direct(YES); auto* apiWatch = setup(api,true,true); auto* watch = setup(direct,true,true);
    const auto beforeApi = snapshot(api), before = snapshot(direct);
    loadGraph(api,input,mode); FF_CHECK(errors.size() == 1);
    FF_CHECK(snapshot(api) == beforeApi);
    FF_CHECK(apiWatch->events == (mode == 1 ? std::vector<labelEvent>{START_LOAD,END_LOAD} : std::vector<labelEvent>{}));
    const auto parsed = ffgraph_rome::parse(input); FF_CHECK(!parsed.ok && parsed.records.empty());
    FF_CHECK(!ffgraph_rome::applyToGraph(parsed,*direct.operator->(),mode==1).ok);
    FF_CHECK(snapshot(direct) == before && watch->events.empty());
    FF_CHECK(parsed.error.find("line 2") != std::string::npos || parsed.error.find("line 3") != std::string::npos ||
             parsed.error.find("line 4") != std::string::npos);
  }
  for (int mode = 0; mode < 2; ++mode) {
    FFGraph graph(YES); auto* watch = setup(graph,true,true); const auto before = snapshot(graph);
    std::string nul = "1 0\n"; nul.push_back('\0'); nul += "2 0\n";
    loadGraph(graph,nul,mode); FF_CHECK(errors.size() == 1 && snapshot(graph) == before);
    FF_CHECK(watch->events == (mode == 1 ? std::vector<labelEvent>{START_LOAD,END_LOAD} : std::vector<labelEvent>{}));
    const std::string longLine = "#" + std::string(65536,'x') + "\n1 0\n2 0\n";
    FFGraph complete(YES); loadGraph(complete,longLine,mode); FF_CHECK(errors.empty());
    oracle(complete,{1,2},{});
  }
}

// Independent, deliberately strict row model for an external Rome collection.
// It does not consume generated-parser records. Corrupt files remain explicit
// rejection/state-preservation cases rather than being skipped.
struct CorpusInput {
  std::string text;
  std::vector<int> nodes;
  std::vector<Edge> edges;
  std::size_t badLine = 0, missing = 0;
};
CorpusInput readCorpus(const std::filesystem::path& path) {
  CorpusInput input;
  std::ifstream file(path, std::ios::binary);
  if (!file) throw std::runtime_error("Cannot open corpus file: " + path.string());
  input.text.assign(std::istreambuf_iterator<char>(file), {});
  if (file.bad() || input.text.find('\0') != std::string::npos)
    throw std::runtime_error("Unreadable or binary corpus input: " + path.string());
  std::istringstream lines(input.text);
  std::set<int> available;
  std::string line;
  std::size_t number = 0;
  while (std::getline(lines,line)) {
    ++number;
    if (line.empty() || line.front() == '#') continue;
    std::istringstream row(line);
    row >> std::ws;
    if (row.eof()) continue;
    std::vector<int> values;
    std::string token;
    bool invalid = false;
    while (row >> token) {
      const std::size_t start = token.front() == '-' ? 1 : 0;
      if (start == token.size() || !std::all_of(token.begin()+start,token.end(),
          [](char c) { return c >= '0' && c <= '9'; })) { invalid = true; break; }
      long long value;
      try { value = std::stoll(token); }
      catch (const std::out_of_range&) {
        invalid = true; break;
      }
      if (value < std::numeric_limits<int>::min() || value > std::numeric_limits<int>::max()) { invalid = true; break; }
      values.push_back(static_cast<int>(value));
    }
    if (invalid || (values.size() != 2 && values.size() != 4)) { input.badLine = number; break; }
    if (values.size() == 2) {
      if (!available.insert(values[0]).second)
        throw std::runtime_error("External corpus oracle requires unique node IDs: " + path.string());
      input.nodes.push_back(values[0]);
    } else if (available.count(values[2]) && available.count(values[3]))
      input.edges.emplace_back(values[0],values[2],values[3]);
    else ++input.missing;
  }
  return input;
}

void compareCorpus(const std::filesystem::path& path, const CorpusInput& input,
                   const ffgraph_rome::Result& parsed, bool graphics, bool merge, int mode) {
  const std::string context = path.string() + " [graphics=" + std::to_string(graphics) +
    ", merge=" + std::to_string(merge) + ", API mode=" + std::to_string(mode) + "] ";
  const auto require = [&](bool ok, const std::string& reason) {
    if (!ok) throw std::runtime_error(context + reason);
  };
  // Unlike the synthetic fixtures' seed 41, this cannot alias real node IDs.
  constexpr int seed = std::numeric_limits<int>::min();
  require(std::find(input.nodes.begin(),input.nodes.end(),seed) == input.nodes.end(),"seed ID collision");
  FFGraph api(YES), modern(YES);
  auto* apiWatch = setup(api,graphics,merge,seed);
  auto* directWatch = setup(modern,graphics,merge,seed);
  // Nested parsing already has dedicated tests; the bulk corpus checks the
  // same graph mutation events without starting another scanner on every node.
  apiWatch->checkNestedParse = directWatch->checkNestedParse = false;
  const auto before = input.badLine ? snapshot(modern) : std::string{};
  const auto beforeApi = input.badLine ? snapshot(api) : std::string{};
  require(loadGraph(api,input.text,mode) == static_cast<int>(input.missing),"API missing-endpoint count");
  require(errors.size() == (input.badLine ? 1u : 0u),"API rejection differs from row oracle");
  const auto applied = ffgraph_rome::applyToGraph(parsed,*modern.operator->(),mode == 1);
  require(parsed.ok == !input.badLine && applied.ok == parsed.ok,"modern acceptance differs from row oracle");

  auto expectedNodes = input.nodes;
  auto expectedEdges = input.edges;
  if (merge) {
    const int oldId = mode == 1 ? -1 : seed;
    require(std::find(input.nodes.begin(),input.nodes.end(),oldId) == input.nodes.end(),"merge ID collision");
    expectedNodes.push_back(oldId); expectedEdges.emplace_back(900,oldId,oldId);
  }
  const auto checkTopology = [&](FFGraph& graph) {
    std::vector<int> nodes; std::vector<Edge> edges;
    std::map<int,std::pair<int,int>> degree;
    for (int id : expectedNodes) degree.emplace(id,std::make_pair(0,0));
    for (const auto& edge : expectedEdges) {
      ++degree.at(std::get<1>(edge)).second; ++degree.at(std::get<2>(edge)).first;
    }
    for (Pix p = graph->allNodes().first(); p; graph->allNodes().next(p)) {
      auto* node = graph->allNodes()(p); nodes.push_back(node->getValue());
      require(degree.count(node->getValue()) != 0,"unexpected node ID");
      require(std::make_pair(node->indeg(),node->outdeg()) == degree.at(node->getValue()),"node adjacency counts");
      require(node->getName() == (merge && node->getValue() == (mode == 1 ? -1 : seed) ? "existing" : ""),"node name");
    }
    for (Pix p = graph->allEdges().first(); p; graph->allEdges().next(p)) {
      auto* edge = graph->allEdges()(p);
      require(graph->allNodes().contains(edge->source()) && graph->allNodes().contains(edge->target()),"foreign edge endpoint");
      edges.emplace_back(edge->getValue(),edge->source()->getValue(),edge->target()->getValue());
      require(edge->getWeight() == (edge->getName() == "existing loop" ? 2.5 : 1.0),"edge weight");
    }
    auto sortedNodes = expectedNodes; auto sortedEdges = expectedEdges;
    std::sort(nodes.begin(),nodes.end()); std::sort(edges.begin(),edges.end());
    std::sort(sortedNodes.begin(),sortedNodes.end()); std::sort(sortedEdges.begin(),sortedEdges.end());
    require(nodes == sortedNodes && edges == sortedEdges,"node/edge multisets differ from independent row oracle");
    require(graph->getNumNodes() == static_cast<int>(nodes.size()) && graph->getNumEdges() == static_cast<int>(edges.size()),"graph counts");
  };
  if (input.badLine) {
    require(snapshot(api) == beforeApi,"rejected API import mutated graph");
    require(apiWatch->events == (mode == 1 ? std::vector<labelEvent>{START_LOAD,END_LOAD} : std::vector<labelEvent>{}),"rejected load lifecycle");
    require(parsed.records.empty() && !parsed.error.empty(),"failed parse retained records");
    require(parsed.error.find("line " + std::to_string(input.badLine) + ",") != std::string::npos,"modern error location");
    require(snapshot(modern) == before && directWatch->events.empty(),"rejected modern import mutated graph");
  } else {
    require(!api->getDirected() && api->getName() == "Rome oracle","API graph properties");
    checkTopology(api);
    require(applied.missing.size() == input.missing,"modern missing-endpoint count");
    checkTopology(modern);
    require(snapshot(api) == snapshot(modern),"full graph snapshots differ");
    auto events = apiWatch->events;
    if (mode == 1) {
      require(events.size() >= 2 && events.front() == START_LOAD && events.back() == END_LOAD,"file load lifecycle");
      events.erase(events.begin()); events.pop_back();
    }
    require(events == directWatch->events,"mutation event sequence");
  }
}

void corpus(const std::filesystem::path& root, bool matrix) {
  if (!std::filesystem::is_directory(root)) throw std::runtime_error("Missing corpus directory: " + root.string());
  std::vector<std::filesystem::path> paths;
  for (const auto& item : std::filesystem::recursive_directory_iterator(root))
    if (item.is_regular_file()) paths.push_back(item.path());
  std::sort(paths.begin(),paths.end());
  if (paths.empty()) throw std::runtime_error("Empty corpus directory: " + root.string());
  if (matrix) {
    // Two representatives (sparsest/densest) per family and node-count decade,
    // plus every corrupt file. Ties resolve by sorted relative path.
    using Key = std::pair<std::string,std::size_t>;
    std::map<Key,std::pair<std::filesystem::path,std::filesystem::path>> choices;
    std::map<std::filesystem::path,std::size_t> edgeCounts;
    std::set<std::filesystem::path> selected;
    for (const auto& path : paths) {
      const auto input = readCorpus(path);
      if (input.badLine) { selected.insert(path); continue; }
      edgeCounts[path] = input.edges.size();
      const auto relative = path.lexically_relative(root);
      const Key key{relative.begin()->string(),input.nodes.size()/10};
      auto& pair = choices[key];
      if (pair.first.empty() || input.edges.size() < edgeCounts.at(pair.first)) pair.first = path;
      if (pair.second.empty() || input.edges.size() > edgeCounts.at(pair.second)) pair.second = path;
    }
    for (const auto& choice : choices) {
      selected.insert(choice.second.first); selected.insert(choice.second.second);
    }
    paths.assign(selected.begin(),selected.end());
  }
  std::size_t accepted = 0, rejected = 0, comparisons = 0, nodes = 0, edges = 0;
  for (const auto& path : paths) {
    const auto input = readCorpus(path);
    const auto parsed = ffgraph_rome::parse(input.text);
    if (matrix) {
      for (bool graphics : {false,true}) for (bool merge : {false,true}) for (int mode = 0; mode < 2; ++mode) {
        compareCorpus(path,input,parsed,graphics,merge,mode); ++comparisons;
      }
    } else { compareCorpus(path,input,parsed,false,false,1); ++comparisons; }
    if (input.badLine) ++rejected; else { ++accepted; nodes += input.nodes.size(); edges += input.edges.size(); }
    if ((accepted+rejected)%500 == 0)
      std::cout << "External Rome: " << accepted+rejected << '/' << paths.size() << " files checked\n" << std::flush;
  }
  std::cout << "External Rome " << (matrix ? "matrix" : "full corpus") << ": " << paths.size()
    << " files, " << accepted << " accepted, " << rejected << " rejected, " << comparisons
    << " comparisons, " << nodes << " nodes and " << edges << " edges in accepted inputs\n";
}
}
int main(int argc, char** argv) {
  const auto previous = userCharErrorHandler; userCharErrorHandler = error;
  try {
    if (argc == 1) fixtures();
    else if (argc == 2 && std::string(argv[1]) == "--generated") generated();
    else if (argc == 2 && std::string(argv[1]) == "--failures") failures();
    else if (argc == 3 && std::string(argv[1]) == "--corpus") corpus(argv[2],false);
    else if (argc == 3 && std::string(argv[1]) == "--corpus-matrix") corpus(argv[2],true);
    else throw std::runtime_error("Usage: test_rome_graph_regression_pass12 [--generated|--failures|--corpus DIR|--corpus-matrix DIR]");
  } catch (const std::exception& failure) {
    userCharErrorHandler = previous;
    std::cerr << failure.what() << '\n';
    return 1;
  }
  userCharErrorHandler = previous;
}
