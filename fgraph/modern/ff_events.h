#ifndef FFGRAPH_MODERN_FF_EVENTS_H
#define FFGRAPH_MODERN_FF_EVENTS_H

#include <initializer_list>
#include <string>
#include <string_view>
#include <vector>

namespace ffgraph_ff {
// Ordered semantic operations, including the legacy right-recursive route
// insertions. No graph pointers or scanner-owned text escape the parse.
enum class Op {
  GraphName, Directed, Node, Target, NodeName, EdgeName,
  GraphLabel, LabelOffset, Scale, RotateX, RotateY, RotateZ,
  NodePosition, Start, End, NodeColor, NodeLabel, Icon, NewIcon, Mask, Radius,
  EdgePoint, EdgeColor, Weight, EdgeLabel
};
struct Event {
  Op op;
  std::vector<int> numbers;
  std::vector<std::string> strings;
};
struct Result {
  bool ok = false;
  std::string error;
  std::vector<Event> events;
};
// Internal per-invocation state shared only by the generated parser/scanner.
struct Context {
  Result result;
  std::string lexeme;
  int line = 1;
  void emit(Op op, std::initializer_list<int> n = {},
            std::initializer_list<std::string> s = {}) {
    result.events.push_back({op, n, s});
  }
};
Result parse(std::string_view input);
} // namespace ffgraph_ff
#endif
