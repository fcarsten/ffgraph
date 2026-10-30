/*
// Copyright 1994 Carsten Friedrich, Universitaet Passau
//
// $Author: friedric $
// $Date: 1995/11/01 12:02:16 $
// $RCSfile: FFTclInspector.h,v $
// $Revision: 1.1 $
// $State: Exp $
//
*/

#ifndef FF_TCL_INSPECTOR
#define FF_TCL_INSPECTOR

#include "FFInspector.h"
#include "FFWindow.h"

class FFTclInspector : public FFWindow, public FFInspector
{
public:
  virtual void hide() {withdraw();}
  virtual void show();
 virtual void update(){FFWindow::update();}
 
  FFTclInspector(FFGraphLabel *g) : FFInspector(g) {}
  virtual ~FFTclInspector(){}
};

#endif
