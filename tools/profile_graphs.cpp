// Opt-in profiling driver. Production loaders/algorithms remain unchanged.
#include <ffgraph/FFGraph.h>
#include <ffgraph/FFNodeClass.h>
#include <ffgraph/FFEdgeClass.h>
#include <ffgraph/FFGraphicNode.h>
#include <ffgraph/FFGraphicEdge.h>
#include <ffgraph/FFRandom.h>
#include <ffgraph/FFSpring.h>
#include <ffgraph/FFSugi.h>
#include <ffgraph/ffError.h>
#include "ff_graph_adapter.h"
#include "gml_graph_adapter.h"
#include "rome_graph_adapter.h"
#include "../tests/sugiyama_layout_snapshot.h"

#include <algorithm>
#include <chrono>
#include <cmath>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <map>
#include <optional>
#include <set>
#include <sstream>
#include <stdexcept>
#include <type_traits>
#include <variant>
#include <vector>

// Optional client requests restrict Callgrind to the same region as the clock.
#if __has_include(<valgrind/callgrind.h>)
#include <valgrind/callgrind.h>
#else
#define CALLGRIND_START_INSTRUMENTATION ((void)0)
#define CALLGRIND_STOP_INSTRUMENTATION ((void)0)
#endif

extern charErrorHandler* userCharErrorHandler;
namespace {
void require(bool okay, const std::string& message) {
  if (!okay) throw std::runtime_error(message);
}
void importError(errGrade, const char* message, const char*, int) {
  throw std::runtime_error(message);
}
struct HandlerGuard {
  charErrorHandler* previous = userCharErrorHandler;
  HandlerGuard() { userCharErrorHandler = importError; }
  ~HandlerGuard() { userCharErrorHandler = previous; }
};
// Bypass inspector creation exactly as the existing headless layout tests do.
template<class Layout> struct Headless : Layout {
  void init(const FFGraph& g) override { FFLayoutLabel::init(g); }
  FFGraphLabel* proto() const override { return new Headless; }
};
using Parsed = std::variant<ffgraph_ff::Result, ffgraph_gml::Result, ffgraph_rome::Result>;
Parsed parse(const std::string& format, const std::string& text) {
  if (format == ".g") return ffgraph_ff::parse(text);
  if (format == ".gml") return ffgraph_gml::parse(text);
  return ffgraph_rome::parse(text);
}
void replay(const Parsed& parsed, FFGraphClass& graph) {
  std::visit([&](const auto& result) {
    require(result.ok, "syntax rejection");
    auto applied = applyToGraph(result, graph, true);
    if constexpr (std::is_same_v<decltype(applied), bool>)
      require(applied, "FF replay failed");
    else {
      require(applied.ok, applied.error);
      if constexpr (std::is_same_v<decltype(applied), ffgraph_rome::ApplyResult>)
        require(applied.missing.empty(), "Rome input has missing endpoints");
    }
  }, parsed);
}
struct Topology {
  std::multiset<int> nodes;
  std::multiset<std::pair<int, int>> edges;
  bool directed = false;
  bool operator==(const Topology& rhs) const {
    return nodes == rhs.nodes && edges == rhs.edges && directed == rhs.directed;
  }
};
Topology topology(FFGraphClass& graph) {
  Topology result;
  result.directed = graph.getDirected();
  std::set<FFNode> owned;
  for (Pix p = graph.allNodes().first(); p; graph.allNodes().next(p)) {
    auto* n = graph.allNodes()(p);
    require(owned.insert(n).second, "duplicate node pointer");
    result.nodes.insert(n->getValue());
    require(n->getGraphicNode(), "missing graphic node");
    for (double v : n->getGraphicNode()->getCoords().v)
      require(std::isfinite(v), "nonfinite coordinates");
  }
  for (Pix p = graph.allEdges().first(); p; graph.allEdges().next(p)) {
    auto* e = graph.allEdges()(p);
    require(owned.count(e->source()) && owned.count(e->target()), "foreign endpoint");
    require(e->getGraphicEdge() && e->getGraphicEdge()->anzPoints() >= 2,
            "invalid edge route");
    for (int i = 0; i < e->getGraphicEdge()->anzPoints(); ++i)
      for (double v : e->getGraphicEdge()->getPointAt(i).v)
        require(std::isfinite(v), "nonfinite edge route");
    result.edges.emplace(e->source()->getValue(), e->target()->getValue());
  }
  return result;
}
// Numeric values may legitimately change during layout; object incidence may not.
using Incidence = std::map<FFEdge, std::pair<FFNode, FFNode>>;
Incidence incidence(FFGraphClass& graph) {
  Incidence result;
  for (Pix p = graph.allEdges().first(); p; graph.allEdges().next(p)) {
    auto* e = graph.allEdges()(p);
    result.emplace(e, std::make_pair(e->source(), e->target()));
  }
  return result;
}
Topology readOracle(const std::string& path) {
  std::ifstream file(path);
  require(bool(file), "cannot read oracle");
  Topology result;
  std::string kind;
  int a, b;
  while (file >> kind >> a) {
    if (kind == "node") result.nodes.insert(a);
    else if (kind == "edge" && file >> b) result.edges.emplace(a, b);
    else throw std::runtime_error("bad oracle record");
  }
  require(file.eof(), "bad oracle");
  return result;
}
long peakRssKiB() {
  // Unlike getrusage's process high-water mark, Linux's mm-specific VmHWM
  // excludes the Python runner's address space inherited before exec().
  std::ifstream status("/proc/self/status");
  std::string line;
  while (std::getline(status, line)) {
    if (line.rfind("VmHWM:", 0) != 0) continue;
    std::istringstream value(line.substr(6));
    long kib;
    std::string unit;
    require(bool(value >> kib >> unit) && unit == "kB", "invalid Linux VmHWM");
    return kib;
  }
  throw std::runtime_error("Linux /proc/self/status VmHWM unavailable");
}
}

