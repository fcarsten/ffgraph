#ifndef FFGRAPH_MODERN_GML_EVENTS_H
#define FFGRAPH_MODERN_GML_EVENTS_H

#include <string>
#include <string_view>
#include <vector>

namespace ffgraph_gml {

// Lossless (within legacy lexer semantics) ordered attribute events.  This
// deliberately does not modify FFGraphClass; enabling the modern parser is a
// compatibility experiment, not a silent application parser replacement.
struct Event {
  enum class Kind { Begin, End, Integer, Real, String } kind;
  std::string key;
  std::string text;
  int integer = 0;
  double real = 0;
};

struct Events {
  std::vector<Event> values;
  std::string error;

  void begin(const char* key);
  void end();
  void integer(const char* key, int value);
  void real(const char* key, double value);
  void string(const char* key, const char* value);
};

// Returns a syntax-error result rather than exiting the process.  The caller
// may discard partial events on failure. Input length is checked before the
// flex scan-bytes API (which takes an int) is invoked.
struct Result {
  bool ok = false;
  Events events;
};
Result parse(std::string_view input);

}  // namespace ffgraph_gml
#endif
