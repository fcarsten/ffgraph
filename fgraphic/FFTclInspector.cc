/*
// Copyright 1994 Carsten Friedrich, Universitaet Passau
//
// $Author: friedric $
// $Date: 1995/11/01 12:02:59 $
// $RCSfile: FFTclInspector.cc,v $
// $Revision: 1.2 $
// $State: Exp $
//
*/

#ifdef RCSID
#undef RCSID
#endif
#define RCSID "$Author: friedric $ $RCSfile: FFTclInspector.cc,v $ $Date: 1995/11/01 12:02:59 $ $Revision: 1.2 $ $State: Exp $"

#include <ffgraph/FFTclInspector.h>
#include <ffgraph/FFTclCompat.h>
#include <ffgraph/ffError.h>
#include <cstring>

bool FFTclInspector::hasLiveWindow() const
{
  if (!interp || Tcl_InterpDeleted(interp))
    return false;
  // Tcl still exists after Tk's root window has been destroyed.
  // A window-manager request to a non-existent root cannot reopen it.
  if (FF_TclEvalObjv(interp, {"winfo", "exists", "."}) != TCL_OK)
    return false;
  return std::strcmp(Tcl_GetStringResult(interp), "1") == 0;
}

void FFTclInspector::show()
{
  if (!hasLiveWindow())
  {
    ffError(RUNTIME_WARNING, "Inspector Tk window no longer exists");
    return;
  }
  // Deiconify directly (iconify is unnecessary and can upset window-manager
  // state). Raise it too: deiconification does not guarantee stacking above
  // the application's graph window on every window manager.
  if (deiconify() != TCL_OK ||
      FF_TclEvalObjv(interp, {"raise", "."}) != TCL_OK)
    ffError(CF_TCL, Tcl_GetStringResult(interp));
}






