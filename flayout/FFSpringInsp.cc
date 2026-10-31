/*
// Copyright 1994 Carsten Friedrich, Universitaet Passau
//
// $Author: schreibe $
// $Date: 1996/08/06 12:02:52 $
// $RCSfile: FFSpringInsp.cc,v $
// $Revision: 1.2 $
// $State: Exp $
//
*/

#ifdef RCSID
#undef RCSID
#endif
#define RCSID "$Author: schreibe $ $RCSfile: FFSpringInsp.cc,v $ $Date: 1996/08/06 12:02:52 $ $Revision: 1.2 $ $State: Exp $"

#include <config.h>
#include <ffgraph/ffError.h>

#include <stream.h>
#include <string.h>
extern "C"{
#include <stdlib.h>
#include <malloc.h>
}

#include <ffgraph/FFSpringInsp.h>

#define SPRING ((FFSpring *)graphLabel)

int FFSpringInsp::recall(Tcl_Interp *, int , const char **c)
{
  String befehl = c[1];    
  if(!SPRING)
    {
      ffError(FATAL, "No SPRINGembedder Layouter found");
    }
  
  if(befehl == "animate")
    {
      if(SPRING->getAnimation())
        {
          SPRING->setAnimation(NO);
        }
      else
        {
          SPRING->setAnimation(YES);
        }
    }
  if(befehl == "toggle3D")
    {
      if(SPRING->getDimension()==2)
       {
        SPRING->setDimension(3);
       }
      else
       {
        SPRING->setDimension(2);
       }
    }
  else if(befehl == "deleteWindow")
    {
      withdraw();
    }
  else if(befehl == "xoff")
    {
      SPRING->setxoff((FFCoord)atoi(c[2]));
    }
  else if(befehl == "yoff")
    {
      SPRING->setyoff((FFCoord)atoi(c[2]));
    }
  else if(befehl == "zoff")
    {
      SPRING->setzoff((FFCoord)atoi(c[2]));
    }
  else if(befehl == "dist")
    {
      SPRING->setOptDistance((FFCoord)atoi(c[2]));
    }
  else if(befehl == "animIters")
    {
      SPRING->setAnimationIntervals(atoi(c[2]));
    }
  else if(befehl == "quality")
    {
     SPRING->setQuality(atoi(c[2]));
    }
  else if(befehl == "stop")
    {
     // cerr << "Stopping ...." << endl;
     
     SPRING->stop();
    }
  else if(befehl == "setEdgeWeight")
    {
      if(SPRING->getUseEdgeWeight() == NO)
	{
	  SPRING->setUseEdgeWeight(YES);
	}
      else
	{
	  SPRING->setUseEdgeWeight(NO);
	}
    }
  else if(befehl == "startLayout")
  {
   if(SPRING->getWindow() && 
      (SPRING->getWindow()->selectedNodes().length()>1))
    {

     FFSet<FFNode> tmpS;
     const FFSet<FFGraphicNode *>& tmpS2 = SPRING->getWindow()->
      selectedNodes();
     
     for(Pix i=tmpS2.first(); i!= 0; tmpS2.next(i))
      tmpS.add(tmpS2(i)->getNode());
     
     SPRING->layout(tmpS);
    }
   else
    {
     SPRING->layout();
    }
  }
  
  return TCL_OK;
}

void FFSpringInsp::inspect()
{		
	if(Tcl_VarEval(interp, "set animate ", 
							(const char *)dec(SPRING->getAnimation()), NULL) != TCL_OK)
		ffError(CF_TCL, Tcl_GetStringResult(interp));
		
	if(Tcl_VarEval(interp, "set 3dlayout ", 
							(const char *)dec(SPRING->getDimension()-2), NULL) != TCL_OK)
		ffError(CF_TCL, Tcl_GetStringResult(interp));
								
	if(Tcl_VarEval(interp, "set edgeweight ", 
							(const char *)dec(SPRING->getUseEdgeWeight()), NULL) != TCL_OK)
		ffError(CF_TCL, Tcl_GetStringResult(interp));
								
	if(Tcl_VarEval(interp, ".positionitems.xoffset.e del 0 end;.positionitems.xoffset.e insert 0 ", 
							(const char *)dec((int)SPRING->getxoff()), NULL) != TCL_OK)
		ffError(CF_TCL, Tcl_GetStringResult(interp));
	if(Tcl_VarEval(interp, ".positionitems.yoffset.e del 0 end;.positionitems.yoffset.e insert 0 ", 
							(const char *)dec((int)SPRING->getyoff()), NULL) != TCL_OK)
		ffError(CF_TCL, Tcl_GetStringResult(interp));
	if(Tcl_VarEval(interp, ".positionitems.zoffset.e del 0 end;.positionitems.zoffset.e insert 0 ", 
							(const char *)dec((int)SPRING->getzoff()), NULL) != TCL_OK)
		ffError(CF_TCL, Tcl_GetStringResult(interp));

	if(Tcl_VarEval(interp, ".positionitems.distance.e del 0 end;.positionitems.distance.e insert 0 ", 
							(const char *)dec((int)SPRING->getOptDistance()), NULL) != TCL_OK)
		ffError(CF_TCL, Tcl_GetStringResult(interp));

	if(Tcl_VarEval(interp, ".quality.scale.scale set ", 
		       (const char *)dec(SPRING->getQuality()), NULL) != TCL_OK)
	  ffError(CF_TCL, Tcl_GetStringResult(interp));
	if(Tcl_VarEval(interp, ".animation.scale set ", 
		       (const char *)dec(SPRING->getAnimationIntervals()), NULL) != TCL_OK)
		ffError(CF_TCL, Tcl_GetStringResult(interp));

}

int springInspRecall(ClientData d,Tcl_Interp *i, int k, const char **c) 
{
  return ((FFSpringInsp *)d)->recall(i,k,c);
}

FFSpringInsp::FFSpringInsp(FFGraphLabel *l) : FFTclInspector(l)
{
  Tcl_CreateCommand(interp, "recall", 
                    springInspRecall, 
		    (int *)this, 
		    nullptr);
 String tmpS;
 
 if(getenv("FF_LIBDIR") == NULL)
  tmpS=String(FF_LIBDIR) + "/tcl/spring.tcl";
 else
  tmpS=String(getenv("FF_LIBDIR")) + "/tcl/spring.tcl";
  
//  spring= s;
  
  initFromFile(tmpS);
  withdraw();
  
  //   free(tmpS);
  
}
