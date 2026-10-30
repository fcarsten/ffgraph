/*
// $Author: deepthou $
// $Date: 1996/10/26 12:44:36 $
// $RCSfile: FFParser.y,v $
// $Revision: 1.6 $
// $State: Exp $
//
*/

/* (C) Universitaet Passau 1986-1991 */

%name FFParser

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

#include <ffgraph/FFNodeLabel.h>
#include <ffgraph/FFGraphLabel.h>
#include <ffgraph/FFEdgeLabel.h>

extern AVLSet<String> unmatchedLabels;

  /* Stacksize for yacc	*/

#define	YYMAXDEPTH 2000

  /* Macro to abort yacc	*/

#define	yacc_error(s)	{ yyerror(s); YYABORT; }

  %}

%union	{		/* stack for parser	*/
  int	nr;			/* Zahl			*/
  char	*text;			/* Label		*/
}
	
%header{
  typedef YY_FFParser_STYPE tokenValue;
%}

%define LEX Scan
%define LEX_BODY = 0
%define ERROR_BODY = 0
%define ERROR PrintError
%define LVAL theTokenValue
%define PARSE iParse 
%define MEMBERS \
protected: \
FFNodeClass	*current_node; \
FFEdgeClass	*current_edge; \
FFGraphClass *parseFFGraph;


%token		GRAPH '=' ';' END GRAPHS
%token		DIRECTED UNDIRECTED
%token		BEGIN_GRAPHED_INTERNALS END_GRAPHED_INTERNALS
%token		NODEPLACE EDGEPLACE UNKNOWN_INTERNAL FF_COLOR FF_LABEL FF_ICON 
%token          FF_COORD FF_NEW_ICON FF_NEW_MASK FF_RADIUS FF_WEIGHT
%token		STARTNODE ENDNODE FF_LABELOFFSET FF_SCALE FF_ROTATE_X 
%token		FF_ROTATE_Y FF_ROTATE_Z
%token	<nr>	NUMBER
%token	<text>	IDENTIFIER

%start		parser_start

%%

parser_start : graph
             | error
             {
                YYABORT;
             }
             ;


graph :	GRAPH IDENTIFIER '=' directedness graph_internals list_of_nodes END
 {
  if (($2 != NULL) && (strlen($2) > 0) &&
      (parseFFGraph->getName() == ""))
    parseFFGraph->setName(String($2));
  free($2); 
 }
 ;

directedness :
{
  parseFFGraph->_setDirected(YES);
}
| DIRECTED
{
  parseFFGraph->_setDirected(YES);
}
| UNDIRECTED
{
  parseFFGraph->_setDirected(NO);
}
 ;

graph_internals :		
| BEGIN_GRAPHED_INTERNALS ffGraphArgList END_GRAPHED_INTERNALS
 ;

ffGraphArgList : 
| ffGraphArg ffGraphArgList
 ;

ffGraphArg : FF_LABEL IDENTIFIER IDENTIFIER
{
  int k;
  FFSet<int> tmpS;
  if((k= parseFFGraph->getGraphLabelIndex(String($2),&tmpS)) >= 0)
    parseFFGraph->getGraphLabel(k)->parse(String($3));
  else if (!unmatchedLabels.contains(String($2)))
    {
      cerr << "Graph has no Label of Type: " << $2 << endl;
      unmatchedLabels.add(String($2));
    }
  free($2);
  free($3);
}
| FF_LABELOFFSET NUMBER NUMBER 
{
  if(parseFFGraph->getWindow() != NULL)
    {
      parseFFGraph->getWindow()->setLabelOffset($2, $3);
    }
}
| FF_SCALE NUMBER
{
 FFCoord tmpC = $2/100.0;
 
 if(parseFFGraph->getWindow() != NULL)
  {
   parseFFGraph->getWindow()->scaleXYZ(tmpC, tmpC, tmpC);
  }
}
| FF_ROTATE_X NUMBER
{
 FFCoord tmpC = $2/100.0;
 if(parseFFGraph->getWindow() != NULL)
  {
   parseFFGraph->getWindow()->rotateX(tmpC);
  }
}
| FF_ROTATE_Y NUMBER
{
 FFCoord tmpC = $2/100.0;
 if(parseFFGraph->getWindow() != NULL)
  {
   parseFFGraph->getWindow()->rotateY(tmpC);
  }
}
| FF_ROTATE_Z NUMBER
{
 FFCoord tmpC = $2/100.0;
 if(parseFFGraph->getWindow() != NULL)
  {
   parseFFGraph->getWindow()->rotateZ(tmpC);
  }
}
| NODEPLACE NUMBER NUMBER
| numbers_and_strings ';'
| UNKNOWN_INTERNAL unknown_internal
| ','
 ;

list_of_nodes :			
| node list_of_nodes
 ;

node : node_nr node_internals nodelabel list_of_edges
{
  current_node = NULL;
}
 ;

node_nr : NUMBER
{
  current_node =parseFFGraph->nodeWithValue($1);
  
  if(current_node == NULL)
    {
      current_node =parseFFGraph->addNode();
      current_node->setValue($1);
    }  
}
 ;

nodelabel : IDENTIFIER
{
  if ($1 != NULL)
    {
      current_node->setName(String($1));
      free($1);
    }
}
 ;

node_internals :		
| BEGIN_GRAPHED_INTERNALS ffNodeArgList END_GRAPHED_INTERNALS
 ;

ffNodeArgList : 
| ffNodeArgList ffNodeArg 
| ffNodeArg 
 ;

