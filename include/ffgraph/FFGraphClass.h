/*
// Copyright 1994 Carsten Friedrich, Universitaet Passau
//
// $Author: friedric $
// $Date: 1996/05/09 09:43:32 $
// $RCSfile: FFGraphClass.h,v $
// $Revision: 1.5 $
// $State: Exp $
//
*/

#ifndef FF_GRAPH_CLASS
#define FF_GRAPH_CLASS

#include <globdefs.h>

class FFEdgeLabel;
class FFNodeLabel;
class FFGraphLabel;
class FFGraphWindow;
class FFNodeClass;
class FFEdgeClass;
class FFGraph;
class FFGraphicNode;
class FFGraphicEdge;

#include <container/AVLSet.h>
#include <container/VOHSet.h>
#include <container/VHSet.h>
#include <container/MPlex.h>
#include <SLList.h>
#include <String.h>
#include <ffgraph/ffError.h>
#include <ffgraph/FFPoint.h>
#include <ffgraph/FFLabelClass.h>

typedef enum 
{
 EVENT_NORMAL,
 EVENT_DISABLED
} eventStatus;

String expand_path(const String&);

class FFGraphClass
{
  //
  // Aenderungen hier muessen auch in der copy methode beruecksichtigt werden !!!
  //
protected:  
  FFSet<FFNodeClass *> knotenSet;
  FFSet<FFEdgeClass *> kantenSet;  
  
  MPlex<FFNodeLabel *> nodeL;
  MPlex<FFEdgeLabel *> edgeL;
  MPlex<FFGraphLabel *> graphLabel;
    
  FFNodeClass * startNode;
  FFSet<FFNodeClass *> endKnotenSet;
  
  mutable int reps;
  int status;
  String name;
  int graphIndex;
  int nodeIndex;
  int edgeIndex;

 eventStatus evStat;

  BOOL directed;

  void _save(ostream& , const FFSet<FFNodeClass *>&);
  void _saveGML(ostream& , const FFSet<FFNodeClass *>&);
  void _sendGraphEvent(labelEvent, labelEventStruct *);
  
  FFNodeLabel *getNodeLabel(int i)
    {if(nodeL.valid(i)) return nodeL[i]; return NULL;}  
  FFEdgeLabel *getEdgeLabel(int i)
    {if(edgeL.valid(i)) return edgeL[i]; return NULL;}  

  
 public:

 void setEventStatus(eventStatus);
 eventStatus getEventStatus() { return evStat;}
 
 
  

  inline BOOL getDirected() {return directed;} // NYFI 
  void _setDirected(BOOL); // NYFI 
  inline int getGraphicNodeIndex() {return nodeIndex;}
  inline int getGraphicEdgeIndex() {return edgeIndex;}
  inline int getGraphWindowIndex() {return graphIndex;}
  
  inline void setNodeIndex(int i) {nodeIndex=i;}
  inline void setEdgeIndex(int i) {edgeIndex=i;}
  inline void setGraphIndex(int i) {graphIndex=i;}
  
  FFGraphClass(BOOL);
  virtual ~FFGraphClass();

  void load(const String& buffer, BOOL datei);
  void saveToFile(const String&);
  void saveToFile(const String& , const FFSet<FFNodeClass *>&);
  void saveToString( String&);
  void saveToString( String& , const FFSet<FFNodeClass *>&);
	
  void die();
  FFNodeClass * addNode(const FFPoint& p=FFPoint(100,100,0));
  FFEdgeClass * addEdge(FFNodeClass * s, FFNodeClass * t);
  void removeNode(FFNodeClass * n);
  void removeEdge(FFEdgeClass * e);

  void clear();
  void allNodes(FFSet<FFNodeClass *>&);
  void allEdges(FFSet<FFEdgeClass *>&);

  const FFSet<FFNodeClass *>& allNodes() const;
  const FFSet<FFEdgeClass *>& allEdges() const;

  int getNodeLabelIndex(const String&, FFSet<int>*set=NULL) const;
  int getEdgeLabelIndex(const String&, FFSet<int>*set=NULL) const;
  int getGraphLabelIndex(const String&, FFSet<int>*set=NULL) const;
  FFGraphLabel *getGraphLabel(const String& s, 
                              FFSet<FFGraphLabel *> *set=NULL); 

  FFGraphLabel *getGraphLabel(int i) 
    {if(graphLabel.valid(i)) return graphLabel[i]; return NULL;}  

