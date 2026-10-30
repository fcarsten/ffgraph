/*
// Copyright 1994 Carsten Friedrich, Universitaet Passau
//
// $Author: schreibe $
// $Date: 1996/10/23 15:35:09 $
// $RCSfile: FFEdgeClass.h,v $
// $Revision: 1.4 $
// $State: Exp $
//
*/

#ifndef FF_EDGE_CLASS
#define FF_EDGE_CLASS

#include "globdefs.h"
#include <container/MPlex.h>
#include "FFEdgeLabel.h"
#include "FFGraph.h"

class FFNodeClass;
class FFEdgeClass;
class FFGraphClass;

class FFGraphicEdge;
class FFGraphWindow;

class FFEdgeClass
{
 FFNodeClass *sourceNode, *destNode;
 FFGraphClass *derGraph;
  
 MPlex<FFEdgeLabel *> *label;
 int value;
 FFCoord weight;
  
 String name;
	
 void copyDataFrom(FFEdgeClass *, BOOL);
  
 FFEdgeClass(MPlex<FFEdgeLabel *>*, FFNodeClass *, FFNodeClass *);
 ~FFEdgeClass();  

 friend FFGraphClass;
 void addLabel(const FFEdgeLabel *); 
 void delLabel(int, BOOL);
 
 void _sendEvent(labelEvent, labelEventStruct *);
 void init(FFGraphClass *);
	
public:

 FFEdgeClass *copy();
 void setValue(int );
 int getValue();
 void setWeight (FFCoord x){weight = x;}
 FFCoord getWeight(){return weight;}
 void setName(const String& s);
 const String& getName(){return name;}

 FFGraph getGraph(){return FFGraph(derGraph);}
 FFNodeClass *source();
 FFNodeClass *target();
 void revert();
 void setTarget(FFNodeClass *);
 void setSource(FFNodeClass *);
 
 void assignLabel(FFEdgeLabel *l, int i=-1);
 void setLabel(FFEdgeLabel *l, int i=-1, BOOL doFree=YES);
 void assignLabel(const MPlex<FFEdgeLabel *>&);
 void setLabel(const MPlex<FFEdgeLabel *>&, BOOL doFree=YES);
 int getLabelIndex(const String&, FFSet<int>*set=NULL) const;
 FFEdgeLabel *getLabel(int i)
 {if(label && label->valid(i)) return (*label)[i]; return NULL;}  
 
 MPlex<FFEdgeLabel *> *getLabels() {return label;}  
 
 void show();
 void hideGraphic() ;
 void showGraphic() ;
  
 inline FFGraphicEdge *getGraphicEdge()
 {if(derGraph->getGraphicEdgeIndex()>=0)
  return (FFGraphicEdge *)((*label)[derGraph->getGraphicEdgeIndex()]);
 else
  return NULL;}
 inline FFGraphicEdge *operator()() 
 {if(derGraph->getGraphicEdgeIndex()>=0)
  return (FFGraphicEdge *)((*label)[derGraph->getGraphicEdgeIndex()]);
 else
  return NULL;}
 inline FFGraphicEdge *operator->() 
 {if(derGraph->getGraphicEdgeIndex()>=0)
  return (FFGraphicEdge *)((*label)[derGraph->getGraphicEdgeIndex()]);
 else
  return NULL;}
};

typedef FFEdgeClass * FFEdge;

#endif

