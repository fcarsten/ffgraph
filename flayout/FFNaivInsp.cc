/*
// Copyright 1994 Carsten Friedrich, Universitaet Passau
//
// $Author: schreibe $
// $Date: 1996/08/06 12:02:52 $
// $RCSfile: FFNaivInsp.cc,v $
// $Revision: 1.2 $
// $State: Exp $
//
*/

#ifdef RCSID
#undef RCSID
#endif
#define RCSID "$Author: schreibe $ $RCSfile: FFNaivInsp.cc,v $ $Date: 1996/08/06 12:02:52 $ $Revision: 1.2 $ $State: Exp $"

#include <config.h>
#include <ffgraph/ffError.h>

#include <stream.h>
#include <string.h>
extern "C"{
#include <stdlib.h>
#include <malloc.h>
}

#include <ffgraph/FFNaivInsp.h>

#define NAIV ((FFNaiv *)graphLabel)

int FFNaivInsp::recall(Tcl_Interp *, int , const char **c)
{
  String befehl = c[1];    
  if(!NAIV)
   {
    ffError(FATAL, "No NAIV Layouter found");
    return TCL_OK;
   }
  
  else if(befehl == "xoff")
   {
    // cerr << "Setting xoff to " << c[2] << endl;
    NAIV->setxoff((FFCoord)atoi(c[2]));
   }
  else if(befehl == "yoff")
   {
    // cerr << "Setting yoff to " << c[2] << endl;
    NAIV->setyoff((FFCoord)atoi(c[2]));
   }
  else if(befehl == "deleteWindow")
   {
    withdraw();
   }
  else if(befehl == "startLayout")
    NAIV->layout();
  
  return TCL_OK;
}

void FFNaivInsp::inspect()
{		
 if(Tcl_VarEval(interp, 
                ".positionitems.xoffset.e del 0 end;\n.positionitems.xoffset.e insert 0 ", 
                (const char *)dec((int)NAIV->getxoff()), NULL) != TCL_OK)
  ffError(CF_TCL, Tcl_GetStringResult(interp));
 
 if(Tcl_VarEval(interp, 
                ".positionitems.yoffset.e del 0 end;\n.positionitems.yoffset.e insert 0 ", 
                (const char *)dec((int)NAIV->getyoff()), NULL) != TCL_OK)
  ffError(CF_TCL, Tcl_GetStringResult(interp));
}

int naivInspRecall(ClientData d,Tcl_Interp *i, int k, const char **c) 
{
  return ((FFNaivInsp *)d)->recall(i,k,c);
}

FFNaivInsp::FFNaivInsp(FFGraphLabel *l) : FFTclInspector(l)
{
 Tcl_CreateCommand(interp, "recall", 
                   naivInspRecall, 
                   (int *)this, 
                   nullptr);
 String tmpS;
 
 if(getenv("FF_LIBDIR") == NULL)
  tmpS=String(FF_LIBDIR) + "/tcl/naiv.tcl";
 else
  tmpS=String(getenv("FF_LIBDIR")) + "/tcl/naiv.tcl";
 
 initFromFile(tmpS);
 withdraw();
}