int main(int argc, char** argv) {
  if (argc == 2 && std::string(argv[1]) == "--help") {
    std::cout << "profile_graphs --phase import|parse|replay|layout --input FILE\n"
                 "  [--repeat N] [--oracle TOPOLOGY.tsv]\n"
                 "  [--layout random|spring|sugi] [--quality 1..10]\n"
                 "  [--sugi-crossing-alg 0|1]\n"
                 "  [--dimension 2|3] [--snapshot FILE]\n"
                 "  [--layout-max-nodes N]\n"
                 "One warmup precedes N measured fresh-graph runs (default 3).\n"
                 "Use .g, .gml or .rome filenames; output is JSON Lines.\n";
    return 0;
  }
  try {
    std::map<std::string, std::string> options;
    const std::set<std::string> keys = {
      "--phase", "--input", "--repeat", "--layout", "--quality", "--oracle", "--layout-max-nodes",
      "--sugi-crossing-alg", "--dimension", "--snapshot"};
    for (int i = 1; i < argc; i += 2) {
      require(i + 1 < argc && keys.count(argv[i]), "expected --option value pairs");
      require(options.emplace(argv[i], argv[i + 1]).second, "duplicate option");
    }
    const auto phase = options.at("--phase");
    const int dimension = options.count("--dimension") ? std::stoi(options.at("--dimension")) : 2;
    require(dimension == 2 || dimension == 3, "invalid dimension");
    require(!options.count("--snapshot") || phase == "layout", "snapshots require layout phase");
    const auto path = std::filesystem::absolute(options.at("--input"));
    const auto format = path.extension().string();
    require(format == ".g" || format == ".gml" || format == ".rome", "unsupported extension");
    require(phase == "import" || phase == "parse" || phase == "replay" || phase == "layout",
            "unknown phase");
    const int repeats = options.count("--repeat") ? std::stoi(options.at("--repeat")) : 3;
    const int quality = options.count("--quality") ? std::stoi(options.at("--quality")) : 2;
    const auto algorithm = options.count("--layout") ? options.at("--layout") : "spring";
    const int crossingAlg = options.count("--sugi-crossing-alg")
                          ? std::stoi(options.at("--sugi-crossing-alg")) : 0;
    require(crossingAlg == 0 || crossingAlg == 1, "invalid Sugiyama crossing algorithm");
    const int layoutMax = options.count("--layout-max-nodes")
                        ? std::stoi(options.at("--layout-max-nodes")) : 1000;
    require(repeats > 0 && repeats <= 10000 && quality >= 1 && quality <= 10, "invalid limits");
    require(algorithm == "spring" || algorithm == "sugi" || algorithm == "random", "unknown layout");
    std::string text;
    if (phase == "parse" || phase == "replay") {
      std::ifstream input(path, std::ios::binary);
      require(bool(input), "cannot open input");
      std::ostringstream buffer;
      buffer << input.rdbuf();
      require(!input.bad(), "input read failed");
      text = buffer.str();
    }
    std::optional<Parsed> prepared;
    if (phase == "replay") prepared.emplace(parse(format, text));
    HandlerGuard handler;
    Topology expected;
    if (options.count("--oracle")) expected = readOracle(options.at("--oracle"));

    for (int sample = -1; sample < repeats; ++sample) { // One untimed warmup.
      FFGraph graph(YES);
      // Rome/FF below describe undirected graphs. GML intentionally retains
      // destination directedness, so establish the same state for all three.
      graph->_setDirected(NO);
      graph->setNodeIndex(graph->addNodeLabel(new FFGraphicNode));
      graph->setEdgeIndex(graph->addEdgeLabel(new FFGraphicEdge));
      FFLayoutLabel* layout = nullptr;
      Incidence before;
      std::set<FFNode> nodesBefore;
      std::map<FFNode, int> snapshotIds;
      int springCap = 0;
      if (phase == "layout") {
        graph->load(String(path.string()), YES);
        auto imported = topology(*graph.operator->());
        if (sample == -1 && !options.count("--oracle")) expected = imported;
        require(imported == expected, "layout import topology mismatch");
        require(!imported.nodes.empty(), "empty layout input");
        require(graph->getNumNodes() <= layoutMax, "layout node limit exceeded");
        before = incidence(*graph.operator->());
        for (Pix p = graph->allNodes().first(); p; graph->allNodes().next(p))
          nodesBefore.insert(graph->allNodes()(p));
        if (options.count("--snapshot")) snapshotIds = ffgraph_test::layoutIds(*graph.operator->());
        if (algorithm == "spring") {
          auto* spring = new Headless<FFSpring>;
          require(spring->setQuality(quality), "spring quality rejected");
          spring->setAnimation(NO);
          spring->setDimension(dimension);
          springCap = std::min(spring->getMaxIteration(),std::max(30, 5000 * quality / graph->getNumNodes()));
          layout = spring;
        } else if (algorithm == "sugi") {
          auto* sugi = new Headless<FFSugi>;
          require(sugi->setCrossingAlg(crossingAlg), "Sugiyama crossing algorithm rejected");
          sugi->setDimension(dimension);
          layout = sugi;
        }
        else layout = new FFRandom;
        graph->addGraphLabel(layout); // Graph owns this label.
      }
      srand48(12345);
      std::srand(12345);
      Parsed parsed;
      if (sample >= 0) CALLGRIND_START_INSTRUMENTATION;
      const auto start = std::chrono::steady_clock::now();
      if (phase == "import") graph->load(String(path.string()), YES);
      else if (phase == "parse") parsed = parse(format, text);
      else if (phase == "replay") replay(*prepared, *graph.operator->());
      else layout->layout();
      const auto finish = std::chrono::steady_clock::now();
      if (sample >= 0) CALLGRIND_STOP_INSTRUMENTATION;
      if (phase == "parse") replay(parsed, *graph.operator->());
      const auto actual = topology(*graph.operator->());
      if (phase == "layout") {
        std::set<FFNode> nodesAfter;
        for (Pix p = graph->allNodes().first(); p; graph->allNodes().next(p))
          nodesAfter.insert(graph->allNodes()(p));
        require(nodesBefore == nodesAfter && before == incidence(*graph.operator->()),
                "layout changed object incidence");
        require(actual.directed == expected.directed, "layout changed directedness");
      } else {
        if (sample == -1 && !options.count("--oracle")) expected = actual;
        require(actual == expected, "import topology mismatch");
      }
      if (sample >= 0) {
        if (sample == 0 && options.count("--snapshot")) {
          const auto snapshotPath = options.at("--snapshot");
          require(!std::filesystem::exists(snapshotPath), "snapshot already exists");
          std::ofstream out(snapshotPath);
          ffgraph_test::writeLayout(out, ffgraph_test::captureLayout(*graph.operator->(), snapshotIds));
          require(bool(out), "snapshot write failed");
        }
        const long peak = peakRssKiB();
        const auto ms = std::chrono::duration<double, std::milli>(finish - start).count();
        std::cout << std::setprecision(9) << "{\"sample\":" << sample
                  << ",\"wall_ms\":" << ms << ",\"peak_rss_kib\":" << peak
                  << ",\"nodes\":" << actual.nodes.size() << ",\"edges\":" << actual.edges.size()
                  << ",\"spring_iteration_cap\":" << springCap
                  << ",\"sugi_crossing_alg\":" << crossingAlg << "}\n";
      }
    }
  } catch (const std::exception& error) {
    std::cerr << "profile_graphs: " << error.what() << '\n';
    return 1;
  }
}
