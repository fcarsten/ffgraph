/*
  // Copyright 1994 Carsten Friedrich, Universitaet Passau
  //
  // $Author: friedric $
  // $Date: 1996/05/06 14:18:04 $
  // $RCSfile: FFTclGraphInsp.cc,v $
  // $Revision: 1.4 $
  // $State: Exp $
  //
*/

#ifdef RCSID
#undef RCSID
#endif
#define RCSID "$Author: friedric $ $RCSfile: FFTclGraphInsp.cc,v $ $Date: 1996/05/06 14:18:04 $ $Revision: 1.4 $ $State: Exp $"

#include <config.h>
#include <ffgraph/ffError.h>

#include <String.h>
#include <stream.h>
#include <malloc.h>
#include <algorithm>
#include <cctype>
#include <vector>
#ifdef HAVE_STRING_H
# include <string.h>
#endif

#include <ffgraph/FFNodeLabel.h>
#include <ffgraph/FFGraphLabel.h>
#include <ffgraph/FFEdgeLabel.h>
#include <ffgraph/FFGraphicNode.h>
#include <ffgraph/FFGraphicEdge.h>
#include <ffgraph/FFTclBasicWindow.h>
#include <ffgraph/FFTclCompat.h>
#include <globdefs.h>
#include <ffgraph/FFTclGraphInsp.h>

#ifdef SUNOS
extern "C"
{
 char *strdup(char *s);
}
#endif

int finspectorRecall(ClientData d,Tcl_Interp *i, int k, const char **c) 
{
 return ((FFTclGraphInsp *)d)->recall(i,k,c);
}

void FFTclGraphInsp::inspect()
{
 String numN = dec(graph->getNumNodes());
 String numE = dec(graph->getNumEdges());

 String name = String(graph->getName());
 String color = String("NONE");

 FFGraphWindow *fenster = graph->getWindow();
 if(fenster != NULL)
  color = fenster->getBackground();
 
 String syntax;
  
 char *tmpA[4];
  
 tmpA[0]= strdup((const char *)numN);
 tmpA[1]= strdup((const char *)numE);
 char *tmpS = Tcl_Merge(2, tmpA);

 if(FF_TclEvalObjv(interp, {"UpdateGraph", (const char *)numN,
                                (const char *)numE}) != TCL_OK)
  {
   ffError(CF_TCL, Tcl_GetStringResult(interp));
  }

 free(tmpA[0]);
 free(tmpA[1]);
 Tcl_Free(tmpS);
  
 FFGraphLabel *gl= graph->getGraphLabel(graphIndex);
 tmpA[3]= strdup("NONE");
  
 if((gl != NULL) && gl->isInspectable())
  {
   if(gl->hasOwnInspector())
    { 
     //      gl->showInspector();
     gl->updateInspector();
    }
   else 
    {  
     free(tmpA[3]);
     tmpA[3]= strdup(gl->type());
     syntax = String(graph->getGraphLabel(graphIndex)->unparse());
    }
  }

 tmpA[0]= strdup((const char *)name);
 tmpA[1]= strdup((const char *)syntax);
 tmpA[2]= strdup((const char *)color);
 tmpS = Tcl_Merge(4, tmpA);
  
 if(FF_TclEvalObjv(interp, {"ShowGraph", tmpA[0], tmpA[1], tmpA[2], tmpA[3]}) != TCL_OK)
  {
   ffError(CF_TCL, Tcl_GetStringResult(interp));
  }

 for(int i=0;i<4; i++)
  free(tmpA[i]);
 Tcl_Free(tmpS);
}


