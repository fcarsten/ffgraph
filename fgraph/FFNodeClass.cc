/*
  // Copyright 1994 Carsten Friedrich, Universitaet Passau
  //
  // $Author: deepthou $
  // $Date: 1996/07/04 07:44:51 $
  // $RCSfile: FFNodeClass.cc,v $
  // $Revision: 1.5 $
  // $State: Exp $
  //
*/

#ifdef RCSID
#undef RCSID
#endif
#define RCSID "$Author: deepthou $ $RCSfile: FFNodeClass.cc,v $ $Date: 1996/07/04 07:44:51 $ $Revision: 1.5 $ $State: Exp $"


extern "C"{
#include <stdlib.h>
 // #include "string.h"
#include "malloc.h"
}

#include <stream.h>
#include <ffgraph/FFGraphClass.h>
#include <ffgraph/FFNodeClass.h>
#include <ffgraph/FFGraph.h>
#include <ffgraph/FFEdgeClass.h>
#include <ffgraph/FFNodeLabel.h>
#include <ffgraph/ffError.h>
#include <ffgraph/FFGraphicNode.h>

#include <container/VOHSet.h>
#include <container/AVLSet.h>

#ifdef FFGRAPHIKNODE
#undef FFGRAPHIKNODE		
#endif

#define FFGRAPHIKNODE ((FFGraphicNode *)((*label)[derGraph->getGraphicNodeIndex()]))

FFNode FFNodeClass::copy()
{
 FFGraphClass *g= derGraph->copy();
 const FFSet<FFNode>& gs = g->allNodes();
  
 for(Pix i=gs.first(); i!=0; gs.next(i))
  {
   if (value == gs(i)->getValue())
    {
     return gs(i);
    }
  }
 return NULL;
}
  
void FFNodeClass::copyDataFrom(FFNodeClass *n, BOOL bare)
{
 if((bare != NO) && (n->label != NULL))
  {
   for(int i=n->label->low(); i<n->label->fence(); n->label->nextInt(i))
    {
     if((*n->label)[i]->shouldCopy())
      assignLabel((*n->label)[i]);
    }
  }
	
 setValue(n->getValue());
 setCoord(n->getCoord());
 setAttribut(n->getAttribut());	
 setName(n->getName());
}

void FFNodeClass::_sendEvent(labelEvent e, labelEventStruct *s)
{
 if((label != NULL) && (derGraph->getEventStatus() == EVENT_NORMAL))
  {	
   for(Pix i=label->first(); i!=0; label->next(i))
    {
     (*label)(i)->event(e, s);
    }
  }
}

void FFNodeClass::init(FFGraphClass *g)
{
 derGraph =g;
 if(label != NULL)
  {
   for(Pix i=label->first(); i!=0; label->next(i))
    {
     (*label)(i)->init(this);
    }
  }
}

FFNodeClass::FFNodeClass(MPlex<FFNodeLabel *> *l)
{
 value= -1;
 coordValue= 0.0;
 hiddenFlag = NO;
 
 attribut= 0;
 label = 0;
 
 if(l != NULL)
  {
   label = new MPlex<FFNodeLabel *>(*l);
   
   for(int i=label->low(); i<label->fence(); label->nextInt(i))
    {
     FFNodeLabel *tmpL=(*label)[i]->proto();
     tmpL->index=i;
     (*label)[i]=tmpL;
    }
  }
}

FFNodeClass::~FFNodeClass()
{	
 if(label != NULL)
  {
   //   for(int i=label->low(); i<label->fence(); label->next(i))
   int i;
   
   while(( i=label->low()) < label->fence())
    {
     delLabel(i, YES);
    }
   delete label;
  }
}
 
void FFNodeClass::addLabel(const FFNodeLabel *l) 
{	
 if(label == NULL)
  label = new MPlex<FFNodeLabel *>;
		
 FFNodeLabel *l2=l->proto();
 int tmpI=label->add(l2);
 l2->index=tmpI;
 l2->init(this);
}

void FFNodeClass::delLabel(int k, BOOL doFree)
{
 if(label != NULL)
  {
   if(doFree)
    delete (*label)[k];
   label->del_index(k);
  }
}

int FFNodeClass::getLabelIndex(const String& s, FFSet<int>*set) const
{
 return derGraph->getNodeLabelIndex(s, set);
}

void FFNodeClass::assignLabel(FFNodeLabel *l, int i)
{
 if((label != NULL) && (i >= 0))
  {
   (*label)[i]->assign(l);
  }
 else
  {
   int k= getLabelIndex(l->type());
   if(k <0)
    k= derGraph->addNodeLabel(l->proto());

   (*label)[k]->assign(l);
  }
}

