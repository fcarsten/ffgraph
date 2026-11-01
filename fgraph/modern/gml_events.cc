#include "gml_events.h"

#include <limits>
#include <stdexcept>
#include <utility>

#include "modern_gml_parser.hpp"

// These declarations are provided by the reentrant flex scanner.  Avoid a
// public flex-generated header so the library API doesn't expose YY macros.
struct yy_buffer_state;
using YY_BUFFER_STATE = yy_buffer_state*;
int ffgml_lex_init(yyscan_t* scanner);
int ffgml_lex_destroy(yyscan_t scanner);
YY_BUFFER_STATE ffgml__scan_bytes(const char* bytes, int length, yyscan_t scanner);

namespace ffgraph_gml {

void Events::begin(const char* key) {
  values.push_back({Event::Kind::Begin, key, {}});
}
void Events::end() {
  values.push_back({Event::Kind::End, {}, {}});
}
void Events::integer(const char* key, int value) {
  Event e{Event::Kind::Integer, key, {}};
  e.integer = value;
  values.push_back(std::move(e));
}
void Events::real(const char* key, double value) {
  Event e{Event::Kind::Real, key, {}};
  e.real = value;
  values.push_back(std::move(e));
}
void Events::string(const char* key, const char* value) {
  Event e{Event::Kind::String, key, {}};
  e.text = value;
  values.push_back(std::move(e));
}

Result parse(std::string_view input) {
  Result result;
  if (input.size() > static_cast<size_t>(std::numeric_limits<int>::max())) {
    result.events.error = "GML input too long for flex scan-bytes API";
    return result;
  }
  yyscan_t scanner = nullptr;
  if (ffgml_lex_init(&scanner) != 0) {
    result.events.error = "Cannot initialise GML scanner";
    return result;
  }
  struct ScannerGuard {
    yyscan_t scanner;
    ~ScannerGuard() {
      // Flex destroys the current scan-bytes buffer when the scanner is
      // destroyed. Deleting it explicitly first would double-free it.
      ffgml_lex_destroy(scanner);
    }
  } guard{scanner};
  if (!ffgml__scan_bytes(input.empty() ? "" : input.data(),
                         static_cast<int>(input.size()), scanner)) {
    result.events.error = "Cannot allocate GML input buffer";
    return result;
  }
  result.ok = (ffgml_parse(scanner, &result.events) == 0);
  if (!result.ok && result.events.error.empty())
    result.events.error = "GML syntax error";
  return result;
}

}  // namespace ffgraph_gml
