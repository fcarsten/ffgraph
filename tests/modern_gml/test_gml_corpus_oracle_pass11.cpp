// Post-retirement regression: production GML import and direct event replay
// must agree, and independent TSV oracles must agree with both. This is not
// legacy-vs-modern parity: the historical GTControl implementation is gone.
#include "gml_events.h"
#include "gml_graph_adapter.h"

#include "../graph_corpus/graph_snapshot.h"

#include <ffgraph/FFGraph.h>
#include <ffgraph/FFGraphClass.h>
#include <ffgraph/FFGraphicEdge.h>
#include <ffgraph/FFGraphicNode.h>

#include <globdefs.h>
#include <stream.h>

#include <cstdio>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>
#include <unistd.h>

namespace {
namespace fs = std::filesystem;

void check(bool condition, const std::string& message) {
  if (!condition) throw std::runtime_error(message);
}

std::vector<std::string> fields(const std::string& line) {
  std::vector<std::string> result;
  size_t begin = 0;
  while (begin <= line.size()) {
    const auto end = line.find('\t', begin);
    result.push_back(line.substr(begin, end == std::string::npos ? end : end - begin));
    if (end == std::string::npos) break;
    begin = end + 1;
  }
  return result;
}

std::string readFile(const fs::path& path) {
  std::ifstream in(path, std::ios::binary);
  check(static_cast<bool>(in), "cannot open " + path.string());
  return {std::istreambuf_iterator<char>(in), std::istreambuf_iterator<char>()};
}

void initialise(FFGraph& graph) {
  graph->setNodeIndex(graph->addNodeLabel(new FFGraphicNode));
  graph->setEdgeIndex(graph->addEdgeLabel(new FFGraphicEdge));
}

ffgraph_test::GraphSnapshot compareGraph(const fs::path& path, int repeats,
                                         const ffgraph_test::GraphSnapshot* oracle = nullptr) {
  const std::string bytes = readFile(path);
  ffgraph_test::GraphSnapshot first;
  for (int iteration = 0; iteration < repeats; ++iteration) {
    FFGraph production(YES), replay(YES);
    initialise(production);
    initialise(replay);
    // The production load path must exercise FFGraphClass::load(), not just
    // the event adapter. The direct replay is an integration cross-check,
    // while the generator's TSV oracle is an independent source of truth.
    production->load(String(path.c_str()), YES);
    const auto parsed = ffgraph_gml::parse(bytes);
    check(parsed.ok, "modern syntax rejection: " + parsed.events.error);
    const auto applied = ffgraph_gml::applyToGraph(parsed, *replay.operator->());
    check(applied.ok, "modern graph adapter rejected input: " + applied.error);
    const auto actual = ffgraph_test::capture(*production.operator->());
    const auto candidate = ffgraph_test::capture(*replay.operator->());
    check(ffgraph_test::difference(actual, candidate).empty(),
          "production/direct replay mismatch: " + ffgraph_test::difference(actual, candidate));
    if (oracle) {
      check(ffgraph_test::difference(*oracle, actual).empty(),
            "production/oracle mismatch: " + ffgraph_test::difference(*oracle, actual));
      check(ffgraph_test::difference(*oracle, candidate).empty(),
            "replay/oracle mismatch: " + ffgraph_test::difference(*oracle, candidate));
    }
    if (iteration == 0) first = actual;
    else check(ffgraph_test::difference(first, actual).empty(),
               "nondeterministic production import across repeated loads");
  }
  return first;
}

// Also verify that *rejecting* syntax does not modify an existing graph.
void checkSyntax(const fs::path& path, bool expected_good, int repeats) {
  const auto bytes = readFile(path);
  FFGraph target(YES);
  initialise(target);
  target->addNode()->setValue(42);
  const auto baseline = ffgraph_test::capture(*target.operator->());
  for (int i = 0; i < repeats; ++i) {
    auto parsed = ffgraph_gml::parse(bytes);
    check(parsed.ok == expected_good, expected_good ? "unexpected syntax rejection" :
                                                 "unexpected syntax acceptance");
    if (!expected_good) {
      const auto applied = ffgraph_gml::applyToGraph(parsed, *target.operator->());
      check(!applied.ok, "adapter accepted failed syntax parse");
      check(ffgraph_test::difference(baseline, ffgraph_test::capture(*target.operator->())).empty(),
            "syntax failure modified target graph");
    }
  }
}

size_t graphCorpus(const fs::path& root, int repeats) {
  std::ifstream listing(root / "cases.tsv");
  check(static_cast<bool>(listing), "missing corpus cases.tsv");
  size_t graph_count = 0, syntax_count = 0, negative_count = 0;
  std::string line;
  while (std::getline(listing, line)) {
    if (line.empty() || line.front() == '#') continue;
    const auto row = fields(line);
    check(row.size() == 5, "invalid graph corpus row");
    const fs::path path = root / row[2];
    try {
      if (row[0] == "graph") {
        std::ifstream oracleFile(root / "oracle" / (row[1] + ".tsv"));
        check(static_cast<bool>(oracleFile), "missing independent oracle");
        const auto oracle = ffgraph_test::readOracle(oracleFile);
        const auto graph = compareGraph(path, repeats, &oracle);
        check(graph.nodes.size() == std::stoul(row[3]) &&
              graph.edges.size() == std::stoul(row[4]), "incorrect advertised graph sizes");
        ++graph_count;
      } else if (row[0] == "syntax") {
        checkSyntax(path, true, repeats);
        ++syntax_count;
      } else if (row[0] == "invalid") {
        checkSyntax(path, false, repeats);
        ++negative_count;
      } else throw std::runtime_error("unknown category " + row[0]);
    } catch (const std::exception& error) {
      throw std::runtime_error("fixture '" + row[1] + "' (" + path.string() + "): " + error.what());
    }
  }
  check(graph_count >= 18, "too few graph fixtures");
  check(syntax_count >= 7 && negative_count >= 9, "missing syntax or negative fixtures");
  std::cout << "graph corpus: " << graph_count << " graph parity/oracle cases; "
            << syntax_count << " grammar-only; " << negative_count << " expected-rejection; "
            << repeats << " repeats\n";
  return graph_count;
}

size_t metamorphicCorpus(const fs::path& root, int repeats) {
  std::ifstream listing(root / "cases.tsv");
  check(static_cast<bool>(listing), "missing metamorphic cases.tsv");
  size_t pairs = 0;
  std::string line;
  while (std::getline(listing, line)) {
    if (line.empty() || line.front() == '#') continue;
    const auto row = fields(line);
    check(row.size() == 6, "invalid metamorphic corpus row");
    try {
      const auto base = compareGraph(root / row[2], repeats);
      const auto variant = compareGraph(root / row[3], repeats);
      check(base.nodes.size() == std::stoul(row[4]) &&
            base.edges.size() == std::stoul(row[5]) &&
            variant.nodes.size() == base.nodes.size() &&
            variant.edges.size() == base.edges.size(), "metamorphic graph counts changed");
      ++pairs;
    } catch (const std::exception& error) {
      throw std::runtime_error("metamorphic '" + row[0] + "/" + row[1] + "': " + error.what());
    }
  }
  check(pairs >= 55, "too few metamorphic pairs");
  std::cout << "metamorphic corpus: " << pairs << " pairs, production/direct replay, "
            << repeats << " repeats\n";
  return pairs;
}
} // namespace

int main(int argc, char** argv) {
  try {
    check(argc == 4, "usage: test_gml_differential_corpus_pass11 graphs|metamorphic DIR REPEATS");
    const std::string mode = argv[1];
    const int repeats = std::stoi(argv[3]);
    check(repeats >= 1 && repeats <= 20, "repeats must be in [1,20]");
    if (mode == "graphs") graphCorpus(argv[2], repeats);
    else if (mode == "metamorphic") metamorphicCorpus(argv[2], repeats);
    else throw std::runtime_error("unknown corpus mode: " + mode);
    return 0;
  } catch (const std::exception& error) {
    std::cerr << "Pass11 GML oracle regression: " << error.what() << '\n';
    return 1;
  }
}
