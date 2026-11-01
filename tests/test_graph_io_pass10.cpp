// End-to-end parser/serializer regression exercising the real sgraph and GML
// paths with stable topology assertions. No display or GUI initialisation.
#include <ffgraph/FFGraph.h>
#include <ffgraph/FFGraphClass.h>
#include <ffgraph/FFNodeClass.h>
#include <ffgraph/FFEdgeClass.h>
#include <ffgraph/FFGraphicNode.h>
#include <ffgraph/FFGraphicEdge.h>
#include <String.h>
#include <ffgraph/ffError.h>

#include <cstdlib>
#include <cstdio>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <set>
#include <sstream>
#include <string>
#include <utility>
#include <unistd.h>

extern charErrorHandler* userCharErrorHandler;

namespace {
void require(bool value, const char* message) {
  if (!value) { std::cerr << "file roundtrip: " << message << '\n'; std::abort(); }
}
using Topology = std::multiset<std::pair<std::string, std::string>>;
Topology topology(FFGraphClass* graph) {
  Topology result;
  for (Pix p = graph->allEdges().first(); p; graph->allEdges().next(p)) {
    const FFEdge edge = graph->allEdges()(p);
    result.emplace(edge->source()->getName().c_str(), edge->target()->getName().c_str());
  }
  return result;
}
struct TemporaryFile {
  std::filesystem::path path;
  explicit TemporaryFile(const char* extension) {
    // mkstemp handles concurrent CTest jobs without predictable filenames.
    std::string pattern = (std::filesystem::temp_directory_path() / "ffgraph-pass10-XXXXXX").string();
    const int fd = mkstemp(pattern.data());
    require(fd >= 0, "mkstemp");
    close(fd);
    path = pattern + extension;
    require(std::rename(pattern.c_str(), path.c_str()) == 0, "rename temp file");
  }
  ~TemporaryFile() { std::error_code ec; std::filesystem::remove(path, ec); }
};
void checkCounts(FFGraphClass* graph, int nodes, int edges) {
  require(graph->getNumNodes() == nodes, "node count");
  require(graph->getNumEdges() == edges, "edge count");
  for (Pix p = graph->allEdges().first(); p; graph->allEdges().next(p)) {
    FFEdge edge = graph->allEdges()(p);
    require(graph->allNodes().contains(edge->source()) &&
            graph->allNodes().contains(edge->target()), "valid edge endpoints");
  }
}
int parseErrors = 0;
void countParseError(errGrade grade, const char*, const char*, int) {
  require(grade == USER_ERROR, "expected FF syntax diagnostic");
  ++parseErrors;
}
void malformedInputs() {
  // Preserve malformed-input ownership coverage through both production APIs.
  // Failed parses must also preserve the destination graph and original IDs.
  struct HandlerGuard {
    charErrorHandler* previous = userCharErrorHandler;
    HandlerGuard() { userCharErrorHandler = countParseError; }
    ~HandlerGuard() { userCharErrorHandler = previous; }
  } handler;
  const char* invalid[] = {
    "GRAPH \"bad\" = 1 \"partially created\";",
    "GRAPH \"g\" = {$ FF_L \"unfinished type\"",
    "GRAPH \"g\" = 1 {$ FF_C \"red\"",
    "GRAPH \"g\" = 1 {$ FF_NI \"icon\" \"selected\" FF_NM \"mask\"",
    "GRAPH \"g\" = 1 \"n\" 2 {$ FF_L \"unfinished type\"",
    "GRAPH \"g\" = END \"unexpected lookahead\"",
    "GRAPH \"g\" = 1 \"unterminated",
    "GRAPH \"g\" = {$ CUSTOM 1 \"ignored\" 2"
  };
  for (int repeat = 0; repeat < 20; ++repeat) {
    for (const char* input : invalid) {
      for (bool file : {false, true}) {
        FFGraph graph(YES);
        graph->setName("keep");
        FFNode original = graph->addNode(); original->setValue(123);
        const int before = parseErrors;
        if (file) {
          TemporaryFile source(".g");
          std::ofstream out(source.path); out << input; out.close();
          require(out.good(), "write malformed FF fixture");
          graph->load(String(source.path.string()), YES);
        } else {
          graph->load(String(input), NO);
        }
        require(parseErrors == before + 1, "malformed input reports an error");
        require(graph->getNumNodes() == 1 && graph->getNumEdges() == 0 &&
                graph->nodeWithValue(123) == original && graph->getName() == "keep",
                "malformed input preserves original graph");
      }
    }
    FFGraph recovered(YES);
    const int before = parseErrors;
    recovered->load("GRAPH \"recovered\" = 1 \"n\" 1 \"loop\"; END", NO);
    require(parseErrors == before, "valid parse after failures");
    checkCounts(recovered.operator->(), 1, 1);
  }
}
void sgraphRoundTrip() {
  FFGraph original(YES);
  original->setName("Pass10 graph");
  original->setNodeIndex(original->addNodeLabel(new FFGraphicNode));
  original->setEdgeIndex(original->addEdgeLabel(new FFGraphicEdge));
  FFNode a = original->addNode(FFPoint(0, 0));
  FFNode b = original->addNode(FFPoint(10, 20));
  FFNode c = original->addNode(FFPoint(20, 10));
  a->setName("alpha"); b->setName("beta"); c->setName("isolated");
  auto e1 = original->addEdge(a, b);
  auto e2 = original->addEdge(a, b);
  auto loop = original->addEdge(b, b);
  e1->setName("first"); e2->setName("parallel"); loop->setName("loop");
  require(original->setStartNode(a), "start node");
  require(original->addEndNode(b), "end node");
  const auto expected = topology(original.operator->());
  for (int n = 0; n < 12; ++n) {
    String serialized;
    original->saveToString(serialized);
    require(serialized.contains("GRAPH"), "sgraph header");
    FFGraph loaded(YES);
    loaded->setNodeIndex(loaded->addNodeLabel(new FFGraphicNode));
    loaded->setEdgeIndex(loaded->addEdgeLabel(new FFGraphicEdge));
    loaded->load(serialized, NO);
    checkCounts(loaded.operator->(), 3, 3);
    require(topology(loaded.operator->()) == expected, "sgraph topology");
    require(loaded->getStartNode() != nullptr, "sgraph start node");
    require(loaded->getEndNodes().length() == 1, "sgraph end node");
    loaded->removeEdge(loaded->allEdges()(loaded->allEdges().first()));
    checkCounts(loaded.operator->(), 3, 2);
    loaded->clear();
    checkCounts(loaded.operator->(), 0, 0);
    // File path invokes a different parser entry point.
    TemporaryFile file(".g");
    std::ofstream out(file.path);
    out << serialized.c_str();
    out.close();
    loaded->load(String(file.path.string()), YES);
    checkCounts(loaded.operator->(), 3, 3);
    require(topology(loaded.operator->()) == expected, "file load topology");
  }
}
void gmlRoundTrip() {
  for (int iteration = 0; iteration < 8; ++iteration) {
  FFGraph original(YES);
  FFNode a = original->addNode(), b = original->addNode();
  a->setName("first"); b->setName("second");
  original->addEdge(a, b)->setName("directed");
  TemporaryFile file(".gml");
  original->saveToFile(String(file.path.string()));
  FFGraph loaded(YES);
  loaded->load(String(file.path.string()), YES);
  checkCounts(loaded.operator->(), 2, 1);
  require(topology(original.operator->()) == topology(loaded.operator->()), "GML topology");
  }
}
}
#ifdef FFGRAPH_TEST_SOURCE_DIR
void sampleGraph() {
  FFGraph graph(YES);
  const std::string sample = std::string(FFGRAPH_TEST_SOURCE_DIR) + "/contrib/out.g";
  graph->load(String(sample), YES);
  checkCounts(graph.operator->(), 499, 499);
  for (Pix p = graph->allEdges().first(); p; graph->allEdges().next(p)) {
    FFEdge edge = graph->allEdges()(p);
    require(graph->allNodes().contains(edge->source()) &&
            graph->allNodes().contains(edge->target()), "bundled graph incidence");
  }
  String serialized;
  graph->saveToString(serialized);
  FFGraph again(YES);
  again->load(serialized, NO);
  require(again->getNumNodes() == graph->getNumNodes(), "bundled graph node roundtrip");
  require(again->getNumEdges() == graph->getNumEdges(), "bundled graph edge roundtrip");
}
#endif
int main() {
  malformedInputs();
  sgraphRoundTrip();
  gmlRoundTrip();
#ifdef FFGRAPH_TEST_SOURCE_DIR
  sampleGraph();
#endif
}