void FFNodeClass::setLabel(const MPlex<FFNodeLabel *>&l, BOOL doFree)
{
 if(l.length() ==0)
  return;
 if(l[l.low()]->node->derGraph == derGraph)
  {
   for(int j=label->low(); j<label->fence(); label->nextInt(j))
    {
     if(doFree)
      delete (*label)[j];
     
     (*label)[j] = l[j];
     (*label)[j]->index = j;
    }		
  }
 else
  {
   for(Pix i=l.first(); i!=0; l.next(i))
    {
     setLabel(l(i), doFree);
    }
  }
}

void FFNodeClass::setLabel(FFNodeLabel *l, int i, BOOL doFree)
{
 if((label != NULL) && (i >= 0))
  {
   if(doFree)
    delete (*label)[i];
   l->index=i;
   (*label)[i]=l;
  }
 else
  {
   int k= getLabelIndex(l->type());
   if(k <0)
    k= derGraph->addNodeLabel(l);

   if(doFree)
    delete (*label)[k];
   l->index=k;
   (*label)[k]=l;
  }
}

void FFNodeClass::assignLabel(const MPlex<FFNodeLabel *>&l)
{
 if(l.length() ==0)
  return;

 if(l[l.low()]->node->derGraph == derGraph)
  {
   for(int j=label->low(); j<label->fence(); label->nextInt(j))
    {
     (*label)[j]->assign(l[j]);
    }		
  }
 else
  {
   for(Pix i=l.first(); i!=0; l.next(i))
    {
     assignLabel(l(i));
    }
  }
}



void FFNodeClass::setName(const String& l)
{
 name= l;
    
 labelEventStruct theEvent;
 theEvent.node=this;
	
 _sendEvent(NAME_CHANGE, &theEvent);
 return;
}

FFSet<FFNode> FFNodeClass::pred()
{
 FFSet<FFNode> l;
 for(Pix i=transFrom.first(); i !=0; transFrom.next(i))
  l.add(transFrom(i)->source());
  
 return l;
}
  
FFSet<FFNode> FFNodeClass::succ()
{
 FFSet<FFNode> l;
 for(Pix i=transTo.first(); i!=0; transTo.next(i))
  l.add(transTo(i)->target());
  
 return l;
}

void FFNodeClass::pred(FFSet<FFNode> &tmpS)
{
 tmpS.clear();
 for(Pix i=transFrom.first(); i!=0; transFrom.next(i))
  tmpS.add(transFrom(i)->source());
  
 return;
}

void FFNodeClass::succ(FFSet<FFNode> &tmpS)
{
 tmpS.clear();
 for(Pix i=transTo.first(); i!=0; transTo.next(i))
  tmpS.add(transTo(i)->target());
  
 return;
}

void FFNodeClass::show()
{
 cerr << "[" << getName() << "] with labels: ";
 if(label != NULL)
  {
   for (Pix ind = label->first(); ind !=0; label->next(ind)) {
    cerr << " " << (*label)(ind)->type() << " ";
   }
  }
 int i = getLabelIndex("MCNodeLabel"); 
 int j = getLabelIndex("SNINodeLabel");
 cerr << " Labels: [" << getName() << "/ MC:";
 if (i!=-1) cerr << (*label)[i]->unparse() << " , SNI: ";
 if (j!=-1) cerr << (*label)[j]->unparse() << "]";
 else cerr<< " ]";
}

void FFNodeClass::eliminateParallelEdges()
{
 FFSet<FFNode> testSet;
 FFSet<FFEdge> loeschSet;
 FFNode tmpN;
 FFEdge tmpE;
 Pix i;
  
 for(i=transTo.first(); i!=0; transTo.next(i))
  {
   tmpE=transTo(i);
   tmpN = tmpE->target();
      
   if(testSet.contains(tmpN) )
    {
     loeschSet.add(tmpE);
    }
   else
    {
     testSet.add(tmpN);
    }
  }
  
 if(!derGraph->getDirected())
  {
   for(i=transFrom.first(); i!= 0; transFrom.next(i))
    {
     tmpE=transFrom(i);
     tmpN=tmpE->source();
     if(testSet.contains(tmpN) )
      {
       loeschSet.add(tmpE);
      }
     else
      {
       testSet.add(tmpN);
      }
    }			
  }
  
 for(i= loeschSet.first(); i!=0; loeschSet.next(i))
  {
   derGraph->removeEdge(loeschSet(i));
  }
 testSet.clear();
  
 return;
}

void FFNodeClass::showGraphic()
{
 hiddenFlag = NO;
 
 if(derGraph->getGraphicNodeIndex() < 0)
  return;
  
 FFGRAPHIKNODE->setWindow(derGraph->getWindow());  
}

void FFNodeClass::hideGraphic()
{
 hiddenFlag = YES;
 
 if(derGraph->getGraphicNodeIndex() >= 0)
  {
   FFGRAPHIKNODE->setWindow(NULL);
  }
 return;
}
