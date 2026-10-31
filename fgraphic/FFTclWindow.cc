/*
// Copyright 1994 Carsten Friedrich, Universitaet Passau
//
// $Author: schreibe $
// $Date: 1996/10/23 15:37:48 $
// $RCSfile: FFTclWindow.cc,v $
// $Revision: 1.7 $
// $State: Exp $
//
*/

#ifdef RCSID
#undef RCSID
#endif
#define RCSID "$Author: schreibe $ $RCSfile: FFTclWindow.cc,v $ $Date: 1996/10/23 15:37:48 $ $Revision: 1.7 $ $State: Exp $"

#include <config.h>

extern "C" {
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <X11/Xatom.h>
}

#include <stream.h>
#include <string.h>

#include <ffgraph/FFGraphClass.h>
#include <ffgraph/FFTclWindow.h>
#include <ffgraph/FFTclCompat.h>
#include <ffgraph/FFGraph.h>
#include <ffgraph/FFGraphicNode.h>
#include <ffgraph/FFGraphicEdge.h>
#include <ffgraph/FFNodeLabel.h>
#include <ffgraph/FFGraphLabel.h>
#include <ffgraph/FFEdgeLabel.h>
#include <ffgraph/FFTclGraphInsp.h> 
#include <ffgraph/FFSubGraphLabel.h>
#include <ffgraph/FFTclSubWindow.h>
#include <ffgraph/ffError.h>

#include <container/AVLSet.h>
#include <container/VOHSet.h>

#include <ffgraph/FFLayoutLabel.h>

#define INSPECTOR ((FFTclGraphInsp *)inspector)



