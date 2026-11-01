/*
  // Copyright 1994 Carsten Friedrich, Universitaet Passau
  //
  // $Author: deepthou $
  // $Date: 1996/10/26 12:43:52 $
  // $RCSfile: FFGraphClass.cc,v $
  // $Revision: 1.12 $
  // $State: Exp $
  //
*/

#ifdef RCSID
#undef RCSID
#endif
#define RCSID "$Author: deepthou $ $RCSfile: FFGraphClass.cc,v $ $Date: 1996/10/26 12:43:52 $ $Revision: 1.12 $ $State: Exp $"

// #define FF_NO_TESTS 1

extern "C" {
#include <pwd.h>
#include <sys/types.h>
}

#include <globdefs.h>
#include <stream.h>
#include <sstream>
#include <unordered_map>
// #include <strstream.h>

#include <ffgraph/FFGraphClass.h>
#include <ffgraph/FFGraph.h>
#include <ffgraph/FFNodeClass.h>
#include <ffgraph/FFEdgeClass.h>
#include <ffgraph/FFEdgeLabel.h>
#include <ffgraph/FFNodeLabel.h>
#include <ffgraph/FFGraphLabel.h>
#include <ffgraph/FFGraphWindow.h>
#include <ffgraph/FFGraphicNode.h>
#include <ffgraph/FFGraphicEdge.h>
#include <ffgraph/FFSubGraphLabel.h>
#include <ffgraph/ffError.h>
#include <container/AVLSet.h>
#include <container/VOHSet.h>
#include <container/AVLMap.h>
#include "FFParser.h"
#include "FFScanner.h"
#include "GTParser.h"
#include "GTScanner.h"
#include "RomeParser.h"
#include "RomeScanner.h"

#ifdef FENSTER
#undef FENSTER
#endif

#define FENSTER ((FFGraphWindow *)graphLabel[graphIndex])

FFSet<FFGraphLabel *> defGraphLabels;
FFSet<FFNodeLabel *> defNodeLabels;
FFSet<FFEdgeLabel *> defEdgeLabels;

// expand initial ~/ or ~name constructs in a pathname; returns the
// expanded path (or the original path unchanged). An initial ~name
// is returned unchanged if it doesn't match a valid user; an initial
// ~/ is returned unchanged if HOME is unset and the real user can't
// be found in the password file.
//
// This function is taken from and probably  copyrighted by the 
// comp.unix.programmer FAQ.

String expand_path(const String& path)
{
 String ffPath(path);
 if (ffPath.length() == 0 || ffPath[0] != '~')
  return ffPath;

 const char *pfx = 0;
 int pos = ffPath.index('/');
 
 if (ffPath.length() == 1 || pos == 1)
  {
   pfx = getenv("HOME");
   if (!pfx)
    {
     // time to punt. We're trying to expand ~/, but HOME isn't set
     // we could use getlogin() and expand by name, but that doesn't
     // work well with su; using getpwuid(getuid()) might be better.

     struct passwd *pw = getpwuid(getuid());
     if (pw)
      pfx = pw->pw_dir;
    }
  }
 else
  {
   String user = ffPath.before('/');
   struct passwd *pw = getpwnam((const char *)user);
   if (pw)
    pfx = pw->pw_dir;
  }
 
 // if we've failed to find an expansion, then return the path unchanged.
 
 if (!pfx)
  return ffPath;
 
 String result(pfx);
 
 if (result.length() == 0 || result[result.length()-1] != '/')
  result += '/';
 
 result += ffPath.after('/');
 
 return result;
}




void addDefaultGraphLabel(FFGraphLabel *gl)
{
 defGraphLabels.add(gl);
}

void addDefaultNodeLabel(FFNodeLabel *gl)
{
 defNodeLabels.add(gl);
}

void addDefaultEdgeLabel(FFEdgeLabel *gl)
{
 defEdgeLabels.add(gl);
}

void FFGraphClass::die()
{
 status = ZOMBIE;
}  

int FFGraphClass::getNumNodes() const
{
 return knotenSet.length();
}

int FFGraphClass::getNumEdges() const
{
 return kantenSet.length();
}

FFGraphClass *FFGraphClass::copy(FFGraph& g, BOOL bare)
{
 return copy(g.graph, NULL, bare);
}

FFGraphClass *FFGraphClass::copy(FFGraphClass *graph,
                                 FFSet<FFNodeClass*> *teilMenge, BOOL bare)
{
 Pix i = 0;

 if(graph == this)
  return this;
 
 if(teilMenge == NULL)
  teilMenge = &knotenSet;
 
 FFGraphClass *tmpGraph=graph;
 // A newly allocated graph is not wrapped in an FFGraph handle until this
 // function returns. During copyDataFrom()/label initialization, however,
 // code may create temporary FFGraph handles via getGraph(). Keep one
 // temporary reference while constructing the graph so those handles cannot
 // decrement the reference count to zero and delete tmpGraph mid-copy.
 const BOOL temporaryGraphReference = (graph == NULL);
 if(temporaryGraphReference)
  {
   tmpGraph = new FFGraphClass(bare);
   tmpGraph->incNumOfReps();
  }
 
 if(!bare)
  {
   for(i= graphLabel.first(); i!=0; graphLabel.next(i))
    {
     if(graphLabel(i)->shouldCopy())
      tmpGraph->assignGraphLabel(graphLabel(i));
    }	  
   for( i= nodeL.first(); i!=0; nodeL.next(i))
    {
     if(nodeL(i)->shouldCopy())
      tmpGraph->addNodeLabel(nodeL(i)->proto());
    }
   for( i= edgeL.first(); i!=0; edgeL.next(i))
    {
     if(edgeL(i)->shouldCopy())
      tmpGraph->addEdgeLabel(edgeL(i)->proto());
    }
   tmpGraph->setName(name);
  }
 
 tmpGraph->_setDirected(directed);
 
 // Map node identity rather than temporarily overwriting the source node's
 // user-visible value. Values are graph data, not scratch indices; the old
 // implementation also renumbered every source edge during copying.
 std::unordered_map<FFNode, FFNode> copiedNodes;
 copiedNodes.reserve(teilMenge->length());

 for(i= teilMenge->first(); i != 0; teilMenge->next(i))
  {
   FFNode oldN = (*teilMenge)(i);
   FFNode neuN = tmpGraph->addNode();
   copiedNodes.emplace(oldN, neuN);
   neuN->copyDataFrom(oldN, bare);

   if(oldN == startNode)
    tmpGraph->setStartNode(neuN);
   if(endKnotenSet.contains(oldN))
    tmpGraph->addEndNode(neuN);
  }

 for(i = kantenSet.first(); i!=0; kantenSet.next(i))
  {
   FFEdge oldE = kantenSet(i);
   const auto source = copiedNodes.find(oldE->source());
   const auto target = copiedNodes.find(oldE->target());
   if(source != copiedNodes.end() && target != copiedNodes.end())
    {
     FFEdge neuE = tmpGraph->addEdge(source->second, target->second);
     neuE->copyDataFrom(oldE, bare);
    }
  }
 // Return the raw pointer with its original zero-reference-count semantics;
 // the caller's FFGraph wrapper will acquire the owning reference.
 if(temporaryGraphReference)
  tmpGraph->decNumOfReps();
 return tmpGraph;
}

void FFGraphClass::clear()
{
 //  while(! knotenSet.empty())
 //   {
 //    removeNode(knotenSet(knotenSet.first()));
 //   }  
 Pix i= knotenSet.first(), tmpI;
 while(i != NULL)
  {
   tmpI=i;
   knotenSet.next(i);   
   removeNode(knotenSet(tmpI));
  } // !!! Funktioniert nur solange die Sortierung innerhalb der Menge sich
 // nicht ndert z.B. durch resize, neuordnen des Baums etc.... !!!
}

BOOL FFGraphClass::setStartNode(FFNode n)
{
 if(!contains(n))
  {
   return NO;
  }

 labelEventStruct theEvent;
 theEvent.node=startNode;
		
 if(startNode != NULL)
  {
   startNode->_sendEvent(DEL_STARTNODE, &theEvent);
  }	
 startNode = n;
 theEvent.node=n;
 _sendGraphEvent(SET_STARTNODE, &theEvent);
 startNode->_sendEvent(SET_STARTNODE, &theEvent);
	
 return YES;
}

FFNode FFGraphClass::getStartNode()
{
 return startNode;
}

BOOL FFGraphClass::addEndNode(FFNode n)
{
 if(!contains(n))
  return NO;

 endKnotenSet.add(n);
 labelEventStruct theEvent;
 theEvent.node=n;
 n->_sendEvent(SET_ENDNODE, &theEvent);
 return YES;  
}
  
