#define YY_FFParser_h_included

/*  A Bison++ parser, made from FFParser.y  */

 /* with Bison++ version bison++ Version 1.21-8, adapted from GNU bison by coetmeur@icdc.fr
  */


#line 1 "/public/friedric/lib/bison.cc"
/* -*-C-*-  Note some compilers choke on comments on `#line' lines.  */
/* Skeleton output parser for bison,
   Copyright (C) 1984, 1989, 1990 Bob Corbett and Richard Stallman

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 1, or (at your option)
   any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.  */

/* HEADER SECTION */
#if defined( _MSDOS ) || defined(MSDOS) || defined(__MSDOS__) 
#define __MSDOS_AND_ALIKE
#endif
#if defined(_WINDOWS) && defined(_MSC_VER)
#define __HAVE_NO_ALLOCA
#define __MSDOS_AND_ALIKE
#endif

#ifndef alloca
#if defined( __GNUC__)
#define alloca __builtin_alloca

#elif (!defined (__STDC__) && defined (sparc)) || defined (__sparc__) || defined (__sparc)  || defined (__sgi)
#include <alloca.h>

#elif defined (__MSDOS_AND_ALIKE)
#include <malloc.h>
#ifndef __TURBOC__
/* MS C runtime lib */
#define alloca _alloca
#endif

#elif defined(_AIX)
#include <malloc.h>
#pragma alloca

#elif defined(__hpux)
#ifdef __cplusplus
extern "C" {
void *alloca (unsigned int);
};
#else /* not __cplusplus */
void *alloca ();
#endif /* not __cplusplus */

#endif /* not _AIX  not MSDOS, or __TURBOC__ or _AIX, not sparc.  */
#endif /* alloca not defined.  */
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
#ifndef __STDC__
#define const
#endif
#endif
#include <stdio.h>
#define YYBISON 1  

/* #line 73 "/public/friedric/lib/bison.cc" */
#line 85 "FFParser.cc"
#line 14 "FFParser.y"

  /************************************************************************/
  /*									*/
  /*				stdparser.y				*/
  /*									*/
  /************************************************************************/



extern void *malloc(), *realloc();

#include <globdefs.h>
#include <ffgraph/FFGraphClass.h>
#include <ffgraph/FFNodeClass.h>
#include <ffgraph/FFEdgeClass.h>
#include <ffgraph/FFGraphWindow.h>
#include <ffgraph/FFGraphicNode.h>
#include <ffgraph/FFGraphicEdge.h>

#include <container/AVLSet.h>
#include <container/VOHSet.h>

#include <ffgraph/FFNodeLabel.h>
#include <ffgraph/FFGraphLabel.h>
#include <ffgraph/FFEdgeLabel.h>

extern AVLSet<String> unmatchedLabels;

// IDENTIFIER text is owned by FFScanner through parse success or failure.
// Semantic actions borrow it; never free parser-stack token pointers here.

  /* Stacksize for yacc	*/

#define	YYMAXDEPTH 2000

  /* Macro to abort yacc	*/

#define	yacc_error(s)	{ yyerror(s); YYABORT; }

  
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

#line 73 "/public/friedric/lib/bison.cc"
/* %{ and %header{ and %union, during decl */
#define YY_FFParser_BISON 1
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
#endif
#endif
#ifdef YYSTYPE
#ifndef YY_FFParser_STYPE 
#define YY_FFParser_STYPE YYSTYPE
#endif
#endif
#ifdef YYDEBUG
#ifndef YY_FFParser_DEBUG
#define  YY_FFParser_DEBUG YYDEBUG
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

/* #line 117 "/public/friedric/lib/bison.cc" */
#line 192 "FFParser.cc"

#line 117 "/public/friedric/lib/bison.cc"
/*  YY_FFParser_PURE */
#endif

/* section apres lecture def, avant lecture grammaire S2 */

/* #line 121 "/public/friedric/lib/bison.cc" */
#line 201 "FFParser.cc"

#line 121 "/public/friedric/lib/bison.cc"
/* prefix */
#ifndef YY_FFParser_DEBUG

/* #line 123 "/public/friedric/lib/bison.cc" */
#line 208 "FFParser.cc"

#line 123 "/public/friedric/lib/bison.cc"
/* YY_FFParser_DEBUG */
#endif


#ifndef YY_FFParser_LSP_NEEDED

/* #line 128 "/public/friedric/lib/bison.cc" */
#line 218 "FFParser.cc"

#line 128 "/public/friedric/lib/bison.cc"
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
      /* We used to use `unsigned long' as YY_FFParser_STYPE on MSDOS,
	 but it seems better to be consistent.
	 Most programs should declare their own type anyway.  */

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
#if YY_FFParser_COMPATIBILITY != 0
/* backward compatibility */
#ifdef YY_FFParser_LTYPE
#ifndef YYLTYPE
#define YYLTYPE YY_FFParser_LTYPE
#else
/* WARNING obsolete !!! user defined YYLTYPE not reported into generated header */
#endif
#endif
#ifndef YYSTYPE
#define YYSTYPE YY_FFParser_STYPE
#else
/* WARNING obsolete !!! user defined YYSTYPE not reported into generated header */
#endif
#ifdef YY_FFParser_PURE
#ifndef YYPURE
#define YYPURE YY_FFParser_PURE
#endif
#endif
#ifdef YY_FFParser_DEBUG
#ifndef YYDEBUG
#define YYDEBUG YY_FFParser_DEBUG 
#endif
#endif
#ifndef YY_FFParser_ERROR_VERBOSE
#ifdef YYERROR_VERBOSE
#define YY_FFParser_ERROR_VERBOSE YYERROR_VERBOSE
#endif
#endif
#ifndef YY_FFParser_LSP_NEEDED
#ifdef YYLSP_NEEDED
#define YY_FFParser_LSP_NEEDED YYLSP_NEEDED
#endif
#endif
#endif
#ifndef YY_USE_CLASS
/* TOKEN C */

/* #line 236 "/public/friedric/lib/bison.cc" */
#line 331 "FFParser.cc"
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


#line 236 "/public/friedric/lib/bison.cc"
 /* #defines tokens */
#else
/* CLASS */
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
#ifndef YY_FFParser_CONSTRUCTOR_CODE
#define YY_FFParser_CONSTRUCTOR_CODE
#endif
#ifndef YY_FFParser_CONSTRUCTOR_INIT
#define YY_FFParser_CONSTRUCTOR_INIT
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

/* #line 280 "/public/friedric/lib/bison.cc" */
#line 408 "FFParser.cc"
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


#line 280 "/public/friedric/lib/bison.cc"
 /* decl const */
#else
enum YY_FFParser_ENUM_TOKEN { YY_FFParser_NULL_TOKEN=0

/* #line 283 "/public/friedric/lib/bison.cc" */
#line 444 "FFParser.cc"
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


#line 283 "/public/friedric/lib/bison.cc"
 /* enum token */
     }; /* end of enum declaration */
