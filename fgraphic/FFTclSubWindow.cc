/*
// Copyright 1994 Carsten Friedrich, Universitaet Passau
//
// $Author: friedric $
// $Date: 1996/05/06 14:18:05 $
// $RCSfile: FFTclSubWindow.cc,v $
// $Revision: 1.5 $
// $State: Exp $
//
*/

#ifdef RCSID
#undef RCSID
#endif
#define RCSID "$Author: friedric $ $RCSfile: FFTclSubWindow.cc,v $ $Date: 1996/05/06 14:18:05 $ $Revision: 1.5 $ $State: Exp $"

#include <config.h>

extern "C" {
#include <malloc.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <X11/Xatom.h>
}

#include <stream.h>
#include <string.h>
#include <ffgraph/FFGraphClass.h>
#include <ffgraph/FFTclSubWindow.h>
#include <ffgraph/FFTclCompat.h>
#include <ffgraph/FFGraph.h>
#include <ffgraph/FFGraphicNode.h>
#include <ffgraph/FFGraphicEdge.h>
#include <ffgraph/FFNodeLabel.h>
#include <ffgraph/FFGraphLabel.h>
#include <ffgraph/FFEdgeLabel.h>

#include <ffgraph/ffError.h>

#include <container/AVLSet.h>
#include <container/VOHSet.h>
								
void fgraphSubWindowTimer(ClientData)
{
//  ((FFTclSubWindow *)d)->timer();
}

int FFTclSubWindow::recall(Tcl_Interp *i, int k, const char **c)
{
//  String befehl =c[1];
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
 
 return FFTclBasicWindow::recall(i, k, c);
}

int fgraphSubWindowRecall(ClientData d,Tcl_Interp *i, int k, const char **c) 
{
  return ((FFTclSubWindow *)d)->recall(i,k,c);
}

FFTclSubWindow::FFTclSubWindow(FFNode dad, Tcl_Interp *i, Tk_Window win,
                               const String& winName, const String& id)  
{
 if(interp)
  {
   FF_TclEvalObjv(interp, {"wm", "withdraw", "."});
   Tcl_DeleteInterp(interp);
  }
 
  parentNode = dad;
  interp=i;
  w=win;
  canvas=winName;
  tkNode=id;
  modal=NO;
  
  // Tcl_CreateCommand registers its own command name; no heap copy needed.
  const String commandName = String("SubRecall.") + canvas;
  Tcl_CreateCommand(interp, commandName.c_str(),
                    fgraphSubWindowRecall,
                    (ClientData)this,
                    nullptr);
}

int fgraphSubWindowSel(ClientData data, int off, char *buffer, int maxBytes)
{
  return ((FFTclBasicWindow *)data)->exportSelection(off, buffer, maxBytes);
}

void FFTclSubWindow::init(const FFGraph& c)
{
  FFTclBasicWindow::init(c);

  if(!alreadyInitialisized)
  {
    alreadyInitialisized=YES;
#if(TK_MAJOR_VERSION < 4)
    Tk_CreateSelHandler(w, XA_STRING, fgraphSubWindowSel, (int *)this, 
                        XA_STRING);  
#else
    Tk_CreateSelHandler(w, XA_PRIMARY, XA_STRING, fgraphSubWindowSel, 
                        (int *)this, XA_STRING);  
#endif
    // Tk_CreateSelHandler(windowId, XA_STRING, fgraphSubWindowSel, (int *)this, XA_STRING);  
  }
}


