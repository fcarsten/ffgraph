/*
// Copyright 1994 Carsten Friedrich, Universitaet Passau
//
// $Author: deepthou $
// $Date: 1996/07/01 09:17:33 $
// $RCSfile: FFTclBasicWindow.cc,v $
// $Revision: 1.14 $
// $State: Exp $
//
*/

#ifdef RCSID
#undef RCSID
#endif
#define RCSID "$Author: deepthou $ $RCSfile: FFTclBasicWindow.cc,v $ $Date: 1996/07/01 09:17:33 $ $Revision: 1.14 $ $State: Exp $"

#include <config.h>

extern "C" {
#include <malloc.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <X11/Xatom.h>
}

#include <stream.h>
#include <sstream>
#include <regex>
#include <string>
#include <string.h>
#include <algorithm>
#include <cstring>
#include <ffgraph/FFGraphClass.h>
#include <ffgraph/FFTclBasicWindow.h>
#include <ffgraph/FFTclCompat.h>
#include <ffgraph/FFTclSubWindow.h>
#include <ffgraph/FFGraph.h>
#include <ffgraph/FFGraphicNode.h>
#include <ffgraph/FFGraphicEdge.h>
#include <ffgraph/FFNodeLabel.h>
#include <ffgraph/FFGraphLabel.h>
#include <ffgraph/FFEdgeLabel.h>
#include <ffgraph/FFSubGraphLabel.h>
#include <ffgraph/FFTclGraphInsp.h> 
#include <ffgraph/FFLayoutLabel.h>
#include <ffgraph/ffError.h>

#include <container/VOHSet.h>
#include <container/AVLSet.h>
#include <container/SLStack.h>

static String currentIcon("__NoIcon__");
static String currentSelIcon("__NoIcon__");
static String currentMask("__NoMask__");
static String currentSelMask("__NoMask__");

void fgraphBasicWindowTimer(ClientData)
{
 // cerr << "Timer :" << d << endl;
//  ((FFTclBasicWindow *)d)->timer();
}



int FFTclBasicWindow::exportSelection(int off, char *buffer, int maxBytes)
{
 // Tcl/Tk's selection callback returns a byte count, not a C string.
 if (off < 0 || maxBytes <= 0 || buffer == nullptr) return -1;
 const std::size_t length = exportString.length();
 if (static_cast<std::size_t>(off) >= length) return -1;
 const std::size_t available = length - static_cast<std::size_t>(off);
 const std::size_t count = std::min(available, static_cast<std::size_t>(maxBytes));
 std::memcpy(buffer, exportString.data() + off, count);
 return static_cast<int>(count);
}

void FFTclBasicWindow::_hideLines()
{
 // No Line hiding in 2D :)
}

int FFTclBasicWindow::getSelection(const char *buffer)
{
  importString += String(buffer);
  return TCL_OK;
}

void FFTclBasicWindow::_buildExportString()
{
 FFSet<FFNode> tmpSet;
 for(Pix ind=currentNSet.first(); ind != 0; currentNSet.next(ind))
  {
   tmpSet.add(currentNSet(ind)->getNode());
  }
 
 graph->saveToString(exportString, tmpSet);
 return;
}

void fgraphWindowLostSel(ClientData) 
{
}

int fgraphWindowGetSel(ClientData data, Tcl_Interp *, const char *portion)
{
  return((FFTclBasicWindow *)data)->getSelection(portion);  
}


