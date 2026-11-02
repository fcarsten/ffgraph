#include "rome_records.h"
#include "modern_rome_parser.hpp"
#include <limits>
#include <utility>

struct yy_buffer_state;
int ffrome_lex_init(void** scanner);
int ffrome_lex_destroy(void* scanner);
yy_buffer_state* ffrome__scan_bytes(const char*, int, void*);

namespace ffgraph_rome {
Result parse(std::string_view input) {
  Context ctx;
  if (input.find('\0') != std::string_view::npos) {
    ctx.result.error = "NUL byte in Rome input";
    return std::move(ctx.result);
  }
  // One optional final newline, plus Flex's two sentinel bytes.
  if (input.size() > static_cast<std::size_t>(std::numeric_limits<int>::max() - 3)) {
    ctx.result.error = "Rome input too long for Flex";
    return std::move(ctx.result);
  }
  // The legacy stream feeder supplies LF even on the last unterminated line.
  std::string text(input);
  if (!text.empty() && text.back() != '\n') text += '\n';
  void* scanner = nullptr;
  if (ffrome_lex_init(&scanner)) {
    ctx.result.error = "Cannot initialise Rome scanner";
    return std::move(ctx.result);
  }
  struct Guard {
    void* scanner;
    ~Guard() { ffrome_lex_destroy(scanner); } // Owns scan-bytes buffer too.
  } guard{scanner};
  ffrome__scan_bytes(text.c_str(), static_cast<int>(text.size()), scanner);
  Parser parser(scanner, ctx);
  ctx.result.ok = parser.parse() == 0 && ctx.result.error.empty();
  if (!ctx.result.ok) ctx.result.records.clear();
  return std::move(ctx.result);
}
} // namespace ffgraph_rome
