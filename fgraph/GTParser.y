/*
// This file is based on the parser from the GraphLet project
//
// $Author: friedric $
// $Date: 1996/05/06 14:17:43 $
// $RCSfile: GTParser.y,v $
// $Revision: 1.5 $
// $State: Exp $
//
*/

/* (C) Universitaet Passau 1986-1991 */

%name GTParser

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
  typedef YY_GTParser_STYPE gtTokenValue;
%}

%define LEX gtScan
%define LEX_BODY = 0
%define ERROR_BODY = 0
%define ERROR gtPrintError
%define LVAL gtTheTokenValue
%define PARSE gtParse 
%define MEMBERS \
protected: \
FFNodeClass	*current_node; \
FFEdgeClass	*current_edge; \
FFGraphClass *parseFFGraph;\
AVLMap<int, FFNode> *gmlHash;


%token BEGIN_LIST END_LIST
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

Attr:   KEY INTEGER
{
 String theAttr($1);
 int theId = $2;
 
 if(theAttr == "id")
  {
   if(current_edge != NULL)
    current_edge->setValue($2); 
   else if(current_node != NULL)
    {
     current_node->setValue($2);
     (*gmlHash)[theId] = current_node;
    }
  }
 if(theAttr == "x") {
  if(current_edge != NULL);
   else if((current_node != NULL) && 
           (current_node->getGraphicNode()))
    (*current_node)->moveTo(FFPoint($2 , (*current_node)->gety()));
  }
 if(theAttr == "y") {
  if(current_edge != NULL); 
  else if((current_node != NULL) && 
          (current_node->getGraphicNode()))
   (*current_node)->moveTo(FFPoint((*current_node)->getx(),$2 ));
  }
 if(theAttr == "z") {
  if(current_edge != NULL); 
   else if((current_node != NULL) && 
           (current_node->getGraphicNode()))
    (*current_node)->moveTo(FFPoint((*current_node)->getx(), 
                                    (*current_node)->gety(),
                                    $2 ));
 }
 if(theAttr == "source")
  {
   if(current_edge != NULL)
    {
     int srcId = $2;
     if(current_edge->source()->getValue() != srcId)
      {
       if(current_node->getValue() != srcId)
        {
         if((*gmlHash)[srcId] != NULL)
          current_node = (*gmlHash)[srcId];
         else
          current_node = parseFFGraph->nodeWithValue(srcId);
        }
       if(current_node == NULL)
        cerr << "Couldn't find source node" << endl;
       else
        current_edge->setSource(current_node);
      }
    }
  }
 if(theAttr == "target")
  {
   if(current_edge != NULL)
    {
     int targetId = $2;
     if(current_edge->target()->getValue() != targetId)
      {
       if((*gmlHash)[targetId] != NULL)
        current_edge->setTarget((*gmlHash)[targetId]);
       else
        current_edge->setTarget(parseFFGraph->nodeWithValue(targetId));
      }
    }
  }
}

|
KEY REAL
{
 String theAttr($1);
 
 if(theAttr == "x") {
  if(current_edge != NULL);
   else if((current_node != NULL) && 
           (current_node->getGraphicNode()))
    (*current_node)->moveTo(FFPoint($2 , (*current_node)->gety()));
  }
 if(theAttr == "y") {
  if(current_edge != NULL); 
  else if((current_node != NULL) && 
          (current_node->getGraphicNode()))
   (*current_node)->moveTo(FFPoint((*current_node)->getx(),$2 ));
  }
 if(theAttr == "z") {
  if(current_edge != NULL); 
   else if((current_node != NULL) && 
           (current_node->getGraphicNode()))
    (*current_node)->moveTo(FFPoint((*current_node)->getx(), 
                                    (*current_node)->gety(),
                                    $2 ));
 }
}
|
KEY STRING
{
 String theAttr($1);
 if(theAttr == "label")
  {
   if(current_edge != NULL)
    current_edge->setName($2);
   else if(current_node != NULL)
    current_node->setName($2);
   else
    parseFFGraph->setName($2);
  }
}
|
KEY BEGIN_LIST
{
 String theAttr($1);
 if(theAttr == "node")
  {
   current_edge = NULL;
   current_node = parseFFGraph->addNode();
  }
 else if(theAttr == "edge")
  {
   if(current_node == NULL)
    cerr << "Consistency Error" << endl;
   else
    current_edge = parseFFGraph->addEdge(current_node, current_node);
  }
 else if(theAttr == "graph")
  {
   current_edge = NULL;
   current_node = NULL;
  }
}
List
END_LIST
{
}

;
%%
