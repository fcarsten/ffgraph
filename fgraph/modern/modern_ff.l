/* FFScanner.l vocabulary (Universitaet Passau 1986-1991), reentrant Flex. */
%option reentrant noyywrap nounput nodefault 8bit
%option prefix="ffff_"
%{
#include "modern_ff_parser.hpp"
#include <charconv>
#include <system_error>
#define YY_DECL ffgraph_ff::Parser::symbol_type ffff_lex(void* yyscanner, ffgraph_ff::Context& ctx)
using Parser = ffgraph_ff::Parser;
%}
%s STRING
%%
<INITIAL>-?[0-9]+ {
  int number = 0;
  const auto converted = std::from_chars(yytext, yytext + yyleng, number);
  if (converted.ec != std::errc{}) {
    ctx.result.error = "FF integer out of range in line " + std::to_string(ctx.line);
    return Parser::make_YYerror();
  }
  return Parser::make_NUMBER(number);
}
<INITIAL>\" { BEGIN(STRING); ctx.lexeme.clear(); }
<STRING>\" { BEGIN(INITIAL); return Parser::make_IDENTIFIER(std::move(ctx.lexeme)); }
<STRING>[^\"\\]+ {
  ctx.lexeme.append(yytext, yyleng);
  for (int i = 0; i < yyleng; ++i) if (yytext[i] == '\n') ++ctx.line;
}
<STRING>\\\n { ctx.lexeme += '\n'; ++ctx.line; }
<STRING>\\. { ctx.lexeme += yytext[1]; }
<STRING>\\ { ctx.result.error = "Unterminated FF escape in line " + std::to_string(ctx.line); return Parser::make_YYerror(); }
<STRING><<EOF>> { ctx.result.error = "Unterminated FF string in line " + std::to_string(ctx.line); return Parser::make_YYerror(); }
\{[^$][^}]* {
  /* Deliberately active in inclusive STRING mode too, as in FFScanner.l.
   * Match excludes the right brace; consume it separately to preserve Flex
   * longest-match decisions against the string-text rule. */
  for (int i = 0; i < yyleng; ++i) if (yytext[i] == '\n') ++ctx.line;
  yyinput(yyscanner);
}
"{$" { return Parser::make_BEGIN_GRAPHED_INTERNALS(); }
"$}" { return Parser::make_END_GRAPHED_INTERNALS(); }
GRAPH { return Parser::make_GRAPH(); }
END { return Parser::make_END(); }
GRAPHS { return Parser::make_GRAPHS(); }
DIRECTED { return Parser::make_DIRECTED(); }
UNDIRECTED { return Parser::make_UNDIRECTED(); }
NP { return Parser::make_NODEPLACE(); }
FF_C { return Parser::make_FF_COLOR(); }
FF_3D { return Parser::make_FF_COORD(); }
FF_R { return Parser::make_FF_RADIUS(); }
FF_L { return Parser::make_FF_LABEL(); }
FF_I { return Parser::make_FF_ICON(); }
FF_NI { return Parser::make_FF_NEW_ICON(); }
FF_WE { return Parser::make_FF_WEIGHT(); }
FF_NM { return Parser::make_FF_NEW_MASK(); }
FF_SC { return Parser::make_FF_SCALE(); }
FF_RX { return Parser::make_FF_ROTATE_X(); }
FF_RY { return Parser::make_FF_ROTATE_Y(); }
FF_RZ { return Parser::make_FF_ROTATE_Z(); }
EL { return Parser::make_EDGEPLACE(); }
SN { return Parser::make_STARTNODE(); }
EN { return Parser::make_ENDNODE(); }
FF_LO { return Parser::make_FF_LABELOFFSET(); }
[a-zA-Z]+ { return Parser::make_UNKNOWN_INTERNAL(); }
\n { ++ctx.line; }
[ \t]+ { }
<INITIAL>\x00 { ctx.result.error = "NUL byte in FF input"; return Parser::make_YYerror(); }
<INITIAL>. { return Parser::symbol_type(static_cast<unsigned char>(yytext[0])); }
<INITIAL><<EOF>> { return Parser::make_YYEOF(); }
%%