int FFTclBasicWindow::recall(Tcl_Interp *, int , const char **c)
{
  static FFPoint pos;
  //  if(!isBored && !boringMode)
  // {
  // Tk_DeleteTimerHandler(tkToken);
  // tkToken = Tk_CreateTimerHandler (30000, fgraphSubWindowTimer, 
  // (ClientData) this);
  // }
  
  String befehl =c[1];
  
  //  cerr << "Befehl: " << befehl << endl;
  
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
  
  if(befehl =="moveCurrent")
   {
    FFPoint tmpP = pos;
    
    BOOL tmpDate = updateFlag;
    updateFlag=NO;
    
    pos = unProj(FFPoint(atof(c[3]), atof(c[4]), (viewDist)));
    
    for(Pix i= currentNSet.first(); i != 0; currentNSet.next(i))
     {
      const FFSet<FFEdge>& tmpEdges= (*currentNSet(i))->transToSucc();
      for(Pix j= tmpEdges.first(); j != 0; tmpEdges.next(j))
       if(currentNSet.contains(tmpEdges(j)->target()->getGraphicNode()))
        (*tmpEdges(j))->rmoveTo(pos- tmpP);
      currentNSet(i)->rmoveTo(pos- tmpP);
     }
    if(currentE != NULL)
     {
      currentE->rmoveTo(pos - tmpP);
      currentE->updateGraphic();
     }
    
    updateFlag=tmpDate;
   }
  else if(befehl == "showNodes")
   {
    if(showNodesFlag == YES)
     {
      showNodesFlag = NO;
      hideNodes();
     }
    else
     { 
      showNodesFlag = YES;
      showNodes();
     }
   }
  else if(befehl == "showEdges")
   {
    if(showEdgesFlag == YES)
     {
      showEdgesFlag = NO;
      hideEdges();
     }
    else
     {
      showEdgesFlag = YES;
      showEdges();
     }
   }

  else if(befehl == "setNodePosition")
   {
    String posStr(c[2]);
    if(posStr == "right")
    {
     labelXoff =1.2;
     labelYoff =0.0;
     updateGraph();
    }
    else if(posStr == "left")
    {
     labelXoff =-1.2;
     labelYoff =0.0;
     updateGraph();
    }
    else if(posStr == "top")
    {
     labelXoff =0.0;
     labelYoff =-1.2;
     updateGraph();
    }
    else if(posStr == "bottom")
    {
     labelXoff =0.0;
     labelYoff =1.2;
     updateGraph();
    }    
    else if(posStr == "Inside")
     {
      labelXoff = 0.0;
      labelYoff = 0.0;
      updateGraph();
     }
   }
  else if(befehl == "setShape")
   {
    String shapeStr(c[2]);
    shape = str2Shape(shapeStr);
   }
  else if(befehl == "setCurrentIcon")
   {
    currentIcon = c[2];
   }
  else if(befehl == "setCurrentSelIcon")
   {
    currentSelIcon = c[2];
   }
  else if(befehl == "setCurrentMask")
   {
    currentMask = c[2];
   }
  else if(befehl == "setCurrentSelMask")
   {
    currentSelMask = c[2];
   }
  else if(befehl == "setNoIcon")
   {
    currentIcon = String("__NoIcon__");
    currentSelIcon = String("__NoIcon__");
    currentMask = String("__NoMask__");
    currentSelMask = String("__NoMask__");
   }
  else if(befehl == "setNoMask")
   {
    currentMask = String("__NoMask__");
   }
  else if(befehl == "setNoSelMask")
   {
    currentSelMask = String("__NoMask__");
   }
  else if(befehl == "setNoSelIcon")
   {
    currentSelIcon = String("__NoIcon__");
    currentSelMask = String("__NoMask__");
   }
  else if(befehl == "changeSize")
   {
    if(currentNSet.length() == 0)
      ffError(NOTICE, "No node selected");
    else
     {
      FFPoint tmpP=pos;
      
      for(Pix i=currentNSet.first(); i != 0; currentNSet.next(i))
       {
        
        FFGraphicNode *gn= currentNSet(i);
        
        pos = unProj(FFPoint(atof(c[3]), atof(c[4]), (viewDist)));
        // cerr << pos() << " " << tmpP() << endl;
        tmpP= (pos -  tmpP);
        
        gn->setRange( gn->getRange() + FFPoint(tmpP.v[0],tmpP.v[0],
                                               tmpP.v[0]));
        
        // cerr << "New Range " << gn->getRange()() << endl;
        
        if(gn->getShape() == FF_WINDOW)
         {
          String tmpId = gn->getId();
          String xOffset(dec((int)tmpP.v[0] * 2));
          String yOffset(dec((int)tmpP.v[1] * 2));
          if(FF_TclEvalObjv(interp, {"changeWindowSize", (const char *)canvas,
                                     (const char *)tmpId, (const char *)xOffset,
                                     (const char *)yOffset}) != TCL_OK)
           ffError(CF_TCL, Tcl_GetStringResult(interp));

         }
        else
         {
          updateGraph(gn);
         }
       }
     }
   }	      
   else if(befehl =="movePoint")
    {
     if(currentE != NULL)
      {
       currentE->rmovePoint(tmpPoint, 
                            unProj(FFPoint(atof(c[3]), atof(c[4]), 
                                           (viewDist)))-pos);
       currentE->updateGraphic();
      }
     pos= unProj(FFPoint(atof(c[3]), atof(c[4]), (viewDist)));
     if(updateFlag)
      FFWindow::update();
    }
  else if(befehl =="newEdge")
   {
    for(Pix i=currentNSet.first(); i!=0; currentNSet.next(i))
     {
      graph->addEdge( (* currentNSet(i))(),
                      (* nodeMap[c[2]])());
     }
   }
  else if(befehl == "moveEye")
   {
    yWinkel = fmod(yWinkel + (atof(c[2])/50.0) , 360);
    xWinkel = fmod(xWinkel - (atof(c[3])/50.0) , 360);
    _buildProjMatrix();
    // updateGraph();
   }
  else if(befehl == "subgraph")
   {
    if(Pix i=currentNSet.first())
     {
      FFNode n= currentNSet(i)->getNode();
      n->getGraph()->getSubGraph(n);
      // ->createGraphic((FFGraphWindow *)
      // this->proto());
     }
   }
  else if(befehl == "keyPress")
   {
    String key(c[2]);
    if(key == "Up")
     {
      viewPoint += FFPoint(0,1,0);
      _buildProjMatrix();
      // updateGraph();
     }
    else if(key == "Down")
     {
      viewPoint += FFPoint(0,-1,0);
      _buildProjMatrix();
      //      updateGraph();
     }
    else if(key == "Left")
     {
      viewPoint += FFPoint(1,0,0);
      _buildProjMatrix();
      //      updateGraph();
     }
    else if(key == "Right")
     {
      viewPoint += FFPoint(-1,0,0);
      _buildProjMatrix();
      //      updateGraph();
     }    
    else if(key == "Front")
     {
      viewPoint += FFPoint(0,0,1);
      _buildProjMatrix();
      //      updateGraph();
     }    
    else if(key == "Back")
     {
      viewPoint += FFPoint(0,0,-1);
      _buildProjMatrix();
      //      updateGraph();
     }    
    else if(currentNSet.length() != 0)
     {
      for(Pix i= currentNSet.first(); i != 0; currentNSet.next(i))
       {
        String tmpStr(c[3]);
        if (key.contains("Return") ||
            key.contains("Enter"))
         tmpStr = '\n';

        FFNode tmpN= (*currentNSet(i))();
        if(key.contains("BackSpace") ||
           key.contains("Delete"))
         {
          String tmpName = tmpN->getName();
          
          tmpN->setName(tmpName.before((int)(tmpName.length() -1)));
         }
        else
         {
          tmpN->setName(tmpN->getName() + tmpStr);
         }        
       }
     }
    else if(currentE != NULL)
     {
      String tmpStr(c[3]);
      if (key.contains("Return") ||
          key.contains("Enter"))
       tmpStr = '\n';
         
      (*currentE)->setName((*currentE)->getName() + tmpStr);      
     }    
   }
  else if(befehl == "modPress")
   {
    String key(c[2]);
    
    if(key == "c")
     {
      _buildExportString();
#if(TK_MAJOR_VERSION < 4)
      Tk_OwnSelection(w, fgraphWindowLostSel, this);
#else
      Tk_OwnSelection(w, XA_PRIMARY, fgraphWindowLostSel, this);
#endif
     }
    else if(key == "x")
     {
      _buildExportString();
#if(TK_MAJOR_VERSION < 4)
      Tk_OwnSelection(w, fgraphWindowLostSel, this);
#else
      Tk_OwnSelection(w, XA_PRIMARY, fgraphWindowLostSel, this);
#endif
      Pix pix = currentNSet.first(), tmpPix;
      
      while(pix != NULL)
       {
        tmpPix=pix;
        currentNSet.next(pix);
        graph->removeNode((*currentNSet(tmpPix))());
       }
     }
    else if(key == "r")
     {
      if(FF_TclEvalObjv(interp, {"InputOneBox", "Regular expression:",
                                 "Input"}) != TCL_OK)
       {
        ffError(CF_TCL, Tcl_GetStringResult(interp));
        return TCL_OK;
       }
         
      std::regex tmpR;
      try
       {
        tmpR = std::regex(Tcl_GetStringResult(interp));
       }
      catch (const std::regex_error&)
       {
        ffError(CF_TCL, "Invalid regular expression");
        return TCL_OK;
       }

      unsetCurrent();
      const FFSet<FFNode>& tmpS= graph->allNodes();
      for(Pix i=tmpS.first(); i!=0; tmpS.next(i))
       {
        const String nodeName = tmpS(i)->getName();
        if(std::regex_search(nodeName.begin(), nodeName.end(), tmpR))
         addToCurrent((*tmpS(i))());
       }
     }
    else if(key == "v")
     {
      importString = String("");
      
#if(TK_MAJOR_VERSION < 4)
      Tk_GetSelection(interp, w, XA_STRING, fgraphWindowGetSel, 
                      (ClientData) this);
#else
      Tk_GetSelection(interp, w, XA_PRIMARY, XA_STRING, 
                      fgraphWindowGetSel, (ClientData) this);
#endif
      graph->load(importString, NO);
      importString = String("");
     }		
    else if(key == "n")
     {
      // FFGraph()->createGraphic(new FFTclWindow);
      FFGraph()->createGraphic((FFGraphWindow *)this->proto());
     }
    else if(key == "m")
     {
      String tmpPath("./demographs");
      
      if(fileName.length())
       tmpPath=fileName.before(fileName.index("/", -1));

      String file = fsBox(String("Select Graph File"), 
                          String("*.g *.gml"), tmpPath);
      if(file.length())
       {
        graph->load(file, YES);
       }
     }
    else if(key == "i")
     {
      showInspector();
     }
    else if(key == "w")
     {
      if(inspector)
       {
        inspector->hide();
        delete inspector;
        inspector = 0;
       }
      graph->removeGraphic();
      return TCL_OK;
     }
    else if(key == "a")
     {
      const FFSet<FFNode>& tmpS= graph->allNodes();
      for(Pix i=tmpS.first(); i!=0; tmpS.next(i))
       {
        addToCurrent((*tmpS(i))());
       }
     }
    else if(key == "l")
     {
      String tmpPath("./demographs");
      
      if(fileName.length())
       tmpPath=fileName.before(fileName.index("/", -1));
      
      String file = fsBox(String("Select Graph File"), 
                          String("*.g *.gml"), tmpPath);
      if(file.length())
       {
        FFGraph tmpG;
        // tmpG->createGraphic(new FFTclWindow);
        FFTclBasicWindow *neuF = (FFTclBasicWindow *)this->proto();
        
        tmpG->createGraphic((FFGraphWindow *)neuF);
        // A graph window is a non-owning label. Without a retained FFGraph
        // handle, tmpG's destructor at the end of this callback destroys
        // both the newly loaded graph and its briefly visible window.
        FFWindow::retainGraph(tmpG);
        tmpG->load(file, YES);
        neuF->setFileName(file);
       }
     }
    else if(key == "S")
     {
      String tmpPath("./demographs");
      
      if(fileName.length())
       tmpPath=fileName.before(fileName.index("/", -1));
      
      String file = fsBox(String("Select File"), 
                          String("*.g *.gml"), tmpPath); 
      if(file.length())
       {
        fileName = file;
        graph->saveToFile(file);
       }
     }
    else if(key == "s")
     {
      if(fileName.length() == 0)
       {
        String file = fsBox(String("Select File"), 
                            String("*.g *.gml"), String("./demographs")); 
        if(!file.length())
         {
          return TCL_OK;
         }
        fileName = file;
       }
      graph->saveToFile(fileName);
     }
    else if(key == "q")
     {
      exit(0);
     }
    else if(key == "w")
     {
      graph->removeGraphic();
     }
    else if(key == "<")
     {
      FFSet<int> tmpSet;
      int tmpIndex= graph->getNodeLabelIndex(ffSubGraphLabel, &tmpSet);
      if(tmpIndex >= 0)
       for(Pix i=currentNSet.first(); i!=0; currentNSet.next(i))
        {
         FFGraphicNode *gn = currentNSet(i);
         if(gn->getNodeWindow() != "")
          {
           FFNode n= gn->getNode();
           FFSubGraphLabel  *nl= 
            (FFSubGraphLabel *)n->getLabel(tmpIndex);        
           
           if(gn->getIcon() != "__NoIcon__")
            {
             // cerr << "Changing back to icon " << gn->getIcon() << endl;
             gn->changeToIcon(gn->getIcon());
            }
           else
            {
             // cerr << "Changing back to circle" << endl;
             gn->changeToFigure(gn->getFigure());
            }
           nl->getGraph()->createGraphic(NULL);
          }
        }
     }
    else if(key == ">")
     {
      FFSet<int> tmpSet;
      
      int tmpIndex= graph->getNodeLabelIndex(ffSubGraphLabel, &tmpSet);
      if(tmpIndex >= 0)
       {
        for(Pix i=currentNSet.first(); i!=0; currentNSet.next(i))
         {
          FFGraphicNode *gn= currentNSet(i);
          FFNode n= (*gn)();
          FFSubGraphLabel  *nl= 
           (FFSubGraphLabel *)n->getLabel(tmpIndex);        
          
          gn->changeToWindow();
          
          String tmpWin = gn->getNodeWindow();
          String tmpId = gn->getId();
          FFTclSubWindow *tmpW= new FFTclSubWindow(gn->getNode(),
                                                   interp, w, tmpWin, tmpId);
          
          FFGraph tmpG= nl->getGraph();
          tmpG->createGraphic(tmpW);
          tmpW->adjustWindowSize();
         }
       }
     }
    
   }
  else if(befehl =="setCurrentNode")
    {
      setCurrent(nodeMap[c[2]]);
      pos= unProj(FFPoint(atof(c[3]), atof(c[4]), (viewDist)));
    }
  else if(befehl =="changeSizeInit")
    {
     // cerr << "Init Changing size " << c[3] << " " << c[4] << endl;
      pos= unProj(FFPoint(atof(c[3]), atof(c[4]), (viewDist)));
    }
  else if(befehl =="selectIfNot")
    {
     selectIfNot(nodeMap[c[2]]);
     pos= unProj(FFPoint(atof(c[3]), atof(c[4]), (viewDist)));
    }		
  else if(befehl =="multiSelect")
    {
      addToCurrent(nodeMap[c[2]]);
		}		
  else if(befehl =="setCurrentEdge")
    {
      setCurrent(edgeMap[c[2]]);
      pos= unProj(FFPoint(atof(c[3]), atof(c[4]), (viewDist)));
      if(currentE != NULL)
        tmpPoint= currentE->getPointNear(FFPoint(atof(c[3]), atof(c[4])));
    }
  else if(befehl =="toggleSplineAll")
   {
    for(Pix i = edgeMap.first(); i != 0; edgeMap.next(i))
     {
      if(edgeMap.contents(i))
       edgeMap.contents(i)->toggleSpline();
     }
   }  
  else if(befehl =="newNode")
   {
    if((shape == FF_IMAGE) &&
       (currentIcon == "__NoIcon__"))
     {
      ffError(USER_ERROR, "Select an icon first");
      return  TCL_ERROR;
     }
    FFPoint tmpP = unProj(FFPoint(atof(c[3]), atof(c[4]), (viewDist)));
    // tmpP.v[2]=0;
    FFNode tmpN = graph->addNode(tmpP);
    
    //    if(tmpN->getGraphicNode() && (currentIcon != String("__NoIcon__")))
    if(tmpN->getGraphicNode())
     {
      if(shape == FF_IMAGE)
       {
        //        cerr << "Creating Icon with " << currentIcon << " and " << 
        //         currentSelIcon <<endl;      
        (*tmpN)->changeToIcon(currentIcon, currentSelIcon);
        
        if( (currentMask != "__NoMask__") ||
            (currentSelMask != "__NoMask__"))
         (*tmpN)->setMask(currentMask, currentSelMask);
       }
      else if(shape > FF_FIGURE)
       {
        (*tmpN)->changeToFigure(shape);
       }
     }
   }
  else if(befehl =="removeNode")
    {
      // Tk_Sleep(100);      
      graph->removeNode((*nodeMap[c[2]])());
    }
  else if(befehl =="removeEdge")
    {
      graph->removeEdge((*edgeMap[c[2]])());
    }
  else if(befehl =="unsetCurrent")
    {
      unsetCurrent();
    }
  else if(befehl == "addInstantPoint")
  {
    if(currentE!= NULL)
    {
      currentE->addPointAt(-1, unProj(FFPoint(atof(c[2]), atof(c[3]), 
                                              (viewDist))));
      currentE->updateGraphic();
    }
  }
  else if(befehl == "removeInstantPoint")
   {
    if(currentE != NULL)
     {
      currentE->deletePointAt(unProj(FFPoint(atof(c[2]), atof(c[3]),
                                             (viewDist))));
      currentE->updateGraphic();
     }
   }
  else if(befehl == "flattenEdge")
   {
    _flattenEdge();
   }
  else if(befehl == "resetScales")
   {
    alpha=beta=gamma=0.0;
    scale=1.0;
    viewPoint= FFPoint(0,0,-1000,1);
    xWinkel = yWinkel = zWinkel = 0.0;
    viewDist = 1000.0;
 
    _buildProjMatrix();
    ctm= initMatrix;	
    invCtm= invInitMatrix;

    updateGraph();
   }
  else if(befehl == "zoom")
   {
    FFCoord tmpX = atof(c[2]);
    scaleXYZ(tmpX, tmpX, tmpX);
   }
  else if(befehl == "zoomScale")
   {
    FFCoord tmpX = (pow((double)(atof(c[2])),(long int)2)/(100*100))/scale;
    updateScale=NO;
    scaleXYZ(tmpX, tmpX, tmpX);
    updateScale=YES;
   }
  else if(befehl == "boxscale")
   {
    FFCoord tmpX = atof(c[2]);
    FFCoord tmpY = std::max(static_cast<FFCoord>(atof(c[3])), tmpX);
    
    // cerr << "Scaling by " << tmpY << endl;
    
    scaleXYZ(tmpY, tmpY, tmpY);
   }
  else if(befehl == "setXWinkel")
   {
    updateScale=NO;
    FFCoord tmpX = atof(c[2]);		
    rotateX(tmpX-alpha);
    updateScale=YES;
   }
  else if(befehl == "setYWinkel")
   {
    updateScale=NO;
    FFCoord tmpX = atof(c[2]);
    rotateY(tmpX-beta);
    updateScale=YES;
   }
  else if(befehl == "setZWinkel")
   {
    updateScale=NO;
    FFCoord tmpX = atof(c[2]);
    rotateZ(tmpX-gamma);
    updateScale=YES;
   }
//   else if(befehl == "rotateZ")
//    {
//     zWinkel += atof(c[2]);
//    }
//   else if(befehl == "rotateX")
//    {
//     xWinkel += atof(c[2]);
//    }
//   else if(befehl == "rotateY")
//    {
//     yWinkel += atof(c[2]);
//    }  
  else if(befehl == "setX")
   {
    viewPoint.v[0] += atof(c[2]);
    _buildProjMatrix();
    //    updateGraph();
   }
  else if(befehl == "setY")
   {
    viewPoint.v[1] += atof(c[2]);
    _buildProjMatrix();
    //    updateGraph();
   }
  else if(befehl == "setZ")
   {
    viewPoint.v[2] += atof(c[2]);
    _buildProjMatrix();
    //    updateGraph();
   }
//   else if(befehl == "setZvalue")
//    {
//     viewPoint.v[2] = atof(c[2]);
//       _buildProjMatrix();
//       //    updateGraph();
//    }
  else if(befehl == "viewDist")
   {
    viewDist = atof(c[2]);
    updateGraph();
   }
  else if(befehl == "sixColor")
   {
    _sixColor();
   }
  
  return TCL_OK;  
}

