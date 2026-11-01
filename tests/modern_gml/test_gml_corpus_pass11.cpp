// Grammar-oriented generated corpus check using the *real* Bison/Flex parser.
// No Tcl/Tk, graph or X11 dependencies. cases.tsv is emitted by generate.py.
#include "gml_events.h"

#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

namespace {
void require(bool okay, const std::string& why) {
  if (!okay) { std::cerr << "generated GML corpus: " << why << '\n'; std::abort(); }
}
std::vector<std::string> split(const std::string& line) {
  std::vector<std::string> fields;
  std::istringstream stream(line);
  std::string field;
  while (std::getline(stream, field, '\t')) fields.push_back(field);
  return fields;
}
std::string contents(const std::filesystem::path& path) {
  std::ifstream stream(path, std::ios::binary);
  require(static_cast<bool>(stream), "cannot open " + path.string());
  return {std::istreambuf_iterator<char>(stream), std::istreambuf_iterator<char>()};
}
}

int main(int argc, char** argv) {
  require(argc == 2, "expected one corpus directory argument");
  const std::filesystem::path root(argv[1]);
  std::ifstream cases(root / "cases.tsv");
  require(static_cast<bool>(cases), "missing cases.tsv");
  std::string line;
  unsigned count = 0;
  unsigned rejected = 0;
  while (std::getline(cases, line)) {
    if (line.empty() || line.front() == '#') continue;
    const auto fields = split(line);
    require(fields.size() == 5, "bad TSV row: " + line);
    const std::string& category = fields[0];
    const std::string& label = fields[1];
    const auto source = contents(root / fields[2]);
    // New scanner for each input. Repeated valid/error runs also exercise
    // key ownership, buffer destruction and parser cleanup under ASan.
    for (int repeat = 0; repeat != 3; ++repeat) {
      const auto parsed = ffgraph_gml::parse(source);
      const bool valid = category != "invalid";
      require(parsed.ok == valid, label + ": unexpected parse result: " + parsed.events.error);
      if (category == "graph") {
        unsigned nodes = 0, edges = 0;
        for (const auto& event : parsed.events.values) {
          if (event.kind == ffgraph_gml::Event::Kind::Begin) {
            nodes += (event.key == "node");
            edges += (event.key == "edge");
          }
        }
        require(nodes == static_cast<unsigned>(std::stoul(fields[3])), label + ": node event count");
        require(edges == static_cast<unsigned>(std::stoul(fields[4])), label + ": edge event count");
      }
    }
    rejected += category == "invalid";
    ++count;
  }
  require(count > 20 && rejected >= 5, "corpus incomplete");
  std::cout << "real GML parser checked " << count << " corpus cases ("
            << rejected << " rejected as expected), three scanner lifetimes each\n";
}
