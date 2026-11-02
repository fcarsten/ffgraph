#include "ff_events.h"
#include "ff_graph_adapter.h"
#include <ffgraph/FFGraph.h>
#include <ffgraph/FFGraphClass.h>
#include <ffgraph/FFNodeClass.h>
#include <ffgraph/FFEdgeClass.h>
#include <ffgraph/FFGraphicNode.h>
#include <ffgraph/FFGraphicEdge.h>
#include <ffgraph/FFNodeLabel.h>
#include <ffgraph/FFEdgeLabel.h>
#include <ffgraph/FFGraphLabel.h>
#include <ffgraph/ffError.h>
#include <algorithm>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <limits>
#include <sstream>
#include <string>
#include <vector>

// ffError.h declares setErrorHandlers, but the implementation is named
// setErrorHandler. Use its existing callback slot without changing that API.
extern charErrorHandler* userCharErrorHandler;

namespace {
void check(bool ok, const std::string& message) {
  if (!ok) { std::cerr << "FF graph regression: " << message << '\n'; std::exit(1); }
}
// Real label callbacks, independent of the FF semantic adapter.
template<class Base> struct Probe : Base {
  String payload;
  Probe() { this->_setType("Probe"); }
  Probe* proto() const override { return new Probe; }
  BOOL parse(const String& text) override { payload = text; return YES; }
  String unparse() override { return payload; }
};
void setup(FFGraph& g, bool graphics) {
  if (graphics) {
    g->setNodeIndex(g->addNodeLabel(new FFGraphicNode));
    g->setEdgeIndex(g->addEdgeLabel(new FFGraphicEdge));
  }
  g->addGraphLabel(new Probe<FFGraphLabel>);
  g->addNodeLabel(new Probe<FFNodeLabel>);
  g->addEdgeLabel(new Probe<FFEdgeLabel>);
}
std::string read(const std::filesystem::path& path) {
  std::ifstream in(path, std::ios::binary);
  check(in.good(), "open " + path.string());
  std::ostringstream out; out << in.rdbuf(); return out.str();
}
std::string text(const String& s) { return s.c_str(); }
std::string snapshot(FFGraph& g) {
  std::ostringstream out;
  out << std::setprecision(std::numeric_limits<double>::max_digits10);
  out << g->getDirected() << ' ' << std::quoted(text(g->getName())) << " start ";
  if (g->getStartNode()) out << g->getStartNode()->getValue(); else out << "none";
  std::vector<int> ends;
  for (Pix p = g->getEndNodes().first(); p; g->getEndNodes().next(p))
    ends.push_back(g->getEndNodes()(p)->getValue());
  std::sort(ends.begin(), ends.end());
  for (int id : ends) out << " end " << id;
  const int gl = g->getGraphLabelIndex("Probe");
  out << " label " << std::quoted(text(g->getGraphLabel(gl)->unparse())) << '\n';
  std::vector<std::string> nodes, edges;
  for (Pix p = g->allNodes().first(); p; g->allNodes().next(p)) {
    auto n = g->allNodes()(p);
    std::ostringstream row;
    row << std::setprecision(std::numeric_limits<double>::max_digits10);
    row << "node " << n->getValue() << ' ' << std::quoted(text(n->getName()))
        << " label " << std::quoted(text(n->getLabel(n->getLabelIndex("Probe"))->unparse()));
    if (auto gn = n->getGraphicNode()) {
      for (int d = 0; d < 3; ++d) row << ' ' << gn->getCoords().v[d];
      for (int d = 0; d < 3; ++d) row << ' ' << gn->getRange().v[d];
      row << ' ' << std::quoted(text(gn->getColor()))
          << ' ' << std::quoted(text(gn->getIcon())) << ' ' << std::quoted(text(gn->getSelIcon()))
          << ' ' << std::quoted(text(gn->getMask())) << ' ' << std::quoted(text(gn->getSelMask()))
          << ' ' << gn->getShape() << ' ' << gn->getFigure();
    }
    nodes.push_back(row.str());
  }
  for (Pix p = g->allEdges().first(); p; g->allEdges().next(p)) {
    auto e = g->allEdges()(p);
    check(g->allNodes().contains(e->source()) && g->allNodes().contains(e->target()), "incidence");
    std::ostringstream row;
    row << std::setprecision(std::numeric_limits<double>::max_digits10);
    row << "edge " << e->source()->getValue() << ' ' << e->target()->getValue() << ' '
        << std::quoted(text(e->getName())) << ' ' << e->getWeight() << " label "
        << std::quoted(text(e->getLabel(e->getLabelIndex("Probe"))->unparse()));
    if (auto ge = e->getGraphicEdge()) {
      row << ' ' << std::quoted(text(ge->getColor())) << " points " << ge->anzPoints();
      for (int i = 0; i < ge->anzPoints(); ++i)
        for (int d = 0; d < 3; ++d) row << ' ' << ge->getPointAt(i).v[d];
    }
    edges.push_back(row.str());
  }
  std::sort(nodes.begin(), nodes.end()); std::sort(edges.begin(), edges.end());
  for (const auto& row : nodes) out << row << '\n';
  for (const auto& row : edges) out << row << '\n';
  return out.str();
}
int loadErrors = 0;
void onError(errGrade, const char*, const char*, int) { ++loadErrors; }
void loadProduction(FFGraph& graph, const std::string& input) {
  const int before = loadErrors;
  graph->load(String(input), NO);
  check(loadErrors == before, "production FF load rejected valid input");
}
void loadModern(FFGraph& g, const std::string& input) {
  auto result = ffgraph_ff::parse(input);
  check(result.ok, "modern parse: " + result.error);
  check(ffgraph_ff::applyToGraph(result, *g.operator->()), "modern apply");
}
void compareAPIs(const std::string& input, const std::string& name, bool graphics, bool merge) {
  FFGraph production(YES), modern(YES);
  setup(production, graphics); setup(modern, graphics);
  if (merge) {
    for (auto* g : {&production, &modern}) {
      (*g)->setName("existing name");
      (*g)->addNode()->setValue(100);
    }
  }
  loadProduction(production, input); loadModern(modern, input);
  const auto a = snapshot(production), b = snapshot(modern);
  check(a == b, name + " production/direct graph states differ\nPRODUCTION\n" + a + "MODERN\n" + b);
}
void independentOracle() {
  // No legacy import or writer supplies these expected values.
  const std::string input = R"(GRAPH "oracle" = UNDIRECTED
1 {$ NP -10 20 SN $} "a" 2 {$ FF_WE -125 EL -10 20 3 4 40 50 $} "ab" 2 "parallel" 1 "loop";
2 {$ FF_3D 40 50 6 EN $} "b";
9 "isolated"; END)";
  FFGraph g(YES); setup(g, true);
  loadProduction(g, input);
  check(g->getNumNodes() == 3 && g->getNumEdges() == 3 && !g->getDirected(), "oracle structure");
  auto a = g->nodeWithValue(1), b = g->nodeWithValue(2);
  check(a && b && g->nodeWithValue(9), "oracle IDs");
  check(a->getName() == "a" && b->getName() == "b", "oracle names");
  check(g->getStartNode() == a && g->getEndNodes().contains(b), "oracle start/end");
  check(a->getGraphicNode()->getx() == -10 && a->getGraphicNode()->gety() == 20 &&
        b->getGraphicNode()->getz() == 6, "oracle geometry");
  int ab = 0, loops = 0;
  for (Pix p = g->allEdges().first(); p; g->allEdges().next(p)) {
    auto e = g->allEdges()(p);
    if (e->source() == a && e->target() == b) ++ab;
    if (e->source() == a && e->target() == a) ++loops;
    if (e->getName() == "ab") {
      check(e->getWeight() == -1.25, "oracle weight");
      auto ge = e->getGraphicEdge();
      check(ge->anzPoints() == 3 && ge->getPointAt(1).v[0] == 3 &&
            ge->getPointAt(1).v[1] == 4, "oracle bend ordering");
    }
  }
  check(ab == 2 && loops == 1, "oracle multiplicity");
}
void attributeOracle(const std::filesystem::path& root) {
  FFGraph g(YES); setup(g, true);
  const auto input = read(root / "tests/modern_ff/fixtures/attributes.g");
  loadProduction(g, input);
  auto n = g->nodeWithValue(0);
  check(n && g->getNumNodes() == 2 && g->getNumEdges() == 1, "attribute oracle structure");
  auto gn = n->getGraphicNode();
  check(gn->getx() == 10 && gn->gety() == 20 && gn->getz() == 30 &&
        gn->getWidth() == 9 && gn->getColor() == "blue", "attribute oracle node graphics");
  check(gn->getIcon() == "icon" && gn->getSelIcon() == "selected icon" &&
        gn->getMask() == "mask" && gn->getSelMask() == "selected mask", "attribute oracle icons/masks");
  check(g->getGraphLabel(g->getGraphLabelIndex("Probe"))->unparse() == "graph payload" &&
        n->getLabel(n->getLabelIndex("Probe"))->unparse() == "node payload", "attribute oracle callbacks");
  auto e = g->allEdges()(g->allEdges().first());
  check(e->getWeight() == 3.75 && e->getName() == "route" &&
        e->getLabel(e->getLabelIndex("Probe"))->unparse() == "edge payload", "attribute oracle edge");
  auto ge = e->getGraphicEdge();
  check(ge->getColor() == "green" && ge->anzPoints() == 5 &&
        ge->getPointAt(1).v[0] == 4 && ge->getPointAt(1).v[2] == 6 &&
        ge->getPointAt(2).v[0] == 10 && ge->getPointAt(3).v[0] == 15,
        "attribute oracle mixed 2D/3D route ordering");
  FFGraph lexical(YES); setup(lexical, false);
  const auto unusual = read(root / "tests/modern_ff/fixtures/string-comment.g");
  loadProduction(lexical, unusual);
  check(lexical->nodeWithValue(1)->getName() == "tail", "inclusive comment/string oracle");
}
void failures() {
  const std::vector<std::string> bad = {
    "", "GRAPH", "GRAPH \"missing", "GRAPH \"g\" = DIRECTED 1 \"node\";",
    "GRAPH \"g\" = 1 {$ FF_3D 1 2 $} \"x\"; END",
    "GRAPH \"g\" = 1 \"x\" 2 {$ FF_WE $} \"e\"; END",
    "GRAPHS \"g\" = END", "GRAPH \"g\" = END junk",
    "GRAPH \"g\" = 2147483648 \"x\"; END", "GRAPH \"g\" = -2147483649 \"x\"; END",
    "GRAPH \"g\" = 1 {$ UNKNOWN {$ $} $} \"x\"; END",
    "GRAPH \"g\" = 1 \"unfinished\\", "GRAPH \"g\" =\r END"
  };
  FFGraph g(YES); setup(g, true); g->addNode()->setValue(123);
  const auto before = snapshot(g);
  auto* original = g->nodeWithValue(123);
  for (int round = 0; round < 20; ++round) {
    for (const auto& input : bad) {
      auto result = ffgraph_ff::parse(input);
      check(!result.ok && !result.error.empty() && result.events.empty(), "malformed input rejected");
      check(!ffgraph_ff::applyToGraph(result, *g.operator->()), "failed parse not applied");
      check(snapshot(g) == before && g->nodeWithValue(123) == original, "failure atomicity");
    }
    check(ffgraph_ff::parse("GRAPH \"recovered\" = END").ok, "fresh scanner after errors");
  }
  const auto diagnostic = ffgraph_ff::parse("GRAPH \"g\" =\n1 \"multi\nline\";\n?");
  check(!diagnostic.ok && diagnostic.error.find("line 4") != std::string::npos, "multiline diagnostic");
  check(ffgraph_ff::parse("GRAPH \"bounds\" = -2147483648 \"min\"; 2147483647 \"max\"; END").ok,
        "int bounds");
  const std::string nul("GRAPH \"g\" = END\0", 16);
  check(!ffgraph_ff::parse(nul).ok, "embedded NUL rejected");
  std::string deep = "GRAPH \"flat\" = {$ ";
  for (int i = 0; i < 10000; ++i) deep += "CUSTOM 1 \"value\" ";
  deep += "$} END";
  check(ffgraph_ff::parse(deep).ok, "large internals with dynamic Bison stack");
}
void corpusOracles(const std::filesystem::path& root) {
  const auto fixtures = root / "tests/modern_ff/fixtures";
  const auto oracles = root / "tests/modern_ff/oracles";
  for (const auto& entry : std::filesystem::directory_iterator(fixtures)) {
    if (entry.path().extension() != ".g") continue;
    const auto expected = read(oracles / (entry.path().stem().string() + ".snapshot"));
    for (bool file : {false, true}) {
      FFGraph graph(YES); setup(graph, true);
      const int before = loadErrors;
      if (file) graph->load(String(entry.path().string()), YES);
      else loadProduction(graph, read(entry.path()));
      check(loadErrors == before && snapshot(graph) == expected,
            entry.path().filename().string() + " differs from frozen graph oracle");
    }
  }
  const auto expected = read(oracles / "historical-out.snapshot");
  for (bool file : {false, true}) {
    FFGraph graph(YES); setup(graph, true);
    const int before = loadErrors;
    if (file) graph->load(String((root / "contrib/out.g").string()), YES);
    else loadProduction(graph, read(root / "contrib/out.g"));
    check(loadErrors == before && snapshot(graph) == expected,
          "historical out.g differs from frozen graph oracle");
  }
}
void generatedRoundTrips() {
  for (int seed = 0; seed < 24; ++seed) {
    FFGraph source(seed % 2); setup(source, true);
    source->setName("writer corpus");
    std::vector<FFNodeClass*> nodes;
    for (int i = 0; i < seed + 1; ++i) {
      auto n = source->addNode(FFPoint(i * 10, -i * 7, seed));
      n->setName(String("node ") + String(std::to_string(i)));
      nodes.push_back(n);
    }
    for (int i = 0; i < seed * 2; ++i) {
      auto e = source->addEdge(nodes[i % nodes.size()], nodes[(i * 3 + seed) % nodes.size()]);
      e->setName("generated edge"); e->setWeight((i - seed)/4.0);
      e->getGraphicEdge()->addPointAt(1, FFPoint(i, seed, -3));
    }
    source->setStartNode(nodes.front()); source->addEndNode(nodes.back());
    String output; source->saveToString(output);
    compareAPIs(text(output), "writer seed " + std::to_string(seed), true, false);
    FFGraph modern(YES); setup(modern, true); loadModern(modern, text(output));
    check(snapshot(source) == snapshot(modern), "writer graph matches independent source graph");
    String second; modern->saveToString(second);
    compareAPIs(text(second), "modern reserialization " + std::to_string(seed), true, false);
  }
}
} // namespace
int main(int argc, char** argv) {
  userCharErrorHandler = onError;
  const auto root = std::filesystem::path(FFGRAPH_TEST_SOURCE_DIR);
  if (argc == 2 && std::string(argv[1]) == "--corpus") {
    corpusOracles(root); independentOracle(); attributeOracle(root);
    std::cout << "Frozen FF corpus and independent graph oracles passed\n";
    return 0;
  }
  if (argc == 2 && std::string(argv[1]) == "--parser-failures") {
    failures();
    std::cout << "Modern FF rejection, recovery, numeric and stack tests passed\n";
    return 0;
  }
  check(argc == 1, "unsupported test mode");
  corpusOracles(root);
  int count = 0;
  for (const auto& entry : std::filesystem::directory_iterator(root / "tests/modern_ff/fixtures")) {
    if (entry.path().extension() != ".g") continue;
    const auto input = read(entry.path());
    for (bool graphics : {false, true})
      for (bool merge : {false, true}) compareAPIs(input, entry.path().filename().string(), graphics, merge);
    // Public file load entry point, independently of string load.
    FFGraph fileGraph(YES), modern(YES); setup(fileGraph, true); setup(modern, true);
    const int before = loadErrors;
    fileGraph->load(String(entry.path().string()), YES); loadModern(modern, input);
    check(loadErrors == before && snapshot(fileGraph) == snapshot(modern), "file/string fixture parity");
    ++count;
  }
  const auto historical = read(root / "contrib/out.g");
  compareAPIs(historical, "contrib/out.g", false, false);
  compareAPIs(historical, "contrib/out.g graphics", true, false);
  FFGraph sample(YES); setup(sample, true); loadModern(sample, historical);
  check(sample->getNumNodes() == 499 && sample->getNumEdges() == 499, "historical count oracle");
  independentOracle(); attributeOracle(root); generatedRoundTrips();
  std::cout << count << " FF fixtures, historical graph, 24 writer graphs/reimports and independent oracles passed\n";
}