BOOL FFGraphClass::setEndNodes(const FFSet<FFNode>& s)
{
 for(Pix i=s.first(); i!=0; s.next(i))
  {
   if(!contains(s(i)))
    return NO;
  }

 labelEventStruct theEvent;
	
 Pix j;
 for(j=endKnotenSet.first(); j!=0; endKnotenSet.next(j))
  {
   theEvent.node=endKnotenSet(j);
   endKnotenSet(j)->_sendEvent(DEL_ENDNODE, &theEvent);
  }
  
 endKnotenSet.clear();
 endKnotenSet |= s;
 for(j=endKnotenSet.first(); j!=0; endKnotenSet.next(j))
  {
   theEvent.node=endKnotenSet(j);
   endKnotenSet(j)->_sendEvent(SET_ENDNODE, &theEvent);
  }
 return YES;
}

BOOL FFGraphClass::delEndNode(FFNode n)
{
 if(!endKnotenSet.contains(n))
  return NO;
 labelEventStruct theEvent;
 theEvent.node=n;
  
 n->_sendEvent(DEL_ENDNODE, &theEvent);  
 endKnotenSet.del(n);
 return YES;
}

void FFGraphClass::clearEndNodes()
{
 labelEventStruct theEvent;
 
 for(Pix j=endKnotenSet.first(); j!=0; endKnotenSet.next(j))
  {
   theEvent.node= endKnotenSet(j);
   endKnotenSet(j)->_sendEvent(DEL_ENDNODE, &theEvent);
  }
 endKnotenSet.clear();
}

const FFSet<FFNode>& FFGraphClass::getEndNodes()
{
 return endKnotenSet;
}

BOOL FFGraphClass::contains(FFNode n)
{
 return (knotenSet.contains(n));
}

BOOL FFGraphClass::contains(FFEdge n)
{
 return (kantenSet.contains(n));
}

void FFGraphClass::rev()
{
 for(Pix i = kantenSet.first(); i != 0; kantenSet.next(i))
  {
   kantenSet(i)->revert();
  }
}

void FFGraphClass::eliminateParallelEdges()
{
 for(Pix i=knotenSet.first(); i!=0; knotenSet.next(i))
  {
   knotenSet(i)->eliminateParallelEdges();
  }  
}

void FFGraphClass::allNodes(FFSet<FFNode> &tmpS)
{
 tmpS.clear();
 tmpS |= knotenSet;
}

void FFGraphClass::allEdges(FFSet<FFEdge> &tmpS)
{
 tmpS.clear();
 tmpS |= kantenSet;
}

const FFSet<FFNode>& FFGraphClass::allNodes() const
{
 return knotenSet;
}

const FFSet<FFEdge>& FFGraphClass::allEdges() const
{
 return kantenSet;
}

void FFGraphClass::setName(const String& s)
{
 name=s;
 labelEventStruct theEvent;
 theEvent.graph= this;
 _sendGraphEvent(NAME_CHANGE, &theEvent);
}

FFNodeClass *FFGraphClass::nodeWithValue(int k)
{
 for(Pix i= knotenSet.first(); i!=0; knotenSet.next(i))
  {
   if(knotenSet(i)->getValue() == k)
    return knotenSet(i);
  }
 return NULL;
}
						
int FFGraphClass::getNodeLabelIndex(const String& s, FFSet<int> *set) const
{
 if(set == NULL)
  {
   for(Pix i= nodeL.first(); i!=0; nodeL.next(i))
    {    		
     //      if(nodeL(i)->type() == s)
     if(nodeL(i)->type().contains(s)) // !!!!!! WIEDER AEDNERN !!!!!!
      return nodeL.Pix_to_index(i);  
    }
  }
 else
  {  
   for(Pix i=nodeL.first(); i!= 0; nodeL.next(i))
    {
     if(nodeL(i)->type().contains(s))
      set->add(nodeL.Pix_to_index(i));
    }
   if(set->length())
    return (*set)(set->first());
  }
 return -1;
}

int FFGraphClass::getEdgeLabelIndex(const String& s, FFSet<int> *set) const
{
 if(set == NULL)
  {
   for(Pix i= edgeL.first(); i!=0; edgeL.next(i))
    {    		
     //      if(edgeL(i)->type() == s)
     if(edgeL(i)->type().contains(s)) // !!!!!! WIEDER AENDERN !!!!!!!
      return edgeL.Pix_to_index(i);  
    }
  }
 else
  {  
   for(Pix i=edgeL.first(); i!= 0; edgeL.next(i))
    {
     if(edgeL(i)->type().contains(s))
      set->add(edgeL.Pix_to_index(i));
    }
   if(set->length())
    return (*set)(set->first());
  }
 return -1;
}

int FFGraphClass::getGraphLabelIndex(const String& s, FFSet<int> *set) const
{
 if(set == NULL)
  {
   for(Pix i= graphLabel.first(); i!=0; graphLabel.next(i))
    {    		
     //      if(graphLabel(i)->type() == s)
     if(graphLabel(i)->type().contains(s)) // !!!!!! WIEDER AENDERN !!!!!!
      return graphLabel.Pix_to_index(i);  
    }
  }
 else
  {  
   for(Pix i=graphLabel.first(); i!= 0; graphLabel.next(i))
    {
     if(graphLabel(i)->type().contains(s))
      set->add(graphLabel.Pix_to_index(i));
    }
   if(set->length())
    return (*set)(set->first());
  }
 return -1;
}

void FFGraphClass::delNodeLabel(int k, BOOL doFree)
{
 if(nodeL.valid(k))
  {
   labelEventStruct theEvent;
   theEvent.index= k;

   if(doFree)
     delete nodeL[k];
   nodeL.del_index(k);
      
   _sendGraphEvent(DEL_NODELABEL, &theEvent);
      
   for(Pix i=knotenSet.first(); i!=0; knotenSet.next(i))
    {
     knotenSet(i)->_sendEvent(DEL_NODELABEL, &theEvent);
     knotenSet(i)->delLabel(k, doFree);
    }
  }
 else
  {
   cerr << "Trying to remove node label with invalid index " 
        << k << endl;
  }
}

void FFGraphClass::delEdgeLabel(int k, BOOL doFree)
{
 if(edgeL.valid(k))
  {	
   labelEventStruct theEvent;
   theEvent.index=k;
         
   if(doFree)
     delete edgeL[k];
   edgeL.del_index(k);
   _sendGraphEvent(DEL_EDGELABEL, &theEvent);
         
   for(Pix i=kantenSet.first(); i!=0; kantenSet.next(i))
    {
     kantenSet(i)->_sendEvent(DEL_EDGELABEL, &theEvent);
     kantenSet(i)->delLabel(k, doFree);
    }
  }
 else
  {
   cerr << "Trying to remove edge label with invalid index " 
        << k << endl;
  }
}

void FFGraphClass::delGraphLabel(int k, BOOL doFree)
{
 if(graphLabel.valid(k))
  {
   labelEventStruct theEvent;
   theEvent.index=k;
		
   _sendGraphEvent(DEL_GRAPHLABEL, &theEvent);
   Pix i;
   for(i=knotenSet.first(); i!=0; knotenSet.next(i))
    {
     knotenSet(i)->_sendEvent(DEL_GRAPHLABEL, &theEvent);
    }

   for(i=kantenSet.first(); i!=0; kantenSet.next(i))
    {
     kantenSet(i)->_sendEvent(DEL_GRAPHLABEL, &theEvent);
    }
		
   if(doFree)
    {
     FFGraphLabel *nase= graphLabel[k];
     delete nase;
    }
   graphLabel.del_index(k);
  }
 else
  {
   cerr << "Trying to remove graph label with invalid index " 
        << k << endl;
  }
}

int FFGraphClass::addNodeLabel(FFNodeLabel *l)
{
 int k=0;
 if((k=getNodeLabelIndex(l->type())) >= 0)
  {
   return k;
  }
 labelEventStruct theEvent;
 
 k= nodeL.add(l);
 
 theEvent.index=k;
 _sendGraphEvent(ADD_NODELABEL, &theEvent);
 
 for(Pix i=knotenSet.first(); i!=0; knotenSet.next(i))
  {
   knotenSet(i)->addLabel(l);
   knotenSet(i)->_sendEvent(ADD_NODELABEL, &theEvent);
  }
 return k;
}

int FFGraphClass::addEdgeLabel(FFEdgeLabel *l)
{
 int k=0;
 if((k=getEdgeLabelIndex(l->type())) >= 0)
  {
   return k;
  }
 k= edgeL.add(l);
 
 labelEventStruct theEvent;
 theEvent.index =k;
 
 _sendGraphEvent(ADD_EDGELABEL, &theEvent);
 
 for(Pix i=kantenSet.first(); i!=0; kantenSet.next(i))
  {
   kantenSet(i)->addLabel(l);
   kantenSet(i)->_sendEvent(ADD_EDGELABEL, &theEvent);
  }
 return k;
}

