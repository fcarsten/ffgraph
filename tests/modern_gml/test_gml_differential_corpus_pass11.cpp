// End-to-end differential test. Both importers run on identical bytes:
// GTControl -> FFGraphClass versus generated Bison/Flex -> events -> adapter.
// Designed for the graph corpus; does not change the application import path.
#include "gml_events.h"
#include "gml_graph_adapter.h"

#include "../graph_corpus/graph_snapshot.h"

#include <ffgraph/FFGraph.h>
#include <ffgraph/FFGraphClass.h>
#include <ffgraph/FFGraphicEdge.h>
#include <ffgraph/FFGraphicNode.h>

#include <globdefs.h>
#include <stream.h>
#include <container/AVLMap.h>
#include "GTParser.h"
#include "GTScanner.h" // Explicit legacy reference, independent of load() selection.

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

// Use an OS-created unique GML filename so parallel CTest runs do not collide.
// Never dispatch the legacy side through FFGraphClass::load(): that API now
// selects the modern backend by default.
struct TempGml {
  fs::path path;
  explicit TempGml(const std::string& bytes) {
    std::string pattern = (fs::temp_directory_path() / "ffgraph-differential-XXXXXX").string();
    int fd = mkstemp(pattern.data());
    check(fd >= 0, "mkstemp failed");
    close(fd);
    path = pattern + ".gml";
    if (std::rename(pattern.c_str(), path.c_str()) != 0) {
      std::remove(pattern.c_str());
      throw std::runtime_error("failed to rename temporary GML file");
    }
    std::ofstream out(path, std::ios::binary);
    out.write(bytes.data(), static_cast<std::streamsize>(bytes.size()));
    check(out.good(), "failed to write temporary GML file");
  }
  ~TempGml() { std::error_code ignore; fs::remove(path, ignore); }
};

void initialise(FFGraph& graph) {
  graph->setNodeIndex(graph->addNodeLabel(new FFGraphicNode));
  graph->setEdgeIndex(graph->addEdgeLabel(new FFGraphicEdge));
}

ffgraph_test::GraphSnapshot compareGraph(const fs::path& path, int repeats,
                                         const ffgraph_test::GraphSnapshot* oracle = nullptr) {
  const std::string bytes = readFile(path);
  TempGml temp(bytes);
  ffgraph_test::GraphSnapshot first;
  for (int iteration = 0; iteration < repeats; ++iteration) {
    FFGraph legacy(YES), modern(YES);
    initialise(legacy);
    initialise(modern);
    std::ifstream legacy_input(temp.path, std::ios::binary);
    check(legacy_input.good(), "cannot open temporary GML for legacy importer");
    GTControl legacy_parser;
    legacy_parser.Parse(legacy_input, legacy.operator->());
    const auto parsed = ffgraph_gml::parse(bytes);
    check(parsed.ok, "modern syntax rejection: " + parsed.events.error);
    const auto applied = ffgraph_gml::applyToGraph(parsed, *modern.operator->());
    check(applied.ok, "modern graph adapter rejected input: " + applied.error);
    const auto reference = ffgraph_test::capture(*legacy.operator->());
    const auto candidate = ffgraph_test::capture(*modern.operator->());
    check(ffgraph_test::difference(reference, candidate).empty(),
          "legacy/modern mismatch: " + ffgraph_test::difference(reference, candidate));
    if (oracle) {
      check(ffgraph_test::difference(*oracle, reference).empty(),
            "legacy/oracle mismatch: " + ffgraph_test::difference(*oracle, reference));
      check(ffgraph_test::difference(*oracle, candidate).empty(),
            "modern/oracle mismatch: " + ffgraph_test::difference(*oracle, candidate));
    }
    if (iteration == 0) first = reference;
    else check(ffgraph_test::difference(first, reference).empty(),
               "nondeterministic legacy import across repeated loads");
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
  std::cout << "metamorphic corpus: " << pairs << " pairs, both importers, "
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
    std::cerr << "Pass11 comprehensive GML differential: " << error.what() << '\n';
    return 1;
  }
}
