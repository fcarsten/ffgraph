/*
// Copyright 1994 Carsten Friedrich, Universitaet Passau
//
// $Author: friedric $
// $Date: 1995/11/01 12:02:17 $
// $RCSfile: FFWindow.h,v $
// $Revision: 1.1 $
// $State: Exp $
//
*/

//
//  FFWindow.h
//
// A Class for interpreting tcl-scripts.
// Copyright 1993 by Carsten Friedrich, Universitaet Passau
//
// no warranties of any kind.
// everything else according to the gnu public license
//

#ifndef FFWINDOW
#define FFWINDOW

extern "C" {
#include <tk.h>
}

#include <String.h>

class FFWindow 
{
protected:
  
  Tk_Window w;
  String display;
  String geometry;
  Tcl_Interp *interp;
  
 public:

  Tcl_Interp *getInterpreter()
   {return interp;}

  int mapWindow();
  int unmapWindow();
  int withdraw();
  int iconify();
  int deiconify();
  virtual void update();

  void structureProc(XEvent);  
  void initFromFile(const String&, int argc=0, char **argv=NULL);
  void setDisplay(const String&);
  virtual int setTitle(const String&);

  virtual void setSecurity(int) {};

  FFWindow();
  virtual ~FFWindow();
};

#endif