int FFGraphClass::addGraphLabel(FFGraphLabel *l)
{
 int k=0;
 if((k=getGraphLabelIndex(l->type()))>= 0)
  {
   return k;
  }
 int tmpI= graphLabel.add(l);

 if(l->type().contains(ffGraphWindowType))
  graphIndex = tmpI;

 l->index=tmpI;
 l->init(FFGraph(this));
	
 labelEventStruct theEvent;
 theEvent.graph= this;
 _sendGraphEvent(ADD_GRAPHLABEL, &theEvent);

 Pix i;
 for(i=knotenSet.first(); i!=0; knotenSet.next(i))
  {
   knotenSet(i)->_sendEvent(ADD_GRAPHLABEL, &theEvent);
  }
	
 for(i=kantenSet.first(); i!=0; kantenSet.next(i))
  {
   kantenSet(i)->_sendEvent(ADD_GRAPHLABEL, &theEvent);
  }
	
 return tmpI;
}

void FFGraphClass::assignGraphLabel(const FFGraphLabel *l, int i)
{
 if(i >=0)
  {
   graphLabel[i]->assign(l);
  }
 else
  {
   int k= getGraphLabelIndex(l->type());
   if(k <0)
    k= addGraphLabel(l->proto());
		
   graphLabel[k]->assign(l);		
  }
}

void FFGraphClass::assignGraphLabel(const MPlex<FFGraphLabel *>& l)
{
 for(Pix i=l.first(); i!=0; l.next(i))
  {
   assignGraphLabel(l(i));
  }
}

void FFGraphClass::setGraphLabel(FFGraphLabel *l, int i, BOOL doFree)
{
 if(i >=0)
  {
   if(graphLabel[i] == l)
    return;
   if(doFree)
    delete graphLabel[i];
   l->index=i;
   graphLabel[i]=l;
   // setGraphLabel() transfers ownership just like addGraphLabel(), and
   // replacement labels must observe their containing graph as well.
   l->init(FFGraph(this));
  }
 else
  {
   int k= getGraphLabelIndex(l->type());
   if(k <0)
    k= addGraphLabel(l);
   else
    {
     if(graphLabel[k] == l)
      return;
     if(doFree)
      delete graphLabel[k];				
     l->index=k;
     graphLabel[k]=l;
     l->init(FFGraph(this));
    }
  }
}

void FFGraphClass::setGraphLabel(const MPlex<FFGraphLabel *>& l, BOOL doFree)
{
 if (&l == &graphLabel)
  return;
 for(Pix i=l.first(); i!=0; l.next(i))
  {
   setGraphLabel(l(i), doFree);
  }
}

FFGraphClass::FFGraphClass(BOOL bare)
{
 // While default labels are initialized, FFGraph(this) creates temporary
 // owning handles. Keep one construction reference so their destructors
 // cannot delete *this before construction finishes. The caller will acquire
 // the real owning reference after the constructor returns.
 reps=1;
 status= VALID;
 startNode = 0;
 graphIndex=-1;
 nodeIndex=-1;
 edgeIndex=-1;
 directed=YES;
 evStat= EVENT_NORMAL;
 if(bare != YES)
  {
   Pix i;
   for(i= defGraphLabels.first(); i!=0; defGraphLabels.next(i))
    addGraphLabel(defGraphLabels(i)->proto());
   for(i= defNodeLabels.first(); i!=0; defNodeLabels.next(i))
    addNodeLabel(defNodeLabels(i)->proto());
   for(i= defEdgeLabels.first(); i!=0; defEdgeLabels.next(i))
    addEdgeLabel(defEdgeLabels(i)->proto());
  }
 // Drop the construction guard without running a destructor on an object
 // that has not yet been returned to its owner.
 --reps;
}

void FFGraphClass::removeNode(FFNode node)
{
 // Check membership before dereferencing; a null, foreign or removed
 // pointer is not necessarily safe to inspect.
 if(!contains(node))
  {
   ffError(RUNTIME_ERROR, "Versuch nichtexistenten Knoten zu loeschen");
   return;
  }

 labelEventStruct theEvent;
 theEvent.node=node;
  
 _sendGraphEvent(DEL_NODE, &theEvent);
 node->_sendEvent(DEL_NODE, &theEvent);

 Pix pix= node->transFromPred().first();
 
 
 while(pix != NULL)
  {
   //   tmpPix= pix;
   //node->transFromPred().next(pix);
   removeEdge(node->transFromPred()(pix));
   pix= node->transFromPred().first();
  }
 
 pix= node->transToSucc().first();
 
 while(pix != NULL)
  {
   //   tmpPix=pix;
   //   node->transToSucc().next(pix);
   removeEdge(node->transToSucc()(pix));
   pix= node->transToSucc().first();
  }
  
 if(startNode == node)
  startNode = 0;    
 if(endKnotenSet.contains(node))
  endKnotenSet.del(node);

 knotenSet.del(node);
 delete node;
 _sendGraphEvent(AFTER_DEL_NODE, &theEvent);
}

void FFGraphClass::removeEdge(FFEdge edge)
{
 if(!contains(edge))
  {
   ffError(RUNTIME_ERROR, "Versuch nichtexistente Kante zu loeschen");
   return;
  }

 labelEventStruct theEvent;
 theEvent.edge=edge;
	
 _sendGraphEvent(DEL_EDGE, &theEvent);
 edge->_sendEvent(DEL_EDGE, &theEvent);
	  
 FFSet<FFEdge>& rTo = (FFSet<FFEdge>&)edge->source()->transToSucc();
 rTo.del(edge);

 FFSet<FFEdge>& rFrom= (FFSet<FFEdge>&)edge->target()->transFromPred();
 rFrom.del(edge);
  
 kantenSet.del(edge);
 delete edge;
 _sendGraphEvent(AFTER_DEL_EDGE, &theEvent);
}

FFNode FFGraphClass::addNode(const FFPoint& tmpP)
{
 FFNodeClass *tmpN = new FFNodeClass(&nodeL);
 knotenSet.add(tmpN);
 tmpN->init(this);
 if((*tmpN)())
  (*tmpN)->moveTo(tmpP);
  
 labelEventStruct theEvent;
 theEvent.node=tmpN;
  
 _sendGraphEvent(ADD_NODE, &theEvent);
 tmpN->_sendEvent(ADD_NODE, &theEvent);
  
 return tmpN;
}


FFEdge FFGraphClass::addEdge(FFNode s, FFNode t)
{
 if(!contains(s) || !contains(t))
  {
   ffError(RUNTIME_ERROR, "Source oder Target Knoten existieren nicht");
   return NULL;
  }

 FFEdgeClass *tmpE = new FFEdgeClass(&edgeL, s, t);

 ((FFSet<FFEdge>&)t->transFromPred()).add(tmpE);
 ((FFSet<FFEdge>&)s->transToSucc()).add(tmpE);
 kantenSet.add(tmpE);

 tmpE->init(this);

 labelEventStruct theEvent;
 theEvent.edge=tmpE;
	
 _sendGraphEvent(ADD_EDGE, &theEvent);
 tmpE->_sendEvent(ADD_EDGE, &theEvent);
 return tmpE; 
}

void FFGraphClass::setEventStatus(eventStatus s)
{
 if((evStat == EVENT_DISABLED) &&
    (s == EVENT_NORMAL))
  {
   evStat = s;
   Pix i= 0;
   labelEventStruct theEvent;

   _sendGraphEvent(MULTIPLE_CHANGES, &theEvent);
   
   for(i= knotenSet.first(); i != 0; knotenSet.next(i))
    {
     knotenSet(i)->_sendEvent(MULTIPLE_CHANGES, &theEvent);
    }
   for(i= kantenSet.first(); i != 0; kantenSet.next(i))
    {
     kantenSet(i)->_sendEvent(MULTIPLE_CHANGES, &theEvent);
    }
  }
 else
  evStat = s;
}

void FFGraphClass::incNumOfReps(int i) const
{
 reps +=i;
}

BOOL FFGraphClass::decNumOfReps(int i)
{
 reps -=i;
 return reps;
}

FFGraphClass::~FFGraphClass()
{
 if(reps != 0)
  {
   ffError(RUNTIME_ERROR, "Non correct removal of a graph");
  }

 // Graph/label destruction can invoke callbacks that construct short-lived
 // FFGraph handles. Keep them from recursively deleting this graph while its
 // destructor is in progress; only the external owning reference should
 // determine when destruction starts.
 ++reps;

 // while (! knotenSet.empty())
 //  removeNode(knotenSet(knotenSet.first()));
 
 clear();

 labelEventStruct theEvent;
 theEvent.graph = this;
 
 _sendGraphEvent(DEL_GRAPH, &theEvent);
  
 Pix i;
 for(i=nodeL.first(); i!=0; nodeL.next(i))
  {
   delete nodeL(i);
  }
 for( i=edgeL.first(); i!=0; edgeL.next(i))
  {
   delete edgeL(i);
  }
 for( i=graphLabel.first(); i!=0; graphLabel.next(i))
  {
   delete graphLabel(i);
  }
 
#ifdef DEBUG
 cerr << "Graph got deleted\n";
#endif
 --reps;
}


