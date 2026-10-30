/*
// Copyright 1994 Carsten Friedrich, Universitaet Passau
//
// $Author: friedric $
// $Date: 1995/11/01 12:02:12 $
// $RCSfile: FFSpringInsp.h,v $
// $Revision: 1.1 $
// $State: Exp $
//
*/

#ifndef FFSPRINGINSP
#define FFSPRINGINSP

extern "C" {
#include <tk.h>
}

#include "globdefs.h"
#include "FFTclInspector.h"
#include "FFGraph.h"
#include "FFSpring.h"

class FFSpringInsp: public FFTclInspector
{
//  FFSpring *spring;
 public:
  int recall(Tcl_Interp *i, int k, const char **c);
  void inspect();
  FFSpringInsp(FFGraphLabel *);
  virtual ~FFSpringInsp() {}
};
#endif
