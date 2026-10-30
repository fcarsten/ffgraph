/*
// Copyright 1994 Carsten Friedrich, Universitaet Passau
//
// $Author: friedric $
// $Date: 1995/11/01 12:02:12 $
// $RCSfile: FFNaivInsp.h,v $
// $Revision: 1.1 $
// $State: Exp $
//
*/

#ifndef FFNAIVINSP
#define FFNAIVINSP

extern "C" {
#include <tk.h>
}

#include "globdefs.h"
#include "FFTclInspector.h"
#include "FFGraph.h"
#include "FFNaiv.h"

class FFNaivInsp: public FFTclInspector
{
//  FFNaiv *naiv;
 public:
  int recall(Tcl_Interp *i, int k, const char **c);
  void inspect();
  FFNaivInsp(FFGraphLabel *);
  virtual ~FFNaivInsp() {}
};
#endif