FFGraphLabel *FFGraphClass::getGraphLabel(const String& s, 
                                          FFSet<FFGraphLabel *> *set)
{
 if(set == NULL)
  return getGraphLabel(getGraphLabelIndex(s));
 
 for(Pix i=graphLabel.first(); i!= 0; graphLabel.next(i))
  {
   if(graphLabel(i)->type().contains(s))
    set->add(graphLabel(i));
  }
 if(set->length())
  return (*set)(set->first());
 return NULL;
}


void FFGraphClass::show()
{
 Pix ind1, ind2;
 FFSet<FFEdgeClass *> succ_trans;
 cerr << "\n";
 cerr << "Display of Graphs: \n";
 for (ind1 = graphLabel.first(); ind1 != 0; graphLabel.next(ind1)) {
  cerr << "Graphlabel: " << graphLabel(ind1)->type() <<endl;
 }
 for (ind1 = nodeL.first(); ind1 != 0; nodeL.next(ind1)) {
  cerr << "Node Label: " << nodeL(ind1)->type() <<endl;
 }
 for (ind1 = edgeL.first(); ind1 != 0; edgeL.next(ind1)) {
  cerr << "Edge Label: " << edgeL(ind1)->type() <<endl;
 }
 cerr << " ( States : [name/label] || " <<
  "Transitions : --(name/label)-> )\n";
 cerr << "==================\n\n";
 for ( ind1 = knotenSet.first(); ind1 != 0; knotenSet.next(ind1) ) {
  knotenSet(ind1)->show();
  succ_trans.clear();
  succ_trans |= knotenSet(ind1)->transToSucc(); 
  for ( ind2 = succ_trans.first(); ind2 != 0; succ_trans.next(ind2) ) { 
   succ_trans(ind2)->show();
   cerr << "\n\t";
  }; 
  cerr << "\n";
 };
 cerr << "\n";
}

void FFGraphClass::createGraphic(FFGraphWindow *w, FFGraphicNode *gn,
				 FFGraphicEdge *ge)
{
 if(graphIndex != -1)
  removeGraphic();
  
 if(w != NULL)
  graphIndex = addGraphLabel(w);
  
 if(gn != NULL)
   nodeIndex= addNodeLabel(gn);
 else 
   {
     if(nodeIndex == -1)
       nodeIndex = addNodeLabel(new FFGraphicNode);
   }

 if(ge != NULL)
   edgeIndex = addEdgeLabel(ge);
 else
   {
     if(edgeIndex == -1)
       edgeIndex = addEdgeLabel(new FFGraphicEdge);
   }
 
 if(graphIndex >= 0)
  {
   if(name != String(""))
    FENSTER->setTitle(name);
   else
    FENSTER->setTitle(String("NoName"));			
   FENSTER->adjustWindowSize();
  }
}

void FFGraphClass::removeGraphic()
{
  if(nodeIndex >=0)
    delNodeLabel(nodeIndex);

  if(edgeIndex >=0)
    delEdgeLabel(edgeIndex);
		
 if(graphIndex >= 0)
  delGraphLabel(graphIndex);
		
 graphIndex = nodeIndex=edgeIndex= -1;
}

FFGraphWindow *FFGraphClass::leaveWindow()
{
 FFGraphWindow *tmpW= 0;

 if(nodeIndex >=0)
  delNodeLabel(nodeIndex);
 
 if(edgeIndex >=0)
  delEdgeLabel(edgeIndex);
  
 if(graphIndex >= 0)
  {  
   tmpW=FENSTER;
   delGraphLabel(graphIndex, NO);
  }

 // graphIndex=-1;
 graphIndex=nodeIndex=edgeIndex=-1;
 return tmpW;
}

FFGraphWindow *FFGraphClass::hideGraphic()
{
 FFGraphWindow *tmpW= 0;

 if(graphIndex >= 0)
  {  
   tmpW=FENSTER;
   delGraphLabel(graphIndex, NO);
  }

 // graphIndex=-1;
 graphIndex=-1;
 return tmpW;
}

void FFGraphClass::saveToFile(const String& datei)
{
 saveToFile(datei, knotenSet);
}

void FFGraphClass::saveToFile(const String& datei, const FFSet<FFNode>& nodes)
{
 ofstream ofDatei(expand_path(datei));
//  if(getName() == "")
//   setName(datei);
 
 if(!ofDatei.good())
  {
   ffError(USER_ERROR, String("Couldn't open File: ") + datei);
  }
 else
  {
   if(datei.contains(".gml"))
    _saveGML(ofDatei, nodes);
   else
    _save(ofDatei, nodes);
  }
}

void FFGraphClass::saveToString(String& buffer)
{
 saveToString(buffer, knotenSet);
}

void FFGraphClass::saveToString(String& buffer, const FFSet<FFNode>& nodes)
{
 std::ostringstream ofDatei;
 _save(ofDatei, nodes);
 buffer = String(ofDatei.str());
}

void FFGraphClass::_sendGraphEvent(labelEvent e, labelEventStruct *s)
{
 if(evStat == EVENT_NORMAL)
  for(Pix i=graphLabel.first(); i!=0; graphLabel.next(i))
   {
    graphLabel(i)->event(e, s);
   }
}

void FFGraphClass::_setDirected(BOOL b)
{
 directed = b;
 labelEventStruct theEvent;
 
 _sendGraphEvent(DIRECTEDNESS_CHANGE, &theEvent);
 Pix i=0;
 for(i= knotenSet.first(); i!=0; knotenSet.next(i))
  {
   knotenSet(i)->_sendEvent(DIRECTEDNESS_CHANGE, &theEvent);
  }
 
 for(i= kantenSet.first(); i!=0; kantenSet.next(i))
  {
   kantenSet(i)->_sendEvent(DIRECTEDNESS_CHANGE, &theEvent);
  }	
}