// int fgraphBasicWindowRecall(ClientData d,Tcl_Interp *i, int k, const char **c) 
// {
//   return ((FFTclBasicWindow *)d)->recall(i,k,c);
// }

//void FFTclBasicWindow::timer()
//{
//}
void FFTclBasicWindow::_sixColor()
{
 FFGraph tmpGraph= graph->copy(NULL, NULL, YES);
 SLStack<FFNode> colorStack6;
 SLStack<FFNode> colorStack5;
 SLStack<FFNode> colorStack;
 int minDeg=4;
 
 while(minDeg <= 6)
  {
   BOOL didDel = YES;
   while(didDel)
    {
     didDel = NO;
     FFSet<FFNode> toDel;
     const FFSet<FFNode>& alleK = tmpGraph->allNodes();
     for(Pix i=alleK.first(); i != 0; alleK.next(i))
      {
       FFNode tmpN = alleK(i);
       
       if(tmpN->indeg() + tmpN->outdeg() < minDeg)
        {
         cerr << "Found < " << minDeg << endl;
         
         didDel= YES;
         toDel.add(tmpN);
        }
      }
     for(Pix j=toDel.first(); j!= 0; toDel.next(j))
      {
       // cerr << "pushing " << graph->nodeWithValue(toDel(j)->getValue()) << endl;
       if(minDeg==6)
        colorStack6.push(graph->nodeWithValue(toDel(j)->getValue()));
       else if(minDeg == 5)
        colorStack5.push(graph->nodeWithValue(toDel(j)->getValue()));
       else
        colorStack.push(graph->nodeWithValue(toDel(j)->getValue()));
       tmpGraph->removeNode(toDel(j));
      }
     if(didDel)
      minDeg = 4;
    }
   if(tmpGraph->getNumNodes() != 0)
    {
     minDeg ++;
    }
   else
    minDeg=7;
  }
 
 while(! colorStack5.empty())
  colorStack.push(colorStack5.pop());
 while(! colorStack6.empty())
  colorStack.push(colorStack6.pop());
 
 const FFSet<FFNode>& alleK = graph->allNodes();

 for(Pix k= alleK.first(); k!= 0; alleK.next(k))
  {
   if(alleK(k)->getGraphicNode())
    alleK(k)->getGraphicNode()->setColor("black");
  }
 
 while(!colorStack.empty())
  {
   SLStack<String> colors;
   colors.push("magenta");
   colors.push("red");
   colors.push("yellow");
   colors.push("green");
   colors.push("blue");
   colors.push("black");
   
   FFNode cand=colorStack.pop();
//    cerr << "popping " << cand << endl;
   
    FFGraphicNode *candG= cand->getGraphicNode();
   if(candG != NULL)
    {
     const FFSet<FFEdge>& kantenVon= cand->transFromPred();
     for(Pix i=kantenVon.first(); i!= 0; kantenVon.next(i))
      {
       FFGraphicNode *tmpGN=kantenVon(i)->source()->getGraphicNode();
       if(tmpGN != NULL)
        colors.del(tmpGN->getColor());
      }
     const FFSet<FFEdge>& kantenNach= cand->transToSucc();
     for(Pix i=kantenNach.first(); i!= 0; kantenNach.next(i))
      {
       FFGraphicNode *tmpGN=kantenNach(i)->target()->getGraphicNode();
       if(tmpGN != NULL)
        colors.del(tmpGN->getColor());
      }
     if(colors.length() >= 1)
      candG->setColor(colors.pop());
    }
  }
}