void FFTclGraphInsp::inspectNode(FFNode n1)
{
 FFGraphicNode *n = (*n1)();

 currentN = n;
 String numN = dec(graph->getNumNodes());
 String numE = dec(graph->getNumEdges());
  
 String name = String(n1->getName());
 String color = n->getColor();
 String syntax;
 char *tmpA[4];

 tmpA[0]= strdup((const char *)numN);
 tmpA[1]= strdup((const char *)numE);
 char *tmpS = Tcl_Merge(2, tmpA);
  
 if(FF_TclEvalObjv(interp, {"UpdateGraph", (const char *)numN,
                                (const char *)numE}) != TCL_OK)
  {
   ffError(CF_TCL, Tcl_GetStringResult(interp));
  } 
  
 free(tmpA[0]);
 free(tmpA[1]);
 Tcl_Free(tmpS);
  
 tmpA[3]= strdup("NONE");
  
 FFNodeLabel *tmpL = n1->getLabel(nodeIndex);
  
 if(tmpL && tmpL->isInspectable())
  {
   free(tmpA[3]);
   tmpA[3]= strdup(tmpL->type());
   syntax = String(n1->getLabel(nodeIndex)->unparse());
  }
  
 tmpA[0]=strdup((const char *)name);
 tmpA[1]=strdup((const char *)syntax);
 tmpA[2]=strdup((const char *)color);
 tmpS = Tcl_Merge(4, tmpA);
  
 if(FF_TclEvalObjv(interp, {"ShowNode", tmpA[0], tmpA[1], tmpA[2], tmpA[3]}) != TCL_OK)
  {
   ffError(CF_TCL, Tcl_GetStringResult(interp));
  } 
 for(int k=0;k<4;k++)
  free(tmpA[k]);
 Tcl_Free(tmpS);
}

void FFTclGraphInsp::inspectGroup(const FFSet<FFGraphicNode *>& nSet)
{
 String numN = dec(graph->getNumNodes());
 String numE = dec(graph->getNumEdges());
  
 String name = dec(nSet.length());
 String color = "";
 String syntax;
 char *tmpA[4];

 tmpA[0]= strdup((const char *)numN);
 tmpA[1]= strdup((const char *)numE);
 char *tmpS = Tcl_Merge(2, tmpA);
  
 if(FF_TclEvalObjv(interp, {"UpdateGraph", (const char *)numN,
                                (const char *)numE}) != TCL_OK)
  {
   ffError(CF_TCL, Tcl_GetStringResult(interp));
  } 
  
 free(tmpA[0]);
 free(tmpA[1]);
 Tcl_Free(tmpS);
  
 tmpA[3]= strdup("NONE");
  
  
 tmpA[0]=strdup((const char *)name);
 tmpA[1]=strdup((const char *)syntax);
 tmpA[2]=strdup((const char *)color);
 tmpS = Tcl_Merge(4, tmpA);
  
 if(FF_TclEvalObjv(interp, {"ShowGroup", tmpA[0], tmpA[1], tmpA[2], tmpA[3]}) != TCL_OK)
  {
   ffError(CF_TCL, Tcl_GetStringResult(interp));
  } 
 for(int k=0;k<4;k++)
  free(tmpA[k]);
 Tcl_Free(tmpS);
}

void FFTclGraphInsp::inspectEdge(FFEdge e1)
{
 FFGraphicEdge *e = (*e1)();
  
  
 currentE= e;
 String numN = dec(graph->getNumNodes());
 String numE = dec(graph->getNumEdges());
  
 String name = String(e1->getName());
 String color = e->getColor();
 String syntax;

 char *tmpA[4];

 tmpA[0]= strdup((const char *)numN);
 tmpA[1]= strdup((const char *)numE);
 char *tmpS = Tcl_Merge(2, tmpA);
  
 if(FF_TclEvalObjv(interp, {"UpdateGraph", (const char *)numN,
                                (const char *)numE}) != TCL_OK)
  {
   ffError(CF_TCL, Tcl_GetStringResult(interp));
  } 
  
 free(tmpA[0]);
 free(tmpA[1]);
 Tcl_Free(tmpS);
  
 tmpA[3]= strdup("NONE");
  
 FFEdgeLabel *tmpL = e1->getLabel(edgeIndex);
  
 if(tmpL && tmpL->isInspectable())
  {
   free(tmpA[3]);
   tmpA[3]= strdup(tmpL->type());
   syntax = String(e1->getLabel(edgeIndex)->unparse());
  }
  
 tmpA[0]= strdup((const char *)name);
 tmpA[1]= strdup((const char *)syntax);
 tmpA[2]= strdup((const char *)color);
 tmpS = Tcl_Merge(4, tmpA);
  
 if(FF_TclEvalObjv(interp, {"ShowEdge", tmpA[0], tmpA[1], tmpA[2], tmpA[3]}) != TCL_OK)
  {
   ffError(CF_TCL, Tcl_GetStringResult(interp));
  }
 for(int k=0;k<4;k++)
  free(tmpA[k]);
 Tcl_Free(tmpS);
}

void FFTclGraphInsp::buildMenus()
{
 _clearMenus();
 _buildGraphMenu();
 _buildNodeMenu();
 _buildEdgeMenu();
}

