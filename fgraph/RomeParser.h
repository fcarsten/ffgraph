#ifndef YY_RomeParser_h_included
#define YY_RomeParser_h_included

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
#line 21 "RomeParser.h"

#line 55 "RomeParser.y"
typedef union	{		/* stack for parser	*/
	int	   integer;
	double real;
	char*  str;
	char*  key;
} yy_RomeParser_stype;
#define YY_RomeParser_STYPE yy_RomeParser_stype
#line 62 "RomeParser.y"

  typedef YY_RomeParser_STYPE romeTokenValue;
#define YY_RomeParser_LEX  romeScan
#define YY_RomeParser_LEX_BODY  = 0
#define YY_RomeParser_ERROR_BODY  = 0
#define YY_RomeParser_ERROR  romePrintError
#define YY_RomeParser_LVAL  romeTheTokenValue
#define YY_RomeParser_PARSE  romeParse 
#define YY_RomeParser_MEMBERS  \
protected: \
FFNodeClass	*current_node; \
FFEdgeClass	*current_edge; \
FFGraphClass *parseFFGraph;\
AVLMap<int, FFNode> *gmlHash;

#line 14 "/cs/postgrad2/csse/friedric/lib/bison.h"
 /* %{ and %header{ and %union, during decl */
#ifndef YY_RomeParser_COMPATIBILITY
#ifndef YY_USE_CLASS
#define  YY_RomeParser_COMPATIBILITY 1
#else
#define  YY_RomeParser_COMPATIBILITY 0
#endif
#endif

#if YY_RomeParser_COMPATIBILITY != 0
/* backward compatibility */
#ifdef YYLTYPE
#ifndef YY_RomeParser_LTYPE
#define YY_RomeParser_LTYPE YYLTYPE
/* WARNING obsolete !!! user defined YYLTYPE not reported into generated header */
/* use %define LTYPE */
#endif
#endif
#ifdef YYSTYPE
#ifndef YY_RomeParser_STYPE 
#define YY_RomeParser_STYPE YYSTYPE
/* WARNING obsolete !!! user defined YYSTYPE not reported into generated header */
/* use %define STYPE */
#endif
#endif
#ifdef YYDEBUG
#ifndef YY_RomeParser_DEBUG
#define  YY_RomeParser_DEBUG YYDEBUG
/* WARNING obsolete !!! user defined YYDEBUG not reported into generated header */
/* use %define DEBUG */
#endif
#endif
#ifdef YY_RomeParser_STYPE
#ifndef yystype
#define yystype YY_RomeParser_STYPE
#endif
#endif
/* use goto to be compatible */
#ifndef YY_RomeParser_USE_GOTO
#define YY_RomeParser_USE_GOTO 1
#endif
#endif

/* use no goto to be clean in C++ */
#ifndef YY_RomeParser_USE_GOTO
#define YY_RomeParser_USE_GOTO 0
#endif

#ifndef YY_RomeParser_PURE

/* #line 63 "/cs/postgrad2/csse/friedric/lib/bison.h" */
#line 99 "RomeParser.h"

#line 63 "/cs/postgrad2/csse/friedric/lib/bison.h"
/* YY_RomeParser_PURE */
#endif

/* #line 65 "/cs/postgrad2/csse/friedric/lib/bison.h" */
#line 106 "RomeParser.h"

#line 65 "/cs/postgrad2/csse/friedric/lib/bison.h"
/* prefix */
#ifndef YY_RomeParser_DEBUG

/* #line 67 "/cs/postgrad2/csse/friedric/lib/bison.h" */
#line 113 "RomeParser.h"

#line 67 "/cs/postgrad2/csse/friedric/lib/bison.h"
/* YY_RomeParser_DEBUG */
#endif
#ifndef YY_RomeParser_LSP_NEEDED

/* #line 70 "/cs/postgrad2/csse/friedric/lib/bison.h" */
#line 121 "RomeParser.h"

#line 70 "/cs/postgrad2/csse/friedric/lib/bison.h"
 /* YY_RomeParser_LSP_NEEDED*/
#endif
/* DEFAULT LTYPE*/
#ifdef YY_RomeParser_LSP_NEEDED
#ifndef YY_RomeParser_LTYPE
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

#define YY_RomeParser_LTYPE yyltype
#endif
#endif
/* DEFAULT STYPE*/
#ifndef YY_RomeParser_STYPE
#define YY_RomeParser_STYPE int
#endif
/* DEFAULT MISCELANEOUS */
#ifndef YY_RomeParser_PARSE
#define YY_RomeParser_PARSE yyparse
#endif
#ifndef YY_RomeParser_LEX
#define YY_RomeParser_LEX yylex
#endif
#ifndef YY_RomeParser_LVAL
#define YY_RomeParser_LVAL yylval
#endif
#ifndef YY_RomeParser_LLOC
#define YY_RomeParser_LLOC yylloc
#endif
#ifndef YY_RomeParser_CHAR
#define YY_RomeParser_CHAR yychar
#endif
#ifndef YY_RomeParser_NERRS
#define YY_RomeParser_NERRS yynerrs
#endif
#ifndef YY_RomeParser_DEBUG_FLAG
#define YY_RomeParser_DEBUG_FLAG yydebug
#endif
#ifndef YY_RomeParser_ERROR
#define YY_RomeParser_ERROR yyerror
#endif

