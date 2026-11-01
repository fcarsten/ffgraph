/* Modern Bison 3.x pilot for the Graphlet/GML attribute syntax.
 * NOT the shipped FFGraphClass GML importer: that remains GTControl until
 * behavioural parity (including errors and semantic actions) is verified.
 * The original GTParser.y uses bison++-only directives and cannot be fed
 * directly to GNU Bison.
 */
%define api.pure full
%define api.prefix {ffgml_}
%define parse.error detailed
%parse-param { yyscan_t scanner }
%parse-param { ffgraph_gml::Events* events }
%lex-param { yyscan_t scanner }
%code requires {
  #include "gml_events.h"
  typedef void* yyscan_t;
}
%code {
  #include <cstdlib>
  int ffgml_lex(FFGML_STYPE* yylval, yyscan_t scanner);
  void ffgml_error(yyscan_t, ffgraph_gml::Events* events, const char* message) {
    events->error = message;
  }
}
%union {
  int integer;
  double real;
  char* text;
}
%token BEGIN_LIST END_LIST
%token <integer> INTEGER
%token <real> REAL
%token <text> KEY STRING
%destructor { std::free($$); } <text>
%start input
%%
input : attributes ;
attributes : %empty | attribute attributes ;
attribute
  : KEY INTEGER {
      events->integer($1, $2);
      std::free($1);
    }
  | KEY REAL {
      events->real($1, $2);
      std::free($1);
    }
  | KEY STRING {
      events->string($1, $2);
      std::free($1);
      std::free($2);
    }
  | KEY BEGIN_LIST {
      // Keep KEY live until the complete list reduces. On a syntax error
      // while parsing nested attributes, Bison discards this still-stacked
      // KEY and applies its %destructor. Freeing here would double-free it.
      events->begin($1);
    } attributes END_LIST {
      events->end();
      std::free($1);
    }
  ;
%%