int  FFTclGraphInsp::recall(Tcl_Interp *, int, const char **c)
{
 String rueck = String(c[1]);
  
 if(rueck == "deleteWindow")
  {
   withdraw();
  }
    
 else if(rueck == "NodeApply")
  {
   if(nodeIndex >= 0)
    (*currentN)->getLabel(nodeIndex)->parse(c[2]);
  }
 else if(rueck == "NodeCancel")
  {
   inspectNode(currentN->getNode());
  }
 else if(rueck == "NodeColor")
  {
   currentN->setColor(String(c[2]));
  }
 else if(rueck == "NodeName")
  {
   (*currentN)->setName(String(c[2]));
  }

 else if(rueck == "GraphApply")
  {
   if(graphIndex >= 0)
    graph->getGraphLabel(graphIndex)->parse(c[2]);
  }
 else if(rueck == "GraphCancel")
  {
   inspect();
  }
 else if(rueck == "GraphColor")
  {
   FFGraphWindow *fenster = graph->getWindow();
   if(fenster != NULL)
    fenster->setBackground(String(c[2]));
  }
 else if(rueck == "GroupColor")
  {
   FFGraphWindow *fenster = graph->getWindow();
   if(fenster != NULL)
    {
     const FFSet<FFGraphicNode *>& selNodes= fenster->selectedNodes();
     for(Pix i=selNodes.first(); i!= 0; selNodes.next(i))
      {
       selNodes(i)->setColor(String(c[2]));
      }
    }
  }
 else if(rueck == "GraphName")
  {
   graph->setName(String(c[2]));
  }

 else if(rueck == "EdgeApply")
  {
   if(edgeIndex >=0)
    (*currentE)->getLabel(edgeIndex)->parse(c[2]);
   //      currentE->getEdge()->setWeight(atof(c[2]));
  }
 else if(rueck == "EdgeCancel")
  {
   inspectEdge(currentE->getEdge());
  }
 else if(rueck == "EdgeName")
  {
   (*currentE)->setName(String(c[2]));
  }
 else if(rueck == "EdgeColor")
  {
   currentE->setColor(String(c[2]));
  }
 else if(rueck == "setMenu")
  {
   //    cerr << "Widget :" << c[2] << endl;
   FFSet<int> tmpSet;
      
   if(String(c[2]).contains(".insp_graph.type.edges"))
    {
     edgeIndex = graph->getEdgeLabelIndex(c[3], &tmpSet);
     if(currentE != NULL)
      inspectEdge(currentE->getEdge());
    }
   else if(String(c[2]).contains(".insp_graph.type.nodes"))
    {
     nodeIndex = graph->getNodeLabelIndex(c[3], &tmpSet);
     if(currentN != NULL)
      inspectNode(currentN->getNode());
    }
   else
    {
     graphIndex = graph->getGraphLabelIndex(c[3], &tmpSet);
     FFGraphLabel *gl= graph->getGraphLabel(graphIndex);
  
     if((gl != NULL) && gl->isInspectable())
      {
       if(gl->hasOwnInspector())
        { 
         gl->showInspector();
        }
      }
     inspect();
    }
  }
  
 return TCL_OK;  
}

void FFTclGraphInsp::_clearMenus()
{
 for(Pix i=menus.first(); i!=0; menus.next(i))
  {
   if(FF_TclEvalObjv(interp, {"clearMenu", (const char *)menus(i)}) != TCL_OK)
    {
     ffError(CF_TCL, Tcl_GetStringResult(interp));
    }       
  }
 menus.clear();
 menus.add(".insp_graph.type.nodes.menubutton2.m");
 menus.add(".insp_graph.type.graph.menubutton2.m");
 menus.add(".insp_graph.type.edges.menubutton2.m");
}

