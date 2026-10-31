#ifdef RCSID
#undef RCSID
#endif
#define RCSID "$Author: deepthou $ $RCSfile: FFSugiInsp.cc,v $ $Date: 1996/12/17 07:44:36 $ $Revision: 1.10 $ $State: Exp $"

#include <config.h>
#include <ffgraph/ffError.h>

#include <stream.h>
#include <string.h>
#include <String.h>
extern "C"
{
#include <stdlib.h>
#include <malloc.h>
}

#include <ffgraph/FFSugiInsp.h>

#define SUGI ((FFSugi *) graphLabel)

int FFSugiInsp::recall (Tcl_Interp *, int k, const char **c)
{
  String befehl = c[1];
  
  if (!SUGI)
    {
      ffError (FATAL, "No Sugiyama Layouter found");
    }
  
  if (befehl == "toggleDecycle")
    {
      if (SUGI->getUseStartNode ())
        {
	  SUGI->setUseStartNode (NO);
        }
      else
        {
          SUGI->setUseStartNode (YES);
        }
      this->inspect ();
    }
  else if (befehl == "adjazenz")
    {
      SUGI->setCrossingAlg (0);
      this->inspect ();
    }
  else if (befehl == "noneRed")
    {
      SUGI->setCrossingAlg (2);
      this->inspect ();
    }
  else if (befehl == "deleteWindow")
    {
      withdraw ();
    }
  else if (befehl == "xoff")
    {
      SUGI->setxoff ((FFCoord) atoi (c[2]));
    }
  else if (befehl == "yoff")
    {
      SUGI->setyoff ((FFCoord) atoi (c[2]));
    }
  else if (befehl == "zoff")
    {
      SUGI->setzoff ((FFCoord) atoi (c[2]));
    }
  else if (befehl == "xdist")
    {
      SUGI->setxDistance ((FFCoord) atoi (c[2]));
    }
  else if (befehl == "ydist")
    {
      SUGI->setyDistance ((FFCoord) atoi (c[2]));
    }	
  else if (befehl == "zdist")
    {
      SUGI->setzDistance ((FFCoord) atoi (c[2]));
    }	
  else if (befehl == "redCrossIters")
    {
      SUGI->setIteration1 (atoi (c[2]));
    }
  else if (befehl == "useGrid")
    {
      if (SUGI->getUseGrid () == NO)
        {
          SUGI->setUseGrid (YES);
        }
      else
        {
          SUGI->setUseGrid (NO);
        }
      this->inspect ();
    }
  else if (befehl == "leavesDown")
    {
      if (SUGI->getLeavesDown () == NO)
        {
          SUGI->setLeavesDown (YES);
        }
      else
        {
          SUGI->setLeavesDown (NO);
        }
      this->inspect ();
    }
  else if (befehl == "toggleDeterministic")
    {
      if (SUGI->getDeterministic () == NO)
        {
          SUGI->setDeterministic (YES);
        }
      else
        {
          SUGI->setDeterministic (NO);
        }
      this->inspect ();
    }
  else if (befehl == "set3D")
    {
      SUGI->setDimension (3);
      this->inspect ();
    }
  else if (befehl == "set2D")
    {
      SUGI->setDimension (2);
      this->inspect ();
    }
  else if (befehl == "setPositionMax")
    {
      SUGI->setSameSize (1);
      this->inspect ();
    }
  else if (befehl == "setPositionRelative")
    {
      SUGI->setSameSize (2);
      this->inspect ();
    }
  else if (befehl == "startLayout") 
    {
      SUGI->layout ();
    } 
  else if (befehl == "orientHorizontal") 
    {
      SUGI->setOrient (1);
      this->inspect ();
    }
  else if (befehl == "orientVertical") 
    {
      SUGI->setOrient (2);
      this->inspect ();
    }
  else 
    {
      cerr << "\nI don't know anything of\n  -->";
      for (int j = 0; j < k; j++) 
	{
	  cerr << "(" << c[j] << ") ";
	}
    }
  return (TCL_OK);
}