void FFGraphClass::_save(ostream& ofDatei, const FFSet<FFNode>& nodes)
{
 int k=1;		
 String tmpString;
 
 // Give every node a unique number
 Pix i;
 for(i=knotenSet.first(); i!=0; knotenSet.next(i))
  {
   knotenSet(i)->setValue(k++);
  }
 
 // Write Graph 
 if(directed)
  ofDatei << "GRAPH \"" << name << "\" = DIRECTED\n{$\n";
 else
  ofDatei << "GRAPH \"" << name << "\" = UNDIRECTED\n{$\n";
 
 if(graphIndex >= 0)
  {
   if(FENSTER != NULL)
    {
     ofDatei << "FF_SC " << (int)(FENSTER->getZoom()*100) << endl;
     ofDatei << "FF_RZ " << (int)(FENSTER->getGamma()) << endl;
    }
  }
 
 // Write Graph Labels
 
 
 for(k=graphLabel.low(); k<graphLabel.fence(); graphLabel.nextInt(k))
  {		
   if(graphLabel[k]->isSaveable())
    {
     // cerr << "Saving Label " << graphLabel[k]->type();
     tmpString = graphLabel[k]->unparse();
     tmpString.gsub("\\", "\\\\");
     tmpString.gsub("\"", "\\\"");
     ofDatei << " FF_L \"" << graphLabel[k]->type()
             << "\" \"" << tmpString << "\"";
    }
   // else
   // cerr << "Skipping Label " << graphLabel[k]->type() << endl;
  }

 ofDatei << "\n$}\n";
 
 // Write Node

 for(i = nodes.first(); i!=0; nodes.next(i))
  {
   
   // Write Node Data
   
   ofDatei << nodes(i)->getValue() << " {$";
   FFGraphicNode *gn=(*nodes(i))();
   
   if(nodes(i) == startNode)
    ofDatei << " SN";
   
   if(endKnotenSet.contains(nodes(i)))
    ofDatei << " EN";
   
   // Write Node Label if available
   if(nodes(i)->getLabels() != NULL)
    {
     for(k=nodes(i)->getLabels()->low(); 
         k<nodes(i)->getLabels()->fence(); 
         nodes(i)->getLabels()->nextInt(k))
      {
       if(nodes(i)->getLabel(k)->isSaveable())
        {
         tmpString = nodes(i)->getLabel(k)->unparse();
         tmpString.gsub("\\", "\\\\");
         tmpString.gsub("\"", "\\\"");
         ofDatei << " FF_L \"" << nodes(i)->getLabel(k)->type()
                 << "\" \"" << tmpString << "\"";
        }
      }
    }
   // Write graphic information if available
   
   if(gn != NULL)
    {
     if(gn->getIcon() != "NoIcon")
      {
       ofDatei << " FF_NI \"" << gn->getIcon()
               << "\"";
       if(gn->getSelIcon() != "NoIcon")
        {
         ofDatei << " \"" << gn->getSelIcon() << "\"";
        }
      }
     
     if(gn->getMask() != "NoMask")
      {
       ofDatei << " FF_NM \"" << gn->getMask() << "\"";
       if(gn->getSelMask() != "NoMask")
        {
         ofDatei << " \"" << gn->getSelMask() << "\"";
        }
      }
     
     ofDatei << " NP " << (int)gn->getx() << " " << (int)gn->gety();
     
     ofDatei << " FF_3D " << (int)gn->getx() << " " << (int)gn->gety()
             << " " << (int)gn->getz();
     
     if(gn->getShape() == FF_CIRCLE)
      ofDatei << " FF_R " << (int)gn->getWidth();
     
     if(gn->getColor().length())
      ofDatei << " FF_C \"" << gn->getColor() << "\"";

    } 
   ofDatei << " $} \"" << nodes(i)->getName() << "\"\n";

   // Write edges going out of this node if their target belongs
   // to the node list
		
   const FFSet<FFEdge>& transTo = nodes(i)->transToSucc();
   for(Pix j=transTo.first(); j!= 0; transTo.next(j))
    {
     FFEdge tmpEdge = transTo(j);
     
     if(nodes.contains(tmpEdge->target()))
      {

       // Write tmpEdge data
			
       ofDatei << " " <<tmpEdge->target()->getValue()<<" {$";

       if(tmpEdge->getWeight() != 1.0)
        ofDatei << " FF_WE " << (int)(tmpEdge->getWeight()*100);

       FFGraphicEdge *ge=(*tmpEdge)();

       // Write egde Label if available
       if(tmpEdge->getLabels() != NULL)
        {
         for(k=tmpEdge->getLabels()->low(); 
             k<tmpEdge->getLabels()->fence(); 
             tmpEdge->getLabels()->nextInt(k))
          {
           if(tmpEdge->getLabel(k)->isSaveable())
            {
             tmpString = tmpEdge->getLabel(k)->unparse();
             tmpString.gsub("\\", "\\\\");
             tmpString.gsub("\"", "\\\"");
             ofDatei << " FF_L \"" << tmpEdge->getLabel(k)->type()
                     << "\" \"" << tmpString << "\"";
            }
          }
        }

       // Write tmpEdge graphic information if available

       if(ge != NULL)
        {
         ofDatei << " FF_3D ";
         for(k=0; k <ge->anzPoints(); k++)
          {
           FFPoint& tmpP= ge->getPointAt(k);
           ofDatei << (int)tmpP.v[0] << " " << (int)tmpP.v[1] << " "
                   << (int)tmpP.v[2] << " ";
          }
         if(ge->getColor().length())
          ofDatei << " FF_C \"" << ge->getColor()
                  << "\"" ;

        }
       ofDatei << " $} \"" << tmpEdge->getName() << "\"\n";
      }
    }
   ofDatei << " ;\n";
  }
 ofDatei << "END" << endl;
 ofDatei.flush();
}

void FFGraphClass::_saveGML(ostream& ofDatei, const FFSet<FFNode>& nodes)
{
 int k=1;		
 String tmpString;
 
 // Give every node a unique number
 Pix i;
 for(i=knotenSet.first(); i!=0; knotenSet.next(i))
  {
   knotenSet(i)->setValue(k++);
  }
 
 // Write Graph 
 ofDatei << "#!" << BINDIR << "/ffgraph" << endl << endl;
 
 ofDatei << "graph" << endl << "[" << endl;
 
 struct passwd *tmpPwd = getpwuid(getuid());
 
 
 ofDatei << "creator \"ffGraph for " << tmpPwd->pw_gecos << "\"" << endl;
 ofDatei << "label \"" << getName() << "\"" << endl;
 
//  if(directed)
//  else
//   ofDatei << "GRAPH \"" << name << "\" = UNDIRECTED\n{$\n";
 

 // Write Node

 for(i = nodes.first(); i!=0; nodes.next(i))
  {
   FFNode tmpNode=nodes(i);
   ofDatei << "node" << endl << "[" << endl;
   
   ofDatei << "id " << tmpNode->getValue() << endl;
   ofDatei << "label \"" << tmpNode->getName()<< "\"" << endl;
   if(tmpNode->getGraphicNode())
    {
     ofDatei << "graphics [" << endl;
     ofDatei << "x " << (*tmpNode)->getx() << endl;
     ofDatei << "y " << (*tmpNode)->gety() << endl;
     ofDatei << "]" << endl;
    }
   
   ofDatei << "]" << endl;
  }
 
   // Write edges going out of this node if their target belongs
   // to the node list
		
 for(i = kantenSet.first(); i!= 0; kantenSet.next(i)) {
  FFEdge tmpEdge = kantenSet(i);
  if(nodes.contains(tmpEdge->source()) &&
     nodes.contains(tmpEdge->target()))
   {
    ofDatei << "edge" << endl << "[" << endl;
    ofDatei << "source " << tmpEdge->source()->getValue()<< endl;
    ofDatei << "target " << tmpEdge->target()->getValue()<< endl;
    ofDatei << "label \"" << tmpEdge->getName()<< "\"" << endl;
    ofDatei << "]" << endl;
   }
 }
 
 ofDatei << "]" << endl;
 ofDatei.flush();
}
				
void FFGraphClass::load(const String& buffer, BOOL datei)
{
 labelEventStruct theEvent;
 FFControl graphParser;
 BOOL tmpDate= NO;
 
 _sendGraphEvent(START_LOAD, &theEvent);
 
 if(graphIndex >= 0)
  {
   tmpDate= FENSTER->getUpdate();
   FENSTER->setUpdate(NO);
  }
 
 for(Pix i=knotenSet.first(); i!=0; knotenSet.next(i))
  knotenSet(i)->setValue(-1);
 
 if(datei)
  {
   ifstream ifdatei(expand_path(buffer));
   
   if(!ifdatei.good())
    {
     ffError(USER_ERROR, String("Couldn't open File: ") + buffer);
    }
   else
    {
     if(buffer.contains(".gml"))
      {
       GTControl gtParser;
       gtParser.Parse(ifdatei, this);
      }
     else if(buffer.contains(".g"))
      graphParser.Parse(ifdatei, this);
     else
      {
       RomeControl romeParser;
       romeParser.Parse(ifdatei, this);
      }
//      if(getName() == "")
//       setName(buffer);
    }
  }
 else
  {
   graphParser.Parse(buffer, this);
  }      
 
 if(graphIndex >= 0)
  {
   FENSTER->setUpdate(tmpDate);
   FENSTER->adjustWindowSize();
   if(tmpDate)
    FENSTER->update();
  }
 _sendGraphEvent(END_LOAD, &theEvent);
}

BOOL FFGraphClass::containsCycle ()
// Funktionalitaet: liefert "YES", falls Graph Zyklen enthaelt, sonst "NO"
// Vorbedingung: gerichteter Graph 
// Nachbedingung: 
// Bemerkung: Von jedem Knoten aus testen, ob dieser Knoten ueber einen 
//   Pfad erreichbar ist (O (n^2))
// ToDo: Verbesserungen: Algorithmus mittels Tiefensuche
{  
 FFSet <FFNode> reachNodes, visitNodes;
 Pix p, q;
 
 for (p = knotenSet.first (); p != 0; knotenSet.next (p))
  {
   visitNodes.clear ();
   reachNodes.clear ();
   reachNodes |= knotenSet (p)->succ ();
     
   // bereits besuchte Knoten finden sich in visitNodes
   q = reachNodes.first ();
   if (q != NULL) 
    {
     while (q != 0)
      {
       while (! visitNodes.contains (reachNodes (q)))
        {
         visitNodes.add (reachNodes (q));
         reachNodes |= reachNodes (q)->succ ();
         q = reachNodes.first ();
        }  

       reachNodes.next (q);
      }
    }
    
   if (reachNodes.contains (knotenSet (p)))
    return (YES);
  }  

 return (NO);
}

