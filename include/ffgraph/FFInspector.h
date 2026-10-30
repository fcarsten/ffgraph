/*
// Copyright 1994 Carsten Friedrich, Universitaet Passau
//
// $Author: friedric $
// $Date: 1996/05/06 14:24:44 $
// $RCSfile: FFInspector.h,v $
// $Revision: 1.2 $
// $State: Exp $
//
*/

#ifndef FF_INSPECTOR
#define FF_INSPECTOR

#include <stdlib.h>
#include <stdio.h>

class FFGraphLabel;

class FFInspector 
{
protected:
  FFGraphLabel *graphLabel;

public:
 virtual void show() {}
 virtual void hide() {}
 virtual void inspect() = 0;
 virtual void update() = 0;
 
 FFInspector(FFGraphLabel *g) {graphLabel = g;}
 virtual ~FFInspector(){}
};

#endif
