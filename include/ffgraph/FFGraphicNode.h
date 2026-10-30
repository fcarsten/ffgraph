/*
// Copyright 1994 Carsten Friedrich, Universitaet Passau
//
// $Author: friedric $
// $Date: 1996/05/09 09:43:33 $
// $RCSfile: FFGraphicNode.h,v $
// $Revision: 1.4 $
// $State: Exp $
//
*/

#ifndef FF_GRAPHIK_NODE
#define FF_GRAPHIK_NODE

#include "globdefs.h"

#include <String.h>
#include "FFPoint.h"

#include "FFNodeClass.h"
class FFGraphWindow;
class FFGraphicEdge;
class FFTclGraphInsp;

#include "FFNodeLabel.h"

#define RADIUS 10
#define ffNodeWindowType String("FFGraphicNode")

typedef enum {
 FF_CUSTOM_SHAPE,
 FF_IMAGE,
FF_SCALABLE,
 FF_WINDOW,
FF_FIGURE,
 FF_OVAL,
 FF_CIRCLE,
 FF_RECTANGLE,
 FF_SQUARE,
 FF_RAUTE,
} ffShape;

String shape2Str(ffShape);
ffShape str2Shape(const String&);

class FFGraphicNode : public FFNodeLabel
{
protected:	  
 String tkNode;
 String tkLabel;
 FFGraphWindow *fenster;

 String farbe;
 
 String icon;
 String selIcon;
 
 String mask;
 String selMask;
 
 String window;
 ffShape shape;
 ffShape figure;
 
 FFPoint coords;
 FFPoint range;  
	
  void _wallNodeChanged();
public:
//  virtual BOOL parse(const String&){return YES;}
//  virtual String unparse(){return String("");}
//  const String& type() const {return ffNodeWindowType;}

  FFNodeLabel *proto() const {return new FFGraphicNode;}
  virtual void event(labelEvent, labelEventStruct *);
  virtual void assign(const FFLabelClass*);
  
  inline FFCoord getx() const {return coords.v[0];}
  inline FFCoord gety() const {return coords.v[1];}
  inline FFCoord getz() const {return coords.v[2];}
  inline const FFPoint& getCoords() const {return coords;}
	
  inline FFCoord getWidth() {return range.v[0];};
  inline FFCoord getHeight() {return range.v[1];};
  inline FFCoord getDepth() {return range.v[2];};
  inline FFPoint& getRange() {return range;};
  
  void setWidth(FFCoord);
  void setHeight(FFCoord);
  void setDepth(FFCoord);
  inline void setRange(const FFPoint&p) {range=p; _wallNodeChanged();}
  
 void changeToFigure(ffShape);	
 void changeToWindow();	
 const String& getNodeWindow() const {return window;}

 void changeToIcon(const String&, const String& =String("__NoIcon__"));
 void setMask(const String&, const String& =String("__NoMask__"));

 inline String& getIcon() {return icon;}
 inline String& getSelIcon() {return selIcon;}
 inline String& getMask() {return mask;}
 inline String& getSelMask() {return selMask;}
 
//   BOOL isScalable();
//   BOOL isCircle();

 virtual void scale(FFPoint&){}
 virtual void rotateX(FFCoord){}
 virtual void rotateY(FFCoord){}
 virtual void rotateZ(FFCoord){}
 
 virtual ffShape getShape() {return shape;}
 virtual ffShape getFigure() {return figure;}
 virtual BOOL isOval();
 virtual BOOL isScalable() 
 {if (shape > FF_SCALABLE) return(YES); else return(NO);}

  FFGraphicNode();
  virtual void init( FFNodeClass *n);
	
  ~FFGraphicNode();
  
  const String& getId() const;
  const String& getLabelId() const;
  
  void setName(const String&);

  void setColor(const String&);         
  String getColor();

  void setWindow(FFGraphWindow *); 

  void moveTo(char *, char *, char *z="0");
  void moveTo(const FFPoint&);
  void rmoveTo(char *, char *, char *z="0");
  void rmoveTo(const FFPoint&);

  void updateGraphic();
  inline FFNodeClass *getNode() {return node;}
  inline FFNodeClass *operator()() {return node;}
  inline FFNodeClass *operator->() {return node;}
  friend class FFTclWindow;
  friend class FFGraphicEdge;
  friend class FFTclGraphInsp;  
};

typedef FFGraphicNode * FFGraphicNodeP;

#endif
