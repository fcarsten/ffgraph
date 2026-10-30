/*
// Copyright 1994 Carsten Friedrich, Universitaet Passau
//
// $Author: friedric $
// $Date: 1995/11/01 12:02:12 $
// $RCSfile: FFOrthoInsp.h,v $
// $Revision: 1.1 $
// $State: Exp $
//
*/

#ifndef FFORTHOINSP
#define FFORTHOINSP

extern "C" {
#include <tk.h>
}

#include "globdefs.h"
#include "FFTclInspector.h"
#include "FFGraph.h"
#include "FFOrtho.h"

class FFOrthoInsp: public FFTclInspector
{
 public:
  int recall(Tcl_Interp *i, int k, const char **c);
  void inspect();
  FFOrthoInsp(FFGraphLabel *);
  virtual ~FFOrthoInsp() {}
};
#endif