FFPoint FFTclBasicWindow::proj(const FFPoint& A, const FFPoint& tmpRange)
{
 FFPoint tmpP= (A*ctm)*projMatrix;
 // cerr << A() << endl << tmpP() << endl << tmpRange() << endl << endl;
 
//  FFPoint prjR = FFPoint(tmpP.v[0], tmpP.v[1]+(RADIUS*scale), tmpP.v[2]);

//  tmpP = tmpP*projMatrix;
 
 
 FFCoord tmpDist = tmpP.v[2];

 if (tmpP.v[2] < viewDist)
  return FFPoint(-1000,-1000,-1);

 FFCoord P_r = scale * (tmpP.v[0] + tmpRange.v[0]/2);
 P_r = (P_r * viewDist)/tmpDist;
 
 FFCoord P_l = scale * (tmpP.v[0] - tmpRange.v[0]/2);  
 P_l = (P_l * viewDist)/tmpDist;
 
 // tmpP.v[2] = RADIUS*2;
 
 tmpP = (tmpP * viewDist)/tmpDist;
 tmpP.v[2] = fabs(P_r - P_l);
 
 return tmpP;
}


FFPoint FFTclBasicWindow::unProj(const FFPoint& A)
{
 return (((A * A.v[2])/ viewDist) * invProjMatrix ) *invCtm;
 //  FFPoint B =  (((A * A.v[2])/ viewDist) * invProjMatrix ) *invCtm;
 
 //   cerr << "Unprojecting " << A() << " is " << B() << endl;
 //   cerr << "Reprojecting is " << proj(B)() << endl;
 
 // return B;
 
}

void FFTclBasicWindow::translate(FFCoord x, FFCoord y, FFCoord z)
{
 FFMatrix tmpM(FFPoint(1,0,0,0), FFPoint(0,1,0,0),
               FFPoint(0,0,1,0), FFPoint(x,y,z,1));
 ctm = tmpM*ctm;
	
 invCtm *= FFMatrix(FFPoint(1,0,0,0), FFPoint(0,1,0,0),
                    FFPoint(0,0,1,0), FFPoint(-x,-y,-z,1));
 
 updateGraph();
}

void FFTclBasicWindow::scaleXYZ(FFCoord sx, FFCoord sy, FFCoord sz)
{
 FFMatrix tmpM(FFPoint(sx,0,0,0), FFPoint(0,sy,0,0),
               FFPoint(0,0,sz,0), FFPoint(0,0,0,1));
 ctm = tmpM*ctm;
 
 invCtm *= FFMatrix(FFPoint(1/sx,0,0,0), FFPoint(0,1/sy,0,0),
                    FFPoint(0,0,1/sz,0), FFPoint(0,0,0,1));
 
 if(updateScale)
  {
   String tmpVal(dec((int)(sqrt(sz * 10000 * scale))));
   
   if(FF_TclEvalObjv(interp, {"updateScalation", (const char *)tmpVal})!= 
      TCL_OK)
    {
     ffError(CF_TCL, Tcl_GetStringResult(interp));
    }
  }
 
 scale *= sx; // Temporaer um Kreisradius anzupassen

 updateGraph();
}

void FFTclBasicWindow::rotateX(FFCoord w)
{
 FFMatrix tmpM(FFPoint(1,0,0,0), FFPoint(0,cos(rad(w)),sin(rad(w)),0),
               FFPoint(0,-sin(rad(w)),cos(rad(w)),0),FFPoint(0,0,0,1));
 ctm = tmpM*ctm;
 
 invCtm *= FFMatrix(FFPoint(1,0,0,0), FFPoint(0,cos(rad(-w)),sin(rad(-w)),0),
                    FFPoint(0,-sin(rad(-w)),cos(rad(-w)),0),FFPoint(0,0,0,1));
 
 if(updateScale)
  {
   String tmpVal(dec((int)(w+alpha)));
   
   if(FF_TclEvalObjv(interp, {"updateRotationX", (const char *)tmpVal})!= 
      TCL_OK)
    {
     ffError(CF_TCL, Tcl_GetStringResult(interp));
    }
  }
 alpha += w;

 updateGraph();
}

void FFTclBasicWindow::rotateY(FFCoord w)
{
 FFPoint tmpP(cos(rad(w)), 0 ,-sin(rad(w)),0);
 FFMatrix tmpM(tmpP,
               FFPoint(0,1,0,0),
               FFPoint(sin(rad(w)),0,cos(rad(w)),0),
               FFPoint(0,0,0,1));
 ctm = tmpM*ctm;
 
 invCtm *= FFMatrix(FFPoint(cos(rad(-w)), 0 ,-sin(rad(-w)),0),
                    FFPoint(0,1,0,0),
                    FFPoint(sin(rad(-w)),0,cos(rad(-w)),0),
                    FFPoint(0,0,0,1));
 
 if(updateScale)
  {
   String tmpVal(dec((int)(w+beta)));
   
   if(FF_TclEvalObjv(interp, {"updateRotationY", (const char *)tmpVal})!= 
      TCL_OK)
    {
     ffError(CF_TCL, Tcl_GetStringResult(interp));
    }
  }
 beta += w;
 updateGraph();
}

BOOL FFTclBasicWindow::_flattenEdge()
{
 if(currentE != NULL)
  {
   BOOL tmpUpdate = getUpdate();
   setUpdate(NO);
   
   FFEdge tmpE = currentE->getEdge();
   FFNode source = tmpE->source();
   FFNode target = tmpE->target();
   FFGraphicNode *tmpGS = source->getGraphicNode();
   FFGraphicNode *tmpGT = target->getGraphicNode();
   FFPoint srcP = tmpGS->getCoords();
   FFPoint tgtP = tmpGT->getCoords() - srcP;
   
   FFMatrix tmpM1(FFPoint(1,0,0,0), FFPoint(0,1,0,0),
                  FFPoint(0,0,1,0), -srcP);
   FFMatrix tmpM3(FFPoint(1,0,0,0), FFPoint(0,1,0,0),
                  FFPoint(0,0,1,0), srcP);

   FFCoord w= asin(tgtP.v[1]/tgtP.abs2D());
   
   if(tgtP.v[0] > srcP.v[0])
    w = -w;
   
   FFPoint tmpP(cos(w), sin(w), 0,0);
   
   FFMatrix tmpM2(tmpP, FFPoint(-sin(w),cos(w),0,0),
                  FFPoint(0,0,1,0), FFPoint(0,0,0,1));
   FFMatrix tmpM = ((tmpM1 * tmpM2) * tmpM3);

   const FFSet<FFNode>& nodes=graph->allNodes();
   for(Pix i= nodes.first(); i!= 0; nodes.next(i))
    {
     FFGraphicNode *tmpGN = nodes(i)->getGraphicNode();
     if(tmpGN != NULL)
      {
//        tmpP = tmpGN->getCoords();
//        cerr << nodes(i)->getName() << ": " << tmpP() << ", " 
//             << (tmpP * tmpM1)() << ", " << (tmpP * (tmpM1 * tmpM2))() << ": "
//             << (tmpP *tmpM)() << endl;
       
       tmpGN->moveTo(tmpGN->getCoords() * tmpM);
      }
    }
   setUpdate(tmpUpdate);
  }
 return TCL_OK;
}

