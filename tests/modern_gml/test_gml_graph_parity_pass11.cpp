// Differential graph-output test: legacy GTControl vs the experimental
// Bison/Flex event stream plus its semantic adapter. Must run with the real
// generated parser; a stand-in event lexer is not a substitute for this test.
#include "gml_events.h"
#include "gml_graph_adapter.h"

#include <ffgraph/FFEdgeClass.h>
#include <ffgraph/FFGraph.h>
#include <ffgraph/FFGraphClass.h>
#include <ffgraph/FFGraphicEdge.h>
#include <ffgraph/FFGraphicNode.h>
#include <ffgraph/FFNodeClass.h>

#include <algorithm>
#include <cstdio>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <tuple>
#include <vector>
#include <unistd.h>

namespace {
void require(bool ok, const std::string& reason) {
  if (!ok) { std::cerr << "Pass11 GML graph parity: " << reason << '\n'; std::abort(); }
}
struct TempGml {
  std::filesystem::path path;
  explicit TempGml(const std::string& content) {
    std::string pattern = (std::filesystem::temp_directory_path() / "ffgraph-pass11-XXXXXX").string();
    const int fd = mkstemp(pattern.data());
    require(fd >= 0, "mkstemp");
    close(fd);
    path = pattern + ".gml";
    require(std::rename(pattern.c_str(), path.c_str()) == 0, "rename to .gml");
    std::ofstream output(path, std::ios::binary);
    require(static_cast<bool>(output << content), "write GML fixture");
  }
  ~TempGml() { std::error_code ec; std::filesystem::remove(path, ec); }
};
void initialise(FFGraph& graph) {
  graph->setNodeIndex(graph->addNodeLabel(new FFGraphicNode));
  graph->setEdgeIndex(graph->addEdgeLabel(new FFGraphicEdge));
}
using Node = std::tuple<int, std::string, double, double, double>;
using Edge = std::tuple<int, int, int, std::string>;
struct Snapshot {
  std::string name;
  std::vector<Node> nodes;
  std::vector<Edge> edges;
  bool operator==(const Snapshot& other) const {
    return name == other.name && nodes == other.nodes && edges == other.edges;
  }
};
Snapshot snapshot(FFGraphClass& graph) {
  Snapshot s;
  s.name = graph.getName().c_str();
  for (Pix p = graph.allNodes().first(); p; graph.allNodes().next(p)) {
    FFNodeClass* n = graph.allNodes()(p);
    FFGraphicNode* gn = n->getGraphicNode();
    require(gn != nullptr, "node has graphic label");
    s.nodes.emplace_back(n->getValue(), std::string(n->getName().c_str()),
                         gn->getx(), gn->gety(), gn->getz());
  }
  for (Pix p = graph.allEdges().first(); p; graph.allEdges().next(p)) {
    FFEdgeClass* e = graph.allEdges()(p);
    require(e->source() && e->target(), "edge endpoints exist");
    s.edges.emplace_back(e->source()->getValue(), e->target()->getValue(),
                         e->getValue(), std::string(e->getName().c_str()));
  }
  std::sort(s.nodes.begin(), s.nodes.end());
  std::sort(s.edges.begin(), s.edges.end());
  return s;
}
void printSnapshot(const Snapshot& snap) {
  std::cerr << "  graph='" << snap.name << "'\n";
  for (const auto& n : snap.nodes) {
    std::cerr << "  node id=" << std::get<0>(n)
              << " name='" << std::get<1>(n) << "' x=" << std::get<2>(n)
              << " y=" << std::get<3>(n) << " z=" << std::get<4>(n) << '\n';
  }
  for (const auto& e : snap.edges) {
    std::cerr << "  edge " << std::get<0>(e) << " -> " << std::get<1>(e)
              << " value=" << std::get<2>(e) << " name='" << std::get<3>(e) << "'\n";
  }
}
void compare(const char* label, const std::string& fixture) {
  TempGml file(fixture);
  for (int iteration = 0; iteration < 5; ++iteration) {
    FFGraph legacy(YES), modern(YES);
    initialise(legacy);
    initialise(modern);
    legacy->load(String(file.path.string()), YES);
    const auto parsed = ffgraph_gml::parse(fixture);
    require(parsed.ok, std::string(label) + ": modern parse: " + parsed.events.error);
    const auto applied = ffgraph_gml::applyToGraph(parsed, *modern.operator->());
    require(applied.ok, std::string(label) + ": adapter: " + applied.error);
    const Snapshot expected = snapshot(*legacy.operator->());
    const Snapshot actual = snapshot(*modern.operator->());
    if (!(expected == actual)) {
      std::cerr << label << ": graph mismatch: "
                << "legacy nodes=" << expected.nodes.size() << " edges=" << expected.edges.size()
                << " name='" << expected.name << "'"
                << "; modern nodes=" << actual.nodes.size() << " edges=" << actual.edges.size()
                << " name='" << actual.name << "'\n";
      std::cerr << "legacy:\n";
      printSnapshot(expected);
      std::cerr << "modern:\n";
      printSnapshot(actual);
      require(false, std::string(label) + ": graph semantic mismatch");
    }
  }
}
}
int main(int argc, char** argv) {
  compare("nodes, coordinates, parallel edges and loop",
    "# column-1 legacy comment\n"
    "graph [ label \"fixture\" directed 1 creator \"ffgraph\"\n"
    " node [ id 1 label \"alpha\" graphics [ x -12.5 y 42 z 3 ] ]\n"
    " node { id 2 label \"beta\" graphics { x 4 y -6.25 } }\n"
    " edge [ id 7 source 1 target 2 label \"first\" ]\n"
    " edge [ source 1 target 2 label \"parallel\" ]\n"
    " edge [ source 2 target 2 label \"self\" ]\n"
    "]\n");
  compare("mixed syntax, unknown attributes and escaped strings",
    "graph { label \"backslash \\\\ and \\\"quotes\\\"\"\n"
    " node [ id 100 label \"node_100\" graphics { x 0 y 2.5 } misc [ count 7 ] ]\n"
    " node [ id 20 label \"node_20\" ]\n"
    " edge { source 20 target 100 label \"going back\" }\n"
    " }\n");
  compare("minimal graph", "graph [ node [ id 1 ] ]\n");
  compare("empty graph", "graph []\n");
  compare("unknown bytes skipped by legacy scanner",
          "graph [ node [ id @ 3 label \"with spaces\" ] ]\n");
  // Parse failure must not mutate an existing graph even if the event stream
  // contains partially emitted nodes/edges before Bison reports the error.
  FFGraph target(YES);
  initialise(target);
  target->addNode()->setValue(42);
  const Snapshot before = snapshot(*target.operator->());
  for (const auto& invalid : {"graph [ node [ id 1", "graph [ node [ id ] ]",
                              "graph [ edge [ source 1 ] ]", "graph ]"}) {
    auto parsed = ffgraph_gml::parse(invalid);
    if (std::string(invalid) == "graph [ edge [ source 1 ] ]") {
      // Syntactically valid, but unsupported legacy consistency case.
      require(parsed.ok, "edge-first syntax is legal GML");
    } else {
      require(!parsed.ok, "malformed GML rejected");
    }
    auto applied = ffgraph_gml::applyToGraph(parsed, *target.operator->());
    require(!applied.ok, "invalid graph rejected by adapter");
    require(snapshot(*target.operator->()) == before, "rejected parse leaves graph unchanged");
  }
  // Optional generated corpus. The TSV is produced deterministically from
  // modern_gml.{y,l}'s supported attribute grammar; only graph-valid rows
  // enter the legacy importer (negative/syntax-only rows are parser tests).
  if (argc == 2) {
    const std::filesystem::path root(argv[1]);
    std::ifstream listing(root / "cases.tsv");
    require(static_cast<bool>(listing), "cannot open corpus cases.tsv");
    std::string line;
    unsigned compared = 0;
    while (std::getline(listing, line)) {
      if (line.empty() || line.front() == '#') continue;
      std::istringstream stream(line);
      std::string category, name, relative, nodes, edges;
      require(static_cast<bool>(std::getline(stream, category, '\t')) &&
              static_cast<bool>(std::getline(stream, name, '\t')) &&
              static_cast<bool>(std::getline(stream, relative, '\t')) &&
              static_cast<bool>(std::getline(stream, nodes, '\t')) &&
              static_cast<bool>(std::getline(stream, edges, '\t')),
              "invalid corpus row");
      if (category != "graph") continue;
      const auto path = root / relative;
      std::ifstream input(path, std::ios::binary);
      require(static_cast<bool>(input), "cannot open " + path.string());
      const std::string fixture((std::istreambuf_iterator<char>(input)),
                                std::istreambuf_iterator<char>());
      compare(name.c_str(), fixture);
      ++compared;
    }
    require(compared >= 15, "too few graph-valid corpus cases");
    std::cout << "compared " << compared << " generated graph fixtures\n";
  } else {
    require(argc == 1, "usage: test_gml_graph_parity_pass11 [corpus_directory]");
  }
}
