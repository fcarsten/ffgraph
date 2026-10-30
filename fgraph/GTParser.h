#ifndef YY_GTParser_h_included
#define YY_GTParser_h_included

#line 1 "/cs/postgrad2/csse/friedric/lib/bison.h"
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

/* #line 14 "/cs/postgrad2/csse/friedric/lib/bison.h" */
#line 21 "GTParser.h"

#line 55 "GTParser.y"
typedef union	{		/* stack for parser	*/
	int	   integer;
	double real;
	char*  str;
	char*  key;
} yy_GTParser_stype;
#define YY_GTParser_STYPE yy_GTParser_stype
#line 62 "GTParser.y"

  typedef YY_GTParser_STYPE gtTokenValue;
#define YY_GTParser_LEX  gtScan
#define YY_GTParser_LEX_BODY  = 0
#define YY_GTParser_ERROR_BODY  = 0
#define YY_GTParser_ERROR  gtPrintError
#define YY_GTParser_LVAL  gtTheTokenValue
#define YY_GTParser_PARSE  gtParse 
#define YY_GTParser_MEMBERS  \
protected: \
FFNodeClass	*current_node; \
FFEdgeClass	*current_edge; \
FFGraphClass *parseFFGraph;\
AVLMap<int, FFNode> *gmlHash;

#line 14 "/cs/postgrad2/csse/friedric/lib/bison.h"
 /* %{ and %header{ and %union, during decl */
#ifndef YY_GTParser_COMPATIBILITY
#ifndef YY_USE_CLASS
#define  YY_GTParser_COMPATIBILITY 1
#else
#define  YY_GTParser_COMPATIBILITY 0
#endif
#endif

#if YY_GTParser_COMPATIBILITY != 0
/* backward compatibility */
#ifdef YYLTYPE
#ifndef YY_GTParser_LTYPE
#define YY_GTParser_LTYPE YYLTYPE
/* WARNING obsolete !!! user defined YYLTYPE not reported into generated header */
/* use %define LTYPE */
#endif
#endif
#ifdef YYSTYPE
#ifndef YY_GTParser_STYPE 
#define YY_GTParser_STYPE YYSTYPE
/* WARNING obsolete !!! user defined YYSTYPE not reported into generated header */
/* use %define STYPE */
#endif
#endif
#ifdef YYDEBUG
#ifndef YY_GTParser_DEBUG
#define  YY_GTParser_DEBUG YYDEBUG
/* WARNING obsolete !!! user defined YYDEBUG not reported into generated header */
/* use %define DEBUG */
#endif
#endif
#ifdef YY_GTParser_STYPE
#ifndef yystype
#define yystype YY_GTParser_STYPE
#endif
#endif
/* use goto to be compatible */
#ifndef YY_GTParser_USE_GOTO
#define YY_GTParser_USE_GOTO 1
#endif
#endif

/* use no goto to be clean in C++ */
#ifndef YY_GTParser_USE_GOTO
#define YY_GTParser_USE_GOTO 0
#endif

#ifndef YY_GTParser_PURE

/* #line 63 "/cs/postgrad2/csse/friedric/lib/bison.h" */
#line 99 "GTParser.h"

#line 63 "/cs/postgrad2/csse/friedric/lib/bison.h"
/* YY_GTParser_PURE */
#endif

/* #line 65 "/cs/postgrad2/csse/friedric/lib/bison.h" */
#line 106 "GTParser.h"

#line 65 "/cs/postgrad2/csse/friedric/lib/bison.h"
/* prefix */
#ifndef YY_GTParser_DEBUG

/* #line 67 "/cs/postgrad2/csse/friedric/lib/bison.h" */
#line 113 "GTParser.h"

#line 67 "/cs/postgrad2/csse/friedric/lib/bison.h"
/* YY_GTParser_DEBUG */
#endif
#ifndef YY_GTParser_LSP_NEEDED

/* #line 70 "/cs/postgrad2/csse/friedric/lib/bison.h" */
#line 121 "GTParser.h"

#line 70 "/cs/postgrad2/csse/friedric/lib/bison.h"
 /* YY_GTParser_LSP_NEEDED*/
#endif
/* DEFAULT LTYPE*/
#ifdef YY_GTParser_LSP_NEEDED
#ifndef YY_GTParser_LTYPE
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

#define YY_GTParser_LTYPE yyltype
#endif
#endif
/* DEFAULT STYPE*/
#ifndef YY_GTParser_STYPE
#define YY_GTParser_STYPE int
#endif
/* DEFAULT MISCELANEOUS */
#ifndef YY_GTParser_PARSE
#define YY_GTParser_PARSE yyparse
#endif
#ifndef YY_GTParser_LEX
#define YY_GTParser_LEX yylex
#endif
#ifndef YY_GTParser_LVAL
#define YY_GTParser_LVAL yylval
#endif
#ifndef YY_GTParser_LLOC
#define YY_GTParser_LLOC yylloc
#endif
#ifndef YY_GTParser_CHAR
#define YY_GTParser_CHAR yychar
#endif
#ifndef YY_GTParser_NERRS
#define YY_GTParser_NERRS yynerrs
#endif
#ifndef YY_GTParser_DEBUG_FLAG
#define YY_GTParser_DEBUG_FLAG yydebug
#endif
#ifndef YY_GTParser_ERROR
#define YY_GTParser_ERROR yyerror
#endif