#endif
public:
 int YY_FFParser_PARSE (YY_FFParser_PARSE_PARAM);
 virtual void YY_FFParser_ERROR(char *msg) YY_FFParser_ERROR_BODY;
#ifdef YY_FFParser_PURE
#ifdef YY_FFParser_LSP_NEEDED
 virtual int  YY_FFParser_LEX (YY_FFParser_STYPE *YY_FFParser_LVAL,YY_FFParser_LTYPE *YY_FFParser_LLOC) YY_FFParser_LEX_BODY;
#else
 virtual int  YY_FFParser_LEX (YY_FFParser_STYPE *YY_FFParser_LVAL) YY_FFParser_LEX_BODY;
#endif
#else
 virtual int YY_FFParser_LEX() YY_FFParser_LEX_BODY;
 YY_FFParser_STYPE YY_FFParser_LVAL;
#ifdef YY_FFParser_LSP_NEEDED
 YY_FFParser_LTYPE YY_FFParser_LLOC;
#endif
 int   YY_FFParser_NERRS;
 int    YY_FFParser_CHAR;
#endif
#if YY_FFParser_DEBUG != 0
 int YY_FFParser_DEBUG_FLAG;   /*  nonzero means print parse trace     */
#endif
public:
 YY_FFParser_CLASS(YY_FFParser_CONSTRUCTOR_PARAM);
public:
 YY_FFParser_MEMBERS 
};
/* other declare folow */
#if YY_FFParser_USE_CONST_TOKEN != 0

/* #line 314 "/public/friedric/lib/bison.cc" */
#line 508 "FFParser.cc"
const int YY_FFParser_CLASS::GRAPH=258;
const int YY_FFParser_CLASS::END=259;
const int YY_FFParser_CLASS::GRAPHS=260;
const int YY_FFParser_CLASS::DIRECTED=261;
const int YY_FFParser_CLASS::UNDIRECTED=262;
const int YY_FFParser_CLASS::BEGIN_GRAPHED_INTERNALS=263;
const int YY_FFParser_CLASS::END_GRAPHED_INTERNALS=264;
const int YY_FFParser_CLASS::NODEPLACE=265;
const int YY_FFParser_CLASS::EDGEPLACE=266;
const int YY_FFParser_CLASS::UNKNOWN_INTERNAL=267;
const int YY_FFParser_CLASS::FF_COLOR=268;
const int YY_FFParser_CLASS::FF_LABEL=269;
const int YY_FFParser_CLASS::FF_ICON=270;
const int YY_FFParser_CLASS::FF_COORD=271;
const int YY_FFParser_CLASS::FF_NEW_ICON=272;
const int YY_FFParser_CLASS::FF_NEW_MASK=273;
const int YY_FFParser_CLASS::FF_RADIUS=274;
const int YY_FFParser_CLASS::FF_WEIGHT=275;
const int YY_FFParser_CLASS::STARTNODE=276;
const int YY_FFParser_CLASS::ENDNODE=277;
const int YY_FFParser_CLASS::FF_LABELOFFSET=278;
const int YY_FFParser_CLASS::FF_SCALE=279;
const int YY_FFParser_CLASS::FF_ROTATE_X=280;
const int YY_FFParser_CLASS::FF_ROTATE_Y=281;
const int YY_FFParser_CLASS::FF_ROTATE_Z=282;
const int YY_FFParser_CLASS::NUMBER=283;
const int YY_FFParser_CLASS::IDENTIFIER=284;


#line 314 "/public/friedric/lib/bison.cc"
 /* const YY_FFParser_CLASS::token */
#endif
/*apres const  */
YY_FFParser_CLASS::YY_FFParser_CLASS(YY_FFParser_CONSTRUCTOR_PARAM) YY_FFParser_CONSTRUCTOR_INIT
{
#if YY_FFParser_DEBUG != 0
YY_FFParser_DEBUG_FLAG=0;
#endif
YY_FFParser_CONSTRUCTOR_CODE;
};
#endif

/* #line 325 "/public/friedric/lib/bison.cc" */
#line 552 "FFParser.cc"


#define	YYFINAL		121
#define	YYFLAG		-32768
#define	YYNTBASE	33

#define YYTRANSLATE(x) ((unsigned)(x) <= 284 ? yytranslate[x] : 58)

static const char yytranslate[] = {     0,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,    32,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     5,     2,
     4,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     1,     2,     3,     6,     7,
     8,     9,    10,    11,    12,    13,    14,    15,    16,    17,
    18,    19,    20,    21,    22,    23,    24,    25,    26,    27,
    28,    29,    30,    31
};

#if YY_FFParser_DEBUG != 0
static const short yyprhs[] = {     0,
     0,     2,     4,    12,    13,    15,    17,    18,    22,    23,
    26,    30,    34,    37,    40,    43,    46,    50,    53,    56,
    58,    59,    62,    67,    69,    71,    72,    76,    77,    80,
    82,    85,    88,    92,    94,    96,   101,   104,   108,   112,
   115,   118,   122,   125,   129,   132,   134,   137,   141,   143,
   145,   146,   150,   151,   154,   156,   159,   162,   165,   168,
   172,   175,   176,   180,   181,   186,   188,   189,   192,   194
};

static const short yyrhs[] = {    34,
     0,     1,     0,     3,    31,     4,    35,    36,    39,     6,
     0,     0,     8,     0,     9,     0,     0,    10,    37,    11,
     0,     0,    38,    37,     0,    16,    31,    31,     0,    25,
    30,    30,     0,    26,    30,     0,    27,    30,     0,    28,
    30,     0,    29,    30,     0,    12,    30,    30,     0,    56,
     5,     0,    14,    55,     0,    32,     0,     0,    40,    39,
     0,    41,    43,    42,    46,     0,    30,     0,    31,     0,
     0,    10,    44,    11,     0,     0,    44,    45,     0,    45,
     0,    14,    55,     0,    30,    30,     0,    12,    30,    30,
     0,    23,     0,    24,     0,    18,    30,    30,    30,     0,
    15,    31,     0,    16,    31,    31,     0,    17,    31,    31,
     0,    21,    30,     0,    17,    31,     0,    19,    31,    31,
     0,    19,    31,     0,    20,    31,    31,     0,    20,    31,
     0,     5,     0,    47,    46,     0,    48,    50,    49,     0,
    30,     0,    31,     0,     0,    10,    51,    11,     0,     0,
    51,    52,     0,    52,     0,    13,    53,     0,    18,    54,
     0,    15,    31,     0,    22,    30,     0,    16,    31,    31,
     0,    14,    55,     0,     0,    30,    30,    53,     0,     0,
    30,    30,    30,    54,     0,    56,     0,     0,    57,    56,
     0,    30,     0,    31,     0
};

#endif

