#ifndef YY_FFParser_h_included
#define YY_FFParser_h_included

#line 1 "/public/friedric/lib/bison.h"
/* before anything */
#ifdef c_plusplus
#ifndef __cplusplus
#define __cplusplus
#endif
#endif
#ifdef __cplusplus
#ifndef YY_USE_CLASS
#define YY_USE_CLASS
#endif
#else
#endif
#include <stdio.h>

/* #line 14 "/public/friedric/lib/bison.h" */
#line 21 "FFParser.h"

#line 52 "FFParser.y"
typedef union	{		/* stack for parser	*/
  int	nr;			/* Zahl			*/
  char	*text;			/* Label		*/
} yy_FFParser_stype;
#define YY_FFParser_STYPE yy_FFParser_stype
#line 57 "FFParser.y"

  typedef YY_FFParser_STYPE tokenValue;
#define YY_FFParser_LEX  Scan
#define YY_FFParser_LEX_BODY  = 0
#define YY_FFParser_ERROR_BODY  = 0
#define YY_FFParser_ERROR  PrintError
#define YY_FFParser_LVAL  theTokenValue
#define YY_FFParser_PARSE  iParse 
#define YY_FFParser_MEMBERS  \
protected: \
FFNodeClass	*current_node; \
FFEdgeClass	*current_edge; \
FFGraphClass *parseFFGraph;

#line 14 "/public/friedric/lib/bison.h"
 /* %{ and %header{ and %union, during decl */
#ifndef YY_FFParser_COMPATIBILITY
#ifndef YY_USE_CLASS
#define  YY_FFParser_COMPATIBILITY 1
#else
#define  YY_FFParser_COMPATIBILITY 0
#endif
#endif

#if YY_FFParser_COMPATIBILITY != 0
/* backward compatibility */
#ifdef YYLTYPE
#ifndef YY_FFParser_LTYPE
#define YY_FFParser_LTYPE YYLTYPE
/* WARNING obsolete !!! user defined YYLTYPE not reported into generated header */
/* use %define LTYPE */
#endif
#endif
#ifdef YYSTYPE
#ifndef YY_FFParser_STYPE 
#define YY_FFParser_STYPE YYSTYPE
/* WARNING obsolete !!! user defined YYSTYPE not reported into generated header */
/* use %define STYPE */
#endif
#endif
#ifdef YYDEBUG
#ifndef YY_FFParser_DEBUG
#define  YY_FFParser_DEBUG YYDEBUG
/* WARNING obsolete !!! user defined YYDEBUG not reported into generated header */
/* use %define DEBUG */
#endif
#endif
#ifdef YY_FFParser_STYPE
#ifndef yystype
#define yystype YY_FFParser_STYPE
#endif
#endif
/* use goto to be compatible */
#ifndef YY_FFParser_USE_GOTO
#define YY_FFParser_USE_GOTO 1
#endif
#endif

/* use no goto to be clean in C++ */
#ifndef YY_FFParser_USE_GOTO
#define YY_FFParser_USE_GOTO 0
#endif

#ifndef YY_FFParser_PURE

/* #line 63 "/public/friedric/lib/bison.h" */
#line 96 "FFParser.h"

#line 63 "/public/friedric/lib/bison.h"
/* YY_FFParser_PURE */
#endif

/* #line 65 "/public/friedric/lib/bison.h" */
#line 103 "FFParser.h"

#line 65 "/public/friedric/lib/bison.h"
/* prefix */
#ifndef YY_FFParser_DEBUG

/* #line 67 "/public/friedric/lib/bison.h" */
#line 110 "FFParser.h"

#line 67 "/public/friedric/lib/bison.h"
/* YY_FFParser_DEBUG */
#endif
#ifndef YY_FFParser_LSP_NEEDED

/* #line 70 "/public/friedric/lib/bison.h" */
#line 118 "FFParser.h"

#line 70 "/public/friedric/lib/bison.h"
 /* YY_FFParser_LSP_NEEDED*/
#endif
/* DEFAULT LTYPE*/
#ifdef YY_FFParser_LSP_NEEDED
#ifndef YY_FFParser_LTYPE
typedef
  struct yyltype
    {
      int timestamp;
      int first_line;
      int first_column;
      int last_line;
      int last_column;
      char *text;
   }
  yyltype;

