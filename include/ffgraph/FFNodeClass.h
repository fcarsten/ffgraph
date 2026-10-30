/*
// Copyright 1994 Carsten Friedrich, Universitaet Passau
//
// $Author: deepthou $
// $Date: 1996/07/04 07:45:09 $
// $RCSfile: FFNodeClass.h,v $
// $Revision: 1.4 $
// $State: Exp $
//
*/

#ifndef FF_NODE_CLASS
#define FF_NODE_CLASS

#include "globdefs.h"

#include <container/AVLSet.h>
#include <container/VOHSet.h>
#include <container/VHSet.h>
#include <container/MPlex.h>
#include "String.h"
#include "FFGraphClass.h"
#include "FFPoint.h"

class FFGraph;
class FFGrahClass;
class FFEdgeClass;

class FFGraphicNode;
class FFGraphWindow;
class FFNodeLabel;

class FFNodeClass
{
protected:
  FFSet<FFEdgeClass *> transFrom;      // Gehen Rein
  FFSet<FFEdgeClass *> transTo;        // Gehen Raus
  
  int value;
  FFCoord coordValue;
  MPlex<FFNodeLabel *> *label;
  int attribut;
 BOOL hiddenFlag;
 
  class FFGraphClass *derGraph;
  String name;

  void copyDataFrom(FFNodeClass *, BOOL);  
  
  friend FFGraphClass;
  void addLabel(const FFNodeLabel *);
  void delLabel(int, BOOL);

  void _sendEvent(labelEvent, labelEventStruct *);

public:
  
  inline int getValue() const {return value;}
  inline void setValue(int i) {value=i;}
  
  inline FFCoord getCoord() const {return coordValue;}
  inline void setCoord(FFCoord i) {coordValue=i;}
   
  inline int getAttribut() const {return attribut;} 
  inline void setAttribut(int i) {attribut = i;}

  FFNodeClass *copy();
  
  FFNodeClass(MPlex<FFNodeLabel *> *);
  ~FFNodeClass();
  void init(FFGraphClass *);
  
 inline int indeg() const {return transFrom.length();}
 inline int outdeg() const {return transTo.length();}
 inline int deg() const {return indeg() + outdeg(); }
 
  void eliminateParallelEdges();

  void assignLabel(FFNodeLabel *l, int i=-1);
  void assignLabel(const MPlex<FFNodeLabel *>&);

  void setLabel(FFNodeLabel *l, int i=-1, BOOL doFree=YES);
  void setLabel(const MPlex<FFNodeLabel *>&, BOOL doFree=YES);

  int getLabelIndex(const String&, FFSet<int>*set=NULL) const;
  FFNodeLabel *getLabel(int i)
  {if(label && label->valid(i)) return (*label)[i]; return NULL;}  
  inline const MPlex<FFNodeLabel *> *getLabels() const {return label;}  

  inline FFGraphClass *getGraph() {return derGraph;}

  void show();
  
  FFSet<FFNodeClass *> succ();
  FFSet<FFNodeClass *> pred();
  inline const FFSet<FFEdgeClass *>& transToSucc() const 
    {return transTo;}
  inline const FFSet<FFEdgeClass *>& transFromPred() const 
    {return transFrom;}

  void succ(FFSet<FFNodeClass *> &tmpS);
  void pred(FFSet<FFNodeClass *> &tmpS);
  
  void setName(const String& s);
  inline const String& getName() const {return name;}
  
  void hideGraphic();
  void showGraphic();
 BOOL isHidden() {return hiddenFlag;}
 
  inline FFGraphicNode *getGraphicNode()
  {if(derGraph->getGraphicNodeIndex()>=0)
    return (FFGraphicNode *)((*label)[derGraph->getGraphicNodeIndex()]);
  else
    return NULL;}
  inline FFGraphicNode *operator()()
  {if(derGraph->getGraphicNodeIndex()>=0)
    return (FFGraphicNode *)((*label)[derGraph->getGraphicNodeIndex()]);
  else
    return NULL;}
  inline FFGraphicNode *operator->()
  {if(derGraph->getGraphicNodeIndex()>=0)
    return (FFGraphicNode *)((*label)[derGraph->getGraphicNodeIndex()]);
  else
    return NULL;}
};

typedef FFNodeClass * FFNode;

#endif
