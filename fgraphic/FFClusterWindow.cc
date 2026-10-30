/*
// Copyright 1994 Carsten Friedrich, Universitaet Passau
//
// $Author: friedric $
// $Date: 1996/05/06 14:25:43 $
// $RCSfile: FFClusterWindow.cc,v $
// $Revision: 1.1 $
// $State: Exp $
//
*/

#ifdef RCSID
#undef RCSID
#endif
#define RCSID "$Author: friedric $ $RCSfile: FFClusterWindow.cc,v $ $Date: 1996/05/06 14:25:43 $ $Revision: 1.1 $ $State: Exp $"

#include <config.h>

extern "C" {
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <X11/Xatom.h>
}

#include <stream.h>
#include <fstream.h>
#include <string.h>

#include <ffgraph/FFGraph.h>
#include <ffgraph/FFGraphClass.h>
#include <ffgraph/FFClusterWindow.h>
#include <ffgraph/FFGraphicEdge.h>
#include <ffgraph/ffError.h>

#include <container/AVLSet.h>
#include <container/VOHSet.h>
#include <container/VHSet.h>


FFClusterWindow::FFClusterWindow()
{
 String tmpS;
 
 if(getenv("FF_LIBDIR") == NULL)
  tmpS=String(FF_LIBDIR);
 else
  tmpS=String(getenv("FF_LIBDIR"));
 
 tclFileName =tmpS + "/tcl/clusterGraph.tcl";
 if(getenv("FF_ICONDIR") == NULL)
  tmpS=String(FF_LIBDIR) + "/icons/";
 else
  tmpS=String(getenv("FF_ICONDIR"));
 iconLibDir = tmpS;
}


int FFClusterWindow::recall(Tcl_Interp *i, int c, const char **v)
{
 // cerr << "ping" << endl;
 String befehl(v[1]);
 
 if(befehl == "hideCluster")
  {
   for(Pix i=currentNSet.first();i!=0;currentNSet.next(i))
    {
     hideCluster(currentNSet(i)->getColor());
    }
  }
 else  if(befehl == "showCluster")
  {
   for(Pix i=currentNSet.first();i!=0;currentNSet.next(i))
    {
     showCluster(currentNSet(i)->getColor());
    }
  }
 
 return FFTclWindow::recall(i,c,v);
}

void FFClusterWindow::hideCluster(const String & farbe)
{
 cerr << "Hiding Cluster " << farbe << endl;
 if(visibleSet.contains(farbe))
  {
   visibleSet.del(farbe);
   updateClusterDisplay();
  }
}

void FFClusterWindow::showCluster(const String & farbe)
{
 cerr << "Showing Cluster " << farbe << endl;
 
 if(! visibleSet.contains(farbe))
  {
   visibleSet.add(farbe);
   updateClusterDisplay();
  }
}

void FFClusterWindow::updateClusterDisplay()
{
 eventStatus tmpStatus= graph->getEventStatus();
 graph->setEventStatus(EVENT_DISABLED);

 BOOL tmpUpdate=YES;
 setBusy ();
 tmpUpdate = setUpdate(NO);

 const FFSet<FFEdge>& tmpES = graph->allEdges();
 for(Pix i= tmpES.first(); i!= 0; tmpES.next(i))
  {
   FFEdge tmpE= tmpES(i);
   String c1 = tmpE->target()->getGraphicNode()->getColor();
   String c2 = tmpE->source()->getGraphicNode()->getColor();
   
   if(! (visibleSet.contains(c1) || visibleSet.contains(c2)))
    {
     tmpE->hideGraphic();
     tmpE->target()->hideGraphic();
     tmpE->source()->hideGraphic();
    }
  }

 for(Pix i= tmpES.first(); i!= 0; tmpES.next(i))
  {
   FFEdge tmpE= tmpES(i);
   String c1 = tmpE->target()->getGraphicNode()->getColor();
   String c2 = tmpE->source()->getGraphicNode()->getColor();
   
   if((visibleSet.contains(c1) || visibleSet.contains(c2)))
    {
     tmpE->showGraphic();
     tmpE->target()->showGraphic();
     tmpE->source()->showGraphic();
    }
  }

 setUpdate(tmpUpdate);
 unsetBusy();
 adjustWindowSize();
 graph->setEventStatus(tmpStatus);
}