#if YY_FFParser_DEBUG != 0
static const short yyrline[] = { 0,
    88,    89,    96,   105,   109,   113,   119,   120,   123,   124,
   127,   141,   148,   157,   165,   173,   181,   182,   183,   184,
   187,   188,   191,   197,   209,   219,   220,   223,   224,   225,
   228,   229,   234,   239,   243,   247,   252,   258,   272,   282,
   289,   295,   304,   310,   319,   328,   329,   332,   338,   352,
   369,   370,   373,   374,   375,   378,   379,   380,   386,   390,
   406,   409,   410,   416,   417,   423,   426,   427,   430,   431
};

static const char * const yytname[] = {   "$","error","$illegal.","GRAPH","'='",
"';'","END","GRAPHS","DIRECTED","UNDIRECTED","BEGIN_GRAPHED_INTERNALS","END_GRAPHED_INTERNALS",
"NODEPLACE","EDGEPLACE","UNKNOWN_INTERNAL","FF_COLOR","FF_LABEL","FF_ICON","FF_COORD",
"FF_NEW_ICON","FF_NEW_MASK","FF_RADIUS","FF_WEIGHT","STARTNODE","ENDNODE","FF_LABELOFFSET",
"FF_SCALE","FF_ROTATE_X","FF_ROTATE_Y","FF_ROTATE_Z","NUMBER","IDENTIFIER","','",
"parser_start","graph","directedness","graph_internals","ffGraphArgList","ffGraphArg",
"list_of_nodes","node","node_nr","nodelabel","node_internals","ffNodeArgList",
"ffNodeArg","list_of_edges","edge","targetnode_nr","edgelabel","edge_internals",
"ffEdgeArgList","ffEdgeArg","edgenumberlist","ffEdgeNumerList","unknown_internal",
"numbers_and_strings","number_or_string",""
};
#endif

static const short yyr1[] = {     0,
    33,    33,    34,    35,    35,    35,    36,    36,    37,    37,
    38,    38,    38,    38,    38,    38,    38,    38,    38,    38,
    39,    39,    40,    41,    42,    43,    43,    44,    44,    44,
    45,    45,    45,    45,    45,    45,    45,    45,    45,    45,
    45,    45,    45,    45,    45,    46,    46,    47,    48,    49,
    50,    50,    51,    51,    51,    52,    52,    52,    52,    52,
    52,    53,    53,    54,    54,    55,    56,    56,    57,    57
};

static const short yyr2[] = {     0,
     1,     1,     7,     0,     1,     1,     0,     3,     0,     2,
     3,     3,     2,     2,     2,     2,     3,     2,     2,     1,
     0,     2,     4,     1,     1,     0,     3,     0,     2,     1,
     2,     2,     3,     1,     1,     4,     2,     3,     3,     2,
     2,     3,     2,     3,     2,     1,     2,     3,     1,     1,
     0,     3,     0,     2,     1,     2,     2,     2,     2,     3,
     2,     0,     3,     0,     4,     1,     0,     2,     1,     1
};

static const short yydefact[] = {     0,
     2,     0,     1,     0,     4,     5,     6,     7,     9,    21,
     0,    67,     0,     0,     0,     0,     0,     0,    69,    70,
    20,     0,     9,     0,    67,    24,     0,    21,    26,     0,
    19,    66,     0,     0,    13,    14,    15,    16,     8,    10,
    18,    68,     3,    22,    28,     0,    17,    11,    12,     0,
    67,     0,     0,     0,     0,     0,     0,     0,    34,    35,
     0,     0,    30,    25,     0,     0,    31,    37,     0,    41,
     0,    43,    45,    40,    32,    27,    29,    46,    49,    23,
     0,    51,    33,    38,    39,     0,    42,    44,    47,    53,
     0,    36,    62,    67,     0,     0,    64,     0,     0,    55,
    50,    48,     0,    56,    61,    58,     0,     0,    57,    59,
    52,    54,    62,    60,     0,    63,    64,    65,     0,     0,
     0
};

static const short yydefgoto[] = {   119,
     3,     8,    10,    22,    23,    27,    28,    29,    65,    46,
    62,    63,    80,    81,    82,   102,    91,    99,   100,   104,
   109,    31,    32,    25
};

static const short yypact[] = {     2,
-32768,   -22,-32768,     7,    -2,-32768,-32768,     5,    -4,   -13,
   -11,   -17,     0,   -10,     4,    16,    17,    18,-32768,-32768,
-32768,    19,    -4,    44,   -17,-32768,    54,   -13,    53,    34,
-32768,-32768,    35,    43,-32768,-32768,-32768,-32768,-32768,-32768,
-32768,-32768,-32768,-32768,    38,    45,-32768,-32768,-32768,    47,
   -17,    52,    55,    58,    57,    59,    60,    62,-32768,-32768,
    63,    21,-32768,-32768,    -1,    64,-32768,-32768,    65,    67,
    69,    70,    71,-32768,-32768,-32768,-32768,-32768,-32768,-32768,
    -1,    75,-32768,-32768,-32768,    73,-32768,-32768,-32768,    66,
    74,-32768,    76,   -17,    77,    78,    80,    81,    56,-32768,
-32768,-32768,    82,-32768,-32768,-32768,    83,    85,-32768,-32768,
-32768,-32768,    76,-32768,    86,-32768,    80,-32768,    95,    97,
-32768
};

static const short yypgoto[] = {-32768,
-32768,-32768,-32768,    42,-32768,    72,-32768,-32768,-32768,-32768,
-32768,    13,    23,-32768,-32768,-32768,-32768,-32768,     8,     6,
     1,   -51,    -7,-32768
};


#define	YYLAST		119


static const short yytable[] = {    67,
   -67,    24,     1,    78,     2,     6,     7,    11,     4,    12,
     5,    13,    19,    20,     9,    24,    26,    42,    30,    34,
    14,    15,    16,    17,    18,    19,    20,    21,    79,    39,
    33,    76,    50,    35,    51,    52,    53,    54,    55,    56,
    57,    58,   105,    59,    60,    36,    37,    38,    41,    50,
    61,    51,    52,    53,    54,    55,    56,    57,    58,    43,
    59,    60,    45,    47,    40,    48,   111,    61,    93,    94,
    95,    96,    49,    97,    77,    64,    66,    98,    93,    94,
    95,    96,    68,    97,    90,    69,    71,    98,    70,    72,
    73,    74,    75,    83,   120,    84,   121,    85,    86,    44,
    87,    88,    92,    89,   101,   103,   112,   106,   107,   108,
   110,   113,     0,   114,   115,   117,     0,   118,   116
};