BOOL FFGraphClass::reachAllNodes ()
// Funktionalitaet: liefert "YES", falls alle Knoten vom Startknoten aus 
//   erreichbar sind, sonst "NO"
// Vorbedingung: gerichteter Graph, Startnoten ist gesetzt
// Nachbedingung: 
// Bemerkung:
{
 FFSet <FFNode> reachNodes, visitNodes;
 Pix p;
  
 if (startNode)
  {
   reachNodes.add (startNode);
   reachNodes |= startNode->succ ();
   visitNodes.add (startNode);
   p = reachNodes.first ();

   // bereits besuchte Knoten finden sich in visitNodes
   while (p != 0)
    {
     if (! visitNodes.contains (reachNodes (p)))
      {
       reachNodes |= reachNodes (p)->succ ();
       visitNodes.add (reachNodes (p));
       p = reachNodes.first ();
      }
     else
      reachNodes.next (p);
    }

   if (reachNodes == knotenSet)
    return (YES);
  }

 return (NO);
}

BOOL FFGraphClass::weakConnected ()
// Funktionalitaet: liefert "YES", falls Graph schwach zusammenhaengend ist, 
//   sonst "NO"
// Vorbedingung: gerichteter Graph 
// Nachbedingung: 
// Bemerkung: Einfacher Zusammenhang im Graphen, alle Kanten als ungerichtet 
//   betrachten.
{
 FFSet <FFNode> weakNodes, visitNodes;
 Pix p;

 p = knotenSet.first ();
 if (p != NULL)
  {
   weakNodes.add (knotenSet (p));

   while (p != 0)
    {
     // bereits besuchte Knoten finden sich in visitNodes
     while (! visitNodes.contains (weakNodes (p)))
      {
       visitNodes.add (weakNodes (p));
       weakNodes |= weakNodes (p)->succ ();
       weakNodes |= weakNodes (p)->pred ();
       p = weakNodes.first ();
      }  

     weakNodes.next (p);
    }    
  }

 if (knotenSet == weakNodes)
  return (YES);
 else
  return (NO);
}

BOOL FFGraphClass::strongConnected ()
// Funktionalitaet: liefert "YES", falls Graph stark zusammenhaengend ist, 
//   sonst "NO"
// Vorbedingung: gerichteter Graph 
// Nachbedingung: 
// Bemerkung: (starker) Zusammenhang im Graphen 
{
 FFNode startN;
  
 // merke alten Startknoten
 startN = startNode;
 for (Pix p = knotenSet.first (); p != 0; knotenSet.next (p))
  {
   startNode = knotenSet (p);
   if (! reachAllNodes ())
    {
     startNode = startN;
     return (NO);
    }
  }

 startNode = startN;
 return (YES);
}

BOOL FFGraphClass::isTree ()
// Funktionalitaet: liefert "YES", falls Graph ein Baum ist, sonst "NO"
// Vorbedingung: gerichteter Graph 
// Nachbedingung: 
// Bemerkung:
// ToDo: 
//    Erreichbarkeit von der Wurzel pruefen;
//    Wie soll ein gesetzter Startknoten behandelt werden?;
//    Pruefung auf n-Baeume (indeg <= n);
//    vollstaendige Baeume;
{
 Pix  p;
 int  anzStartNodes = 0;

 for (p = knotenSet.first (); p != 0; knotenSet.next (p))
  {
   if (knotenSet (p)->indeg () > 1)
    return (NO);
   if (knotenSet (p)->indeg () == 0)
    anzStartNodes++;
  }

 if (anzStartNodes != 1)
  return (NO);

 return (YES);
}

void FFGraphClass::straightenEdges ()
// Funktionalitaet: alle Kantenstuetzpunkte entfernen
// Vorbedingung: gerichteter Graph
// Nachbedingung: aus allen Kanten sind die Stuetzpunkte entfernt
{    
 FFGraphicEdge *ge;
 Pix p;

 for (p = kantenSet.first (); p != 0; kantenSet.next (p)) 
  {
   ge = (*kantenSet (p)) ();
   if (ge != 0)
     while (ge->anzPoints () > 2)  
       ge->deletePointAt (1);
  }
}

void FFGraphClass::levelDAG (int choice, FFCoord distance)
// Funktionalitaet: Setze Abstaende der Ebenen in y-Richtung 
//   (Soehne hoeheren y-Wert als Vaeter)
// Vorbedingung: gerichteter azyklische Graph (DAG) mit einer Wurzel 
//   (Startknoten == Wurzel!), alle Knoten vom Starknoten aus erreichbar,
//   Startknoten ist gesetzt
// Nachbedingung: Veraendert Knotenkoordinaten
// Bemerkung:
//   choice == 
//   1: gleicher Abstand zwischen Ebenen;
//   2: Abstand(Level(i)) == 1/2 von Abstand(Level(i-1)),
//      Abstand(Level(1)) == distance;
//   3: Abstand abhaengig von maximaler Zahl der ein-/ausgehenden Kanten eines
//      Knotens in diesem Level, hier: (Kantenzahl/3) * distance;
// ToDo: 
//    Uebergabe einer Funktion fuer Abstaende;
//    x-Richtung; diagonale Richtungen;
//    choice 3
{
 FFCoord level = 0.0;
 FFSet <FFNode> allNodes, 
  levelNodes, 
  predLevelNodes;
 Pix p;
 FFGraphicNode *gn;
 FFPoint point;

 if (startNode != NULL)
  {
   allNodes |= knotenSet;
   allNodes.del (startNode);
   levelNodes |= startNode->succ ();

   gn = (*startNode) ();
   if (gn)
    level = gn->gety ();

   if (choice == 1)
    level += distance;
   if (choice == 2)
    {
     level += distance;
     distance /= 2;
    }

   while (!allNodes.empty ())
    {
     for (p = levelNodes.first (); p != 0; levelNodes.next (p))
      {
       gn = (*levelNodes  (p)) ();
       if (gn)
        {
         point.v[0] = gn->getx ();
         point.v[1] = level;
         point.v[2] = gn->getz ();
         gn->moveTo (point);
        }
      }

     if (choice == 1)
      level += distance;
     if (choice == 2)
      {
       level += distance;
       distance /= 2;
      }

     predLevelNodes.clear ();
     predLevelNodes |= levelNodes;
     levelNodes.clear ();
     for (p = predLevelNodes.first (); p != 0; predLevelNodes.next (p))
      {
       levelNodes |= predLevelNodes (p)->succ ();
       allNodes.del (predLevelNodes (p));
      }
    }
  }
}

void FFGraphClass::setInLevel (SLList <FFNode> &levelNodes, int, 
			       FFCoord distance)
// Funktionalitaet: Setzt alle Knoten, welche in der Liste stehen,
//   entsprechend ihrer Reihenfolge in der Liste innerhalb einer Ebene.
// Vorbedingung: distance >= 0.0
// Nachbedingung: 
// Bemerkung:
//   choice == 
//   1: gleicher Abstand zwischen Knoten, 
//      Knoten immer mit gleicher oder groesserer x-Koord. als Vaterknoten
//      (fuer Baeume);
// ToDo: 
//   Uebergabe einer Funktion fuer Abstaende;
//   Beruecksichtigung mehrerer Vaeter;
//   Startwert fuer pos bestimmen
{
 FFCoord pos = -10000.0;
 FFGraphicNode *gn, *gPredN;
 FFNode tmpN;
 FFSet <FFNode> predNodes;
 Pix p;
 FFPoint point;

 // TODO :Startwert fuer pos!
 while (! levelNodes.empty ())
  {
   tmpN = levelNodes.remove_front ();
   gn = (*tmpN) ();
   if (gn)
    {
     predNodes.clear ();
     predNodes |= tmpN->pred ();
     p = predNodes.first ();
     if (p != NULL)
      {
       gPredN = (*predNodes (p)) ();
       if (gPredN)
        {
         if (gPredN->getx () > pos)
          pos = gPredN->getx ();
        }
      }
     else
      {          
       if (gn->getx () > pos)
        pos = gn->getx ();
      }

     point.v[0] = pos;
     point.v[1] = gn->gety ();
     point.v[2] = gn->getz ();
     gn->moveTo (point);
     pos += distance;
    }
  }
}

FFGraphClass * FFGraphClass::getSubGraph(FFNodeClass *knoten, BOOL firstTime)
{
 static FFSet<FFNodeClass *> testNodes;
 
 if(firstTime)
  {
   int k=0;
   
   for(Pix i=knotenSet.first(); i != 0; knotenSet.next(i))
    knotenSet(i)->setValue(++k);
   testNodes.clear();
  }
 
 if (testNodes.contains(knoten))
  return NULL;
 
 testNodes.add(knoten);
 
 const FFSet<FFEdgeClass *>& nachFolger = knoten->transToSucc();

 Pix i;
 
 for(i = nachFolger.first(); i != 0; nachFolger.next(i))
  getSubGraph(nachFolger(i)->target(), NO);
 
 if(firstTime)
  {
   return copy(NULL, &testNodes);
  } 
 return NULL;
}

