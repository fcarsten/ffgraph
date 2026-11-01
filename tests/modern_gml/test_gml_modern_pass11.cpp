#include "gml_events.h"

#include <cstdlib>
#include <iostream>
#include <string>

using ffgraph_gml::Event;
using ffgraph_gml::parse;

namespace {
void require(bool condition, const char* message) {
  if (!condition) {
    std::cerr << "modern GML: " << message << '\n';
    std::abort();
  }
}
}
int main() {
  const std::string representative =
    "# generated GML sample\n"
    "graph [\n"
    "  directed 1\n"
    "  node [ id 1 label \"alpha\" graphics [ x -12.5 y 42 ] ]\n"
    "  node { id 2 label \"beta\" }\n"
    "  edge [ source 1 target 2 label \"with \\\"escaped\\\" quotes\" ]\n"
    "  edge [ source 2 target 2 ]\n"
    "]\n";
  for (int i = 0; i < 100; ++i) {
    auto result = parse(representative);
    require(result.ok, "representative graph parse");
    require(result.events.error.empty(), "no diagnostics for valid file");
    const auto& events = result.events.values;
    require(events.size() == 24, "expected attributes/events");
    require(events[0].kind == Event::Kind::Begin && events[0].key == "graph", "root list");
    require(events[1].kind == Event::Kind::Integer && events[1].key == "directed", "integer event");
    require(events[1].integer == 1, "integer value");
    require(events[6].kind == Event::Kind::Real && events[6].key == "x", "real event");
    require(events[6].real == -12.5, "real value");
    require(events[7].kind == Event::Kind::Integer && events[7].key == "y", "graphics integer");
    require(events.back().kind == Event::Kind::End, "root close");
    require(events[17].kind == Event::Kind::String &&
            events[17].text == "with \\\"escaped\\\" quotes",
            "quoted content stays escaped");
  }
  require(parse("").ok, "empty attribute list");  // legacy grammar accepts an empty attribute list
  require(parse("graph []").ok, "empty nested list");
  require(parse("graph [ id @ 7 ]").ok, "legacy unknown-byte skip");
  // Every invocation creates and destroys a scanner. Exercise successful,
  // empty, and rejected inputs repeatedly, including the error cleanup path.
  // With ASan enabled, this catches duplicate frees and scanner-buffer leaks.
  for (int i = 0; i < 100; ++i) {
    require(parse("").ok, "repeated empty input");
    require(parse("graph [ id 7 ]").ok, "repeated valid input");
    // Each of these aborts after the list-opening midrule action has run.
    // The list KEY must remain owned by Bison until its whole rule reduces:
    // otherwise %destructor double-frees it while unwinding syntax errors.
    require(!parse("graph [ node [ id 1").ok, "truncated nested list");
    require(!parse("graph [ node [ id 1 ]").ok, "truncated outer list");
    require(!parse("graph [ node [ id ] ]").ok, "missing nested value");
    require(!parse("graph [ node [ label \"x\" ] stray").ok,
            "trailing nested key without value");
    require(!parse("root [ first [ second [ third [ n 1 ] ] ]").ok,
            "truncated list at several depths");
    require(!parse("label").ok, "missing value");
    require(!parse("graph ]").ok, "unmatched close");
    // An error must not poison the next scanner/parser instance.
    require(parse("graph [ id 7 ]").ok, "fresh parse after failure");
  }
}