static const short yycheck[] = {    51,
     5,     9,     1,     5,     3,     8,     9,    12,    31,    14,
     4,    16,    30,    31,    10,    23,    30,    25,    30,    30,
    25,    26,    27,    28,    29,    30,    31,    32,    30,    11,
    31,    11,    12,    30,    14,    15,    16,    17,    18,    19,
    20,    21,    94,    23,    24,    30,    30,    30,     5,    12,
    30,    14,    15,    16,    17,    18,    19,    20,    21,     6,
    23,    24,    10,    30,    23,    31,    11,    30,    13,    14,
    15,    16,    30,    18,    62,    31,    30,    22,    13,    14,
    15,    16,    31,    18,    10,    31,    30,    22,    31,    31,
    31,    30,    30,    30,     0,    31,     0,    31,    30,    28,
    31,    31,    30,    81,    31,    30,    99,    31,    31,    30,
    30,    30,    -1,    31,    30,    30,    -1,   117,   113
};

#line 325 "/public/friedric/lib/bison.cc"
 /* fattrs + tables */

/* parser code folow  */


/* This is the parser code that is written into each bison parser
  when the %semantic_parser declaration is not specified in the grammar.
  It was written by Richard Stallman by simplifying the hairy parser
  used when %semantic_parser is specified.  */

/* Note: dollar marks section change
   the next  is replaced by the list of actions, each action
   as one case of the switch.  */ 

