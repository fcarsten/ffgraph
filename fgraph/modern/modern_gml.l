/* GNU Flex 2.6+ reentrant scanner for the optional GML parser pilot.
 * Preserve GTScanner's lexical vocabulary: both {} and [] denote lists,
 * quoted string contents are not unescaped, and unknown bytes are ignored.
 */
%option reentrant bison-bridge noyywrap noinput nounput nodefault 8bit
%option prefix="ffgml_"
%{
#include "modern_gml_parser.hpp"
#define YYSTYPE FFGML_STYPE
#include <cstdlib>
#include <cstring>
#include <new>

// The grammar owns KEY/STRING tokens; Bison destructors cover error paths.
static char* copy_token(const char* text, size_t length) {
  char* result = static_cast<char*>(std::malloc(length + 1));
  if (!result) throw std::bad_alloc();
  std::memcpy(result, text, length);
  result[length] = '\0';
  return result;
}
%}
digit [0-9]
number -?{digit}+
real -?{digit}+\.{digit}+
%%
{real} {
  yylval->real = std::strtod(yytext, nullptr);
  return REAL;
}
{number} {
  // The old scanner used atoi; preserve the ordinary int-domain values.
  yylval->integer = std::atoi(yytext);
  return INTEGER;
}
\"([^\"\\]|\\.|\\\n)*\" {
  yylval->text = copy_token(yytext + 1, static_cast<size_t>(yyleng - 2));
  return STRING;
}
"{"|"[" { return BEGIN_LIST; }
"}"|"]" { return END_LIST; }
[a-zA-Z_][a-zA-Z0-9_:]* {
  yylval->text = copy_token(yytext, static_cast<size_t>(yyleng));
  return KEY;
}
^\#[^\n]* { /* Legacy scanner only recognises comments at column 1. */ }
[ \t\r\n]+ { /* whitespace */ }
.|\n { /* Legacy scanner silently discards unknown characters. */ }
%%
