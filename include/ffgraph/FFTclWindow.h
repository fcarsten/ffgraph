/*
// Copyright 1994 Carsten Friedrich, Universitaet Passau
//
// $Author: deepthou $
// $Date: 1996/06/18 06:56:39 $
// $RCSfile: FFTclWindow.h,v $
// $Revision: 1.3 $
// $State: Exp $
//
*/

#ifndef FF_TCL_WINDOW
#define FF_TCL_WINDOW

#include <tk.h>
#include "globdefs.h"

#include "FFTclBasicWindow.h"

class FFTclWindow: public FFTclBasicWindow
{
protected:
  BOOL  updateInsp;
  String iconLibDir;
  static void recallCommandDeleted(ClientData d)
  {
    static_cast<FFTclWindow*>(d)->recallCommand_ = nullptr;
  }
  Tcl_Command recallCommand_ = nullptr;
  
 // virtual void _buildExportString();
  virtual void _buildLayoutMenu();
  virtual void _updateLayouter() 
    {FFTclBasicWindow::_updateLayouter();
      _buildLayoutMenu();}

public:
 virtual FFGraphLabel *proto() const {return new FFTclWindow;}
  virtual void event(labelEvent, labelEventStruct *);
  
  virtual int recall(Tcl_Interp *i, int k, const char **c);
//  virtual void timer();

  virtual void setCurrent(FFGraphicNode *);
  virtual void setCurrent(FFGraphicEdge *);
  virtual void addToCurrent(FFGraphicNode *);

  virtual void updateInspector();
  virtual void showInspector();
  
  virtual void init(const FFGraph& c);
  FFTclWindow();
  virtual ~FFTclWindow();
};

int fgraphWindowSel(ClientData data, int off, char *buffer, int maxBytes);
void fgraphWindowLostSel(ClientData);
int fgraphWindowGetSel(ClientData data, Tcl_Interp *, const char *portion);

#endif