#if YY_FFParser_USE_GOTO != 0
/* 
 SUPRESSION OF GOTO : on some C++ compiler (sun c++)
  the goto is strictly forbidden if any constructor/destructor
  is used in the whole function (very stupid isn't it ?)
 so goto are to be replaced with a 'while/switch/case construct'
 here are the macro to keep some apparent compatibility
*/
#define YYGOTO(lb) {yy_gotostate=lb;continue;}
#define YYBEGINGOTO  enum yy_labels yy_gotostate=yygotostart; \
                     for(;;) switch(yy_gotostate) { case yygotostart: {
#define YYLABEL(lb) } case lb: {
#define YYENDGOTO } } 
#define YYBEGINDECLARELABEL enum yy_labels {yygotostart
#define YYDECLARELABEL(lb) ,lb
#define YYENDDECLARELABEL  };
#else
/* macro to keep goto */
#define YYGOTO(lb) goto lb
#define YYBEGINGOTO 
#define YYLABEL(lb) lb:
#define YYENDGOTO
#define YYBEGINDECLARELABEL 
#define YYDECLARELABEL(lb)
#define YYENDDECLARELABEL 
#endif
/* LABEL DECLARATION */
YYBEGINDECLARELABEL
  YYDECLARELABEL(yynewstate)
  YYDECLARELABEL(yybackup)
/* YYDECLARELABEL(yyresume) */
  YYDECLARELABEL(yydefault)
  YYDECLARELABEL(yyreduce)
  YYDECLARELABEL(yyerrlab)   /* here on detecting error */
  YYDECLARELABEL(yyerrlab1)   /* here on error raised explicitly by an action */
  YYDECLARELABEL(yyerrdefault)  /* current state does not do anything special for the error token. */
  YYDECLARELABEL(yyerrpop)   /* pop the current state because it cannot handle the error token */
  YYDECLARELABEL(yyerrhandle)  
YYENDDECLARELABEL
/* ALLOCA SIMULATION */
/* __HAVE_NO_ALLOCA */
#ifdef __HAVE_NO_ALLOCA
int __alloca_free_ptr(char *ptr,char *ref)
{if(ptr!=ref) free(ptr);
 return 0;}

#define __ALLOCA_alloca(size) malloc(size)
#define __ALLOCA_free(ptr,ref) __alloca_free_ptr((char *)ptr,(char *)ref)

#ifdef YY_FFParser_LSP_NEEDED
#define __ALLOCA_return(num) \
            return( __ALLOCA_free(yyss,yyssa)+\
		    __ALLOCA_free(yyvs,yyvsa)+\
		    __ALLOCA_free(yyls,yylsa)+\
		   (num))
#else
#define __ALLOCA_return(num) \
            return( __ALLOCA_free(yyss,yyssa)+\
		    __ALLOCA_free(yyvs,yyvsa)+\
		   (num))
#endif
#else
#define __ALLOCA_return(num) return(num)
#define __ALLOCA_alloca(size) alloca(size)
#define __ALLOCA_free(ptr,ref) 
#endif

/* ENDALLOCA SIMULATION */

#define yyerrok         (yyerrstatus = 0)
#define yyclearin       (YY_FFParser_CHAR = YYEMPTY)
#define YYEMPTY         -2
#define YYEOF           0
#define YYACCEPT        __ALLOCA_return(0)
#define YYABORT         __ALLOCA_return(1)
#define YYERROR         YYGOTO(yyerrlab1)
/* Like YYERROR except do call yyerror.
   This remains here temporarily to ease the
   transition to the new meaning of YYERROR, for GCC.
   Once GCC version 2 has supplanted version 1, this can go.  */
#define YYFAIL          YYGOTO(yyerrlab)
#define YYRECOVERING()  (!!yyerrstatus)
#define YYBACKUP(token, value) \
do                                                              \
  if (YY_FFParser_CHAR == YYEMPTY && yylen == 1)                               \
    { YY_FFParser_CHAR = (token), YY_FFParser_LVAL = (value);                 \
      yychar1 = YYTRANSLATE (YY_FFParser_CHAR);                                \
      YYPOPSTACK;                                               \
      YYGOTO(yybackup);                                            \
    }                                                           \
  else                                                          \
    { YY_FFParser_ERROR ("syntax error: cannot back up"); YYERROR; }   \
while (0)

#define YYTERROR        1
#define YYERRCODE       256

#ifndef YY_FFParser_PURE
/* UNPURE */
#define YYLEX           YY_FFParser_LEX()
#ifndef YY_USE_CLASS
/* If nonreentrant, and not class , generate the variables here */
int     YY_FFParser_CHAR;                      /*  the lookahead symbol        */
YY_FFParser_STYPE      YY_FFParser_LVAL;              /*  the semantic value of the */
				/*  lookahead symbol    */
int YY_FFParser_NERRS;                 /*  number of parse errors so far */
#ifdef YY_FFParser_LSP_NEEDED
YY_FFParser_LTYPE YY_FFParser_LLOC;   /*  location data for the lookahead     */
			/*  symbol                              */
#endif
#endif


#else
/* PURE */
#ifdef YY_FFParser_LSP_NEEDED
#define YYLEX           YY_FFParser_LEX(&YY_FFParser_LVAL, &YY_FFParser_LLOC)
#else
#define YYLEX           YY_FFParser_LEX(&YY_FFParser_LVAL)
#endif
#endif
#ifndef YY_USE_CLASS
#if YY_FFParser_DEBUG != 0
int YY_FFParser_DEBUG_FLAG;                    /*  nonzero means print parse trace     */
/* Since this is uninitialized, it does not stop multiple parsers
   from coexisting.  */
#endif
#endif



/*  YYINITDEPTH indicates the initial size of the parser's stacks       */

#ifndef YYINITDEPTH
#define YYINITDEPTH 200
#endif

/*  YYMAXDEPTH is the maximum size the stacks can grow to
    (effective only if the built-in stack extension method is used).  */

#if YYMAXDEPTH == 0
#undef YYMAXDEPTH
#endif

#ifndef YYMAXDEPTH
#define YYMAXDEPTH 10000
#endif


#if __GNUC__ > 1                /* GNU C and GNU C++ define this.  */
#define __yy_bcopy(FROM,TO,COUNT)       __builtin_memcpy(TO,FROM,COUNT)
#else                           /* not GNU C or C++ */

/* This is the most reliable way to avoid incompatibilities
   in available built-in functions on various systems.  */

#ifdef __cplusplus
static void __yy_bcopy (char *from, char *to, int count)
#else
#ifdef __STDC__
static void __yy_bcopy (char *from, char *to, int count)
#else
static void __yy_bcopy (from, to, count)
     char *from;
     char *to;
     int count;
#endif
#endif
{
  register char *f = from;
  register char *t = to;
  register int i = count;

  while (i-- > 0)
    *t++ = *f++;
}
#endif

int
#ifdef YY_USE_CLASS
 YY_FFParser_CLASS::
#endif
     YY_FFParser_PARSE(YY_FFParser_PARSE_PARAM)
#ifndef __STDC__
#ifndef __cplusplus
#ifndef YY_USE_CLASS
/* parameter definition without protypes */
YY_FFParser_PARSE_PARAM_DEF
#endif
#endif
#endif
{
  register int yystate;
  register int yyn;
  register short *yyssp;
  register YY_FFParser_STYPE *yyvsp;
  int yyerrstatus;      /*  number of tokens to shift before error messages enabled */
  int yychar1=0;          /*  lookahead token as an internal (translated) token number */

  short yyssa[YYINITDEPTH];     /*  the state stack                     */
  YY_FFParser_STYPE yyvsa[YYINITDEPTH];        /*  the semantic value stack            */

  short *yyss = yyssa;          /*  refer to the stacks thru separate pointers */
  YY_FFParser_STYPE *yyvs = yyvsa;     /*  to allow yyoverflow to reallocate them elsewhere */

#ifdef YY_FFParser_LSP_NEEDED
  YY_FFParser_LTYPE yylsa[YYINITDEPTH];        /*  the location stack                  */
  YY_FFParser_LTYPE *yyls = yylsa;
  YY_FFParser_LTYPE *yylsp;

#define YYPOPSTACK   (yyvsp--, yyssp--, yylsp--)
#else
#define YYPOPSTACK   (yyvsp--, yyssp--)
#endif

  int yystacksize = YYINITDEPTH;

#ifdef YY_FFParser_PURE
  int YY_FFParser_CHAR;
  YY_FFParser_STYPE YY_FFParser_LVAL;
  int YY_FFParser_NERRS;
#ifdef YY_FFParser_LSP_NEEDED
  YY_FFParser_LTYPE YY_FFParser_LLOC;
#endif
#endif

  YY_FFParser_STYPE yyval;             /*  the variable used to return         */
				/*  semantic values from the action     */
				/*  routines                            */

  int yylen;
/* start loop, in which YYGOTO may be used. */
YYBEGINGOTO

#if YY_FFParser_DEBUG != 0
  if (YY_FFParser_DEBUG_FLAG)
    fprintf(stderr, "Starting parse\n");
#endif
  yystate = 0;
  yyerrstatus = 0;
  YY_FFParser_NERRS = 0;
  YY_FFParser_CHAR = YYEMPTY;          /* Cause a token to be read.  */

  /* Initialize stack pointers.
     Waste one element of value and location stack
     so that they stay on the same level as the state stack.
     The wasted elements are never initialized.  */

  yyssp = yyss - 1;
  yyvsp = yyvs;
#ifdef YY_FFParser_LSP_NEEDED
  yylsp = yyls;
#endif

/* Push a new state, which is found in  yystate  .  */
/* In all cases, when you get here, the value and location stacks
   have just been pushed. so pushing a state here evens the stacks.  */
YYLABEL(yynewstate)

  *++yyssp = yystate;

  if (yyssp >= yyss + yystacksize - 1)
    {
      /* Give user a chance to reallocate the stack */
      /* Use copies of these so that the &'s don't force the real ones into memory. */
      YY_FFParser_STYPE *yyvs1 = yyvs;
      short *yyss1 = yyss;
#ifdef YY_FFParser_LSP_NEEDED
      YY_FFParser_LTYPE *yyls1 = yyls;
#endif

      /* Get the current used size of the three stacks, in elements.  */
      int size = yyssp - yyss + 1;

#ifdef yyoverflow
      /* Each stack pointer address is followed by the size of
	 the data in use in that stack, in bytes.  */
#ifdef YY_FFParser_LSP_NEEDED
      /* This used to be a conditional around just the two extra args,
	 but that might be undefined if yyoverflow is a macro.  */
      yyoverflow("parser stack overflow",
		 &yyss1, size * sizeof (*yyssp),
		 &yyvs1, size * sizeof (*yyvsp),
		 &yyls1, size * sizeof (*yylsp),
		 &yystacksize);
#else
      yyoverflow("parser stack overflow",
		 &yyss1, size * sizeof (*yyssp),
		 &yyvs1, size * sizeof (*yyvsp),
		 &yystacksize);
#endif

      yyss = yyss1; yyvs = yyvs1;
#ifdef YY_FFParser_LSP_NEEDED
      yyls = yyls1;
#endif
#else /* no yyoverflow */
      /* Extend the stack our own way.  */
      if (yystacksize >= YYMAXDEPTH)
	{
	  YY_FFParser_ERROR("parser stack overflow");
	  __ALLOCA_return(2);
	}
      yystacksize *= 2;
      if (yystacksize > YYMAXDEPTH)
	yystacksize = YYMAXDEPTH;
      yyss = (short *) __ALLOCA_alloca (yystacksize * sizeof (*yyssp));
      __yy_bcopy ((char *)yyss1, (char *)yyss, size * sizeof (*yyssp));
      __ALLOCA_free(yyss1,yyssa);
      yyvs = (YY_FFParser_STYPE *) __ALLOCA_alloca (yystacksize * sizeof (*yyvsp));
      __yy_bcopy ((char *)yyvs1, (char *)yyvs, size * sizeof (*yyvsp));
      __ALLOCA_free(yyvs1,yyvsa);
#ifdef YY_FFParser_LSP_NEEDED
      yyls = (YY_FFParser_LTYPE *) __ALLOCA_alloca (yystacksize * sizeof (*yylsp));
      __yy_bcopy ((char *)yyls1, (char *)yyls, size * sizeof (*yylsp));
      __ALLOCA_free(yyls1,yylsa);
#endif
#endif /* no yyoverflow */

      yyssp = yyss + size - 1;
      yyvsp = yyvs + size - 1;
#ifdef YY_FFParser_LSP_NEEDED
      yylsp = yyls + size - 1;
#endif

#if YY_FFParser_DEBUG != 0
      if (YY_FFParser_DEBUG_FLAG)
	fprintf(stderr, "Stack size increased to %d\n", yystacksize);
#endif

      if (yyssp >= yyss + yystacksize - 1)
	YYABORT;
    }

#if YY_FFParser_DEBUG != 0
  if (YY_FFParser_DEBUG_FLAG)
    fprintf(stderr, "Entering state %d\n", yystate);
#endif

  YYGOTO(yybackup);
YYLABEL(yybackup)

/* Do appropriate processing given the current state.  */
/* Read a lookahead token if we need one and don't already have one.  */
/* YYLABEL(yyresume) */

  /* First try to decide what to do without reference to lookahead token.  */

  yyn = yypact[yystate];
  if (yyn == YYFLAG)
    YYGOTO(yydefault);

  /* Not known => get a lookahead token if don't already have one.  */

  /* yychar is either YYEMPTY or YYEOF
     or a valid token in external form.  */

  if (YY_FFParser_CHAR == YYEMPTY)
    {
#if YY_FFParser_DEBUG != 0
      if (YY_FFParser_DEBUG_FLAG)
	fprintf(stderr, "Reading a token: ");
#endif
      YY_FFParser_CHAR = YYLEX;
    }

  /* Convert token to internal form (in yychar1) for indexing tables with */

  if (YY_FFParser_CHAR <= 0)           /* This means end of input. */
    {
      yychar1 = 0;
      YY_FFParser_CHAR = YYEOF;                /* Don't call YYLEX any more */

#if YY_FFParser_DEBUG != 0
      if (YY_FFParser_DEBUG_FLAG)
	fprintf(stderr, "Now at end of input.\n");
#endif
    }
  else
    {
      yychar1 = YYTRANSLATE(YY_FFParser_CHAR);

#if YY_FFParser_DEBUG != 0
      if (YY_FFParser_DEBUG_FLAG)
	{
	  fprintf (stderr, "Next token is %d (%s", YY_FFParser_CHAR, yytname[yychar1]);
	  /* Give the individual parser a way to print the precise meaning
	     of a token, for further debugging info.  */
#ifdef YYPRINT
	  YYPRINT (stderr, YY_FFParser_CHAR, YY_FFParser_LVAL);
#endif
	  fprintf (stderr, ")\n");
	}
#endif
    }

  yyn += yychar1;
  if (yyn < 0 || yyn > YYLAST || yycheck[yyn] != yychar1)
    YYGOTO(yydefault);

  yyn = yytable[yyn];

  /* yyn is what to do for this token type in this state.
     Negative => reduce, -yyn is rule number.
     Positive => shift, yyn is new state.
       New state is final state => don't bother to shift,
       just return success.
     0, or most negative number => error.  */

  if (yyn < 0)
    {
      if (yyn == YYFLAG)
	YYGOTO(yyerrlab);
      yyn = -yyn;
      YYGOTO(yyreduce);
    }
  else if (yyn == 0)
    YYGOTO(yyerrlab);

  if (yyn == YYFINAL)
    YYACCEPT;

  /* Shift the lookahead token.  */

#if YY_FFParser_DEBUG != 0
  if (YY_FFParser_DEBUG_FLAG)
    fprintf(stderr, "Shifting token %d (%s), ", YY_FFParser_CHAR, yytname[yychar1]);
#endif

  /* Discard the token being shifted unless it is eof.  */
  if (YY_FFParser_CHAR != YYEOF)
    YY_FFParser_CHAR = YYEMPTY;

  *++yyvsp = YY_FFParser_LVAL;
#ifdef YY_FFParser_LSP_NEEDED
  *++yylsp = YY_FFParser_LLOC;
#endif

  /* count tokens shifted since error; after three, turn off error status.  */
  if (yyerrstatus) yyerrstatus--;

  yystate = yyn;
  YYGOTO(yynewstate);

/* Do the default action for the current state.  */
YYLABEL(yydefault)

  yyn = yydefact[yystate];
  if (yyn == 0)
    YYGOTO(yyerrlab);

/* Do a reduction.  yyn is the number of a rule to reduce with.  */
YYLABEL(yyreduce)
  yylen = yyr2[yyn];
  if (yylen > 0)
    yyval = yyvsp[1-yylen]; /* implement default value of the action */

#if YY_FFParser_DEBUG != 0
  if (YY_FFParser_DEBUG_FLAG)
    {
      int i;

      fprintf (stderr, "Reducing via rule %d (line %d), ",
	       yyn, yyrline[yyn]);

      /* Print the symbols being reduced, and their result.  */
      for (i = yyprhs[yyn]; yyrhs[i] > 0; i++)
	fprintf (stderr, "%s ", yytname[yyrhs[i]]);
      fprintf (stderr, " -> %s\n", yytname[yyr1[yyn]]);
    }
#endif


/* #line 811 "/public/friedric/lib/bison.cc" */
#line 1240 "FFParser.cc"

  switch (yyn) {

case 2:
#line 90 "FFParser.y"
{
                YYABORT;
             ;
    break;}
case 3:
#line 97 "FFParser.y"
{
  if ((yyvsp[-5].text != NULL) && (strlen(yyvsp[-5].text) > 0) &&
      (parseFFGraph->getName() == ""))
    parseFFGraph->setName(String(yyvsp[-5].text));
 ;
    break;}
case 4:
#line 106 "FFParser.y"
{
  parseFFGraph->_setDirected(YES);
;
    break;}
case 5:
#line 110 "FFParser.y"
{
  parseFFGraph->_setDirected(YES);
;
    break;}
case 6:
#line 114 "FFParser.y"
{
  parseFFGraph->_setDirected(NO);
;
    break;}
case 11:
#line 128 "FFParser.y"
{
  int k;
  FFSet<int> tmpS;
  if((k= parseFFGraph->getGraphLabelIndex(String(yyvsp[-1].text),&tmpS)) >= 0)
    parseFFGraph->getGraphLabel(k)->parse(String(yyvsp[0].text));
  else if (!unmatchedLabels.contains(String(yyvsp[-1].text)))
    {
      cerr << "Graph has no Label of Type: " << yyvsp[-1].text << endl;
      unmatchedLabels.add(String(yyvsp[-1].text));
    }
;
    break;}
case 12:
#line 142 "FFParser.y"
{
  if(parseFFGraph->getWindow() != NULL)
    {
      parseFFGraph->getWindow()->setLabelOffset(yyvsp[-1].nr, yyvsp[0].nr);
    }
;
    break;}
case 13:
#line 149 "FFParser.y"
{
 FFCoord tmpC = yyvsp[0].nr/100.0;
 
 if(parseFFGraph->getWindow() != NULL)
  {
   parseFFGraph->getWindow()->scaleXYZ(tmpC, tmpC, tmpC);
  }
;
    break;}
case 14:
#line 158 "FFParser.y"
{
 FFCoord tmpC = yyvsp[0].nr/100.0;
 if(parseFFGraph->getWindow() != NULL)
  {
   parseFFGraph->getWindow()->rotateX(tmpC);
  }
;
    break;}
case 15:
#line 166 "FFParser.y"
{
 FFCoord tmpC = yyvsp[0].nr/100.0;
 if(parseFFGraph->getWindow() != NULL)
  {
   parseFFGraph->getWindow()->rotateY(tmpC);
  }
;
    break;}
case 16:
#line 174 "FFParser.y"
{
 FFCoord tmpC = yyvsp[0].nr/100.0;
 if(parseFFGraph->getWindow() != NULL)
  {
   parseFFGraph->getWindow()->rotateZ(tmpC);
  }
;
    break;}
case 23:
#line 192 "FFParser.y"
{
  current_node = 0;
;
    break;}
case 24:
#line 198 "FFParser.y"
{
  current_node =parseFFGraph->nodeWithValue(yyvsp[0].nr);
  
  if(current_node == NULL)
    {
      current_node =parseFFGraph->addNode();
      current_node->setValue(yyvsp[0].nr);
    }  
;
    break;}
case 25:
#line 210 "FFParser.y"
{
  if (yyvsp[0].text != NULL)
    {
      current_node->setName(String(yyvsp[0].text));
    }
;
    break;}
case 32:
#line 230 "FFParser.y"
{
  if((*current_node)() != NULL)
	  (*current_node)()->moveTo(FFPoint(yyvsp[-1].nr, yyvsp[0].nr));
;
    break;}
case 33:
#line 235 "FFParser.y"
{
  if((*current_node)() != NULL)
	  (*current_node)()->moveTo(FFPoint(yyvsp[-1].nr, yyvsp[0].nr));  
;
    break;}
case 34:
#line 240 "FFParser.y"
{
	parseFFGraph->setStartNode(current_node);
;
    break;}
case 35:
#line 244 "FFParser.y"
{
	parseFFGraph->addEndNode(current_node);
;
    break;}
case 36:
#line 248 "FFParser.y"
{
  if((*current_node)() != NULL)
	  (*current_node)()->moveTo(FFPoint(yyvsp[-2].nr, yyvsp[-1].nr, yyvsp[0].nr));  
;
    break;}
case 37:
#line 253 "FFParser.y"
{
  if((*current_node)() != NULL)
	  (*current_node)()->setColor(String(yyvsp[0].text));  
;
    break;}
case 38:
#line 259 "FFParser.y"
{
  int k;
  FFSet<int> tmpS;
  if((k= current_node->getLabelIndex(String(yyvsp[-1].text),&tmpS)) >= 0)
    current_node->getLabel(k)->parse(String(yyvsp[0].text));
  else if (!unmatchedLabels.contains(String(yyvsp[-1].text)))
    {
      cerr << "Node has no Label of Type: " << yyvsp[-1].text << endl;
      unmatchedLabels.add(String(yyvsp[-1].text));
    }
;
    break;}
case 39:
#line 273 "FFParser.y"
{
  if((*current_node)() != NULL)
    {
      (*current_node)()->changeToIcon(String(yyvsp[-1].text)); 
      (*current_node)()->setMask(String(yyvsp[0].text)); 
    }
;
    break;}
case 40:
#line 283 "FFParser.y"
{
 if((*current_node)() != NULL)
  {
   (*current_node)()->setRange(FFPoint(yyvsp[0].nr,yyvsp[0].nr,yyvsp[0].nr)); 
  }
;
    break;}
case 41:
#line 290 "FFParser.y"
{
  if((*current_node)() != NULL)
    (*current_node)()->changeToIcon(String(yyvsp[0].text)); 
;
    break;}
case 42:
#line 296 "FFParser.y"
{
  if((*current_node)() != NULL)
    {
      (*current_node)()->changeToIcon(String(yyvsp[-1].text), String(yyvsp[0].text)); 
    }
;
    break;}
case 43:
#line 305 "FFParser.y"
{
  if((*current_node)() != NULL)
    (*current_node)()->changeToIcon(String(yyvsp[0].text)); 
;
    break;}
case 44:
#line 311 "FFParser.y"
{
  if((*current_node)() != NULL)
    {
      (*current_node)()->setMask(String(yyvsp[-1].text), String(yyvsp[0].text)); 
    }
;
    break;}
case 45:
#line 320 "FFParser.y"
{
  if((*current_node)() != NULL)
    (*current_node)()->setMask(String(yyvsp[0].text)); 
;
    break;}
case 48:
#line 333 "FFParser.y"
{
  current_edge = 0;
;
    break;}
case 49:
#line 339 "FFParser.y"
{
  FFNodeClass *tmpNode= parseFFGraph->nodeWithValue(yyvsp[0].nr);
  
  if(tmpNode == NULL)
    {
      tmpNode = parseFFGraph->addNode();
      tmpNode->setValue(yyvsp[0].nr);
    }


  current_edge= parseFFGraph->addEdge(current_node, tmpNode);
;
    break;}
case 50:
#line 353 "FFParser.y"
{
  FFGraphicEdge *ge= (*current_edge)();
  if((ge != 0) && (ge->anzPoints() > 2))
    {
      ge->deletePointAt(ge->anzPoints()-1);
      ge->deletePointAt(ge->anzPoints()-1);
    }
  
  if (yyvsp[0].text != NULL)
   {
    current_edge->setName(String(yyvsp[0].text));  
   }
;
    break;}
case 58:
#line 381 "FFParser.y"
{
  if((*current_edge)() != NULL)
	  (*current_edge)()->setColor(String(yyvsp[0].text));  	
;
    break;}
case 59:
#line 387 "FFParser.y"
{
 current_edge->setWeight( ((double)yyvsp[0].nr)/100.0);
;
    break;}
case 60:
#line 391 "FFParser.y"
{
  FFSet<int> tmpS;
  int k=current_edge->getLabelIndex(String(yyvsp[-1].text), &tmpS);
  if(k >= 0)
      {
          current_edge->getLabel(k)->parse(String(yyvsp[0].text));
      }
  else if (!unmatchedLabels.contains(String(yyvsp[-1].text)))
    {
      cerr << "Edge has no Label of Type: " << yyvsp[-1].text << endl;
      unmatchedLabels.add(String(yyvsp[-1].text));
    }
;
    break;}
case 63:
#line 411 "FFParser.y"
{
  if((*current_edge)() != NULL)
	  (*current_edge)()->addPointAt(0, FFPoint(yyvsp[-2].nr, yyvsp[-1].nr));
;
    break;}
case 65:
#line 418 "FFParser.y"
{
  if((*current_edge)() != NULL)
	  (*current_edge)()->addPointAt(0, FFPoint(yyvsp[-3].nr, yyvsp[-2].nr, yyvsp[-1].nr));
;
    break;}
case 70:
#line 431 "FFParser.y"
{ /* borrowed token text */;
    break;}
}

#line 811 "/public/friedric/lib/bison.cc"
   /* the action file gets copied in in place of this dollarsign  */
  yyvsp -= yylen;
  yyssp -= yylen;
#ifdef YY_FFParser_LSP_NEEDED
  yylsp -= yylen;
#endif

#if YY_FFParser_DEBUG != 0
  if (YY_FFParser_DEBUG_FLAG)
    {
      short *ssp1 = yyss - 1;
      fprintf (stderr, "state stack now");
      while (ssp1 != yyssp)
	fprintf (stderr, " %d", *++ssp1);
      fprintf (stderr, "\n");
    }
#endif

  *++yyvsp = yyval;

#ifdef YY_FFParser_LSP_NEEDED
  yylsp++;
  if (yylen == 0)
    {
      yylsp->first_line = YY_FFParser_LLOC.first_line;
      yylsp->first_column = YY_FFParser_LLOC.first_column;
      yylsp->last_line = (yylsp-1)->last_line;
      yylsp->last_column = (yylsp-1)->last_column;
      yylsp->text = 0;
    }
  else
    {
      yylsp->last_line = (yylsp+yylen-1)->last_line;
      yylsp->last_column = (yylsp+yylen-1)->last_column;
    }
#endif

  /* Now "shift" the result of the reduction.
     Determine what state that goes to,
     based on the state we popped back to
     and the rule number reduced by.  */

  yyn = yyr1[yyn];

  yystate = yypgoto[yyn - YYNTBASE] + *yyssp;
  if (yystate >= 0 && yystate <= YYLAST && yycheck[yystate] == *yyssp)
    yystate = yytable[yystate];
  else
    yystate = yydefgoto[yyn - YYNTBASE];

  YYGOTO(yynewstate);

YYLABEL(yyerrlab)   /* here on detecting error */

  if (! yyerrstatus)
    /* If not already recovering from an error, report this error.  */
    {
      ++YY_FFParser_NERRS;

#ifdef YY_FFParser_ERROR_VERBOSE
      yyn = yypact[yystate];

      if (yyn > YYFLAG && yyn < YYLAST)
	{
	  int size = 0;
	  char *msg;
	  int x, count;

	  count = 0;
	  /* Start X at -yyn if nec to avoid negative indexes in yycheck.  */
	  for (x = (yyn < 0 ? -yyn : 0);
	       x < (sizeof(yytname) / sizeof(char *)); x++)
	    if (yycheck[x + yyn] == x)
	      size += strlen(yytname[x]) + 15, count++;
	  msg = (char *) malloc(size + 15);
	  if (msg != 0)
	    {
	      strcpy(msg, "parse error");

	      if (count < 5)
		{
		  count = 0;
		  for (x = (yyn < 0 ? -yyn : 0);
		       x < (sizeof(yytname) / sizeof(char *)); x++)
		    if (yycheck[x + yyn] == x)
		      {
			strcat(msg, count == 0 ? ", expecting `" : " or `");
			strcat(msg, yytname[x]);
			strcat(msg, "'");
			count++;
		      }
		}
	      YY_FFParser_ERROR(msg);
	      free(msg);
	    }
	  else
	    YY_FFParser_ERROR ("parse error; also virtual memory exceeded");
	}
      else
#endif /* YY_FFParser_ERROR_VERBOSE */
	YY_FFParser_ERROR("parse error");
    }

  YYGOTO(yyerrlab1);
YYLABEL(yyerrlab1)   /* here on error raised explicitly by an action */

  if (yyerrstatus == 3)
    {
      /* if just tried and failed to reuse lookahead token after an error, discard it.  */

      /* return failure if at end of input */
      if (YY_FFParser_CHAR == YYEOF)
	YYABORT;

#if YY_FFParser_DEBUG != 0
      if (YY_FFParser_DEBUG_FLAG)
	fprintf(stderr, "Discarding token %d (%s).\n", YY_FFParser_CHAR, yytname[yychar1]);
#endif

      YY_FFParser_CHAR = YYEMPTY;
    }

  /* Else will try to reuse lookahead token
     after shifting the error token.  */

  yyerrstatus = 3;              /* Each real token shifted decrements this */

  YYGOTO(yyerrhandle);

YYLABEL(yyerrdefault)  /* current state does not do anything special for the error token. */

#if 0
  /* This is wrong; only states that explicitly want error tokens
     should shift them.  */
  yyn = yydefact[yystate];  /* If its default is to accept any token, ok.  Otherwise pop it.*/
  if (yyn) YYGOTO(yydefault);
#endif

YYLABEL(yyerrpop)   /* pop the current state because it cannot handle the error token */

  if (yyssp == yyss) YYABORT;
  yyvsp--;
  yystate = *--yyssp;
#ifdef YY_FFParser_LSP_NEEDED
  yylsp--;
#endif

#if YY_FFParser_DEBUG != 0
  if (YY_FFParser_DEBUG_FLAG)
    {
      short *ssp1 = yyss - 1;
      fprintf (stderr, "Error: state stack now");
      while (ssp1 != yyssp)
	fprintf (stderr, " %d", *++ssp1);
      fprintf (stderr, "\n");
    }
#endif

YYLABEL(yyerrhandle)

  yyn = yypact[yystate];
  if (yyn == YYFLAG)
    YYGOTO(yyerrdefault);

  yyn += YYTERROR;
  if (yyn < 0 || yyn > YYLAST || yycheck[yyn] != YYTERROR)
    YYGOTO(yyerrdefault);

  yyn = yytable[yyn];
  if (yyn < 0)
    {
      if (yyn == YYFLAG)
	YYGOTO(yyerrpop);
      yyn = -yyn;
      YYGOTO(yyreduce);
    }
  else if (yyn == 0)
    YYGOTO(yyerrpop);

  if (yyn == YYFINAL)
    YYACCEPT;

#if YY_FFParser_DEBUG != 0
  if (YY_FFParser_DEBUG_FLAG)
    fprintf(stderr, "Shifting error token, ");
#endif

  *++yyvsp = YY_FFParser_LVAL;
#ifdef YY_FFParser_LSP_NEEDED
  *++yylsp = YY_FFParser_LLOC;
#endif

  yystate = yyn;
  YYGOTO(yynewstate);
/* end loop, in which YYGOTO may be used. */
  YYENDGOTO
}

/* END */

/* #line 1010 "/public/friedric/lib/bison.cc" */
#line 1787 "FFParser.cc"
#line 433 "FFParser.y"

//static	int	yyerror (char *string)
//{
//  fprintf (stderr, "error: %s\n", string);
//  return 0;
//}

//void load_graph (FFGraphClass *g)
//{
//  parseFFGraph = g;
//}
