/*
// This file is based on the parser from the GraphLet project
//
// $Author: friedric $
// $Date: 1996/05/06 14:17:43 $
// $RCSfile: RomeParser.y,v $
// $Revision: 1.5 $
// $State: Exp $
//
*/

/* (C) Universitaet Passau 1986-1991 */

%name RomeParser

%{
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
#include <container/AVLMap.h>

#include <ffgraph/FFNodeLabel.h>
#include <ffgraph/FFGraphLabel.h>
#include <ffgraph/FFEdgeLabel.h>

// extern AVLSet<String> unmatchedLabels;

  /* Stacksize for yacc	*/

#define	YYMAXDEPTH 1000000

  /* Macro to abort yacc	*/

#define	yacc_error(s)	{ yyerror(s); YYABORT; }

  %}

%union	{		/* stack for parser	*/
	int	   integer;
	double real;
	char*  str;
	char*  key;
}
	
%header{
  typedef YY_RomeParser_STYPE romeTokenValue;
%}

%define LEX romeScan
%define LEX_BODY = 0
%define ERROR_BODY = 0
%define ERROR romePrintError
%define LVAL romeTheTokenValue
%define PARSE romeParse 
%define MEMBERS \
protected: \
FFNodeClass	*current_node; \
FFEdgeClass	*current_edge; \
FFGraphClass *parseFFGraph;\
AVLMap<int, FFNode> *gmlHash;


%token BEGIN_LIST END_LIST
%token HASHMARK
%token NEWLINE
%token <integer> INTEGER
%token <real>    REAL
%token <key>     KEY
%token <str>     STRING

/* %type <attr> Attr */
/* %type <list> List */

%start		parser_start

%%

parser_start:  
{
}
List
{
}
| error
{
 /* Syntax Error	*/
 YYABORT;
}
;

List:   Attr List | ;

Attr:   INTEGER INTEGER INTEGER INTEGER NEWLINE
{
 FFNode s=NULL,t=NULL;

 if((*gmlHash)[$3] != NULL)
  s = (*gmlHash)[$3];
 else
  s = parseFFGraph->nodeWithValue($3);

 if((*gmlHash)[$4] != NULL)
  t = (*gmlHash)[$4];
 else
  t = parseFFGraph->nodeWithValue($4);
 
 if( (s==NULL) || (t==NULL))
  cerr << "Couldn't find nodes for edge" << endl;
 else
  parseFFGraph->addEdge(s,t)->setValue($1);
}
|
INTEGER INTEGER NEWLINE
{
 parseFFGraph->addNode()->setValue($1);
}
|
HASHMARK NEWLINE
{
}
|
NEWLINE
{
}

;
%%