#ifndef YY_RomeParser_PARSE_PARAM
#ifndef __STDC__
#ifndef __cplusplus
#ifndef YY_USE_CLASS
#define YY_RomeParser_PARSE_PARAM
#ifndef YY_RomeParser_PARSE_PARAM_DEF
#define YY_RomeParser_PARSE_PARAM_DEF
#endif
#endif
#endif
#endif
#ifndef YY_RomeParser_PARSE_PARAM
#define YY_RomeParser_PARSE_PARAM void
#endif
#endif

/* TOKEN C */
#ifndef YY_USE_CLASS

#ifndef YY_RomeParser_PURE
extern YY_RomeParser_STYPE YY_RomeParser_LVAL;
#endif


/* #line 143 "/cs/postgrad2/csse/friedric/lib/bison.h" */
#line 199 "RomeParser.h"
#define	BEGIN_LIST	258
#define	END_LIST	259
#define	HASHMARK	260
#define	NEWLINE	261
#define	INTEGER	262
#define	REAL	263
#define	KEY	264
#define	STRING	265


#line 143 "/cs/postgrad2/csse/friedric/lib/bison.h"
 /* #defines token */
/* after #define tokens, before const tokens S5*/
#else
#ifndef YY_RomeParser_CLASS
#define YY_RomeParser_CLASS RomeParser
#endif

#ifndef YY_RomeParser_INHERIT
#define YY_RomeParser_INHERIT
#endif
#ifndef YY_RomeParser_MEMBERS
#define YY_RomeParser_MEMBERS 
#endif
#ifndef YY_RomeParser_LEX_BODY
#define YY_RomeParser_LEX_BODY  
#endif
#ifndef YY_RomeParser_ERROR_BODY
#define YY_RomeParser_ERROR_BODY  
#endif
#ifndef YY_RomeParser_CONSTRUCTOR_PARAM
#define YY_RomeParser_CONSTRUCTOR_PARAM
#endif
/* choose between enum and const */
#ifndef YY_RomeParser_USE_CONST_TOKEN
#define YY_RomeParser_USE_CONST_TOKEN 0
/* yes enum is more compatible with flex,  */
/* so by default we use it */
#endif
#if YY_RomeParser_USE_CONST_TOKEN != 0
#ifndef YY_RomeParser_ENUM_TOKEN
#define YY_RomeParser_ENUM_TOKEN yy_RomeParser_enum_token
#endif
#endif

class YY_RomeParser_CLASS YY_RomeParser_INHERIT
{
public:
#if YY_RomeParser_USE_CONST_TOKEN != 0
/* static const int token ... */

/* #line 182 "/cs/postgrad2/csse/friedric/lib/bison.h" */
#line 252 "RomeParser.h"
static const int BEGIN_LIST;
static const int END_LIST;
static const int HASHMARK;
static const int NEWLINE;
static const int INTEGER;
static const int REAL;
static const int KEY;
static const int STRING;


#line 182 "/cs/postgrad2/csse/friedric/lib/bison.h"
 /* decl const */
#else
enum YY_RomeParser_ENUM_TOKEN { YY_RomeParser_NULL_TOKEN=0

/* #line 185 "/cs/postgrad2/csse/friedric/lib/bison.h" */
#line 269 "RomeParser.h"
	,BEGIN_LIST=258
	,END_LIST=259
	,HASHMARK=260
	,NEWLINE=261
	,INTEGER=262
	,REAL=263
	,KEY=264
	,STRING=265


#line 185 "/cs/postgrad2/csse/friedric/lib/bison.h"
 /* enum token */
     }; /* end of enum declaration */
#endif
public:
 int YY_RomeParser_PARSE(YY_RomeParser_PARSE_PARAM);
 virtual void YY_RomeParser_ERROR(char *msg) YY_RomeParser_ERROR_BODY;
#ifdef YY_RomeParser_PURE
#ifdef YY_RomeParser_LSP_NEEDED
 virtual int  YY_RomeParser_LEX(YY_RomeParser_STYPE *YY_RomeParser_LVAL,YY_RomeParser_LTYPE *YY_RomeParser_LLOC) YY_RomeParser_LEX_BODY;
#else
 virtual int  YY_RomeParser_LEX(YY_RomeParser_STYPE *YY_RomeParser_LVAL) YY_RomeParser_LEX_BODY;
#endif
#else
 virtual int YY_RomeParser_LEX() YY_RomeParser_LEX_BODY;
 YY_RomeParser_STYPE YY_RomeParser_LVAL;
#ifdef YY_RomeParser_LSP_NEEDED
 YY_RomeParser_LTYPE YY_RomeParser_LLOC;
#endif
 int YY_RomeParser_NERRS;
 int YY_RomeParser_CHAR;
#endif
#if YY_RomeParser_DEBUG != 0
public:
 int YY_RomeParser_DEBUG_FLAG;	/*  nonzero means print parse trace	*/
#endif
public:
 YY_RomeParser_CLASS(YY_RomeParser_CONSTRUCTOR_PARAM);
public:
 YY_RomeParser_MEMBERS 
};
/* other declare folow */
#endif


#if YY_RomeParser_COMPATIBILITY != 0
/* backward compatibility */
#ifndef YYSTYPE
#define YYSTYPE YY_RomeParser_STYPE
#endif

#ifndef YYLTYPE
#define YYLTYPE YY_RomeParser_LTYPE
#endif
#ifndef YYDEBUG
#ifdef YY_RomeParser_DEBUG 
#define YYDEBUG YY_RomeParser_DEBUG
#endif
#endif

#endif
/* END */

/* #line 236 "/cs/postgrad2/csse/friedric/lib/bison.h" */
#line 334 "RomeParser.h"
#endif