int FFTclWindow::recall(Tcl_Interp *i, int k, const char **c)
{
// for(int j= 0; j < k; j++)
//    {
//      cerr << c[j] << " ";
//    }
  
//  cerr << endl;
 
  String befehl =c[1];
  
  switch(modal)
   {
   case NO:
    break;
   default:
    {
     ffError(NOTICE, "Finish modal Action first");
     return TCL_OK;
    }
   }
  
  if(befehl =="clear")
   {
    graph->clear();
   }
  else if(befehl =="revert")
   {
    graph->rev();
   }
  else if(befehl =="genEps")
   {
    String tmpPath("./demographs");
    
    if(fileName.length())
     tmpPath=fileName.before(fileName.index("/", -1));
    
    String file = fsBox(String("Select Eps File"), 
                        String("*.eps *.ps"), tmpPath);
    if(file.length())
     {
      if (Tcl_Eval(interp, "update") != TCL_OK ||
          FF_TclEvalObjv(interp, {".frame0.canvas2", "bbox", "all"}) != TCL_OK)
       {
        ffError(CF_TCL, Tcl_GetStringResult(interp));
       }
      else if (strlen(Tcl_GetStringResult(interp)))
       {
        String bbox = Tcl_GetStringResult(interp);
        String x = bbox.before(" ");
        bbox = bbox.after(" ");
        String y = bbox.before(" ");
        bbox = bbox.after(" ");
        String w = String(dec(atoi(String(bbox.before(" "))) + 100));
        String h = String(dec(atoi(String(bbox.after(" "))) + 100));
        if (FF_TclEvalObjv(interp, {".frame0.canvas2", "postscript",
                                   "-width", (const char *)w,
                                   "-height", (const char *)h,
                                   "-x", (const char *)x,
                                   "-y", (const char *)y,
                                   "-file", (const char *)file}) != TCL_OK)
         {
          ffError(CF_TCL, Tcl_GetStringResult(interp));
         }
       }
     }
   }
  else if(befehl =="revertEdge")
   {
    (*currentE)->revert();
   }  
  else if(befehl == "changeDirectedness")
   {
    if(graph->getDirected())
     graph->_setDirected(NO);
    else
     graph->_setDirected(YES);
   }
  else if(befehl =="setWeightZero")
   {
    (*currentE)->setWeight (0.0);
   }  
  else if(befehl =="eliminateParallelEdges")
   {
    graph->eliminateParallelEdges();
   }
  else if(befehl =="nodeEliminateParallelEdges")
   {
    if(currentNSet.length() ==1)
     currentNSet(currentNSet.first())->getNode()->eliminateParallelEdges();
   }
  else if(befehl =="layout")
   {
    Pix j;
    for(j= layouter.first(); j!= 0; layouter.next(j))
      if(layouter(j)->type().contains(c[2]))
        break;

    if(j != NULL)
    {
      if(currentNSet.length() > 1)
      {
        FFSet<FFNode> tmpN;

        for(Pix i=currentNSet.first();i!=0;currentNSet.next(i))
        {
          tmpN.add(currentNSet(i)->getNode());
        }
        layouter(j)->layout(tmpN);
      }
      else
        layouter(j)->layout();
    }
  }
  else if(befehl =="toggleSpline")
    {
      currentE->toggleSpline();
    }
  else if(befehl =="deleteWindow")
   {
    cerr << "Use 'close' to close window!" << endl;
    // graph->removeGraphic();   // !!!!!! loescht genau diese Instanz. 
    //   evtl. nicht sicher, funktioniert aber. ???????
    // return TCL_OK;
    
   }
//   else if(befehl == "resetScales")
//    {
//     alpha=beta=gamma=0.0;
//     scale=1.0;
//     ctm= initMatrix;	
//     invCtm= invInitMatrix;
//     viewPoint.v[2] = 1000;
//     updateGraph();
//    }
  else if(befehl == "toggleStartNode")
   {
    if( (currentNSet.length() == 1) &&
        (graph->getStartNode() == (*currentNSet(currentNSet.first()))()))
     {
      graph->setStartNode(NULL);
     }
    else
     {
      graph->setStartNode((*currentNSet(currentNSet.first()))());
     }
   }
  else if(befehl == "toggleEndNode")
   {
    for(Pix i=currentNSet.first(); i!=0; currentNSet.next(i))
     {
      if(graph->getEndNodes().contains((*currentNSet(i))()))
       {
        graph->delEndNode((*currentNSet(i))());
       }
      else
       {
        graph->addEndNode((*currentNSet(i))());
       }
     }
   }
  else if(befehl == "openURL")
   {
    String tmpStr("netscape -remote \"openURL(http://");
    tmpStr += (*currentNSet(currentNSet.first()))->getName() + ")\"";
    system(tmpStr);
   }
  else if(befehl == "switchNode")
   {
    if(currentNSet.length() == 2)
     {
      Pix tmpPix=currentNSet.first();
      FFGraphicNode *eins= currentNSet(tmpPix);
      currentNSet.next(tmpPix);
      FFGraphicNode *zwei= currentNSet(tmpPix);
      
      FFPoint tmpP = eins->getCoords();
      eins->moveTo(zwei->getCoords());
      zwei->moveTo(tmpP);
     }
   }
  else if(befehl == "splitNode")
   {
    FFNode derKnoten= currentNSet(currentNSet.first())->getNode();
    const FFSet<FFEdge> nach = derKnoten->transToSucc();
    const FFSet<FFEdge> von = derKnoten->transFromPred();
    if(nach.length() != 0)
     {
      Tcl_SetObjResult(interp, Tcl_NewStringObj("Node has outgoing edges!!!", -1));
      return TCL_ERROR;
     }
    for(Pix q=von.first(); q != 0; von.next(q))
     {
      FFNode tmpN = graph->addNode();
      tmpN->setName(derKnoten->getName());
      von(q)->setTarget(tmpN);
     }
    graph->removeNode(derKnoten);
   }
  else if(befehl == "edgify")
   {
    FFNode derKnoten= currentNSet(currentNSet.first())->getNode();
    const FFSet<FFEdge> nach = derKnoten->transToSucc();
    const FFSet<FFEdge> von = derKnoten->transFromPred();
    for(Pix q=von.first(); q != 0; von.next(q))
     for(Pix p=nach.first(); p != 0; nach.next(p))
      {
       FFEdge tmpE = graph->addEdge(von(q)->source(), nach(p)->target());
       tmpE->setName(derKnoten->getName());
      }
    graph->removeNode(derKnoten);
   }
  else if(befehl == "normNode")
   {
    BOOL tmpDate= getUpdate();
    setUpdate(NO);

    if(currentNSet.length())
     {
      FFGraphicNode *tmpN= currentNSet(currentNSet.first());
      FFPoint tmpP = tmpN->getCoords();
      Pix i;
      for(i=edgeMap.first(); i!= 0; edgeMap.next(i))
       {
        if(edgeMap.contents(i))
         edgeMap.contents(i)->rmoveTo(-tmpP);
        else
         cerr << "Internal edgeMap error" << endl;
       }
      for(i=nodeMap.first(); i!= 0; nodeMap.next(i))
       {
        if(nodeMap.contents(i))
         nodeMap.contents(i)->rmoveTo(-tmpP);
        else
         cerr << "Internal nodeMap error" << endl;
       }
     }
    setUpdate(tmpDate);
    
   }
  else
   FFTclBasicWindow::recall(i,k,c);
  return TCL_OK;  
}

