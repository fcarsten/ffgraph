#ifndef FFGRAPH_MODERN_ROME_RECORDS_H
#define FFGRAPH_MODERN_ROME_RECORDS_H

#include <cstddef>
#include <ostream>
#include <string>
#include <string_view>
#include <vector>

namespace ffgraph_rome {
struct Position { int line = 1, column = 1; };
struct Location { Position begin, end; };
inline std::ostream& operator<<(std::ostream& out, const Location& location) {
  return out << location.begin.line << ':' << location.begin.column << '-'
    << location.end.line << ':' << location.end.column;
}
enum class Kind { Node, Edge };
struct Record {
  Kind kind;
  int id;
  int source = 0, target = 0;
  std::size_t line = 1;
};
struct Result {
  bool ok = false;
  std::string error;
  std::vector<Record> records;
};
// Per-call scanner/parser state. No graph pointers or borrowed text escape.
struct Context {
  Result result;
  int line = 1, column = 1;
  void advance(const char* text, int length) {
    for (int i = 0; i < length; ++i) {
      if (text[i] == '\n') { ++line; column = 1; }
      else ++column;
    }
  }
};
Result parse(std::string_view input);
} // namespace ffgraph_rome
#endif