void FFTclGraphInsp::_buildGraphMenu()
{
 String menuString(".insp_graph.type.graph.menubutton2.m");

 if(FF_TclEvalObjv(interp, {"addMenu", (const char *)menuString,
                              ".insp_graph.type.graph", "NONE", "YES"}) != TCL_OK)
  {
   ffError(CF_TCL, Tcl_GetStringResult(interp));
  } 
 const MPlex<FFGraphLabel *>& graphLabel = graph->getGraphLabels();

 for(Pix i= graphLabel.first(); i!=0; graphLabel.next(i))
  {
   String tmpS = graphLabel(i)->type();
   
   std::vector<String> subTypes;
   std::string::size_type partStart = 0;
   while (true)
    {
     std::string::size_type separator = tmpS.find(':', partStart);
     subTypes.emplace_back(tmpS.substr(partStart,
                         separator == String::npos ? String::npos : separator - partStart));
     if (separator == String::npos)
      break;
     partStart = separator + 1;
    }
   int realNum = static_cast<int>(subTypes.size());
   menuString = String(".insp_graph.type.graph.menubutton2.m");
    
   for(int k=1; k<realNum -1; k++)
    {
     String tmpType = subTypes[k];
     std::transform(tmpType.begin(), tmpType.end(), tmpType.begin(),
                    [](unsigned char ch) { return static_cast<char>(std::tolower(ch)); });
     if(!menus.contains(menuString + String(".") + tmpType))
      {
       if(FF_TclEvalObjv(interp, {"addSubMenu", (const char *)menuString,
                                    (const char *)tmpType,
                                    (const char *)subTypes[k]}) != TCL_OK)
        ffError(CF_TCL, Tcl_GetStringResult(interp));
       menus.add(menuString + String(".") + tmpType);
      }
     menuString += String(".") + tmpType;
    }      
   String inspFlag("NO");
   if(graphLabel(i)->isInspectable())
    inspFlag= "YES";
      
   if(FF_TclEvalObjv(interp, {"addMenu", (const char *)menuString,
                                ".insp_graph.type.graph",
                                (const char *)graphLabel(i)->baseType(),
                                (const char *)inspFlag}) != TCL_OK)
    {
     ffError(CF_TCL, Tcl_GetStringResult(interp));
    } 
  } 
}

void FFTclGraphInsp::_buildNodeMenu()
{
 String menuString(".insp_graph.type.nodes.menubutton2.m");

 if(FF_TclEvalObjv(interp, {"addMenu", (const char *)menuString,
                              ".insp_graph.type.nodes", "NONE", "YES"}) != TCL_OK)
  {
   ffError(CF_TCL, Tcl_GetStringResult(interp));
  } 
 const MPlex<FFNodeLabel *>& nodeLabel = graph->getNodeLabels();
 for(Pix i= nodeLabel.first(); i!=0; nodeLabel.next(i))
  {
   String tmpS = nodeLabel(i)->baseType();
   String inspFlag("NO");
   if(nodeLabel(i)->isInspectable())
    inspFlag= "YES";

   if(FF_TclEvalObjv(interp, {"addMenu", (const char *)menuString,
                                ".insp_graph.type.nodes",
                                (const char *)tmpS, (const char *)inspFlag}) != TCL_OK)
    {
     ffError(CF_TCL, Tcl_GetStringResult(interp));
    } 
  } 
}

void FFTclGraphInsp::_buildEdgeMenu()
{
 String menuString(".insp_graph.type.edges.menubutton2.m");

 if(FF_TclEvalObjv(interp, {"addMenu", (const char *)menuString,
                              ".insp_graph.type.edges", "NONE", "YES"}) != TCL_OK)
  {
   ffError(CF_TCL, Tcl_GetStringResult(interp));
  } 
 const MPlex<FFEdgeLabel *>& edgeLabel = graph->getEdgeLabels();
 for(Pix i= edgeLabel.first(); i!=0; edgeLabel.next(i))
  {
   String tmpS = edgeLabel(i)->baseType();
   String inspFlag("NO");
   if(edgeLabel(i)->isInspectable())
    inspFlag= "YES";

   if(FF_TclEvalObjv(interp, {"addMenu", (const char *)menuString,
                                ".insp_graph.type.edges",
                                (const char *)tmpS, (const char *)inspFlag}) != TCL_OK)
    {
     ffError(CF_TCL, Tcl_GetStringResult(interp));
    } 
  }
}		 

FFTclGraphInsp::FFTclGraphInsp(FFGraphLabel *g) : FFTclInspector(g)
{
 graph = g->getGraph();
  
 currentN = 0;
 currentE = 0;
 nodeIndex = edgeIndex = graphIndex =-1;
 recallCommand_ = Tcl_CreateCommand(interp, "recall",
                   finspectorRecall, 
                   (int *)this, 
                   recallCommandDeleted);
  
 String tmpS;
 
 if(getenv("FF_LIBDIR") == NULL)
  tmpS=String(FF_LIBDIR) + "/tcl/finspector.tcl";
 else
  tmpS=String(getenv("FF_LIBDIR")) + "/tcl/finspector.tcl";

 initFromFile(tmpS);
 buildMenus();
}
