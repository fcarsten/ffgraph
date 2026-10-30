/*
// Copyright 1994 Carsten Friedrich, Universitaet Passau
//
// $Author: friedric $
// $Date: 1995/11/01 12:02:14 $
// $RCSfile: FFSugiInsp.h,v $
// $Revision: 1.1 $
// $State: Exp $
//
*/

#ifndef FFSUGIINSP
#define FFSUGIINSP

#include "globdefs.h"
#include "FFTclInspector.h"
#include "FFGraph.h"
#include "FFSugi.h"

class FFSugiInsp: public FFTclInspector
{
//  FFSugi *sugi;
 public:
  int recall(Tcl_Interp *i, int k, const char **c);
  void inspect();
  FFSugiInsp(FFGraphLabel *l);
};
#endif
