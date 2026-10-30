/*
// Copyright 1994 Carsten Friedrich, Universitaet Passau
//
// $Author: friedric $
// $Date: 1996/05/06 14:24:48 $
// $RCSfile: FFTcl3DWindow.h,v $
// $Revision: 1.1 $
// $State: Exp $
//
*/

#ifndef FF_TCL_3D_WINDOW
#define FF_TCL_3D_WINDOW

#include <tk.h>
#include "globdefs.h"

#include "FFTclWindow.h"

class FFTcl3DWindow: public FFTclWindow
{
protected:
 void _savePOV(const String&);
 
public:
 virtual FFGraphLabel *proto() const {return new FFTcl3DWindow;}
 virtual int recall(Tcl_Interp *, int , const char **);

 virtual void _hideLines();
 
 
 // virtual int recall(Tcl_Interp *, int , const char **);
 // virtual void init(const FFGraph& );
 FFTcl3DWindow();
 // virtual ~FFTclWindow();
};

#endif