#ifndef YY_GTParser_PARSE_PARAM
#ifndef __STDC__
#ifndef __cplusplus
#ifndef YY_USE_CLASS
#define YY_GTParser_PARSE_PARAM
#ifndef YY_GTParser_PARSE_PARAM_DEF
#define YY_GTParser_PARSE_PARAM_DEF
#endif
#endif
#endif
#endif
#ifndef YY_GTParser_PARSE_PARAM
#define YY_GTParser_PARSE_PARAM void
#endif
#endif

/* TOKEN C */
#ifndef YY_USE_CLASS

#ifndef YY_GTParser_PURE
extern YY_GTParser_STYPE YY_GTParser_LVAL;
#endif


/* #line 143 "/cs/postgrad2/csse/friedric/lib/bison.h" */
#line 199 "GTParser.h"
#define	BEGIN_LIST	258
#define	END_LIST	259
#define	INTEGER	260
#define	REAL	261
#define	KEY	262
#define	STRING	263


#line 143 "/cs/postgrad2/csse/friedric/lib/bison.h"
 /* #defines token */
/* after #define tokens, before const tokens S5*/
#else
#ifndef YY_GTParser_CLASS
#define YY_GTParser_CLASS GTParser
#endif

#ifndef YY_GTParser_INHERIT
#define YY_GTParser_INHERIT
#endif
#ifndef YY_GTParser_MEMBERS
#define YY_GTParser_MEMBERS 
#endif
#ifndef YY_GTParser_LEX_BODY
#define YY_GTParser_LEX_BODY  
#endif
#ifndef YY_GTParser_ERROR_BODY
#define YY_GTParser_ERROR_BODY  
#endif
#ifndef YY_GTParser_CONSTRUCTOR_PARAM
#define YY_GTParser_CONSTRUCTOR_PARAM
#endif
/* choose between enum and const */
#ifndef YY_GTParser_USE_CONST_TOKEN
#define YY_GTParser_USE_CONST_TOKEN 0
/* yes enum is more compatible with flex,  */
/* so by default we use it */
#endif
#if YY_GTParser_USE_CONST_TOKEN != 0
#ifndef YY_GTParser_ENUM_TOKEN
#define YY_GTParser_ENUM_TOKEN yy_GTParser_enum_token
#endif
#endif

class YY_GTParser_CLASS YY_GTParser_INHERIT
{
public:
#if YY_GTParser_USE_CONST_TOKEN != 0
/* static const int token ... */

/* #line 182 "/cs/postgrad2/csse/friedric/lib/bison.h" */
#line 250 "GTParser.h"
static const int BEGIN_LIST;
static const int END_LIST;
static const int INTEGER;
static const int REAL;
static const int KEY;
static const int STRING;


#line 182 "/cs/postgrad2/csse/friedric/lib/bison.h"
 /* decl const */
#else
enum YY_GTParser_ENUM_TOKEN { YY_GTParser_NULL_TOKEN=0

/* #line 185 "/cs/postgrad2/csse/friedric/lib/bison.h" */
#line 265 "GTParser.h"
	,BEGIN_LIST=258
	,END_LIST=259
	,INTEGER=260
	,REAL=261
	,KEY=262
	,STRING=263


#line 185 "/cs/postgrad2/csse/friedric/lib/bison.h"
 /* enum token */
     }; /* end of enum declaration */
#endif
public:
 int YY_GTParser_PARSE(YY_GTParser_PARSE_PARAM);
 virtual void YY_GTParser_ERROR(char *msg) YY_GTParser_ERROR_BODY;
#ifdef YY_GTParser_PURE
#ifdef YY_GTParser_LSP_NEEDED
 virtual int  YY_GTParser_LEX(YY_GTParser_STYPE *YY_GTParser_LVAL,YY_GTParser_LTYPE *YY_GTParser_LLOC) YY_GTParser_LEX_BODY;
#else
 virtual int  YY_GTParser_LEX(YY_GTParser_STYPE *YY_GTParser_LVAL) YY_GTParser_LEX_BODY;
#endif
#else
 virtual int YY_GTParser_LEX() YY_GTParser_LEX_BODY;
 YY_GTParser_STYPE YY_GTParser_LVAL;
#ifdef YY_GTParser_LSP_NEEDED
 YY_GTParser_LTYPE YY_GTParser_LLOC;
#endif
 int YY_GTParser_NERRS;
 int YY_GTParser_CHAR;
#endif
#if YY_GTParser_DEBUG != 0
public:
 int YY_GTParser_DEBUG_FLAG;	/*  nonzero means print parse trace	*/
#endif
public:
 YY_GTParser_CLASS(YY_GTParser_CONSTRUCTOR_PARAM);
public:
 YY_GTParser_MEMBERS 
};
/* other declare folow */
#endif


#if YY_GTParser_COMPATIBILITY != 0
/* backward compatibility */
#ifndef YYSTYPE
#define YYSTYPE YY_GTParser_STYPE
#endif

#ifndef YYLTYPE
#define YYLTYPE YY_GTParser_LTYPE
#endif
#ifndef YYDEBUG
#ifdef YY_GTParser_DEBUG 
#define YYDEBUG YY_GTParser_DEBUG
#endif
#endif

#endif
/* END */

/* #line 236 "/cs/postgrad2/csse/friedric/lib/bison.h" */
#line 328 "GTParser.h"
#endif