ffNodeArg : UNKNOWN_INTERNAL unknown_internal
| NUMBER NUMBER
{
  if((*current_node)() != NULL)
	  (*current_node)()->moveTo(FFPoint($1, $2));
}
| NODEPLACE NUMBER NUMBER
{
  if((*current_node)() != NULL)
	  (*current_node)()->moveTo(FFPoint($2, $3));  
}
| STARTNODE
{
	parseFFGraph->setStartNode(current_node);
}
| ENDNODE
{
	parseFFGraph->addEndNode(current_node);
}
| FF_COORD NUMBER NUMBER NUMBER
{
  if((*current_node)() != NULL)
	  (*current_node)()->moveTo(FFPoint($2, $3, $4));  
}
| FF_COLOR IDENTIFIER
{
  if((*current_node)() != NULL)
	  (*current_node)()->setColor(String($2));  
  free($2);
}
| FF_LABEL IDENTIFIER IDENTIFIER
{
  int k;
  FFSet<int> tmpS;
  if((k= current_node->getLabelIndex(String($2),&tmpS)) >= 0)
    current_node->getLabel(k)->parse(String($3));
  else if (!unmatchedLabels.contains(String($2)))
    {
      cerr << "Node has no Label of Type: " << $2 << endl;
      unmatchedLabels.add(String($2));
    }
  free($2);
  free($3);
}
| FF_ICON IDENTIFIER IDENTIFIER
{
  if((*current_node)() != NULL)
    {
      (*current_node)()->changeToIcon(String($2)); 
      (*current_node)()->setMask(String($3)); 
    }
  free($2);
  free($3);
}	
| FF_RADIUS NUMBER
{
 if((*current_node)() != NULL)
  {
   (*current_node)()->setRange(FFPoint($2,$2,$2)); 
  }
}
| FF_ICON IDENTIFIER 
{
  if((*current_node)() != NULL)
    (*current_node)()->changeToIcon(String($2)); 
  free($2);
}	
| FF_NEW_ICON IDENTIFIER IDENTIFIER
{
  if((*current_node)() != NULL)
    {
      (*current_node)()->changeToIcon(String($2), String($3)); 
    }
  free($2);
  free($3);
}	
| FF_NEW_ICON IDENTIFIER 
{
  if((*current_node)() != NULL)
    (*current_node)()->changeToIcon(String($2)); 
  free($2);
}	
| FF_NEW_MASK IDENTIFIER IDENTIFIER
{
  if((*current_node)() != NULL)
    {
      (*current_node)()->setMask(String($2), String($3)); 
    }
  free($2);
  free($3);
}	
| FF_NEW_MASK IDENTIFIER 
{
  if((*current_node)() != NULL)
    (*current_node)()->setMask(String($2)); 
  free($2);
}	
 ;


list_of_edges : ';'
| edge list_of_edges
 ;

edge : targetnode_nr edge_internals edgelabel
{
  current_edge = NULL;
}
 ;

targetnode_nr : NUMBER
{
  FFNodeClass *tmpNode= parseFFGraph->nodeWithValue($1);
  
  if(tmpNode == NULL)
    {
      tmpNode = parseFFGraph->addNode();
      tmpNode->setValue($1);
    }


  current_edge= parseFFGraph->addEdge(current_node, tmpNode);
};

edgelabel : IDENTIFIER
{
  FFGraphicEdge *ge= (*current_edge)();
  if((ge != 0) && (ge->anzPoints() > 2))
    {
      ge->deletePointAt(ge->anzPoints()-1);
      ge->deletePointAt(ge->anzPoints()-1);
    }
  
  if ($1 != NULL)
   {
    current_edge->setName(String($1));  
    free($1);
   }
}
 ;

edge_internals :		
| BEGIN_GRAPHED_INTERNALS ffEdgeArgList END_GRAPHED_INTERNALS
 ;

ffEdgeArgList : 
| ffEdgeArgList ffEdgeArg 
| ffEdgeArg
 ;

ffEdgeArg : EDGEPLACE edgenumberlist 
| FF_COORD ffEdgeNumerList
| FF_COLOR IDENTIFIER
{
  if((*current_edge)() != NULL)
	  (*current_edge)()->setColor(String($2));  	
  free($2);
}
| FF_WEIGHT NUMBER
{
 current_edge->setWeight( ((double)$2)/100.0);
}
| FF_LABEL IDENTIFIER IDENTIFIER
{
  FFSet<int> tmpS;
  int k=current_edge->getLabelIndex(String($2), &tmpS);
  if(k >= 0)
      {
          current_edge->getLabel(k)->parse(String($3));
      }
  else if (!unmatchedLabels.contains(String($2)))
    {
      cerr << "Edge has no Label of Type: " << $2 << endl;
      unmatchedLabels.add(String($2));
    }
  free($2);
  free($3);
}
| UNKNOWN_INTERNAL unknown_internal
 ;  
 
edgenumberlist:		
| NUMBER NUMBER edgenumberlist
{
  if((*current_edge)() != NULL)
	  (*current_edge)()->addPointAt(0, FFPoint($1, $2));
}
			
ffEdgeNumerList: 
| NUMBER NUMBER NUMBER ffEdgeNumerList
{
  if((*current_edge)() != NULL)
	  (*current_edge)()->addPointAt(0, FFPoint($1, $2, $3));
}
	
unknown_internal :	numbers_and_strings
 ;

numbers_and_strings :		
| number_or_string numbers_and_strings
 ;

number_or_string :	  NUMBER
| IDENTIFIER { free($1);}
 ;
%%
//static	int	yyerror (char *string)
//{
//  fprintf (stderr, "error: %s\n", string);
//  return 0;
//}

//void load_graph (FFGraphClass *g)
//{
//  parseFFGraph = g;
//}
