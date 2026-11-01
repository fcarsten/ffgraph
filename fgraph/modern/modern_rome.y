/* Rome grammar translated from RomeParser.y (Universitaet Passau 1986-1991).
 * Retain row order; the graph adapter resolves endpoints at each edge row.
 */
%require "3.8"
%skeleton "lalr1.cc"
%define api.namespace {ffgraph_rome}
%define api.parser.class {Parser}
%define api.value.type variant
%define api.token.constructor
%define parse.error detailed
%define parse.lac full
%locations
%define api.location.type {ffgraph_rome::Location}
%expect 0
%parse-param { void* scanner } { ffgraph_rome::Context& ctx }
%lex-param { void* scanner } { ffgraph_rome::Context& ctx }
%code requires {
#include "rome_records.h"
}
%code {
ffgraph_rome::Parser::symbol_type ffrome_lex(void* scanner, ffgraph_rome::Context& ctx);
#define yylex ffrome_lex
using ffgraph_rome::Kind;
}
%token <int> INTEGER
%token NEWLINE HASHMARK BEGIN_LIST END_LIST REAL KEY STRING
%start rows
%%
rows: %empty | rows row;
row:
  INTEGER INTEGER NEWLINE
    { ctx.result.records.push_back({Kind::Node, $1, 0, 0,
        static_cast<std::size_t>(@1.begin.line)}); }
| INTEGER INTEGER INTEGER INTEGER NEWLINE
    { ctx.result.records.push_back({Kind::Edge, $1, $3, $4,
        static_cast<std::size_t>(@1.begin.line)}); }
| HASHMARK NEWLINE
| NEWLINE;
%%
void ffgraph_rome::Parser::error(const location_type& location, const std::string& message) {
  if (ctx.result.error.empty())
    ctx.result.error = message + " in line " + std::to_string(location.begin.line) +
      ", column " + std::to_string(location.begin.column);
}
