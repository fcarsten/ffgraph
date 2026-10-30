/*
// Copyright 1994 Carsten Friedrich, Universitaet Passau
//
// $Author: friedric $
// $Date: 1996/05/06 14:24:50 $
// $RCSfile: FFTclSubWindow.h,v $
// $Revision: 1.4 $
// $State: Exp $
//
*/

#ifndef FF_TCL_SUB_WINDOW
#define FF_TCL_SUB_WINDOW

#include "FFTclBasicWindow.h"

#define cfEmpty String("")

class FFTclSubWindow: public FFTclBasicWindow
{
protected:
 String tkNode;
 FFNode parentNode;
 //  	Tk_Window windowId;
 
public:
 
 virtual FFGraphLabel *proto() const {return new FFTclSubWindow;}
 
 virtual int recall(Tcl_Interp *i, int k, const char **c);  
 virtual void init(const FFGraph& c);
 
 FFTclSubWindow(FFNode dad= NULL, Tcl_Interp *i=NULL, Tk_Window win=0, 
                const String& w=cfEmpty, const String& id=cfEmpty);
 virtual ~FFTclSubWindow(){interp= 0;}
};

#endif