#define YY_FFParser_LTYPE yyltype
#endif
#endif
/* DEFAULT STYPE*/
#ifndef YY_FFParser_STYPE
#define YY_FFParser_STYPE int
#endif
/* DEFAULT MISCELANEOUS */
#ifndef YY_FFParser_PARSE
#define YY_FFParser_PARSE yyparse
#endif
#ifndef YY_FFParser_LEX
#define YY_FFParser_LEX yylex
#endif
#ifndef YY_FFParser_LVAL
#define YY_FFParser_LVAL yylval
#endif
#ifndef YY_FFParser_LLOC
#define YY_FFParser_LLOC yylloc
#endif
#ifndef YY_FFParser_CHAR
#define YY_FFParser_CHAR yychar
#endif
#ifndef YY_FFParser_NERRS
#define YY_FFParser_NERRS yynerrs
#endif
#ifndef YY_FFParser_DEBUG_FLAG
#define YY_FFParser_DEBUG_FLAG yydebug
#endif
#ifndef YY_FFParser_ERROR
#define YY_FFParser_ERROR yyerror
#endif

#ifndef YY_FFParser_PARSE_PARAM
#ifndef __STDC__
#ifndef __cplusplus
#ifndef YY_USE_CLASS
#define YY_FFParser_PARSE_PARAM
#ifndef YY_FFParser_PARSE_PARAM_DEF
#define YY_FFParser_PARSE_PARAM_DEF
#endif
#endif
#endif
#endif
#ifndef YY_FFParser_PARSE_PARAM
#define YY_FFParser_PARSE_PARAM void
#endif
#endif

/* TOKEN C */
#ifndef YY_USE_CLASS

#ifndef YY_FFParser_PURE
extern YY_FFParser_STYPE YY_FFParser_LVAL;
#endif


/* #line 143 "/public/friedric/lib/bison.h" */
#line 196 "FFParser.h"
#define	GRAPH	258
#define	END	259
#define	GRAPHS	260
#define	DIRECTED	261
#define	UNDIRECTED	262
#define	BEGIN_GRAPHED_INTERNALS	263
#define	END_GRAPHED_INTERNALS	264
#define	NODEPLACE	265
#define	EDGEPLACE	266
#define	UNKNOWN_INTERNAL	267
#define	FF_COLOR	268
#define	FF_LABEL	269
#define	FF_ICON	270
#define	FF_COORD	271
#define	FF_NEW_ICON	272
#define	FF_NEW_MASK	273
#define	FF_RADIUS	274
#define	FF_WEIGHT	275
#define	STARTNODE	276
#define	ENDNODE	277
#define	FF_LABELOFFSET	278
#define	FF_SCALE	279
#define	FF_ROTATE_X	280
#define	FF_ROTATE_Y	281
#define	FF_ROTATE_Z	282
#define	NUMBER	283
#define	IDENTIFIER	284


#line 143 "/public/friedric/lib/bison.h"
 /* #defines token */
/* after #define tokens, before const tokens S5*/
#else
#ifndef YY_FFParser_CLASS
#define YY_FFParser_CLASS FFParser
#endif

#ifndef YY_FFParser_INHERIT
#define YY_FFParser_INHERIT
#endif
#ifndef YY_FFParser_MEMBERS
#define YY_FFParser_MEMBERS 
#endif
#ifndef YY_FFParser_LEX_BODY
#define YY_FFParser_LEX_BODY  
#endif
#ifndef YY_FFParser_ERROR_BODY
#define YY_FFParser_ERROR_BODY  
#endif
#ifndef YY_FFParser_CONSTRUCTOR_PARAM
#define YY_FFParser_CONSTRUCTOR_PARAM
#endif
/* choose between enum and const */
#ifndef YY_FFParser_USE_CONST_TOKEN
#define YY_FFParser_USE_CONST_TOKEN 0
/* yes enum is more compatible with flex,  */
/* so by default we use it */ 
#endif
#if YY_FFParser_USE_CONST_TOKEN != 0
#ifndef YY_FFParser_ENUM_TOKEN
#define YY_FFParser_ENUM_TOKEN yy_FFParser_enum_token
#endif
#endif

