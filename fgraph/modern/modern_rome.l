/* RomeScanner.l vocabulary (Universitaet Passau 1986-1991), reentrant Flex.
 * Keep longest-match behavior, column-one comments and ignored punctuation.
 */
%option reentrant noyywrap nounput noinput nodefault 8bit
%option prefix="ffrome_"
%{
#include "modern_rome_parser.hpp"
#include <charconv>
#include <system_error>
#define YY_DECL ffgraph_rome::Parser::symbol_type ffrome_lex(void* yyscanner, ffgraph_rome::Context& ctx)
using Parser = ffgraph_rome::Parser;
static Parser::location_type point(const ffgraph_rome::Context& ctx) {
  Parser::location_type loc;
  loc.begin.line = loc.end.line = ctx.line;
  loc.begin.column = loc.end.column = ctx.column;
  return loc;
}
#define YY_USER_ACTION \
  loc = point(ctx); \
  ctx.advance(yytext, yyleng); \
  loc.end.line = ctx.line; loc.end.column = ctx.column;
%}
%%
%{
  Parser::location_type loc;
%}
-?[0-9]+ {
  int value = 0;
  const auto converted = std::from_chars(yytext, yytext + yyleng, value);
  if (converted.ec != std::errc{}) {
    ctx.result.error = "Rome integer out of range in line " +
      std::to_string(loc.begin.line) + ", column " + std::to_string(loc.begin.column);
    return Parser::make_YYerror(loc);
  }
  return Parser::make_INTEGER(value, loc);
}
-?[0-9]+\.[0-9]+ { return Parser::make_REAL(loc); }
\"([^\"\\]|\\.|\\\n)*\" { return Parser::make_STRING(loc); }
[\{\[] { return Parser::make_BEGIN_LIST(loc); }
[\}\]] { return Parser::make_END_LIST(loc); }
"#" { return Parser::make_HASHMARK(loc); }
\n { return Parser::make_NEWLINE(loc); }
[a-zA-Z_][a-zA-Z0-9_:]* { return Parser::make_KEY(loc); }
[ \t]+ { }
^"#".* { }
. { /* Historical Rome silently ignores unrecognized punctuation/CR. */ }
<<EOF>> { return Parser::make_YYEOF(point(ctx)); }
%%
