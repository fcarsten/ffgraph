/*
// Copyright 1994 Carsten Friedrich, Universitaet Passau
//
// $Author: friedric $
// $Date: 1996/05/06 14:24:42 $
// $RCSfile: FFGraphWindow.h,v $
// $Revision: 1.4 $
// $State: Exp $
//
*/

#ifndef FF_GRAPH_WINDOW
#define FF_GRAPH_WINDOW

#include "globdefs.h"

class FFGraph;
// class FFGraphicNode;
class FFGraphicEdge;

#include <container/AVLSet.h>
#include <container/VOHSet.h>
#include <container/VHSet.h>
#include "FFPoint.h"
#include "FFGraphLabel.h"
#include "FFGraphicNode.h"

#define ffGraphWindowType String("FFGraphWindow")

#ifndef FF_EMPTY_STRING
#define FF_EMPTY_STRING
static String ffEmptyString("");
#endif

class FFGraphWindow  : public FFGraphLabel
{
public:
 virtual BOOL parse(const String&){return YES;}
 virtual String unparse(){return String("");}
 //	const String& type() const {return ffGraphWindowType;}
 
 virtual void scaleXYZ(FFCoord , FFCoord , FFCoord )= 0;
 virtual void rotateX(FFCoord)= 0;
 virtual void rotateY(FFCoord)= 0;
 virtual void rotateZ(FFCoord)= 0;
 virtual void translate(FFCoord, FFCoord, FFCoord)= 0;
	
 virtual FFCoord getZoom() {return 1.0;}
 virtual FFCoord getAlpha() {return 0.0;}
 virtual FFCoord getBeta() {return 0.0;}
 virtual FFCoord getGamma() {return 0.0;}

 virtual FFPoint proj(const FFPoint&, const FFPoint& = FFPoint(0,0,0))= 0;
 virtual FFPoint unProj(const FFPoint& A)= 0;

 virtual void setBackground(const String&) { }
 virtual const String& getBackground() {return ffEmptyString;}
 
 virtual FFPoint adjust(FFGraphicNode *, FFPoint ) = 0;
 virtual void setLabelOffset(int, int)= 0;
 
 virtual void adjustWindowSize()= 0;
 
 virtual String addNode(FFGraphicNode *n)= 0;
 virtual String addEdge(FFGraphicEdge *e)= 0;
 virtual String addLabel(FFGraphicNode *n)= 0;
 virtual String addLabel(FFGraphicEdge *e)= 0;
 
 virtual void changeColor(FFGraphicNode *n, String& )= 0;
 virtual String changeToIcon(FFGraphicNode *n, const String&, 
                              const String& )= 0;
 virtual String changeToFigure(FFGraphicNode *, ffShape)= 0;
 virtual String changeToWindow(FFGraphicNode *n)= 0;
 virtual void changeMask(FFGraphicNode *, const String& , const String&)= 0;
 virtual void changeColor(FFGraphicEdge *, String& )= 0;
 virtual void changeLabel(FFGraphicNode *, const String& )= 0;
 virtual void changeLabel(FFGraphicEdge *, const String& )= 0;

 virtual FFPoint getRange(const FFGraphicNode *) = 0;

 virtual void setSpline(FFGraphicEdge *e, BOOL spline)= 0;
 
 virtual void remove(FFGraphicNode *n)= 0;
 virtual void remove(FFGraphicEdge *e)= 0;
 
 virtual void removeLabel(FFGraphicNode *n)= 0;
 virtual void removeLabel(FFGraphicEdge *e)= 0;
 
 // wird u.a. bei verschieben aufgerufen
 virtual void updateGraph()= 0;
 virtual void updateGraph(FFGraphicNode *n)= 0;
 virtual void updateGraph(FFGraphicEdge *e)= 0;
 
 virtual void setCurrent(FFGraphicNode *n)= 0;
 virtual void setCurrent(FFGraphicEdge *e)= 0;
 virtual void addToCurrent(FFGraphicNode *n)= 0;
 virtual void selectIfNot(FFGraphicNode *n)= 0;
 virtual void unsetCurrent()= 0;
 virtual const FFSet<FFGraphicNode *>& selectedNodes()= 0;
 virtual BOOL isCurrent(FFGraphicNode *n)= 0;
  
// Die zwei Methoden werden benutzt wenn man sich nicht sicher ist ob das
 // Element wirklich selektiert ist.                              
 virtual void unset(FFGraphicNode *n)= 0;
 virtual void unset(FFGraphicEdge *e)= 0;

//  virtual void updateInspector()= 0;
 virtual void sizeTo(int x, int y)= 0;
  
 virtual BOOL setUpdate(BOOL f)= 0;
 virtual BOOL getUpdate()= 0;
 virtual void update()= 0;
 virtual int setTitle(const String& s)= 0;
 virtual void setBusy()= 0;
 virtual void unsetBusy()= 0;
 
 virtual int mapWindow()= 0;
 virtual int unmapWindow()= 0;
 virtual int withdraw()= 0;
 virtual int iconify()= 0;
 virtual int deiconify()= 0;
 
 FFGraphWindow(){_setType(ffGraphWindowType);}
 virtual ~FFGraphWindow(){}
};

#endif