class YY_FFParser_CLASS YY_FFParser_INHERIT
{
public: 
#if YY_FFParser_USE_CONST_TOKEN != 0
/* static const int token ... */

/* #line 182 "/public/friedric/lib/bison.h" */
#line 268 "FFParser.h"
static const int GRAPH;
static const int END;
static const int GRAPHS;
static const int DIRECTED;
static const int UNDIRECTED;
static const int BEGIN_GRAPHED_INTERNALS;
static const int END_GRAPHED_INTERNALS;
static const int NODEPLACE;
static const int EDGEPLACE;
static const int UNKNOWN_INTERNAL;
static const int FF_COLOR;
static const int FF_LABEL;
static const int FF_ICON;
static const int FF_COORD;
static const int FF_NEW_ICON;
static const int FF_NEW_MASK;
static const int FF_RADIUS;
static const int FF_WEIGHT;
static const int STARTNODE;
static const int ENDNODE;
static const int FF_LABELOFFSET;
static const int FF_SCALE;
static const int FF_ROTATE_X;
static const int FF_ROTATE_Y;
static const int FF_ROTATE_Z;
static const int NUMBER;
static const int IDENTIFIER;


#line 182 "/public/friedric/lib/bison.h"
 /* decl const */
#else
enum YY_FFParser_ENUM_TOKEN { YY_FFParser_NULL_TOKEN=0

/* #line 185 "/public/friedric/lib/bison.h" */
#line 304 "FFParser.h"
	,GRAPH=258
	,END=259
	,GRAPHS=260
	,DIRECTED=261
	,UNDIRECTED=262
	,BEGIN_GRAPHED_INTERNALS=263
	,END_GRAPHED_INTERNALS=264
	,NODEPLACE=265
	,EDGEPLACE=266
	,UNKNOWN_INTERNAL=267
	,FF_COLOR=268
	,FF_LABEL=269
	,FF_ICON=270
	,FF_COORD=271
	,FF_NEW_ICON=272
	,FF_NEW_MASK=273
	,FF_RADIUS=274
	,FF_WEIGHT=275
	,STARTNODE=276
	,ENDNODE=277
	,FF_LABELOFFSET=278
	,FF_SCALE=279
	,FF_ROTATE_X=280
	,FF_ROTATE_Y=281
	,FF_ROTATE_Z=282
	,NUMBER=283
	,IDENTIFIER=284


#line 185 "/public/friedric/lib/bison.h"
 /* enum token */
     }; /* end of enum declaration */
#endif
public:
 int YY_FFParser_PARSE(YY_FFParser_PARSE_PARAM);
 virtual void YY_FFParser_ERROR(char *msg) YY_FFParser_ERROR_BODY;
#ifdef YY_FFParser_PURE
#ifdef YY_FFParser_LSP_NEEDED
 virtual int  YY_FFParser_LEX(YY_FFParser_STYPE *YY_FFParser_LVAL,YY_FFParser_LTYPE *YY_FFParser_LLOC) YY_FFParser_LEX_BODY;
#else
 virtual int  YY_FFParser_LEX(YY_FFParser_STYPE *YY_FFParser_LVAL) YY_FFParser_LEX_BODY;
#endif
#else
 virtual int YY_FFParser_LEX() YY_FFParser_LEX_BODY;
 YY_FFParser_STYPE YY_FFParser_LVAL;
#ifdef YY_FFParser_LSP_NEEDED
 YY_FFParser_LTYPE YY_FFParser_LLOC;
#endif
 int YY_FFParser_NERRS;
 int YY_FFParser_CHAR;
#endif
#if YY_FFParser_DEBUG != 0
public:
 int YY_FFParser_DEBUG_FLAG;	/*  nonzero means print parse trace	*/
#endif
public:
 YY_FFParser_CLASS(YY_FFParser_CONSTRUCTOR_PARAM);
public:
 YY_FFParser_MEMBERS 
};
/* other declare folow */
#endif


#if YY_FFParser_COMPATIBILITY != 0
/* backward compatibility */
#ifndef YYSTYPE
#define YYSTYPE YY_FFParser_STYPE
#endif

#ifndef YYLTYPE
#define YYLTYPE YY_FFParser_LTYPE
#endif
#ifndef YYDEBUG
#ifdef YY_FFParser_DEBUG 
#define YYDEBUG YY_FFParser_DEBUG
#endif
#endif

#endif
/* END */

/* #line 236 "/public/friedric/lib/bison.h" */
#line 388 "FFParser.h"
#endif