void FFTclBasicWindow::rotateZ(FFCoord w)
{
 FFPoint tmpP(cos(rad(w)),sin(rad(w)),0,0);
 FFMatrix tmpM(tmpP,
               FFPoint(-sin(rad(w)),cos(rad(w)),0,0),
               FFPoint(0,0,1,0), FFPoint(0,0,0,1));
 
 ctm = tmpM*ctm;
 
 invCtm *= FFMatrix(FFPoint(cos(rad(-w)),sin(rad(-w)),0,0),
                    FFPoint(-sin(rad(-w)),cos(rad(-w)),0,0),
                    FFPoint(0,0,1,0), FFPoint(0,0,0,1));
 
 if(updateScale)
  {
   String tmpVal(dec((int)(w+gamma)));
   
   if(FF_TclEvalObjv(interp, {"updateRotationZ", (const char *)tmpVal})!= 
      TCL_OK)
    {
     ffError(CF_TCL, Tcl_GetStringResult(interp));
    }
  }
 gamma += w;
 updateGraph();
}

void FFTclBasicWindow::_buildProjMatrix()
{
  projMatrix = FFMatrix(FFPoint(1,0,0,0), FFPoint(0,1,0,0),
                       FFPoint(0,0,1,0), -viewPoint);
  projMatrix = projMatrix * FFMatrix(FFPoint(1,0,0,0), 
                                     FFPoint(0,cos(rad(-xWinkel)),
                                             sin(rad(-xWinkel)),0),
                                     FFPoint(0,-sin(rad(-xWinkel)),
                                             cos(rad(-xWinkel)),0),
                                     FFPoint(0,0,0,1));
 
  projMatrix = projMatrix * FFMatrix(FFPoint(cos(rad(-yWinkel)),0,
                                             -sin(rad(-yWinkel)),0),
                                     FFPoint(0,1,0,0),
                                     FFPoint(sin(rad(-yWinkel)),0,
                                             cos(rad(-yWinkel)),0),
                                     FFPoint(0,0,0,1));

  invProjMatrix =  FFMatrix(FFPoint(cos(rad(yWinkel)),0,
                                    -sin(rad(yWinkel)),0),
                            FFPoint(0,1,0,0),
                            FFPoint(sin(rad(yWinkel)),0,
                                    cos(rad(yWinkel)),0),
                            FFPoint(0,0,0,1));
  
  invProjMatrix = invProjMatrix * FFMatrix(FFPoint(1,0,0,0), 
                                           FFPoint(0,cos(rad(xWinkel)),
                                                   sin(rad(xWinkel)),0),
                                           FFPoint(0,-sin(rad(xWinkel)),
                                                   cos(rad(xWinkel)),0),
                                           FFPoint(0,0,0,1));
  
  invProjMatrix = invProjMatrix * FFMatrix(FFPoint(1,0,0,0), FFPoint(0,1,0,0),
                                           FFPoint(0,0,1,0), viewPoint);
  
  updateGraph();
}

int fgraphWindowProj(ClientData d,Tcl_Interp *i, int k, const char **c) 
{
 FFPoint tmpP;
 std::ostringstream tmpS;
 
 if (k == 3)
  {
   tmpP.v[0] = atof(c[1]);
   tmpP.v[1] = atof(c[2]);   
  }
 else if (k == 4)
  {
   tmpP.v[0] = atof(c[1]);
   tmpP.v[1] = atof(c[2]);   
   tmpP.v[2] = atof(c[3]);   
  }
 else
  {
   Tcl_SetObjResult(i, Tcl_NewStringObj("Wrong Number of arguments", -1));
   return TCL_ERROR;
  }
 tmpP = ((FFTclBasicWindow *)d)->proj(tmpP);

 tmpS << tmpP.v[0] << " " << tmpP.v[1] << " " << tmpP.v[2];
 Tcl_SetObjResult(i, Tcl_NewStringObj(tmpS.str().c_str(), -1));

 return TCL_OK;
}

int fgraphWindowUnproj(ClientData d,Tcl_Interp *i, int k, const char **c) 
{
 FFPoint tmpP;
 std::ostringstream tmpS;
 
 if (k == 3)
  {
   tmpP.v[0] = atof(c[1]);
   tmpP.v[1] = atof(c[2]);   
   tmpP.v[2] = ((FFTclBasicWindow *)d)->getViewDist();
   
  }
 else
  {
   Tcl_SetObjResult(i, Tcl_NewStringObj("Wrong Number of arguments", -1));
   return TCL_ERROR;
  }
 tmpP = ((FFTclBasicWindow *)d)->unProj(tmpP);

 tmpS << tmpP.v[0] << " " << tmpP.v[1] << " " << tmpP.v[2];
 Tcl_SetObjResult(i, Tcl_NewStringObj(tmpS.str().c_str(), -1));

 return TCL_OK;
  
}

FFTclBasicWindow::FFTclBasicWindow() :  nodeMap(NULL), edgeMap(NULL)
{
 viewPoint= FFPoint(0,0,-1000,1);
 xWinkel = yWinkel = zWinkel = 0.0;
 viewDist = 1000.0;
 updateScale = YES;
 
 updateFlag = NO;
 _buildProjMatrix();
 
 ctm= initMatrix;
 invCtm= invInitMatrix;
 
 //  inspector= 0;
 //  updateInsp= YES;
 showNodesFlag = YES;
 showEdgesFlag = YES;
 
 shape = FF_CIRCLE;
 
 width=0;
 height=0;
 labelXoff = 0.0;
 labelYoff = 1.2;
 alreadyInitialisized=NO;
 alpha=beta=gamma=0.0;
    
 modal=NO;
 //  isBored=NO;
 updateFlag = YES;
 
 currentE = 0;
 
 scale = 1.0;
 
 projCommand_ = Tcl_CreateCommand(interp, "proj",
                   fgraphWindowProj,
                   (ClientData)this,
                   projectionCommandDeleted);

 unprojCommand_ = Tcl_CreateCommand(interp, "unproj",
                   fgraphWindowUnproj,
                   (ClientData)this,
                   unprojectionCommandDeleted);
}


void FFTclBasicWindow::event(labelEvent e, labelEventStruct *)
{
  switch(e)
   {
   case DIRECTEDNESS_CHANGE:
    {
     int tmpResult;
     if(graph->getDirected())
      {
       tmpResult =FF_TclEvalObjv(interp, {"setDirected", (const char *)canvas});
      }
     else
      {
       tmpResult =FF_TclEvalObjv(interp, {"unsetDirected", (const char *)canvas});
      }
     if(tmpResult != TCL_OK)
      {
       ffError(RUNTIME_ERROR, Tcl_GetStringResult(interp));
      }		
     break;
    }
   case ADD_GRAPHLABEL:
   case DEL_GRAPHLABEL:
    {
     _updateLayouter();
    }
   default:
    {
     break;
    }
   }
}

int fgraphWindowSel(ClientData data, int off, char *buffer, int maxBytes)
{
  return ((FFTclBasicWindow *)data)->exportSelection(off, buffer, maxBytes);
}

void FFTclBasicWindow::init(const FFGraph& c)
{
 FFGraphLabel::init(c);
 
 if(alreadyInitialisized)
  {
   nodeMap.clear();
   edgeMap.clear();
   currentNSet.clear();
   currentE= 0;
  }	
 else
  {
   if(tclFileName.length() != 0)
    {
     initFromFile(tclFileName);
#if(TK_MAJOR_VERSION < 4)
     Tk_CreateSelHandler(w, XA_STRING, fgraphWindowSel, (int *)this, 
                         XA_STRING);  
#else
     Tk_CreateSelHandler(w, XA_PRIMARY, XA_STRING, fgraphWindowSel, 
                         (int *)this, XA_STRING);  
#endif
     ownsSelectionHandler_ = true;
     mapWindow();
     update();
    }
  }

 int tmpResult = TCL_OK;
 if(graph->getDirected() && interp)
  {
   tmpResult =FF_TclEvalObjv(interp, {"setDirected", (const char *)canvas});
  }
 else if (interp)
  {
   tmpResult =FF_TclEvalObjv(interp, {"unsetDirected", (const char *)canvas});
  }
 if(tmpResult != TCL_OK)
  {
   ffError(RUNTIME_ERROR, Tcl_GetStringResult(interp));
  }		
}

void FFTclBasicWindow::_updateLayouter()
{
  layouter.clear();
  FFSet<FFGraphLabel *> tmpSet;
  graph->getGraphLabel(ffLayoutLabelType, &tmpSet);
  for(Pix i= tmpSet.first(); i!=0; tmpSet.next(i))
    layouter.add((FFLayoutLabel *)tmpSet(i));

}

void FFTclBasicWindow::setCurrent(FFGraphicNode *n)
{
 if( (n == NULL) || currentNSet.contains(n))
  {
   return;
  }
 
 unsetCurrent();
 
 if(FF_TclEvalObjv(interp, {"setSelected", (const char *)n->getId(),
                              (const char *)canvas})!= TCL_OK)
  {
   ffError(CF_TCL, Tcl_GetStringResult(interp));
  }
 
 currentNSet.add(n);
//  if(! currentNSet.contains(n))
//   {
//    cerr << "Internal Set Error!" << endl;
//    cerr << "Set should contain " << n << endl;
//    cerr << "Set contains " << currentNSet.length() << " items" << endl;
//    cerr << "First item is " << currentNSet(currentNSet.first()) << endl;
//   }
 
}

