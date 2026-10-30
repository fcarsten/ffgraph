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

#ifndef FF_CLUSTER_WINDOW
#define FF_CLUSTER_WINDOW

#include <tk.h>
#include "globdefs.h"
#include <container/AVLSet.h>
#include "FFTclWindow.h"

class FFClusterWindow: public FFTclWindow
{
protected:
 FFSet<String> visibleSet;
 void hideCluster(const String&);
 void showCluster(const String&);
 void updateClusterDisplay();
 
public:
 virtual FFGraphLabel *proto() const {return new FFClusterWindow;}
 virtual int recall(Tcl_Interp *, int , const char **);
 
 FFClusterWindow();
};

#endif


