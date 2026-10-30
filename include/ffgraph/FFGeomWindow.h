/*
// Copyright 1994 Carsten Friedrich, Universitaet Passau
//
// $Author: friedric $
// $Date: 1996/05/06 14:24:40 $
// $RCSfile: FFGeomWindow.h,v $
// $Revision: 1.3 $
// $State: Exp $
//
*/

#ifndef FF_GEOM_WINDOW
#define FF_GEOM_WINDOW

#include "globdefs.h"

class FFGraph;
class FFGraphicNode;
class FFGraphicEdge;

#include <container/AVLSet.h>
#include <container/VOHSet.h>
#include <container/VHSet.h>

#include <ffgraph/FFPoint.h>
#include <ffgraph/FFGraphLabel.h>
#include <ffgraph/FFGraphWindow.h>

class FFGeomWindow  : public FFGraphWindow
{	
  FFSet<FFGraphicNode *> currentNSet;
  BOOL updateFlag;
  BOOL alreadyInitialisized;
public:
  virtual void init(const FFGraph& c);
	virtual FFGraphLabel *proto() const
		{return new FFGeomWindow;}
	virtual void scaleXYZ(FFCoord , FFCoord , FFCoord ) {}
	virtual void rotateX(FFCoord) {}
	virtual void rotateY(FFCoord) {}
	virtual void rotateZ(FFCoord) {}
	virtual void translate(FFCoord, FFCoord, FFCoord) {}

	virtual FFPoint proj(const FFPoint& A, const FFPoint&)
         {return FFPoint(A);}
	virtual FFPoint unProj(const FFPoint& A) {return FFPoint(A);}
	
  virtual FFPoint adjust(FFGraphicNode *, FFPoint B) {return B;}
	virtual void setLabelOffset(int, int) {}
    
  virtual void adjustWindowSize() {}
  
  virtual String addNode(FFGraphicNode *n) 
		{return dec((int)n);}
  virtual String addEdge(FFGraphicEdge *e)
		{return dec((int)e);}
  virtual String addLabel(FFGraphicNode *n)
		{return String("l")+dec((int)n);}
  virtual String addLabel(FFGraphicEdge *e)
		{return String("l")+dec((int)e);}

  virtual FFPoint getRange(const FFGraphicNode *) {return FFPoint();}

  virtual void changeColor(FFGraphicNode *, String& ){}
  virtual String changeToWindow(FFGraphicNode *){return String("");}
  virtual String changeToCircle(FFGraphicNode *){return String("");}
  virtual String changeToIcon(FFGraphicNode *n, const String& , const String&)
		{return dec((int)n);}
  virtual void changeMask(FFGraphicNode *, const String& , const String&){}
  virtual void changeColor(FFGraphicEdge *, String& ){}
  virtual void changeLabel(FFGraphicNode *, const String& ){}
  virtual void changeLabel(FFGraphicEdge *, const String& ){}

  virtual void setSpline(FFGraphicEdge *, BOOL ){}

  virtual void remove(FFGraphicNode *){}
  virtual void remove(FFGraphicEdge *){}

  virtual void removeLabel(FFGraphicNode *){}
  virtual void removeLabel(FFGraphicEdge *){}

  // wird u.a. bei verschieben aufgerufen
  virtual void updateGraph();
  virtual void updateGraph(FFGraphicNode *){}
  virtual void updateGraph(FFGraphicEdge *){}

  virtual void setCurrent(FFGraphicNode *){}
  virtual void setCurrent(FFGraphicEdge *){}
  virtual void addToCurrent(FFGraphicNode *){}
  virtual void selectIfNot(FFGraphicNode *){}
  virtual void unsetCurrent(){}
  virtual FFSet<FFGraphicNode *>& selectedNodes()
		{return currentNSet;}
  virtual BOOL isCurrent(FFGraphicNode *)
		{return NO;}
  
// Die zwei Methoden werden benutzt wenn man sich nicht sicher ist ob das
// Element wirklich selektiert ist.                              
  virtual void unset(FFGraphicNode *){}
  virtual void unset(FFGraphicEdge *){}

  virtual void updateInspector(){}
  virtual void sizeTo(int , int ){}
  
  virtual BOOL setUpdate(BOOL f)
		{BOOL fl=updateFlag; updateFlag = f; return fl;}
  virtual BOOL getUpdate()
		{return updateFlag;}
  virtual void update() {}
	virtual int setTitle(const String& ) {return 1;}
	virtual void setBusy() {}
	virtual void unsetBusy() {}
	
	virtual int mapWindow() {return 1;}
  virtual int unmapWindow() {return 1;}
	virtual int withdraw() {return 1;}
  virtual int iconify() {return 1;}
  virtual int deiconify() {return 1;}

  FFGeomWindow();
  virtual ~FFGeomWindow(){}
};

#endif