void FFTclBasicWindow::addToCurrent(FFGraphicNode *n)
{
 if(currentNSet.length() > 0)
  {
   if(!currentNSet.contains(n))
    {
     if(FF_TclEvalObjv(interp, {"setSelected", (const char *)n->getId(),
                                  (const char *)canvas})!= TCL_OK)
      {
       ffError(CF_TCL, Tcl_GetStringResult(interp));
      }
     currentNSet.add(n);
    }
  }
 else
  {
   setCurrent(n);
  }
}

void FFTclBasicWindow::selectIfNot(FFGraphicNode *n)
{
  if(n!=NULL)
  {
    if(currentNSet.contains(n))
    {
      unset(n);
//       cerr << "unsetting Node" << endl;
    }
    else
    {
      addToCurrent(n);
//       cerr << "adding Node" << endl;
    }
  }
}

void FFTclBasicWindow::setCurrent(FFGraphicEdge *e)
{
 if((e == NULL) || (e == currentE))
  {
   return;
  }
 
 unsetCurrent();
 
 if(interp)
  {
   FF_TclEvalObjv(interp, {"setSelected", (const char *)e->getId(),
                              (const char *)canvas});
  }
 
 currentE= e;
}

void FFTclBasicWindow::unsetCurrent()
{
 Pix pix= currentNSet.first(), tmpPix;
 
 while(pix !=NULL)
  {
   tmpPix =pix;
   currentNSet.next(pix);
   FFGraphicNode *tmpNode = currentNSet(tmpPix);
   
   if(FF_TclEvalObjv(interp, {"unsetSelected", (const char *)tmpNode->getId(),
                                (const char *)canvas,
                                (const char *)tmpNode->getColor()}) 
      !=TCL_OK) 
    {
     ffError(CF_TCL, Tcl_GetStringResult(interp));
    }
   currentNSet.del(currentNSet(tmpPix));
  }

 if(currentE != NULL)
  {
   if(FF_TclEvalObjv(interp, {"unsetSelected", (const char *)currentE->getId(),
                                (const char *)canvas,
                                (const char *)currentE->getColor()}) != TCL_OK)
    {
     ffError(CF_TCL, Tcl_GetStringResult(interp));
    }
  }
 currentE = 0;
 updateInspector();
}


void FFTclBasicWindow::unset(FFGraphicNode *n)
{
 // cerr << "Unsetting node" << endl;
 
 if(currentNSet.contains(n))
  {
   // cerr << "Really unsettinge" << endl;
   
   if(FF_TclEvalObjv(interp, {"unsetSelected", (const char *)n->getId(),
                             (const char *)canvas, (const char *)n->getColor()})
      != TCL_OK) 
    {
     ffError(CF_TCL, Tcl_GetStringResult(interp));
    }
   
   currentNSet.del(n);
   if(currentNSet.length() < 2)
    updateInspector();
  }		
}

void FFTclBasicWindow::unset(FFGraphicEdge *n)
{
  if(n == currentE)
    unsetCurrent();
}

void FFTclBasicWindow::sizeTo(int x, int y)
{
  width =x;
  height=y;
  String xValue(dec(x));
  String yValue(dec(y));
  FF_TclEvalObjv(interp, {"adjXY", (const char *)canvas, "0", "0",
                          (const char *)xValue, (const char *)yValue});

}

FFPoint FFTclBasicWindow::getRange(const FFGraphicNode *n)
{
  if(FF_TclEvalObjv(interp, {(const char *)canvas, "bbox",
                             (const char *)n->getId()}) != TCL_OK)
          ffError(CF_TCL, Tcl_GetStringResult(interp));
  
  std::istringstream result(Tcl_GetStringResult(interp));
  FFCoord x1 = 0, y1 = 0, x2 = 0, y2 = 0;
  if (!(result >> x1 >> y1 >> x2 >> y2))
   {
    ffError(CF_TCL, "Invalid canvas bounding-box result");
    return FFPoint(0, 0);
   }
  FFPoint tmpCoord(x1, y1);
  FFPoint tmpRange(x2, y2);
//  cerr << "Found Range :" << (tmpRange - tmpCoord)() << endl;
  return tmpRange-tmpCoord;
}

FFTclBasicWindow::~FFTclBasicWindow()
{
 if (interp && !Tcl_InterpDeleted(interp))
  {
   if (projCommand_)
    Tcl_DeleteCommandFromToken(interp, projCommand_);
   if (unprojCommand_)
    Tcl_DeleteCommandFromToken(interp, unprojCommand_);

   // A Tcl script may already have destroyed the root window. `w` then
   // points to freed Tk storage, so only unregister its selection handler
   // while it still matches the interpreter's current main window.
   if (ownsSelectionHandler_ && w && Tk_MainWindow(interp) == w)
    {
#if(TK_MAJOR_VERSION < 4)
     Tk_DeleteSelHandler(w, XA_STRING);
#else
     Tk_DeleteSelHandler(w, XA_PRIMARY, XA_STRING);
#endif
    }
  }
}

//
// If knoten is scalable return the unprojected adjusted value
// If knoten is not scalable return the allready projected adjusted value

FFPoint FFTclBasicWindow::_adjust(FFGraphicNode *knoten,  FFPoint B,
                                  BOOL dontProj) 
{
  FFPoint result;
  FFPoint range= knoten->getRange();
  FFPoint A=knoten->getCoords();
  
//   if(knoten->isScalable())
//    {
//     range= proj(range);
//     A= proj(A);
//     B = proj(B);
//    }

  if(knoten->isOval())
   {
    B -= A;		
    return( A+B*range.v[0]/B.abs());
   }

  FFCoord aa, bb;
  FFCoord width, height;
  
  if(knoten->getShape() < FF_FIGURE)
   {
    width = range.v[0]/2;
    height = range.v[1]/2;
   }
  else // Figures haben eh schon Radius
   {
    width = range.v[0];
    height = range.v[1];
   }
  
  if(!knoten->isScalable() &&
     !dontProj)
   {
    A = proj(A);
    B = proj(B);
   }
  
  if (A.v[1] == B.v[1]) {
   aa = width;
   bb = 0;
  }
  else if (A.v[0] == B.v[0]) {
   aa = 0;
   bb = height;
  }
  else {
   aa = (fabs(A.v[0]-B.v[0])*height)/fabs(A.v[1]-B.v[1]);
   bb = (fabs(A.v[1]-B.v[1])*width)/fabs(A.v[0]-B.v[0]);
   if (aa > width)
    aa = width;
   else
    bb = height;
  }
  
  if (A.v[0] > B.v[0])
   aa = -aa;
  if (A.v[1] > B.v[1])
   bb = -bb;
  
  result.v[0] = A.v[0]+aa;
  result.v[1] = A.v[1]+bb;

  return result;
}

// FFPoint FFTclBasicWindow::adjust(FFGraphicNode *nodeA, FFPoint B)
// {
//  return _adjust(nodeA, B);
// }

String FFTclBasicWindow::addNode(FFGraphicNode *n)
{
 FFPoint tmpCoord = proj(n->getCoords(), n->getRange());
 String xValue(dec((int)tmpCoord.v[0]));
 String yValue(dec((int)tmpCoord.v[1]));
 String radius(dec((int)tmpCoord.v[2]));
 
 if (FF_TclEvalObjv(interp, {"addNode", (const char *)canvas,
                             (const char *)xValue, (const char *)yValue,
                             (const char *)radius, (const char *)n->getColor(),
                             (const char *)n->getIcon(),
                             (const char *)n->getSelIcon()}) != TCL_OK)
  {
   ffError(RUNTIME_ERROR, Tcl_GetStringResult(interp));
   return "";
  }
 
 String tmpRes(Tcl_GetStringResult(interp));
 
 if(FF_TclEvalObjv(interp, {"changeMask", (const char *)tmpRes,
                             (const char *)n->getMask(),
                             (const char *)n->getSelMask(),
                             (const char *)canvas}) != TCL_OK)
  ffError(CF_TCL, Tcl_GetStringResult(interp));

 nodeMap[tmpRes] = n;
 
 updateInspector();
 return tmpRes;
}

