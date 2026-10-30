/*
// Copyright 1994 Carsten Friedrich, Universitaet Passau
//
// $Author: holzmann $
// $Date: 1996/01/09 12:31:48 $
// $RCSfile: FFGraphicEdge.h,v $
// $Revision: 1.2 $
// $State: Exp $
//
*/

#ifndef FF_GRAPHIK_EDGE
#define FF_GRAPHIK_EDGE

#include "globdefs.h"
#include "FFPoint.h"

#include <String.h>
#include <container/MPlex.h>

class FFEdgeClass;

class FFGraphWindow;
class FFGraphicNode;
class FFTclGraphInsp;
#include "FFEdgeLabel.h"

#define ffEdgeWindowType String("FFGraphicEdge")

class FFGraphicEdge : public FFEdgeLabel
{
protected:  
  FFGraphicNode *source;
  FFGraphicNode *target;  

  String tkEdge;
  String tkLabel;

  FFGraphWindow *fenster;

  MPlex<FFPoint> edgePoints;
	
  BOOL spline;
  String farbe;
  friend class FFGraphWindow;
  friend class FFGraphicNode;

  FFCoord dist(const FFPoint&, const FFPoint& );
  FFCoord dist(const FFPoint&, const FFPoint&, const FFPoint&);
  BOOL modifyPointAt(int, FFPoint&);
  void targetChanged();        // intern
  void sourceChanged();        // intern
  void adjustEdge();           // intern

public:

//  virtual BOOL parse(const String&){return YES;}
//  virtual String unparse(){return String("");}

  FFEdgeLabel *proto() const {return new FFGraphicEdge;}
  virtual void event(labelEvent, labelEventStruct *);
  virtual void assign(const FFLabelClass*);
    
  inline FFCoord getxAt(int p) {return edgePoints[p].v[0];};
  inline FFCoord getyAt(int p) {return edgePoints[p].v[1];};
  inline FFCoord getzAt(int p) {return edgePoints[p].v[2];};

  int getPointNear(const FFPoint&);

  BOOL addPointAt(int, const FFPoint&);
  BOOL deletePointAt(int);
  BOOL deletePointAt(const FFPoint&);
  int anzPoints(){return edgePoints.length();}
  FFPoint& getPointAt(int k) {return edgePoints[k];}
  const MPlex<FFPoint>& getEdgePoints() {return edgePoints;}
		
  String getId();
  String getLabelId();

  void revert();

  void setName(const String& );

  FFGraphicEdge();
  virtual ~FFGraphicEdge();
  virtual void init( FFEdgeClass *);
	
  void setWindow(FFGraphWindow *);

  void updateGraphic();
  
  void toggleSpline();

  void setColor(const String& c);
  String getColor();

  void rmoveTo(const FFPoint&);
  void rmovePoint(int p, const FFPoint&);
  void movePoint(int p, const FFPoint&);

  inline FFEdgeClass *getEdge() {return edge;}
  inline FFEdgeClass *operator()() {return edge;}
  inline FFEdgeClass *operator->() {return edge;}
  friend class FFTclGraphInsp;
  
};

typedef FFGraphicEdge * FFGraphicEdgeP;

#endif
