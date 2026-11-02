// Production Rome filename dispatch, graph semantics, lifecycle and rejection.
#include "test_check.h"
#include "rome_graph_adapter.h"
#include <ffgraph/FFGraph.h>
#include <ffgraph/FFGraphClass.h>
#include <ffgraph/FFNodeClass.h>
#include <ffgraph/FFEdgeClass.h>
#include <ffgraph/FFGraphLabel.h>
#include <ffgraph/ffError.h>
#include <algorithm>
#include <filesystem>
#include <fstream>
#include <sstream>
#include <string>
#include <tuple>
#include <vector>
#include <unistd.h>

extern charErrorHandler* userCharErrorHandler;
namespace {
std::vector<std::string> errors;
void error(errGrade grade, const char* message, const char*, int) {
  FF_CHECK(grade == USER_ERROR); errors.emplace_back(message);
}
struct Events final : FFGraphLabel {
  int starts = 0, ends = 0;
  Events() { _setType("RomeProductionEvents"); }
  Events* proto() const override { return new Events; }
  void event(labelEvent kind, labelEventStruct*) override {
    if (kind == START_LOAD) ++starts;
    if (kind == END_LOAD) ++ends;
  }
};
struct File {
  std::string path = (std::filesystem::temp_directory_path()/"ff-rome-api-XXXXXX.rome").string();
  explicit File(const std::string& text) {
    const int fd = mkstemps(path.data(),5); FF_CHECK(fd >= 0); close(fd);
    std::ofstream out(path,std::ios::binary); out << text; out.close(); FF_CHECK(out.good());
  }
  ~File() { std::error_code e; std::filesystem::remove(path,e); }
};
using Edge = std::tuple<int,int,int>;
struct Expected {
  const char* name;
  std::vector<int> nodes;
  std::vector<Edge> edges;
  int missing = 0;
  int errorLine = 0;
};
void run(const Expected& expected, int mode) {
  const std::string path = std::string(FFGRAPH_TEST_SOURCE_DIR)+"/tests/rome_fixtures/"+expected.name+".rome";
  std::ifstream in(path,std::ios::binary); FF_CHECK(in.good());
  std::string contents{std::istreambuf_iterator<char>(in),{}};
  if (mode == 2)
    for (std::size_t p = 0; (p = contents.find('\n',p)) != std::string::npos; p += 2) contents.insert(p,1,'\r');
  if (mode == 3) while (!contents.empty() && contents.back() == '\n') contents.pop_back();
  FFGraph graph(YES); graph->setName("keep name"); graph->_setDirected(YES);
  auto* events = new Events; graph->addGraphLabel(events); errors.clear();
  std::ostringstream diagnostics; auto* previous = std::cerr.rdbuf(diagnostics.rdbuf());
  if (mode == 0) graph->load(String(path),YES);
  else { File file(contents); graph->load(String(file.path),YES); }
  std::cerr.rdbuf(previous);
  FF_CHECK(events->starts == 1 && events->ends == 1);
  FF_CHECK(graph->getName() == "keep name");
  if (expected.errorLine) {
    FF_CHECK(errors.size() == 1 && errors[0].find("Invalid Rome file:") != std::string::npos);
    FF_CHECK(errors[0].find("line "+std::to_string(expected.errorLine)+",") != std::string::npos);
    FF_CHECK(graph->getDirected() && graph->getNumNodes() == 0 && graph->getNumEdges() == 0);
  } else {
    FF_CHECK(errors.empty() && !graph->getDirected());
    std::vector<int> nodes; std::vector<Edge> edges;
    for (Pix p = graph->allNodes().first(); p; graph->allNodes().next(p)) nodes.push_back(graph->allNodes()(p)->getValue());
    for (Pix p = graph->allEdges().first(); p; graph->allEdges().next(p)) {
      auto* edge = graph->allEdges()(p);
      FF_CHECK(graph->allNodes().contains(edge->source()) && graph->allNodes().contains(edge->target()));
      edges.emplace_back(edge->getValue(),edge->source()->getValue(),edge->target()->getValue());
    }
    std::sort(nodes.begin(),nodes.end()); std::sort(edges.begin(),edges.end());
    FF_CHECK(nodes == expected.nodes && edges == expected.edges);
  }
  const std::string output = diagnostics.str(), message = "Couldn't find nodes for edge";
  int missing = 0;
  for (std::size_t p = 0; (p = output.find(message,p)) != std::string::npos; p += message.size()) ++missing;
  FF_CHECK(missing == expected.missing);
}
void mergeAndRecovery() {
  FFGraph graph(YES); graph->setName("retained"); graph->_setDirected(YES);
  auto* original = graph->addNode(); original->setValue(123); original->setName("original");
  auto* edge = graph->addEdge(original,original); edge->setValue(456); edge->setWeight(2.5);
  graph->setStartNode(original); graph->addEndNode(original);
  auto* events = new Events; graph->addGraphLabel(events);
  const std::vector<std::string> bad = {
    "1 0\n2 0 0\n", "1 0\nunexpected\n", "1 0\n2147483648 0\n", std::string("1 0\n\0bad\n",9)
  };
  int loads = 0;
  for (const auto& text : bad) {
    File file(text); errors.clear(); graph->load(String(file.path),YES); ++loads;
    FF_CHECK(errors.size() == 1 && graph->getNumNodes() == 1 && graph->getNumEdges() == 1);
    FF_CHECK(graph->getDirected() && graph->getName() == "retained" && original->getValue() == 123);
    FF_CHECK(graph->allNodes().contains(original) && graph->allEdges().contains(edge));
    FF_CHECK(edge->getValue() == 456 && edge->getWeight() == 2.5);
    FF_CHECK(graph->getStartNode() == original && graph->getEndNodes().contains(original));
    FF_CHECK(events->starts == loads && events->ends == loads);
  }
  {
    File absent(""); std::filesystem::remove(absent.path); errors.clear();
    graph->load(String(absent.path),YES); ++loads;
    FF_CHECK(errors.size() == 1 && errors[0].find("Couldn't open File:") != std::string::npos);
    FF_CHECK(original->getValue() == 123 && graph->getDirected() && graph->getNumNodes() == 1);
  }
  // Native string imports remain FF, never Rome.
  errors.clear(); graph->load("1 0\n",NO); ++loads;
  FF_CHECK(errors.size() == 1 && errors[0].find("Invalid FF input:") != std::string::npos);
  FF_CHECK(original->getValue() == 123 && graph->getNumNodes() == 1 && graph->getDirected());
  const std::string valid = "1 0\n2 0\n99 1 1 2\n";
  File good(valid); errors.clear(); graph->load(String(good.path),YES); ++loads;
  FF_CHECK(errors.empty() && !graph->getDirected() && graph->getName() == "retained");
  FF_CHECK(original->getValue() == -1 && graph->allNodes().contains(original));
  FF_CHECK(graph->getNumNodes() == 3 && graph->getNumEdges() == 2 && edge->getWeight() == 2.5);
  FF_CHECK(events->starts == loads && events->ends == loads);
  // Direct replay still offers the internal no-reset behavior.
  FFGraph direct(YES); auto* seed = direct->addNode(); seed->setValue(123);
  FF_CHECK(ffgraph_rome::applyToGraph(ffgraph_rome::parse(valid),*direct.operator->()).ok);
  FF_CHECK(seed->getValue() == 123 && direct->getNumNodes() == 3);
}
}
int main() {
  const auto previous = userCharErrorHandler; userCharErrorHandler = error;
  const Expected cases[] = {
    {"structure",{1,2,9},{{100,1,2},{101,1,2},{102,1,1}}},
    {"duplicates",{-5,-5,7},{{20,-5,7},{21,7,7}}},
    {"forward-missing",{1,2},{{81,2,1}},2},
    {"punctuation",{1,2},{{3,1,2}}},
    {"partial-error",{},{},0,4}, {"keyword-error",{},{},0,2}, {"string-error",{},{},0,2},
    {"empty",{},{}}
  };
  for (const auto& expected : cases) for (int mode = 0; mode < 4; ++mode) run(expected,mode);
  mergeAndRecovery(); userCharErrorHandler = previous;
}