String FFTclBasicWindow::addLabel(FFGraphicNode *n)
{
  const char *l = (const char *)(*n)->getName();
  const char *anker = "center";
  
  FFPoint tmpCoord = proj(n->getCoords(), n->getRange());
  
  if(n->getShape() < FF_SCALABLE)
  {
   tmpCoord.v[0] += (10) * labelXoff;
   tmpCoord.v[1] += (10) * labelYoff;
  }
  else
   {
    tmpCoord.v[0] += (tmpCoord.v[2]) *labelXoff;
    tmpCoord.v[0] += (tmpCoord.v[2]) *labelYoff;
   }
   
  if(labelYoff >= 1)
   anker = "n";
  else if (labelYoff <= -1)
   anker = "s";
  else
   {
    if(labelXoff >= 1)
     anker = "w";
    if(labelXoff <= -1)
     anker = "e";
   }
  
  String xValue(dec((int)tmpCoord.v[0]));
  String yValue(dec((int)tmpCoord.v[1]));
    
  if (FF_TclEvalObjv(interp, {"addText", (const char *)canvas,
                              (const char *)xValue, (const char *)yValue,
                              (const char *)l, (const char *)n->getId(), anker}) != TCL_OK)
    {
      ffError(RUNTIME_ERROR, Tcl_GetStringResult(interp));
      return "";
    }

  updateInspector();
//  n->tkLabel = Tcl_GetStringResult(interp); !!! Achtung, evtl. noetig
  return Tcl_GetStringResult(interp);
}

void FFTclBasicWindow::changeLabel(FFGraphicNode *n, const String& label)
{
 const char *l = (const char *)label;
 const char *lid = (const char *)n->getLabelId();
 
 if(FF_TclEvalObjv(interp, {"changeText", (const char *)canvas,
                             lid, l}) != TCL_OK)
  ffError(CF_TCL, Tcl_GetStringResult(interp));
 
 if(currentNSet.contains(n) && (currentNSet.length() ==1))
  updateInspector(); 
 
 return;
}

void FFTclBasicWindow::setSpline(FFGraphicEdge *e, BOOL spline)
{
  String id= e->getId();
//  cerr << "Spline Flag:= " << spline << endl;
  
  String splineFlag(spline ? "1" : "0");
  if(FF_TclEvalObjv(interp, {"smoothLine", (const char *)id,
                             (const char *)splineFlag,
                             (const char *)canvas}) != TCL_OK)
    ffError(CF_TCL, Tcl_GetStringResult(interp));
}


void FFTclBasicWindow::remove(FFGraphicNode *n)
{
 unset(n);  
 if(Tcl_VarEval(interp, "removeItem ", 
                (const char *)canvas, " ",
                (const char *)n->getId(), NULL) !=TCL_OK)
  {
   ffError(CF_TCL, Tcl_GetStringResult(interp));
  }
 if(Tcl_VarEval(interp, "removeItem ", 
                (const char *)canvas, " ",
                (const char *)n->getLabelId(), 
                NULL) != TCL_OK)
  {
   ffError(CF_TCL, Tcl_GetStringResult(interp));
  }
 
 nodeMap.del(n->getId());
}

void FFTclBasicWindow::removeLabel(FFGraphicNode *n)
{
  if(Tcl_VarEval(interp, "removeItem  ", 
                (const char *)canvas, " ",
                (const char *)n->getLabelId(), 
                 NULL) != TCL_OK)
    {
      ffError(CF_TCL, Tcl_GetStringResult(interp));
    }
  updateInspector();
  return;
}

void FFTclBasicWindow::removeLabel(FFGraphicEdge *n)
{
  if(Tcl_VarEval(interp, "removeItem ", 
                (const char *)canvas, " ",
                (const char *)n->getLabelId(), 
                 NULL) != TCL_OK)
    {
      ffError(CF_TCL, Tcl_GetStringResult(interp));
    }
  updateInspector();
  return;
}

void FFTclBasicWindow::updateGraph(FFGraphicNode *n)
{
 if(n->getId().length() == 0)
  return;
  
 FFPoint tmpCoord = proj(n->getCoords(), n->getRange());
 std::ostringstream tmpS;
 
 tmpS << " " << (int)tmpCoord.v[0] << " " << (int)tmpCoord.v[1]
      << " " << (int)tmpCoord.v[2];
 
 if(Tcl_VarEval(interp, "moveNode ", (const char *)canvas,
                " ", (const char *)n->getId(), tmpS.str().c_str(),
                NULL) != TCL_OK)
  {
   ffError(CF_TCL, Tcl_GetStringResult(interp));
  }

 if(n->getLabelId().length() == 0)
  return;
 
 if(n->getShape() < FF_SCALABLE)
  {
   // We are assuming the anchor of the icon is in the middle
   
   tmpCoord.v[0] += (n->getWidth()/2) * labelXoff;
   if((fabs(labelXoff) < 1) && (fabs(labelYoff) < 1))
    tmpCoord.v[1] += (n->getHeight()/2) * 1.2; // Icon Labels sollen nie
                                               // innerhalb haengen
   else
    tmpCoord.v[1] += (n->getHeight()/2) * labelYoff;
  }
 else
  {
   tmpCoord.v[0] += (tmpCoord.v[2]) *labelXoff;
   tmpCoord.v[1] += (tmpCoord.v[2]) *labelYoff;
  }
 
 tmpS.str(std::string());
 tmpS.clear();
 
 tmpS << " " << (int)(tmpCoord.v[0]) << " " << (int)(tmpCoord.v[1]);
 
 if(Tcl_VarEval(interp, "moveLabel ", (const char *)canvas,
                " ", (const char *)n->getLabelId(),
                tmpS.str().c_str(), " ", (const char *)n->getId(), NULL) != TCL_OK)
  {
   ffError(CF_TCL, Tcl_GetStringResult(interp));
  }

}

String FFTclBasicWindow::addEdge(FFGraphicEdge *e)
{
  String tmpS=modifyEdge(e, String("addLine ")+canvas+
    " {"+e->getColor() +"}");
  
  edgeMap[tmpS] = e;
  updateInspector();
  return tmpS;
}

String FFTclBasicWindow::addLabel(FFGraphicEdge *e)
{
  FFPoint tmpCoord = proj((e->getPointAt(0) + e->getPointAt(1))/2);
  String xValue(dec(static_cast<int>(tmpCoord.v[0])));
  String yValue(dec(static_cast<int>(tmpCoord.v[1])));
  const String edgeName = (*e)->getName();
  const String edgeId = e->getId();

  if(FF_TclEvalObjv(interp, {"addText", (const char *)canvas,
                             (const char *)xValue, (const char *)yValue,
                             (const char *)edgeName, (const char *)edgeId}) != TCL_OK)
   {
    ffError(CF_TCL, Tcl_GetStringResult(interp));
    return "";
   }
  updateInspector();
  return Tcl_GetStringResult(interp);
}

void FFTclBasicWindow::changeColor(FFGraphicNode *n, String& color)
{
 if(Tcl_VarEval(interp, "changeNodeColor ", (const char *)n->getId(), " {", 
                (const char *)color , "} ", (const char *)canvas,NULL) 
    != TCL_OK)
  ffError(CF_TCL, Tcl_GetStringResult(interp));
 
 if(currentNSet.contains(n) && (currentNSet.length() ==1))
  updateInspector();
}

String FFTclBasicWindow::changeToWindow(FFGraphicNode *const n)
{
 if(Tcl_VarEval(interp, "changeToWindow ", (const char *)n->getId(),
                " ", (const char *)canvas, NULL) != TCL_OK)
  {
   ffError(CF_TCL, Tcl_GetStringResult(interp));
   return String("");
  }
 
 String tmpS(Tcl_GetStringResult(interp));
 
 if(strlen(Tcl_GetStringResult(interp)) > 0)
  {
   nodeMap.del(n->getId());
   nodeMap[tmpS.after(tmpS.index(RXwhite, -1))] = n;
  }
 return tmpS;	  
}

String FFTclBasicWindow::changeToIcon(FFGraphicNode *n, const String& icon, 
                                      const String& selIcon)
{
 FFPoint tmpCoord = proj(n->getCoords(), n->getRange());
 std::ostringstream tmpScti;
 
 tmpScti << (int)tmpCoord.v[0] << " " << (int)tmpCoord.v[1];

 if(Tcl_VarEval(interp, "changeToIcon {", (const char *)n->getId(), "} {",
                (const char *)icon, "} {", (const char *)selIcon, "} ", 
                tmpScti.str().c_str(), " ", (const char *)canvas, NULL) 
    != TCL_OK)
  { 
   ffError(CF_TCL, Tcl_GetStringResult(interp));
   if(Tcl_VarEval(interp, "changeToIcon {", (const char *)n->getId(), 
                  "} ", "default.xbm",  " default.xbm ", 
                tmpScti.str().c_str(), " ", (const char *)canvas, NULL) 
      != TCL_OK)
    { 
     ffError(CF_TCL, "Couldn't find default icon, please check installation");
     ffError(CF_TCL, Tcl_GetStringResult(interp));
     return String("");
    }
  }
 
 if(strlen(Tcl_GetStringResult(interp)) > 0) // War vorher kein Icon
  {
   nodeMap.del(n->getId());
   nodeMap[String(Tcl_GetStringResult(interp))] = n;
  }
 else  // War vorher schon ein Icon
  {
   // cerr << "Error while changing Icon: new ID is empty" << endl;
  }
 
 return String(Tcl_GetStringResult(interp));	
}

