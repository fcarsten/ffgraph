/*
// Copyright 1994 Carsten Friedrich, Universitaet Passau
//
// $Author: schreibe $
// $Date: 1996/08/06 12:02:52 $
// $RCSfile: FFOrthoInsp.cc,v $
// $Revision: 1.2 $
// $State: Exp $
//
*/

#ifdef RCSID
#undef RCSID
#endif
#define RCSID "$Author: schreibe $ $RCSfile: FFOrthoInsp.cc,v $ $Date: 1996/08/06 12:02:52 $ $Revision: 1.2 $ $State: Exp $"

#include <config.h>
#include <ffgraph/ffError.h>

#include <stream.h>
#include <string.h>
extern "C"{
#include <stdlib.h>
#include <malloc.h>
}

#include <ffgraph/FFOrthoInsp.h>

#define ORTHO ((FFOrtho *)graphLabel)

int FFOrthoInsp::recall(Tcl_Interp *, int , const char **c)
{
  String befehl = c[1];    
  if(!ORTHO)
   {
    ffError(FATAL, "No ORTHO Layouter found");
    return TCL_OK;
   }
  
  else if(befehl == "xoff")
   {
    // cerr << "Setting xoff to " << c[2] << endl;
    ORTHO->setxoff((FFCoord)atoi(c[2]));
   }
  else if(befehl == "yoff")
   {
    // cerr << "Setting yoff to " << c[2] << endl;
    ORTHO->setyoff((FFCoord)atoi(c[2]));
   }
  else if(befehl == "deleteWindow")
   {
    withdraw();
   }
  else if(befehl == "toggle3D")
   {
    if(ORTHO->get3D())
     ORTHO->set3D(NO);
    else
     ORTHO->set3D(YES);
   }
  else if(befehl == "toggleAnim")
   {
    if(ORTHO->getAnim())
     ORTHO->setAnim(NO);
    else
     ORTHO->setAnim(YES);
   }
  else if(befehl == "toggleSwapping")
   {
    if(ORTHO->getSwapping())
     ORTHO->setSwapping(NO);
    else
     ORTHO->setSwapping(YES);
   }
  else if(befehl == "toggleSwapBadies")
   {
    if(ORTHO->getSwapBadies())
     ORTHO->setSwapBadies(NO);
    else
     ORTHO->setSwapBadies(YES);
   }
  else if(befehl == "toggleEdgeAdjust")
   {
    if(ORTHO->getEdgeAdjust())
     ORTHO->setEdgeAdjust(NO);
    else
     ORTHO->setEdgeAdjust(YES);
   }
  else if(befehl == "setRandom")
   {
    ORTHO->setInitPlacement("random");
   }
  else if(befehl == "setAdjust")
   {
    ORTHO->setInitPlacement("adjust");
   }
  else if(befehl == "startLayout")
    ORTHO->layout();
  
  return TCL_OK;
}

void FFOrthoInsp::inspect()
{		
 if(Tcl_VarEval(interp, 
                ".positionitems.xoffset.e del 0 end;\n.positionitems.xoffset.e insert 0 ", 
                (const char *)dec((int)ORTHO->getxoff()), NULL) != TCL_OK)
  ffError(CF_TCL, Tcl_GetStringResult(interp));
 
 if(Tcl_VarEval(interp, 
                ".positionitems.yoffset.e del 0 end;\n.positionitems.yoffset.e insert 0 ", 
                (const char *)dec((int)ORTHO->getyoff()), NULL) != TCL_OK)
  ffError(CF_TCL, Tcl_GetStringResult(interp));

 if(ORTHO->get3D())
  {
   if(Tcl_VarEval(interp, ".checkBox.3d select ", NULL) != TCL_OK)
    ffError(CF_TCL, Tcl_GetStringResult(interp));
  }
 else
  if(Tcl_VarEval(interp, ".checkBox.3d deselect ", NULL) != TCL_OK)
   ffError(CF_TCL, Tcl_GetStringResult(interp));

 if(ORTHO->getAnim())
  {
   if(Tcl_VarEval(interp, ".checkBox.anim select ", NULL) != TCL_OK)
    ffError(CF_TCL, Tcl_GetStringResult(interp));
  }
 else
  if(Tcl_VarEval(interp, ".checkBox.anim deselect ", NULL) != TCL_OK)
   ffError(CF_TCL, Tcl_GetStringResult(interp));

 if(ORTHO->getSwapping())
  {
   if(Tcl_VarEval(interp, ".stepBox.swapping select ", NULL) != TCL_OK)
    ffError(CF_TCL, Tcl_GetStringResult(interp));
  }
 else
  if(Tcl_VarEval(interp, ".stepBox.swapping deselect ", NULL) != TCL_OK)
   ffError(CF_TCL, Tcl_GetStringResult(interp));

 if(ORTHO->getSwapBadies())
  {
   if(Tcl_VarEval(interp, ".stepBox.swapbad select ", NULL) != TCL_OK)
    ffError(CF_TCL, Tcl_GetStringResult(interp));
  }
 else
  if(Tcl_VarEval(interp, ".stepBox.swapbad deselect ", NULL) != TCL_OK)
   ffError(CF_TCL, Tcl_GetStringResult(interp));

 if(ORTHO->getEdgeAdjust())
  {
   if(Tcl_VarEval(interp, ".stepBox.edgeadjust select ", NULL) != TCL_OK)
    ffError(CF_TCL, Tcl_GetStringResult(interp));
  }
 else
  if(Tcl_VarEval(interp, ".stepBox.edgeadjust deselect ", NULL) != TCL_OK)
   ffError(CF_TCL, Tcl_GetStringResult(interp));

 if(Tcl_VarEval(interp, "global initPlacement; set initPlacement ", 
                (const char *)ORTHO->getInitPlacement(),  NULL) != TCL_OK)
  ffError(CF_TCL, Tcl_GetStringResult(interp));

}

int orthoInspRecall(ClientData d,Tcl_Interp *i, int k, const char **c) 
{
  return ((FFOrthoInsp *)d)->recall(i,k,c);
}

FFOrthoInsp::FFOrthoInsp(FFGraphLabel *l) : FFTclInspector(l)
{
 Tcl_CreateCommand(interp, "recall", 
                   orthoInspRecall, 
                   (int *)this, 
                   nullptr);
 String tmpS;
 
 if(getenv("FF_LIBDIR") == NULL)
  tmpS=String(FF_LIBDIR) + "/tcl/ortho.tcl";
 else
  tmpS=String(getenv("FF_LIBDIR")) + "/tcl/ortho.tcl";
 
 initFromFile(tmpS);
 withdraw();
}
