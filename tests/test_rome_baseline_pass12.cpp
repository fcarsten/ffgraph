// Characterize legacy Rome grammar/loading semantics before migration.
#include "test_check.h"
#include <ffgraph/FFGraph.h>
#include <ffgraph/FFGraphClass.h>
#include <ffgraph/FFNodeClass.h>
#include <ffgraph/FFEdgeClass.h>
#include <ffgraph/FFGraphLabel.h>
#include <container/AVLMap.h>
#include "../fgraph/RomeParser.h"
#include "../fgraph/RomeScanner.h"
#include <algorithm>
#include <fstream>
#include <sstream>
#include <string>
#include <tuple>
#include <vector>

extern charErrorHandler* userCharErrorHandler;
namespace {
std::vector<std::string> errors;
void error(errGrade grade, const char* message, const char*, int) {
  FF_CHECK(grade == USER_ERROR);
  errors.emplace_back(message);
}
struct Events final : FFGraphLabel {
  int starts = 0, ends = 0;
  Events() { _setType("RomeBaselineEvents"); }
  Events* proto() const override { return new Events; }
  void event(labelEvent kind, labelEventStruct*) override {
    if (kind == START_LOAD) ++starts;
    if (kind == END_LOAD) ++ends;
  }
};
using Edge = std::tuple<int, int, int>; // id, source id, target id
struct Expected {
  const char* name;
  std::vector<int> nodes;
  std::vector<Edge> edges;
  int missing = 0;
  int syntaxErrors = 0;
};
void check(FFGraph& graph, const Expected& expected) {
  FF_CHECK(!graph->getDirected());
  FF_CHECK(graph->getName() == "keep name");
  std::vector<int> nodes;
  for (Pix p = graph->allNodes().first(); p; graph->allNodes().next(p))
    nodes.push_back(graph->allNodes()(p)->getValue());
  std::vector<Edge> edges;
  for (Pix p = graph->allEdges().first(); p; graph->allEdges().next(p)) {
    auto* edge = graph->allEdges()(p);
    FF_CHECK(graph->allNodes().contains(edge->source()));
    FF_CHECK(graph->allNodes().contains(edge->target()));
    edges.emplace_back(edge->getValue(), edge->source()->getValue(),
                       edge->target()->getValue());
  }
  std::sort(nodes.begin(), nodes.end());
  std::sort(edges.begin(), edges.end());
  FF_CHECK(nodes == expected.nodes);
  FF_CHECK(edges == expected.edges);
}
void run(const Expected& expected, int mode) {
  const std::string path = std::string(FFGRAPH_TEST_SOURCE_DIR) +
    "/tests/rome_fixtures/" + expected.name + ".rome";
  std::ifstream input(path, std::ios::binary);
  FF_CHECK(input.good());
  std::string contents{std::istreambuf_iterator<char>(input), {}};
  FFGraph graph(YES);
  graph->setName("keep name");
  graph->_setDirected(YES);
  auto* events = new Events;
  graph->addGraphLabel(events);
  errors.clear();
  std::ostringstream diagnostics;
  auto* previous = std::cerr.rdbuf(diagnostics.rdbuf());
  if (mode == 0) {
    graph->load(String(path), YES); // Real public filename dispatch.
  } else {
    if (mode == 2) { // CRLF; scanner ignores CR.
      for (std::size_t pos = 0; (pos = contents.find('\n', pos)) != std::string::npos; pos += 2)
        contents.insert(pos, 1, '\r');
    } else if (mode == 3) { // Input feeder synthesizes the final newline.
      while (!contents.empty() && contents.back() == '\n') contents.pop_back();
    }
    RomeControl parser;
    if (mode == 1) {
      std::istringstream stream(contents);
      parser.Parse(stream, graph.operator->());
    } else {
      parser.Parse(String(contents), graph.operator->());
    }
  }
  std::cerr.rdbuf(previous);
  FF_CHECK(events->starts == (mode == 0 ? 1 : 0));
  FF_CHECK(events->ends == events->starts);
  FF_CHECK(errors.size() == static_cast<std::size_t>(expected.syntaxErrors));
  if (!errors.empty()) FF_CHECK(errors.front().find("in line 1") != std::string::npos);
  int missing = 0;
  const std::string output = diagnostics.str(), message = "Couldn't find nodes for edge";
  for (std::size_t pos = 0; (pos = output.find(message, pos)) != std::string::npos; pos += message.size())
    ++missing;
  FF_CHECK(missing == expected.missing);
  check(graph, expected);
}
void merge() {
  // Direct Parse retains existing IDs; public Rome file loads reset them early.
  for (bool file : {false, true}) {
    FFGraph graph(YES);
    auto* old = graph->addNode(); old->setValue(123); old->setName("old");
    const std::string path = std::string(FFGRAPH_TEST_SOURCE_DIR) +
      "/tests/rome_fixtures/structure.rome";
    if (file) graph->load(String(path), YES);
    else {
      std::ifstream input(path);
      RomeControl parser; parser.Parse(input, graph.operator->());
    }
    FF_CHECK(graph->getNumNodes() == 4 && graph->getNumEdges() == 3);
    FF_CHECK(graph->allNodes().contains(old) && old->getName() == "old");
    FF_CHECK(old->getValue() == (file ? -1 : 123));
  }
}
}
int main() {
  const auto previous = userCharErrorHandler;
  userCharErrorHandler = error;
  const Expected cases[] = {
    {"structure", {1, 2, 9}, {{100, 1, 2}, {101, 1, 2}, {102, 1, 1}}},
    {"duplicates", {-5, -5, 7}, {{20, -5, 7}, {21, 7, 7}}},
    {"forward-missing", {1, 2}, {{81, 2, 1}}, 2},
    {"punctuation", {1, 2}, {{3, 1, 2}}},
    {"partial-error", {1, 2}, {{77, 1, 2}}, 0, 1},
    {"keyword-error", {1}, {}, 0, 1},
    {"string-error", {1}, {}, 0, 1},
    {"empty", {}, {}}
  };
  for (const auto& expected : cases)
    for (int mode = 0; mode < 4; ++mode) run(expected, mode);
  merge();
  userCharErrorHandler = previous;
}
