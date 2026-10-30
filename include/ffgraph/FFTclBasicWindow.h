/*
// Copyright 1994 Carsten Friedrich, Universitaet Passau
//
// $Author: friedric $
// $Date: 1996/05/09 09:43:33 $
// $RCSfile: FFTclBasicWindow.h,v $
// $Revision: 1.5 $
// $State: Exp $
//
*/

#ifndef FF_TCL_BASIC_WINDOW
#define FF_TCL_BASIC_WINDOW

#include <tk.h>
#include <globdefs.h>

#include <ffgraph/FFPoint.h>
#include <ffgraph/FFMatrix.h>
#include <ffgraph/FFWindow.h>
#include <ffgraph/FFGraphWindow.h>
#include <ffgraph/FFGraph.h>
#include <ffgraph/FFNodeClass.h>
#include <ffgraph/FFEdgeClass.h>
#include <container/AVLMap.h>
// #include "FFSugiInsp.h"
// #include "FFSpringInsp.h"
// #include "FFSugi.h"
// #include "FFSpring.h"
class FFLayoutLabel;

#define initMatrix FFMatrix(FFPoint(1,0,0,0), FFPoint(0,1,0,0), FFPoint(0,0,1,0), FFPoint(300,200,0,1))
#define invInitMatrix FFMatrix(FFPoint(1,0,0,0), FFPoint(0,1,0,0), FFPoint(0,0,1,0), FFPoint(-300,-200,0,1))

#define einheitsMatrix FFMatrix(FFPoint(1,0,0,0), FFPoint(0,1,0,0), FFPoint(0,0,1,0), FFPoint(0,0,0,1))
								
// inline FFCoord rad(FFCoord x)
// {
//  return(x*M_PI/180);
// }

typedef enum {
  CREATING_EDGE =1,
  ADD_POINT,
  RM_POINT
} modalModes;

// #define RADIUS 10

class FFTclGraphInsp;

class FFTclBasicWindow: public FFGraphWindow, public FFWindow
{
protected:

 // Wird auf NO gesetzt wenn die Skaliergung mittels Schieber geaendert
 // wurde. Ansonsten muss der Schieber angepasst werden.
 
 BOOL updateScale;

 String exportString;
 String importString;
 int tmpPoint;              // der gewaehlte Punkt in einer Kante
 int width, height;
 FFCoord labelXoff, labelYoff;
 
 BOOL showNodesFlag;
 BOOL showEdgesFlag;
 
 FFCoord alpha, beta, gamma;
 FFCoord scale;
 
 BOOL alreadyInitialisized;
// Gegeben
 FFMatrix ctm;
 FFMatrix invCtm;
 FFMatrix projMatrix;
 FFMatrix invProjMatrix;

 FFPoint viewPoint;	  
 FFCoord xWinkel, yWinkel, zWinkel;
 FFCoord viewDist;

 ffShape shape;
 
 String canvas;
 String fileName;
 String tclFileName;

 String canvColor;
 
 AVLMap<String, FFGraphicNode *> nodeMap;
 AVLMap<String, FFGraphicEdge *> edgeMap;
 FFSet<FFGraphicNode *> currentNSet;
 
 FFSet<FFLayoutLabel *> layouter;
    
  BOOL modal;
  BOOL updateFlag;
    
  FFGraphicEdge *currentE;
  
  virtual String modifyEdge(FFGraphicEdge *e, String command);
	virtual void _buildProjMatrix();
	virtual void _hideLines();
	virtual void _updateLayouter();
	
 virtual FFPoint _adjust(FFGraphicNode *,  FFPoint,
                         BOOL = NO);

 virtual void _buildExportString();
 // virtual void _buildLayoutMenu();
 BOOL _flattenEdge();
 void _sixColor();
 
public:
 
 // virtual FFPoint proj(const FFPoint&, const FFPoint&);
 virtual FFPoint proj(const FFPoint&, const FFPoint&  = FFPoint(0,0,0));
 virtual FFPoint unProj(const FFPoint&);

 virtual const FFPoint& getViewPoint() const { return viewPoint;}
 virtual FFCoord getViewDist() const { return viewDist;}
 
 virtual FFGraphLabel *proto() const {return new FFTclBasicWindow;}
 virtual void event(labelEvent, labelEventStruct *);
 
 virtual int exportSelection(int , char*, int);
 virtual int getSelection(const char*);
 
 virtual const String& getFileName() const {return fileName;}
 virtual void setFileName(const String& s)  { fileName=s;}
 
