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
 // FFWindow/FFTclBasicWindow constructed a private interpreter, but
 // subwindows borrow their parent's interpreter instead. Discard the
 // private interpreter and its command tokens before adopting the parent.
 if (interp)
  Tcl_DeleteInterp(interp);
 projCommand_ = nullptr;
 unprojCommand_ = nullptr;
 
  parentNode = dad;
  interp=i;
  // Prevent Tcl_DeleteInterp in the parent from freeing this interpreter
  // while a subwindow still holds a callback into it.
  if (interp)
    Tcl_Preserve(interp);
  w=win;
  canvas=winName;
  tkNode=id;
  modal=NO;
  
  // Tcl_CreateCommand registers its own command name; no heap copy needed.
  const String commandName = String("SubRecall.") + canvas;
  if (interp && !Tcl_InterpDeleted(interp))
    subRecallCommand_ = Tcl_CreateCommand(interp, commandName.c_str(),
                                          fgraphSubWindowRecall,
                                          (ClientData)this,
                                          subRecallCommandDeleted);
}

FFTclSubWindow::~FFTclSubWindow()
{
  // Parent and subwindow share the interpreter and main Tk window.
  // Remove only our command; the base classes must not withdraw the
  // parent's window or delete its projection/selection callbacks.
  Tcl_Interp *borrowedInterp = interp;
  if (borrowedInterp)
   {
    if (!Tcl_InterpDeleted(borrowedInterp) && subRecallCommand_)
      Tcl_DeleteCommandFromToken(borrowedInterp, subRecallCommand_);
    interp = nullptr;
    Tcl_Release(borrowedInterp);
   }
}

void FFTclSubWindow::init(const FFGraph& c)
{
  FFTclBasicWindow::init(c);
  // A subwindow borrows the parent's Tk_Window. Registering a selection
  // callback here would overwrite the parent's handler with a pointer to
  // this object, which becomes dangling on subwindow deletion.
  alreadyInitialisized = YES;
}
