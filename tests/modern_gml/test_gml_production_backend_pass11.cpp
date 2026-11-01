// End-to-end regression through the public FFGraphClass::load() entry point.
// Exercises the selected production backend, not applyToGraph() directly.
#include <ffgraph/FFGraph.h>
#include <ffgraph/FFGraphClass.h>
#include <ffgraph/FFNodeClass.h>
#include <ffgraph/FFEdgeClass.h>

#include <cstdio>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <string>
#include <unistd.h>

namespace {
void require(bool ok, const char* message) {
  if (!ok) { std::cerr << "production GML: " << message << '\n'; std::abort(); }
}

struct GmlFile {
  std::string path;
  explicit GmlFile(const std::string& data) {
    char name[] = "/tmp/ffgraph-gml-backend-XXXXXX";
    const int fd = mkstemp(name);
    require(fd >= 0, "mkstemp");
    close(fd);
    path = std::string(name) + ".gml";
    require(std::rename(name, path.c_str()) == 0, "rename");
    std::ofstream out(path, std::ios::binary);
    require(bool(out << data), "write fixture");
  }
  ~GmlFile() { std::remove(path.c_str()); }
};

void load(FFGraph& g, const GmlFile& file) {
  g->load(String(file.path.c_str()), YES);
}

void verifyValidFile() {
  for (int n = 0; n < 10; ++n) {
    FFGraph graph(YES);
    FFNode existing = graph->addNode();
    existing->setName("existing");
    existing->setValue(99);
    GmlFile file(R"(graph [
      label "production"
      node [ id 17 label "alpha" ]
      node [ id 29 label "beta" ]
      edge [ source 17 target 29 label "first" ]
      edge [ source 17 target 29 label "parallel" ]
      edge [ source 29 target 29 label "loop" ]
    ])");
    load(graph, file);
    require(graph->getNumNodes() == 3, "valid node count");
    require(graph->getNumEdges() == 3, "valid edge count");
    require(graph->allNodes().contains(existing), "preserves existing node identity");
    require(existing->getValue() == -1, "resets old IDs on successful import");
    require(graph->nodeWithValue(17) && graph->nodeWithValue(29), "imported IDs");
    int loops = 0, parallels = 0;
    for (Pix p = graph->allEdges().first(); p; graph->allEdges().next(p)) {
      const FFEdge edge = graph->allEdges()(p);
      require(graph->allNodes().contains(edge->source()) &&
              graph->allNodes().contains(edge->target()), "endpoints owned by graph");
      if (edge->source() == edge->target()) ++loops;
      if (edge->source()->getValue() == 17 && edge->target()->getValue() == 29)
        ++parallels;
    }
    require(loops == 1 && parallels == 2, "loop and parallel-edge semantics");
  }
}

void verifyRejectedFile(const std::string& source) {
  FFGraph graph(YES);
  FFNode original = graph->addNode();
  original->setName("do-not-modify");
  original->setValue(12345);
  GmlFile file(source);
  for (int n = 0; n < 12; ++n) {
    load(graph, file);
    require(graph->getNumNodes() == 1 && graph->getNumEdges() == 0,
            "failed import must not add nodes or edges");
    require(graph->allNodes().contains(original), "failed import preserves identity");
    require(original->getValue() == 12345, "failed import preserves existing node ID");
    require(original->getName() == "do-not-modify", "failed import preserves label");
  }
}
} // namespace

int main() {
  verifyValidFile();
  verifyRejectedFile("graph [ node [ id 1 ] edge [ source 1 target 999 ] ]");
  verifyRejectedFile("graph [ node [ id 1 ]");
  std::cout << "production GML backend integration passed\n";
}