  const MPlex<FFGraphLabel *>& getGraphLabels() const {return graphLabel;}  
  const MPlex<FFNodeLabel *>& getNodeLabels() const {return nodeL;}  
  const MPlex<FFEdgeLabel *>& getEdgeLabels() const {return edgeL;}  

  int addNodeLabel(FFNodeLabel *);
  int addEdgeLabel(FFEdgeLabel *);
  int addGraphLabel(FFGraphLabel *);

  void delNodeLabel(int, BOOL doFree=YES);
  void delEdgeLabel(int, BOOL doFree=YES);
  void delGraphLabel(int, BOOL doFree=YES);
  
  void delNodeLabel(const String& s, BOOL doFree=YES)
    {delNodeLabel(getNodeLabelIndex(s), doFree);}
  void delEdgeLabel(const String& s, BOOL doFree=YES) 
    {delEdgeLabel(getEdgeLabelIndex(s), doFree);}
  void delGraphLabel(const String& s, BOOL doFree=YES) 
    {delGraphLabel(getGraphLabelIndex(s), doFree);}
  
  void assignGraphLabel(const FFGraphLabel *, int i=-1);
  void assignGraphLabel(const MPlex<FFGraphLabel *>&);
  
  void setGraphLabel(FFGraphLabel *, int i=-1, BOOL doFree=YES);
  void setGraphLabel(const MPlex<FFGraphLabel *>&, BOOL doFree=YES);
  
  void setName(const String& s);
  const String& getName(){return name;}
  
  FFNodeClass *nodeWithValue(int k);
      
  void rev();
  void eliminateParallelEdges();

  BOOL contains(FFNodeClass * n);
  BOOL contains(FFEdgeClass * e);

  BOOL setStartNode(FFNodeClass * n);
  FFNodeClass * getStartNode();

  BOOL setEndNodes(const FFSet<FFNodeClass *>& s);
  BOOL addEndNode(FFNodeClass * n);
  BOOL delEndNode(FFNodeClass * n);
  void clearEndNodes();
  const FFSet<FFNodeClass *>& getEndNodes();
  
  int getNumNodes() const;      // liefert Anzahl der Knoten des Graphen
  int getNumEdges() const;      //   "                Kanten   "

 FFGraphClass *copy(FFGraphClass * g= NULL, 
                    FFSet<FFNodeClass*> *s= NULL, BOOL f=NO);
 FFGraphClass *copy(FFGraph&, BOOL f=NO);
 void incNumOfReps(int i=1) const;
 BOOL decNumOfReps(int i=1);  /* Falls YES, wurde letzte Huelle entfernt */

 FFGraphClass *getSubGraph(FFNodeClass *, BOOL firstTime=YES);
 
 
  BOOL containsCycle();
  BOOL weakConnected();
  BOOL strongConnected();
  BOOL isTree();
  BOOL startNodeChoice (int choice); 
  BOOL reachAllNodes ();
  SLList <FFNodeClass *> dfs ();  
  SLList <FFNodeClass *> bfs ();  
  int getNumCrossings (); 
  void removeRefEdges (); 
  void removeParEdges (); 
  void removeCycles (int choice); 
  void straightenEdges (); 
  void levelDAG (int choice, FFCoord distance); 
  FFNodeClass * setNodeInEdge (FFEdgeClass* e); 
  void removeNodeFromEdge (FFNodeClass* n);   
  void straightenEdgesLocal (FFSet <FFNodeClass *> locNodes, int choice);
  void setInLevel (SLList <FFNodeClass *> &levelNodes, int choice, 
                   FFCoord distance);
  void setDAGLevels (int choice, FFCoord distance);
  void copyCoord (FFGraph &graph, int choice);
  
  void show();
  void createGraphic(FFGraphWindow *w = NULL, FFGraphicNode *gn=NULL,
		     FFGraphicEdge *ge=NULL);
  FFGraphWindow *getWindow() const 
    {
      if(graphIndex >= 0)
	return (FFGraphWindow *)graphLabel[graphIndex];
      else
	return NULL;
    }
  FFGraphWindow *leaveWindow();
  FFGraphWindow *hideGraphic();
  void removeGraphic();
};

void addDefaultGraphLabel(class FFGraphLabel *);
void addDefaultNodeLabel(class FFNodeLabel *);
void addDefaultEdgeLabel(class FFEdgeLabel *);

#endif
