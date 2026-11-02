// Exercise the mandatory production FF loader for both file and string imports.
// Graph expectations are independent of parser event replay.
#include "test_check.h"
#include <ffgraph/FFGraph.h>
#include <ffgraph/FFGraphClass.h>
#include <ffgraph/FFGraphLabel.h>
#include <ffgraph/FFNodeClass.h>
#include <ffgraph/FFEdgeClass.h>
#include <ffgraph/FFGraphicNode.h>
#include <ffgraph/FFGraphicEdge.h>
#include <ffgraph/ffError.h>
#include <filesystem>
#include <fstream>
#include <string>
#include <unistd.h>

extern charErrorHandler* userCharErrorHandler;
namespace {
int errors = 0;
void error(errGrade grade, const char*, const char*, int) {
  FF_CHECK(grade == USER_ERROR);
  ++errors;
}
struct LoadEvents final : FFGraphLabel {
  int starts = 0, ends = 0;
  LoadEvents() { _setType("LoadEvents"); }
  LoadEvents* proto() const override { return new LoadEvents; }
  void event(labelEvent kind, labelEventStruct*) override {
    if (kind == START_LOAD) ++starts;
    if (kind == END_LOAD) ++ends;
  }
};
struct File {
  std::string path = (std::filesystem::temp_directory_path() / "ff-pass12-XXXXXX.g").string();
  explicit File(const std::string& contents) {
    const int fd = mkstemps(path.data(), 2);
    FF_CHECK(fd >= 0); close(fd);
    std::ofstream out(path, std::ios::binary);
    out << contents;
    out.close(); FF_CHECK(out.good());
  }
  ~File() { std::error_code e; std::filesystem::remove(path, e); }
};
void setup(FFGraph& graph) {
  graph->setNodeIndex(graph->addNodeLabel(new FFGraphicNode));
  graph->setEdgeIndex(graph->addEdgeLabel(new FFGraphicEdge));
}
const char* valid = R"(GRAPH "production" = UNDIRECTED
1 {$ NP -10 20 SN $} "a"
 2 {$ FF_WE -125 EL -10 20 3 4 40 50 $} "ab" 2 "parallel" 1 "loop";
2 {$ FF_3D 40 50 6 EN $} "b";
9 "isolated"; END)";
void load(FFGraph& graph, const std::string& input, bool file) {
  if (file) { File source(input); graph->load(String(source.path), YES); }
  else graph->load(String(input), NO);
}
void validLoads(bool file) {
  FFGraph graph(YES); setup(graph);
  auto* events = new LoadEvents; graph->addGraphLabel(events);
  const int before = errors;
  load(graph, valid, file);
  FF_CHECK(errors == before && events->starts == 1 && events->ends == 1);
  FF_CHECK(graph->getName() == "production" && !graph->getDirected());
  FF_CHECK(graph->getNumNodes() == 3 && graph->getNumEdges() == 3);
  auto* a = graph->nodeWithValue(1); auto* b = graph->nodeWithValue(2);
  FF_CHECK(a && b && graph->nodeWithValue(9));
  FF_CHECK(a->getName() == "a" && b->getName() == "b");
  FF_CHECK(a->getGraphicNode()->getx() == -10 && b->getGraphicNode()->getz() == 6);
  FF_CHECK(graph->getStartNode() == a && graph->getEndNodes().contains(b));
  int parallel = 0, loops = 0;
  for (Pix p = graph->allEdges().first(); p; graph->allEdges().next(p)) {
    auto* edge = graph->allEdges()(p);
    FF_CHECK(graph->allNodes().contains(edge->source()) && graph->allNodes().contains(edge->target()));
    if (edge->source() == a && edge->target() == b) ++parallel;
    if (edge->source() == a && edge->target() == a) ++loops;
    if (edge->getName() == "ab") {
      FF_CHECK(edge->getWeight() == -1.25);
      auto* graphic = edge->getGraphicEdge();
      FF_CHECK(graphic->anzPoints() == 3 && graphic->getPointAt(1).v[0] == 3 &&
               graphic->getPointAt(1).v[1] == 4);
    }
  }
  FF_CHECK(parallel == 2 && loops == 1);
  load(graph, "GRAPH \"merge name\" = 100 \"appended\"; END", file);
  FF_CHECK(errors == before && events->starts == 2 && events->ends == 2);
  FF_CHECK(graph->getName() == "production" && graph->getNumNodes() == 4);
  FF_CHECK(graph->allNodes().contains(a) && a->getValue() == -1 && a->getName() == "a");
  FF_CHECK(graph->nodeWithValue(100)->getName() == "appended");
}
void rejectedLoad(bool file) {
  FFGraph graph(YES); setup(graph);
  graph->setName("keep");
  auto* original = graph->addNode(FFPoint(7, 8, 9));
  original->setValue(123); original->setName("original");
  auto* events = new LoadEvents; graph->addGraphLabel(events);
  const int before = errors;
  load(graph, "GRAPH \"bad\" = UNDIRECTED 777 \"partial\";", file);
  FF_CHECK(errors == before + 1 && events->starts == 1 && events->ends == 1);
  FF_CHECK(graph->allNodes().contains(original) && graph->getName() == "keep");
  FF_CHECK(graph->getNumNodes() == 1 && graph->getDirected() && original->getValue() == 123);
  FF_CHECK(original->getName() == "original" && original->getGraphicNode()->getz() == 9);
}
void missingFile() {
  FFGraph graph(YES);
  auto* original = graph->addNode(); original->setValue(123);
  File file(""); FF_CHECK(std::filesystem::remove(file.path));
  const int before = errors;
  graph->load(String(file.path), YES);
  FF_CHECK(errors == before + 1 && graph->getNumNodes() == 1);
  FF_CHECK(original->getValue() == 123);
}
} // namespace
int main() {
  userCharErrorHandler = error;
  for (int repeat = 0; repeat < 10; ++repeat)
    for (bool file : {false, true}) { validLoads(file); rejectedLoad(file); }
  missingFile();
  FFGraph graph(YES);
  auto* original = graph->addNode(); original->setValue(123);
  const int before = errors;
  graph->load("GRAPH \"overflow\" = 2147483648 \"bad\"; END", NO);
  FF_CHECK(errors == before + 1 && graph->getNumNodes() == 1 && original->getValue() == 123);
  FFGraph longLine(YES);
  const std::string name(10000, 'x');
  longLine->load(String("GRAPH \"long\" = 1 \"") + name.c_str() + "\"; END", NO);
  FF_CHECK(errors == before + 1 && longLine->nodeWithValue(1)->getName() == name.c_str());
  std::cout << "Production FF file/string graph oracles, merge and rejection tests passed\n";
}
