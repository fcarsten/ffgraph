/* FF/sgraph pilot, translated from FFParser.y (Universitaet Passau 1986-1991).
 * Keep reduction/action order: forward nodes and route insertions depend on it.
 */
%require "3.8"
%skeleton "lalr1.cc"
%define api.namespace {ffgraph_ff}
%define api.parser.class {Parser}
%define api.value.type variant
%define api.token.constructor
%define parse.error detailed
/* Legacy ambiguities: greedy unknown-internal values and redundant first
 * node/edge argument alternatives. Keep shift
 * resolution and make any change in the conflict count a generation error. */
%expect 23
%parse-param { void* scanner } { ffgraph_ff::Context& ctx }
%lex-param { void* scanner } { ffgraph_ff::Context& ctx }
%code requires {
#include "ff_events.h"
}
%code {
ffgraph_ff::Parser::symbol_type ffff_lex(void* scanner, ffgraph_ff::Context& ctx);
#define yylex ffff_lex
using ffgraph_ff::Op;
}
%token GRAPH END GRAPHS DIRECTED UNDIRECTED
%token BEGIN_GRAPHED_INTERNALS END_GRAPHED_INTERNALS
%token NODEPLACE EDGEPLACE UNKNOWN_INTERNAL FF_COLOR FF_LABEL FF_ICON
%token FF_COORD FF_NEW_ICON FF_NEW_MASK FF_RADIUS FF_WEIGHT
%token STARTNODE ENDNODE FF_LABELOFFSET FF_SCALE FF_ROTATE_X FF_ROTATE_Y FF_ROTATE_Z
%token <int> NUMBER
%token <std::string> IDENTIFIER
%start graph
%%
graph: GRAPH IDENTIFIER '=' directedness graph_internals list_of_nodes END
  { ctx.emit(Op::GraphName, {}, {$2}); };
directedness:
  %empty { ctx.emit(Op::Directed, {1}); }
| DIRECTED { ctx.emit(Op::Directed, {1}); }
| UNDIRECTED { ctx.emit(Op::Directed, {0}); };
graph_internals: %empty | BEGIN_GRAPHED_INTERNALS ffGraphArgList END_GRAPHED_INTERNALS;
ffGraphArgList: %empty | ffGraphArg ffGraphArgList;
ffGraphArg:
  FF_LABEL IDENTIFIER IDENTIFIER { ctx.emit(Op::GraphLabel, {}, {$2, $3}); }
| FF_LABELOFFSET NUMBER NUMBER { ctx.emit(Op::LabelOffset, {$2, $3}); }
| FF_SCALE NUMBER { ctx.emit(Op::Scale, {$2}); }
| FF_ROTATE_X NUMBER { ctx.emit(Op::RotateX, {$2}); }
| FF_ROTATE_Y NUMBER { ctx.emit(Op::RotateY, {$2}); }
| FF_ROTATE_Z NUMBER { ctx.emit(Op::RotateZ, {$2}); }
| NODEPLACE NUMBER NUMBER | numbers_and_strings ';'
| UNKNOWN_INTERNAL unknown_internal | ',';
list_of_nodes: %empty | node list_of_nodes;
node: node_nr node_internals nodelabel list_of_edges;
node_nr: NUMBER { ctx.emit(Op::Node, {$1}); };
nodelabel: IDENTIFIER { ctx.emit(Op::NodeName, {}, {$1}); };
node_internals: %empty | BEGIN_GRAPHED_INTERNALS ffNodeArgList END_GRAPHED_INTERNALS;
/* Retain the redundant alternative and legacy shift resolution. */
ffNodeArgList: %empty | ffNodeArgList ffNodeArg | ffNodeArg;
ffNodeArg:
  UNKNOWN_INTERNAL unknown_internal
| NUMBER NUMBER { ctx.emit(Op::NodePosition, {$1, $2, 0}); }
| NODEPLACE NUMBER NUMBER { ctx.emit(Op::NodePosition, {$2, $3, 0}); }
| STARTNODE { ctx.emit(Op::Start); }
| ENDNODE { ctx.emit(Op::End); }
| FF_COORD NUMBER NUMBER NUMBER { ctx.emit(Op::NodePosition, {$2, $3, $4}); }
| FF_COLOR IDENTIFIER { ctx.emit(Op::NodeColor, {}, {$2}); }
| FF_LABEL IDENTIFIER IDENTIFIER { ctx.emit(Op::NodeLabel, {}, {$2, $3}); }
| FF_ICON IDENTIFIER IDENTIFIER { ctx.emit(Op::Icon, {}, {$2, $3}); }
| FF_RADIUS NUMBER { ctx.emit(Op::Radius, {$2}); }
| FF_ICON IDENTIFIER { ctx.emit(Op::Icon, {}, {$2}); }
| FF_NEW_ICON IDENTIFIER IDENTIFIER { ctx.emit(Op::NewIcon, {}, {$2, $3}); }
| FF_NEW_ICON IDENTIFIER { ctx.emit(Op::NewIcon, {}, {$2}); }
| FF_NEW_MASK IDENTIFIER IDENTIFIER { ctx.emit(Op::Mask, {}, {$2, $3}); }
| FF_NEW_MASK IDENTIFIER { ctx.emit(Op::Mask, {}, {$2}); };
list_of_edges: ';' | edge list_of_edges;
edge: targetnode_nr edge_internals edgelabel;
targetnode_nr: NUMBER { ctx.emit(Op::Target, {$1}); };
edgelabel: IDENTIFIER { ctx.emit(Op::EdgeName, {}, {$1}); };
edge_internals: %empty | BEGIN_GRAPHED_INTERNALS ffEdgeArgList END_GRAPHED_INTERNALS;
ffEdgeArgList: %empty | ffEdgeArgList ffEdgeArg | ffEdgeArg;
ffEdgeArg:
  EDGEPLACE edgenumberlist | FF_COORD ffEdgeNumerList
| FF_COLOR IDENTIFIER { ctx.emit(Op::EdgeColor, {}, {$2}); }
| FF_WEIGHT NUMBER { ctx.emit(Op::Weight, {$2}); }
| FF_LABEL IDENTIFIER IDENTIFIER { ctx.emit(Op::EdgeLabel, {}, {$2, $3}); }
| UNKNOWN_INTERNAL unknown_internal;
edgenumberlist: %empty | NUMBER NUMBER edgenumberlist
  { ctx.emit(Op::EdgePoint, {$1, $2, 0}); };
ffEdgeNumerList: %empty | NUMBER NUMBER NUMBER ffEdgeNumerList
  { ctx.emit(Op::EdgePoint, {$1, $2, $3}); };
unknown_internal: numbers_and_strings;
numbers_and_strings: %empty | number_or_string numbers_and_strings;
number_or_string: NUMBER | IDENTIFIER;
%%
void ffgraph_ff::Parser::error(const std::string& message) {
  if (ctx.result.error.empty())
    ctx.result.error = message + " in line " + std::to_string(ctx.line);
}