void FFSugiInsp::inspect ()
{
  if (Tcl_VarEval (interp, "set toggleDecycle ", 
		   (const char *) dec (SUGI->getUseStartNode ()), NULL) 
      != TCL_OK)
    ffError (CF_TCL, Tcl_GetStringResult(interp));
  
  if (Tcl_VarEval (interp, "set useGrid ", 
		   (const char *) dec (SUGI->getUseGrid ()), NULL) 
      != TCL_OK)
    ffError (CF_TCL, Tcl_GetStringResult(interp));
  
  if (Tcl_VarEval (interp, "set leavesDown ", 
		 (const char *) dec (SUGI->getLeavesDown ()), NULL) 
      != TCL_OK)
    ffError (CF_TCL, Tcl_GetStringResult(interp));

  if (Tcl_VarEval (interp, "set position ", 
		   (const char *) dec (SUGI->getSameSize ()), NULL) 
      != TCL_OK)
    ffError (CF_TCL, Tcl_GetStringResult(interp));
  
  if (Tcl_VarEval (interp, "set orientation ", 
		   (const char *) dec (SUGI->getOrient ()), NULL) 
      != TCL_OK)
    ffError (CF_TCL, Tcl_GetStringResult(interp));
  
  if (Tcl_VarEval (interp, "set dimension ", 
		   (const char *) dec (SUGI->getDimension ()), NULL) 
      != TCL_OK)
    ffError (CF_TCL, Tcl_GetStringResult(interp));
  
  if (Tcl_VarEval (interp, "set deterministic ", 
		   (const char *) dec (SUGI->getDeterministic ()), NULL) 
      != TCL_OK)
    ffError (CF_TCL, Tcl_GetStringResult(interp));
  
  if (Tcl_VarEval (interp, ".middleframe.reduce_items.scale.scale set ", 
		   (const char *) dec ((int) SUGI->getIteration1 ()), NULL) 
      != TCL_OK)
    ffError (CF_TCL, Tcl_GetStringResult(interp));
  
  if (Tcl_VarEval (interp, 
		   "Sugi_EntryInsert .topframe.position.top.offset.x.e ", 
		   (const char *) dec ((int) SUGI->getxoff ()), NULL) 
      != TCL_OK)
    ffError (CF_TCL, Tcl_GetStringResult(interp));

  if (Tcl_VarEval (interp, 
		   "Sugi_EntryInsert .topframe.position.top.offset.y.e ", 
		   (const char *) dec ((int) SUGI->getyoff ()), NULL) 
      != TCL_OK)
    ffError (CF_TCL, Tcl_GetStringResult(interp));
  
  if (Tcl_VarEval (interp, 
		   "Sugi_EntryInsert .topframe.position.top.offset.z.e ", 
		   (const char *) dec ((int) SUGI->getzoff ()), NULL) 
      != TCL_OK)
    ffError (CF_TCL, Tcl_GetStringResult(interp));
  
  if (Tcl_VarEval (interp, 
		   "Sugi_EntryInsert .topframe.position.top.distance.x.e ", 
		   (const char *) dec ((int) SUGI->getxDistance ()), NULL) 
      != TCL_OK)
    ffError (CF_TCL, Tcl_GetStringResult(interp));
  if (Tcl_VarEval (interp, 
		   "Sugi_EntryInsert .topframe.position.top.distance.y.e ", 
		   (const char *) dec ((int) SUGI->getyDistance ()), NULL) 
      != TCL_OK)
    ffError (CF_TCL, Tcl_GetStringResult(interp));
  if (Tcl_VarEval (interp, 
		   "Sugi_EntryInsert .topframe.position.top.distance.z.e ", 
		   (const char *) dec ((int) SUGI->getzDistance ()), NULL) 
      != TCL_OK)
    ffError (CF_TCL, Tcl_GetStringResult(interp));
}

int sugiInspRecall (ClientData d,Tcl_Interp *i, int k, const char **c) 
{
  return ((FFSugiInsp *) d)->recall (i, k, c);
}

FFSugiInsp::FFSugiInsp (FFGraphLabel *l) : FFTclInspector (l)
{
  recallCommand_ = Tcl_CreateCommand (interp, "recall",
		     sugiInspRecall, 
		     (int *) this, 
		     recallCommandDeleted);
 String tmpS;
 
 if (getenv ("FF_LIBDIR") == NULL)
   tmpS=String (FF_LIBDIR) + "/tcl/sugi.tcl";
 else
   tmpS=String (getenv ("FF_LIBDIR")) + "/tcl/sugi.tcl";
 
 initFromFile (tmpS); 
 withdraw ();
}