int fgraphWindowRecall(ClientData d,Tcl_Interp *i, int k, const char **c) 
{
  return ((FFTclWindow *)d)->recall(i,k,c);
}


FFTclWindow::FFTclWindow() 
{
 updateInsp = YES;
 canvas = String(".frame0.canvas2");
 
 String tmpS;
 
 if(getenv("FF_LIBDIR") == NULL)
  tmpS=String(FF_LIBDIR);
 else
  tmpS=String(getenv("FF_LIBDIR"));
 
 tclFileName =tmpS + "/tcl/plainGraph.tcl";


 if(getenv("FF_ICONDIR") == NULL)
  tmpS=String(FF_LIBDIR) + "/icons/";
 else
  tmpS=String(getenv("FF_ICONDIR"));
 iconLibDir = tmpS;
 
  recallCommand_ = Tcl_CreateCommand(interp, "recall",
                    fgraphWindowRecall,
                    (ClientData)this,
                    recallCommandDeleted);
//  if(!boringMode)
//    {
//      tkToken = Tk_CreateTimerHandler (30000, fgraphWindowTimer, 
//                                       (ClientData) this);      
//    }
}

void FFTclWindow::event(labelEvent e, labelEventStruct *theEvent)
{
  FFTclBasicWindow::event(e,theEvent);
  switch(e)
    {
    case NAME_CHANGE:
      {
	setTitle(graph->getName());
	break;
      }
    case ADD_NODELABEL:
    case DEL_NODELABEL:
    case DEL_EDGELABEL:
    case ADD_EDGELABEL:
      {
	updateInspector();
	break;
      }
    case ADD_GRAPHLABEL:
    case DEL_GRAPHLABEL:
     {
      _buildLayoutMenu();
     }
    default:
      {
	break;
      }
    }
}

void FFTclWindow::setCurrent(FFGraphicNode *n)
{
  BOOL tmp=updateInsp;
  updateInsp = NO;

  FFTclBasicWindow::setCurrent(n);

  updateInsp=tmp;
  updateInspector();
}

void FFTclWindow::setCurrent(FFGraphicEdge *e)
{
  BOOL tmp=updateInsp;
  updateInsp = NO;

  FFTclBasicWindow::setCurrent(e);

  updateInsp=tmp;
  updateInspector();
}

void FFTclWindow::addToCurrent(FFGraphicNode *n)
{
  BOOL tmp=updateInsp;
  updateInsp = NO;

  FFTclBasicWindow::addToCurrent(n);

  updateInsp=tmp;
  updateInspector();
}

void FFTclWindow::init(const FFGraph& c)
{
  FFTclBasicWindow::init(c);
  char *tmpP = strdup((const char *) iconLibDir);
  Tcl_SetVar(interp,"ICON_LIB_DIR", tmpP, TCL_GLOBAL_ONLY);
 _buildLayoutMenu();
 free(tmpP);
}

void FFTclWindow::_buildLayoutMenu()
{
 // _updateLayouter();
  if(FF_TclEvalObjv(interp, {"clearLayoutMenu"}) != TCL_OK)
   ffError(CF_TCL, Tcl_GetStringResult(interp));
  
  for(Pix i=layouter.first(); i!= 0; layouter.next(i))
  {
    FF_TclEvalObjv(interp, {"addLayoutMenu", (const char *)layouter(i)->baseType()});
  }
}


void FFTclWindow::showInspector()
{
  // The Tcl interpreter can survive an external destruction of its Tk root.
  // Do not keep trying to show a dead inspector; create a fresh one instead.
  if (inspector && !static_cast<FFTclGraphInsp*>(inspector)->hasLiveWindow())
  {
    delete inspector;
    inspector = nullptr;
  }

  if (inspector)
    inspector->show();
  else
    inspector = new FFTclGraphInsp(this);

  updateInspector();
}

void FFTclWindow::updateInspector()
{
  if(INSPECTOR)
   {
    INSPECTOR->buildMenus();
    if(updateInsp)
     {
      if(currentNSet.length() == 1)
       INSPECTOR->inspectNode((*currentNSet(currentNSet.first()))());
      else if(currentE != NULL)
       INSPECTOR->inspectEdge((*currentE)());
      else if(currentNSet.length() > 1)
       INSPECTOR->inspectGroup(currentNSet);
      else
       INSPECTOR->inspect();
     }
   }
}

FFTclWindow::~FFTclWindow()
{
  if (interp && !Tcl_InterpDeleted(interp))
   {
    if (recallCommand_)
     Tcl_DeleteCommandFromToken(interp, recallCommand_);
    FF_TclEvalObjv(interp, {"destroyWindow"});
   }
}