String FFTclBasicWindow::changeToFigure(FFGraphicNode *n, ffShape shape)
{
 FFPoint tmpCoord = proj(n->getCoords(), n->getRange());
 std::ostringstream tmpSctf;
 
 tmpSctf << (int)tmpCoord.v[0] << " " << (int)tmpCoord.v[1]
         << " " <<  (int)tmpCoord.v[2];
 if(Tcl_VarEval(interp, "changeToFigure {", (const char *)n->getId(), "} {", 
                (const char *)n->getColor(), "} ", 
                (const char *)shape2Str(shape), " ",
                tmpSctf.str().c_str(), " ", (const char *)canvas, NULL) 
    != TCL_OK)
  {
   ffError(CF_TCL, Tcl_GetStringResult(interp));
   return String("");
  }
 
 if(strlen(Tcl_GetStringResult(interp)) > 0)
  {
   nodeMap.del(n->getId());
   nodeMap[String(Tcl_GetStringResult(interp))] = n;
  }
 return String(Tcl_GetStringResult(interp));	
}

void FFTclBasicWindow::changeMask(FFGraphicNode *n, const String& mask, 
                                  const String& selMask)
{
  if(Tcl_VarEval(interp, "changeMask ", (const char *)n->getId(), " {",
		 (const char *)mask, "} {", (const char *)selMask,
                 "} ", (const char *)canvas, NULL) 
     != TCL_OK)
    ffError(CF_TCL, Tcl_GetStringResult(interp));
}

void FFTclBasicWindow::changeColor(FFGraphicEdge *e, String& color)
{
  if(Tcl_VarEval(interp, "changeEdgeColor ", (const char *)e->getId(), " {", 
                 (const char *)color , "} ", (const char *)canvas, NULL) != TCL_OK)
    ffError(CF_TCL, Tcl_GetStringResult(interp));
  if(e == currentE)
    updateInspector();
}

void FFTclBasicWindow::showNodes(BOOL flag)
{
  const FFSet<FFNode>& knoten = graph->allNodes();
  Pix i;
  
  for(i=knoten.first(); i != 0; knoten.next(i))
  {
    if(flag == NO)
      knoten(i)->hideGraphic();
    else
      knoten(i)->showGraphic();
  }      
}   

void FFTclBasicWindow::showEdges(BOOL flag)
{
  const FFSet<FFEdge>& kanten = graph->allEdges();
  Pix i;
  
  for(i=kanten.first(); i != 0; kanten.next(i))
  {
    if(flag == NO)
      kanten(i)->hideGraphic();
    else
      kanten(i)->showGraphic();
  }      
}   


void FFTclBasicWindow::changeLabel(FFGraphicEdge *e, const String& label)
{
  const char *l = (const char *)label;
  const char *lid = (const char *)e->getLabelId();
  
  if(FF_TclEvalObjv(interp, {"changeText", (const char *)canvas, lid, l}) != TCL_OK)
   ffError(CF_TCL, Tcl_GetStringResult(interp));
  
  if(e==currentE)
    updateInspector(); 

  return;
}

void FFTclBasicWindow::remove(FFGraphicEdge *n)
{     
  unset(n);
  if(Tcl_VarEval(interp, "removeItem ", 
                (const char *)canvas, " ",
                (const char *)n->getId(), 
                 NULL) != TCL_OK)
    {
      ffError(CF_TCL, Tcl_GetStringResult(interp));
    }    
  if(Tcl_VarEval(interp, "removeItem ", 
                (const char *)canvas, " ",
                (const char *)n->getLabelId(), 
                 NULL) != TCL_OK)
    {
      ffError(CF_TCL, Tcl_GetStringResult(interp));
    }

  edgeMap.del(n->getId());
  
  updateInspector();
}      

void FFTclBasicWindow::updateGraph()
{
 BOOL tmpDate= getUpdate();
 setUpdate(NO);
 
 Pix i;
 for(i=nodeMap.first(); i!=0; nodeMap.next(i))
  if(nodeMap.contents(i))
   updateGraph(nodeMap.contents(i));
 
 for(i=edgeMap.first(); i!=0; edgeMap.next(i))
  if(edgeMap.contents(i))
			updateGraph(edgeMap.contents(i));
 
 _hideLines();
 
 setUpdate(tmpDate);
 if(tmpDate)
 {
  adjustWindowSize();
  FFWindow::update();	
  }
}		

void FFTclBasicWindow::updateGraph(FFGraphicEdge *e)
{
 if(e->getId().length() != 0)
   {
     modifyEdge(e, String("moveLine ") + canvas+" "+e->getId());
   }
}

String FFTclBasicWindow::modifyEdge(FFGraphicEdge *e, String command)
{
 std::ostringstream tmpArg;

 int numPoints = e->anzPoints();
 
  tmpArg << command;
  
  int validPoints=0;

  FFPoint tmpCoord;
//   BOOL circleS = YES;
//   BOOL circleT = YES;

  FFGraphicNode *gnS = (*e)->source()->getGraphicNode() ;
  FFGraphicNode *gnT = (*e)->target()->getGraphicNode() ;
  
  if(gnS && !gnS->isScalable())
   tmpCoord = _adjust(gnS, e->getPointAt(1));
  else
   tmpCoord = proj(e->getPointAt(0));

  if(tmpCoord.v[2] != -1)
   {
    tmpArg << " " << (int)tmpCoord.v[0] << " " << (int)tmpCoord.v[1];
    validPoints++;
   }
  
  for(int k=1; k< numPoints-1; k++)
   {
    tmpCoord = proj(e->getPointAt(k));
    if(tmpCoord.v[2] != -1)
     {
      tmpArg << " " << (int)tmpCoord.v[0] << " " << (int)tmpCoord.v[1];
      validPoints++;
     }
   }

  // tmpCoord = e->getPointAt(e->anzPoints()-1);    

  if(gnT && !gnT->isScalable())
   tmpCoord = _adjust(gnT, e->getPointAt(numPoints-2));
  else
   tmpCoord  = proj(e->getPointAt(numPoints-1));
  
  if(tmpCoord.v[2] != -1)
   {
    tmpArg << " " << (int)tmpCoord.v[0] << " " << (int)tmpCoord.v[1];
    validPoints++;
   }

  if(validPoints < 2)
   {
    if(Tcl_VarEval(interp, (const char *)command , " -1000 -1000 -1000 -1000 ", 
                   NULL) != TCL_OK)
     {
      ffError(CF_TCL, Tcl_GetStringResult(interp));
     }
   }
  else
   {
    if(Tcl_VarEval(interp, tmpArg.str().c_str(), NULL) != TCL_OK)
     {
      ffError(CF_TCL, Tcl_GetStringResult(interp));
     }

   }
  
  String  tmp = Tcl_GetStringResult(interp);
  
  if(e->getLabelId().length())
   {
    tmpCoord = proj((e->getPointAt(0) + e->getPointAt(1))/2);
    std::ostringstream tmpS;
    
    tmpS << " " << (int)tmpCoord.v[0] << " " << (int)tmpCoord.v[1];
    
    if(Tcl_VarEval(interp, "moveLabel ", (const char *)canvas,
                   " ", (const char *)e->getLabelId(),
                   tmpS.str().c_str(), NULL) != TCL_OK)
     {
      ffError(CF_TCL, Tcl_GetStringResult(interp));
     }

   }
  
  return tmp;
}

void FFTclBasicWindow::adjustWindowSize()
{
  if(Tcl_VarEval(interp, "adjWindow ", (const char *)canvas, NULL) != TCL_OK)
  {
    ffError(CF_TCL, Tcl_GetStringResult(interp));
  }
}
  
String FFTclBasicWindow::fsBox(String message, String type, String name)
{
  if(Tcl_VarEval(interp, "FileSelectorBox {", (const char *)message, "} {", 
                (const char *)type, "} {", (const char *)name, "}",
                NULL) != TCL_OK)
  {
   ffError(CF_TCL, Tcl_GetStringResult(interp));
   return "";
  }
 
 return (Tcl_GetStringResult(interp));
}

void FFTclBasicWindow::setBackground(const String& color)
{
  if(Tcl_VarEval(interp, (const char *)canvas," configure -bg  {", 
                 (const char *)color, "}",
                 NULL) != TCL_OK)
   {
    ffError(CF_TCL, Tcl_GetStringResult(interp));
    if(Tcl_VarEval(interp, (const char *)canvas," configure -bg  {", 
                   (const char *)canvColor, "}",
                   NULL) != TCL_OK)
     {
      ffError(CF_TCL, Tcl_GetStringResult(interp));
     } 
   } 
  else
   {
    canvColor = color;
   }
}
