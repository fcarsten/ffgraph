#include "ff_events.h"
#include "modern_ff_parser.hpp"

#include <limits>
#include <utility>

struct yy_buffer_state;
int ffff_lex_init(void** scanner);
int ffff_lex_destroy(void* scanner);
yy_buffer_state* ffff__scan_bytes(const char*, int, void*);

namespace ffgraph_ff {
Result parse(std::string_view input) {
  Context ctx;
  if (input.find('\0') != std::string_view::npos) {
    ctx.result.error = "NUL byte in FF input";
    return std::move(ctx.result);
  }
  // Flex's scan-bytes interface takes int and allocates two sentinel bytes.
  if (input.size() > static_cast<size_t>(std::numeric_limits<int>::max() - 2)) {
    ctx.result.error = "FF input too long for Flex";
    return std::move(ctx.result);
  }
  void* scanner = nullptr;
  if (ffff_lex_init(&scanner)) {
    ctx.result.error = "Cannot initialise FF scanner";
    return std::move(ctx.result);
  }
  struct Guard {
    void* scanner;
    ~Guard() { ffff_lex_destroy(scanner); } // Also owns active scan-bytes buffer.
  } guard{scanner};
  ffff__scan_bytes(input.empty() ? "" : input.data(), static_cast<int>(input.size()), scanner);
  Parser parser(scanner, ctx);
  ctx.result.ok = parser.parse() == 0 && ctx.result.error.empty();
  // Discard partial operations. Failed syntax never reaches the graph adapter.
  if (!ctx.result.ok) ctx.result.events.clear();
  return std::move(ctx.result);
}
} // namespace ffgraph_ff