FFNode findNextNode (FFSet <FFNode> &levelNodes, int choice)
// Funktionalitaet:
// Vorbedingung: 
// Nachbedingung: 
// Bemerkung:
{
 Pix p, q;
 FFNode nextN = 0;
 FFCoord predX = 10000.0;
 FFGraphicNode *gn;
 FFSet <FFNode> predNodes;

 if (choice == 1)
  {
   p = levelNodes.first ();
   nextN = levelNodes (p);

   while (p != 0) 
    {
     predNodes.clear ();
     predNodes |= levelNodes (p)->pred ();
     q = predNodes.first ();
     if (q != NULL)
      {
       gn = (*predNodes (q)) ();
       if (gn)
        {
         if (gn->getx () < predX)
          {
           nextN = levelNodes (p);
           predX = gn->getx ();
          }
        }  
      }
     levelNodes.next (p);
    }
  }
  
 levelNodes.del (nextN);
 return (nextN);
}

void FFGraphClass::setDAGLevels (int, FFCoord distance)
// Funktionalitaet: Bestimmt ausgehend vom Startknoten die einzelnen Ebenen
//   (z. B. alle Soehne in eine Ebene oder alle Knoten mit gleichen y-Koord.
//   in eine Ebene), sortiert die gewaehlten Knoten innerhalb dieser Liste
//   und ruft g->setInLevel auf.
// Vorbedingung: gerichteter azyklischer Graph (DAG), 
//   Startknoten muss gesetzt sein, alle Knoten 
//   muessen vom Startknoten erreichbar sein
// Nachbedingung: 
// Bemerkung:
//   choice == 
//   1: gleicher Abstand zwischen Knoten;
// ToDo: 
//   Uebergabe einer Funktion fuer Abstaende;
//   Abstaende abhaengig vom Level;
//   Sortieren der Liste
{
 SLList <FFNode> nodeList;
 FFSet <FFNode> succNodes, predNodes;
 FFNode tmpN, nextN;
 Pix p;

 tmpN = startNode;
 succNodes |= startNode->succ ();

 while (! succNodes.empty ())
  {
   predNodes.clear ();
   predNodes |= succNodes;
   p = succNodes.first ();
   while (p != 0) 
    {
     nextN = findNextNode (succNodes, 1);
     nodeList.append (nextN);
     p = succNodes.first ();
    }  
   setInLevel (nodeList, 1, distance);
      
   succNodes.clear ();
   for (p =predNodes.first (); p != 0; predNodes.next (p))
    {
     tmpN = predNodes (p);
     succNodes |= tmpN->succ ();
    }  
  }
}

BOOL FFGraphClass::startNodeChoice (int choice)
// Funktionalitaet: Setzen eines Startknotens, liefert "YES", falls
//   Startknoten gesetzt und der gewaehlten Bedingung entspricht, sonst "NO"
// Vorbedingung: gerichterter Graph
// Nachbedingung: 
// Bemerkung:
//   choice == 
//   1: Knoten mit Indeg == 0 wird Startknoten;
//   2: Knoten mit Indeg == 0 wird Startknoten, falls schon Startknoten 
//      gesetzt, dann diesen nicht aendern (Test auf Indeg == 0);
//   3: beliebiger Knoten wird Startknoten;
//   4: Knoten, von dem aus alle anderen Knoten erreichbar sind, wird
//      Startknoten;
//   5: Knoten, von dem aus alle anderen Knoten erreichbar sind, wird
//      Startknoten, falls schon Startknoten gesetzt, dann diesen nicht 
//      aendern (Test, ob alle anderen Knoten erreichbar sind);
//   6: Knoten mit Indeg == 0 und: alle anderen Knoten sind von diesem Knoten
//      aus erreichbar, wird Startknoten
// ToDo: Verschiedene Wahlmoegl.
{
 Pix p;

 if (choice == 1)
  {
   for (p = knotenSet.first (); p != 0; knotenSet.next (p))
    if (knotenSet (p)->indeg () == 0)
     {
      setStartNode (knotenSet (p));
      return (YES);
     }
   return (NO);
  }

 if (choice == 2)
  {
   if (startNode != NULL)
    {
     if (startNode->indeg () == 0)
      return (YES);    
     else
      return (NO);
    }
   else 
    return (startNodeChoice (1));
  }

 if (choice == 3)
  {
   p = knotenSet.first ();
   if (p != NULL)
    {
     setStartNode (knotenSet (p));
     return (YES);
    }
   return (NO);
  } 

 if (choice == 4)
  {
   for (p = knotenSet.first (); p != 0; knotenSet.next (p))
    {
     startNode = knotenSet (p);
     if (reachAllNodes ())
      return (YES);
    }

   return (NO);
  }

 if (choice == 5)
  {
   if (startNode != NULL)
    {
     if (reachAllNodes ())
      return (YES);    
     else
      return (NO);
    }
   else 
    return (startNodeChoice (4));
  }

 if (choice == 6)
  {
   for (p = knotenSet.first (); p != 0; knotenSet.next (p))
    {
     startNode = knotenSet (p);
     if (reachAllNodes () && startNode->indeg () == 0)
      return (YES);
    }

   return (NO);
  }

 return (NO);
}

void FFGraphClass::copyCoord (FFGraph &graph, int)
// Funktionalitaet: Kopiert Knotenkoordinaten aus g in den Graphen 
// Vorbedingung: Knoten in this und graph haben zusammengehoerige 
//   Value-Werte (Knotenidentifizierung ueber Value-Wert)
// Nachbedingung: 
// Bemerkung:
// TODO: Kantenstuetzpunkte (ja/nein mittels choice)
{
 FFSet <FFNode> allNodes;
 FFSet <FFEdge> allEdges;
 Pix p;
 FFNode tmpN;
 //  FFEdge tmpE;
 FFPoint point;
 FFGraphicNode *gn, *gnOld;
 //  FFGraphicEdge *ge;
 FFGraphicEdge geOld;

 graph->allNodes (allNodes);
 for (p = knotenSet.first (); p != 0; knotenSet.next (p))
  {
   tmpN = graph->nodeWithValue (knotenSet (p)->getValue ());
   if (tmpN)
    {
     gnOld = (*tmpN) ();
     gn = (*knotenSet (p)) ();
     if (gn && gnOld)
      {
       point.v[0] = gnOld->getx ();
       point.v[1] = gnOld->gety ();
       point.v[2] = gnOld->getz ();
       gn->moveTo (point);
      }
    }
  }

 /*  graph->allEdges (allEdges);
     for (p = kantenSet.first (); p != 0; kantenSet.next (p))  
     {
     // tmpE  = graph->edgeWithValue (kantenSet (p)->getValue ());
     tmpE = allEdges (allEdges.first ());
     if (tmpE)
     {
     geOld = (*tmpE) ();
     ge = (*kantenSet (p)) ();
     if (ge && geOld)
     {
     if (geOld->anzPoints () > 2)
     for (i = 1; i < geOld->anzPoints () - 1; i++)
     {
     point.v[0] = geOld->getxAt (i);
     point.v[1] = geOld->getyAt (i);
     point.v[2] = geOld->getzAt (i);
     ge->addPointAt (i, point);
     }
     }
     }
     }  */
 cerr << "kopieren von Kantenstuetzpunkten noch nicht implementiert" << endl;

}

FFNode FFGraphClass::setNodeInEdge (FFEdge edge)
// Funktionalitaet: In eine Kante wird ein Knoten eingefuegt.
// Vorbedingung: existente Kante
// Nachbedingung: Beide Kantenteilstuecke haben danach den gleiche 
//   Wert in Value 
// Bemerkung: Wert von Value der Kante muss erhalten bleiben
//   (fuer removeNodeFromEdge)
{
 int val = edge->getValue ();
 FFNode node = addNode ();
 FFEdge tmpE;
  
 tmpE = addEdge (edge->source (), node);
 tmpE->setValue (val);
 tmpE = addEdge (node, edge->target ());
 tmpE->setValue (val);
  
 removeEdge (edge);
 return (node);
}
  
void FFGraphClass::removeNodeFromEdge (FFNode node)
// Funktionalitaet: Entfernt einen Knoten und setzt einen Kantenstuetzpunkt
//   in die resultierende Kante
// Vorbedingung: Knoten hat Indeg == Outdeg == 1,
//   Value von eingehender Kante == Value von ausgehender Kante
//   Wert der neuen Kante == Value der beiden alten Kanten
// Nachbedingung: 
// Bemerkung:
{
 FFSet <FFEdge> predEdges, succEdges;
 FFEdge predE, succE, newE;
 FFNode predN, succN;
 int eValue;
 Pix p;
  
 predEdges |= node->transFromPred ();
 succEdges |= node->transToSucc ();

 p = predEdges.first ();
 if (p != 0)
  {
   predE = predEdges (p);

   p = succEdges.first ();
   if (p != 0)
    {
     succE = succEdges (p);

     if (predE->getValue () == succE->getValue ())
      {
       eValue = predE->getValue ();
       predN = predE->source ();
       succN = succE->target ();

       removeNode (node);
       newE = addEdge (predN, succN);
       newE->setValue (eValue);
      }
    }
  }
}