 virtual void scaleXYZ(FFCoord , FFCoord , FFCoord );
 virtual void rotateX(FFCoord);
 virtual void rotateY(FFCoord);
 virtual void rotateZ(FFCoord);
 virtual void translate(FFCoord, FFCoord, FFCoord);

 virtual FFCoord getZoom() {return scale;}
 virtual FFCoord getAlpha() {return alpha;}
 virtual FFCoord getBeta() {return beta;}
 virtual FFCoord getGamma() {return gamma;}
	
 virtual FFPoint adjust(FFGraphicNode *A, FFPoint B)
 {return _adjust(A,B,YES);}


  virtual void setBackground(const String&);
  virtual const String& getBackground() {return canvColor;}
  
  virtual BOOL hasOwnInspector() const {return NO;}
  virtual void init(const FFGraph& c);
  virtual void adjustWindowSize();
  virtual void setLabelOffset(int x, int y) 
   {labelXoff=x/10.0; labelYoff=y/10.0; updateGraph();}
   
   virtual String addNode(FFGraphicNode *n);
   virtual String addEdge(FFGraphicEdge *e);
   virtual String addLabel(FFGraphicNode *n);
   virtual String addLabel(FFGraphicEdge *e);
   
  virtual void changeColor(FFGraphicNode *n, String& );
  virtual String changeToIcon(FFGraphicNode *n, const String& , const 
                              String& =String("__NoIcon__"));
  virtual String changeToFigure(FFGraphicNode *, ffShape);
  virtual String changeToWindow(FFGraphicNode *n);
  virtual void changeMask(FFGraphicNode *, const String& , 
                          const String& =String("__NoMask__"));
  virtual void changeColor(FFGraphicEdge *e, String& );
  virtual void changeLabel(FFGraphicNode *n, const String& );
  virtual void changeLabel(FFGraphicEdge *n, const String& );

  virtual void remove(FFGraphicNode *n);
  virtual void remove(FFGraphicEdge *e);

  virtual void removeLabel(FFGraphicNode *n);
  virtual void removeLabel(FFGraphicEdge *e);

  // wird u.a. bei verschieben aufgerufen
  virtual void updateGraph(); 
  virtual void updateGraph(FFGraphicNode *); 
  virtual void updateGraph(FFGraphicEdge *);  
  virtual void setSpline(FFGraphicEdge *, BOOL);
  
  virtual void setCurrent(FFGraphicNode *);
  virtual void setCurrent(FFGraphicEdge *);
  virtual void addToCurrent(FFGraphicNode *);
  virtual void selectIfNot(FFGraphicNode *);
  virtual void unsetCurrent();
  virtual BOOL isCurrent(FFGraphicNode *n)
		{return currentNSet.contains(n);}
  virtual const FFSet<FFGraphicNode *>& selectedNodes()
   {return currentNSet;}

// Die zwei Methoden werden benutzt wenn man sich nicht sicher ist ob das
// Element wirklich selektiert ist.                              
  virtual void unset(FFGraphicNode *n); 
  virtual void unset(FFGraphicEdge *e); 

  virtual int recall(Tcl_Interp *i, int k, const char **c);  
  const Tcl_Interp *getInterp() {return interp;}
  virtual void sizeTo(int x, int y);
  FFPoint getRange(const FFGraphicNode *);
  
  virtual BOOL setUpdate(BOOL f) 
            {BOOL fl=updateFlag; updateFlag = f; return fl;}
            
  virtual BOOL getUpdate() {return updateFlag;};
  virtual void update() { FFWindow::update();}
  
  virtual void showNodes(BOOL flag = YES);
  virtual void hideNodes() {showNodes(NO);}

  virtual void showEdges(BOOL flag = YES);
  virtual void hideEdges() {showEdges(NO);}
  
  virtual int setTitle(const String &s) {return FFWindow::setTitle(s);}
  virtual void setBusy() {
   Tcl_VarEval(interp, "setBusy ", NULL);
   update();}
  virtual void unsetBusy() {
   Tcl_VarEval(interp, "unsetBusy ", NULL);
   update();}

  virtual int mapWindow(){return FFWindow::mapWindow();}
  virtual int unmapWindow(){return FFWindow::unmapWindow();}
  virtual int withdraw(){return FFWindow::withdraw();}
  virtual int iconify(){return FFWindow::iconify();}
  virtual int deiconify(){return FFWindow::deiconify();}

  String fsBox(String message= String("Select File:"), 
               String type= String("*"), String name=String(""));
  FFTclBasicWindow();
  virtual ~FFTclBasicWindow();
};

#endif