void FFGraphClass::removeRefEdges ()
// Funktionalitaet: Entfernt alle reflexiven Kanten
// Vorbedingung: 
// Nachbedingung: 
// Bemerkung:
{
 Pix p;

 p = kantenSet.first ();

 while (p != 0)
  {
   if (kantenSet (p)->source () == kantenSet (p)->target ())
    {
     removeEdge (kantenSet (p));
     p = kantenSet.first ();
    }
   else
    kantenSet.next (p);
  }
}



void FFGraphClass::removeParEdges ()
// Funktionalitaet: Entfernt alle parallelen Kanten
// Vorbedingung: 
// Nachbedingung: 
// Bemerkung:
// Entfernt alle parallelen Kanten
{
 eliminateParallelEdges ();
}

void FFGraphClass::removeCycles (int)
// Funktionalitaet: Beseitigt Zyklen im gerichteten Graphen durch 
//   Umdrehen von Kanten, beseitigt reflexive Kanten durch 
//   vollstaendiges Entfernen der Kanten
// Vorbedingung: 
// Nachbedingung: u. U. enthaelt der Graph jetzt weniger Kanten,
//   Richtung von Kanten kann geaendert werden.
// Bemerkung:
//   choice == 
//   1: mittels Breitensuche
//   2:    "    Tiefensuche
//   Falls Startknoten gesetzt, so beginnt die Suche am Startknoten
{
 /*  SLList <FFNode> nodeList;
     FFSet <FFNode> visitNodes;
     FFSet <FFEdge> succEdges;
     Pix p;
     FFNode source;

     if (choice == 1)
     nodeList = this->bfs ();
     if (choice == 2)
     nodeList = this->dfs ();
    
     this->removeRefEdges ();

     source = nodeList.remove_front ();
     visitNodes.add (source);
    
     while (! nodeList.empty ())
     {
     source = nodeList.remove_front ();
     visitNodes.add (source);
     succEdges.clear ();
     succEdges |= source->transToSucc ();
    
     p = succEdges.first ();
     while (p != 0)
     {
     if (visitNodes.contains (succEdges (p)->target ()) && ...)
     succEdges (p)->revert ();
    
     succEdges.next (p);
     }
     } */
  
 cerr << "noch nicht implementiert" << endl;
}

void dfs1 (SLList  <FFNode> &dfsList, FFSet <FFNode> &visitN, FFNode node)
// Funktionalitaet:
// Vorbedingung: 
// Nachbedingung: 
// Bemerkung:
//   fuer Tiefensuche
{
 FFSet <FFNode> succN;

 visitN.add (node);
 dfsList.append (node);
 succN |= node->succ ();

 for (Pix p = succN.first (); p != 0; succN.next (p))
  if (! visitN.contains (succN (p)))
   dfs1 (dfsList, visitN, succN (p));
} 


SLList <FFNodeClass *> FFGraphClass::dfs ()
// Funktionalitaet: liefert Liste der Knoten des Graphen entsprechend 
//   der Tiefensuche, falls Startknoten gesetzt, dann vom Startknoten aus
// Vorbedingung: gerichteter Graph
// Nachbedingung: 
// Bemerkung:
{
 SLList <FFNode> dfsList;
 FFSet <FFNode> visitN, succN;
 Pix p;
 FFNode startN;

 if (knotenSet.length () > 0)
  {
   if (startNode != NULL)
    startN = startNode;
   else
    startN = knotenSet (knotenSet.first ());

   succN |= startN->succ ();
   p = succN.first ();
   if (p != 0)
    dfs1 (dfsList, visitN, succN (p));

   for (p = knotenSet.first (); p != 0; knotenSet.next (p))
    if (! visitN.contains (knotenSet (p)))
     dfs1 (dfsList, visitN, knotenSet (p));
  }

 return (dfsList);
}


SLList <FFNodeClass *> FFGraphClass::bfs ()
// Funktionalitaet: liefert Liste der Knoten des Graphen entsprechend 
//   der Breitensuche, falls Startknoten gesetzt, dann vom Startknoten aus
// Vorbedingung: 
// Nachbedingung: 
// Bemerkung:
{
 SLList <FFNode> bfsList, actNodeL;
 FFSet <FFNode> visitN, succN;
 Pix p;
 FFNode tmpN;

 if (knotenSet.length () > 0)
  {
   if (startNode != NULL)
    actNodeL.append (startNode);

   while (! actNodeL.empty ())
    {
     tmpN = actNodeL.remove_front ();
     bfsList.append (tmpN);
     visitN.add (tmpN);
     succN.clear ();
     succN |= tmpN->succ ();
     p = succN.first ();
     while (p != 0)
      {
       if (!visitN.contains (succN (p)))
        actNodeL.append (succN (p));
       succN.next (p); 
      }
    }

   for (p = knotenSet.first (); p != 0; knotenSet.next (p))
    if (! visitN.contains (knotenSet (p)))
     {
      actNodeL.append (knotenSet (p));

      while (! actNodeL.empty ())
       {
        tmpN = actNodeL.remove_front () ;
        bfsList.append (tmpN);
        visitN.add (tmpN);
        succN.clear ();
        succN |= tmpN->succ ();
        p = succN.first ();
        while (p != 0)
         {
          if (!visitN.contains (succN (p)))
           actNodeL.append (succN (p));
          succN.next (p);
         }
       }
     }
  }

 return (bfsList);
}

int ccw (FFPoint &p0, FFPoint &p1, FFPoint &p2) 
// Funktionalitaet: fuer Anzahl Ueberkreuzungen
// Vorbedingung: 
// Nachbedingung: 
// Bemerkung:
//   Fuer 2-D: Lage dreier Punkte zueinander 
//   return ==
//      0 : p2 auf Linie zw. p0 und p1
//      -1: p0 zw. p2 und p1
//       0: p2 zw. p0 und p1
//       1: p1 zw. p0 und p2 
{
 FFPoint d1, d2;
  
 d1.v[0] = p1.v[0] - p0.v[0];
 d1.v[1] = p1.v[1] - p0.v[1];
 d2.v[0] = p2.v[0] - p0.v[0];
 d2.v[1] = p2.v[1] - p0.v[1];

 if (d1.v[0] * d2.v[1] > d1.v[1] * d2.v[0])
  return (1);
 if (d1.v[0] * d2.v[1] < d1.v[1] * d2.v[0])
  return (-1);
 if ((d1.v[0] * d2.v[0] < 0) || (d1.v[1] * d2.v[1] < 0))
  return (-1);
 if ((d1.v[0] * d1.v[0] + d1.v[1] * d1.v[1]) < (d2.v[0] * d2.v[0] + d2.v[1] * d2.v[1]))
  return (1);
    
 return (0); 
}

int intersect (FFPoint &line1_p1, FFPoint &line1_p2, 
	       FFPoint &line2_p1, FFPoint line2_p2) 
// Funktionalitaet: liefert "1", falls sich die beiden Linien schneiden, 
//   sonst "0"
// Vorbedingung: Die  beiden Linien werden mit Start- und Endpunkt 
//   uebergeben. 
// Nachbedingung: 
// Bemerkung:
{
 return (((ccw (line1_p1, line1_p2, line2_p1) *
           ccw (line1_p1, line1_p2, line2_p2)) <= 0) &&
         ((ccw (line2_p1, line2_p2, line1_p1) *
           ccw (line2_p1, line2_p2, line1_p2)) <= 0));
}

int FFGraphClass::getNumCrossings ()
// Funktionalitaet: Liefert die Anzahl von Kantenkreuzungen.
// Vorbedingung: Kanten enthalten keine Stuetzpunkte, 
//   keine parallelen oder reflexiven Kanten.
// Nachbedingung: 
// Bemerkung:
{
 int anzC = 0;
 // FFSet <FFEdge> allEdges;
 Pix p, q;
 FFGraphicEdge *ge1, *ge2;

 // allEdges |= this->allEdges ();
 for (p = kantenSet.first (); p != 0; kantenSet.next (p))
  for (q = p; q != 0; kantenSet.next (q))
   if (kantenSet (p) != kantenSet (q))
    {
     ge1 = (*kantenSet (p)) ();
     ge2 = (*kantenSet (q)) ();
     if (ge1 && ge2)
      anzC = anzC + intersect (ge1->getPointAt (0), ge1->getPointAt (1),
                               ge2->getPointAt (0), ge2->getPointAt (1));
    }

 return (anzC);
}

