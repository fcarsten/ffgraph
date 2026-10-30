/*
// Copyright 1994 Carsten Friedrich, Universitaet Passau
//
// $Author: deepthou $
// $Date: 1996/07/04 07:44:51 $
// $RCSfile: FFEdgeClass.cc,v $
// $Revision: 1.6 $
// $State: Exp $
//
*/

#ifdef RCSID
#undef RCSID
#endif
#define RCSID "$Author: deepthou $ $RCSfile: FFEdgeClass.cc,v $ $Date: 1996/07/04 07:44:51 $ $Revision: 1.6 $ $State: Exp $"


extern "C"{
// #include <string.h>
#include <math.h>
#include <malloc.h>
}

#include <stream.h>

#include <ffgraph/FFGraphClass.h>
#include <ffgraph/FFEdgeClass.h>
#include <ffgraph/FFNodeClass.h>
#include <ffgraph/FFGraph.h>
#include <ffgraph/FFNodeClass.h>
#include <ffgraph/FFEdgeClass.h>
#include <ffgraph/ffError.h>
#include <ffgraph/FFEdgeLabel.h>
#include <ffgraph/FFGraphicEdge.h>

#ifdef FFGRAPHIKEDGE
#undef FFGRAPHIKEDGE		
#endif

#define FFGRAPHIKEDGE ((FFGraphicEdge *)((*label)[derGraph->getGraphicEdgeIndex()]))

FFEdge FFEdgeClass::copy()
{
  FFEdge n= 0;
  FFEdge tmpN;
  
  FFGraph g= derGraph->copy();
  const FFSet<FFEdge>& l = g->allEdges(); // ???? erst setValue() ????
  
  for(Pix i = l.first(); i!= 0; l.next(i))
   {
    tmpN=l(i);
    if (value == tmpN->getValue())
     {
      n = tmpN;
     }
   }
  
  return n;
}

void FFEdgeClass::setValue(int i)
{
  value=i;
  return;
}

void FFEdgeClass::setName(const String& s)
{
	name =s;
	labelEventStruct theEvent;
	theEvent.edge=this;
	_sendEvent(NAME_CHANGE, &theEvent);
	
//	if(derGraph->getGraphicEdgeIndex() >= 0)
//		FFGRAPHIKEDGE->setName(name);
}
  
int FFEdgeClass::getValue()
{
  return value;
}

void FFEdgeClass::copyDataFrom(FFEdgeClass *n, BOOL bare)
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
 setWeight(n->getWeight());  
 setName(n->getName());
}

void FFEdgeClass::_sendEvent(labelEvent e, labelEventStruct *s)
{
 if((label != NULL) && (derGraph->getEventStatus() == EVENT_NORMAL))
  {
   for(Pix i=label->first(); i!=0; label->next(i))
    {
     (*label)(i)->event(e, s);
    }
  }
}

void FFEdgeClass::addLabel(const FFEdgeLabel *l)
{
 if(label == NULL)
  label= new MPlex<FFEdgeLabel *>;
 
 FFEdgeLabel *l2=l->proto();
 int tmpI=label->add(l2);
 l2->index=tmpI;
 l2->init(this);
}

void FFEdgeClass::delLabel(int k, BOOL doFree)
{
 if(label != NULL)
  {
   if(doFree)
    delete (*label)[k];
   label->del_index(k);
  }
}


void FFEdgeClass::assignLabel(const MPlex<FFEdgeLabel *>&l)
{
 int k=l.length();
 if(k ==0)
  return;
 
 if(l[l.low()]->edge->derGraph == derGraph)
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

void FFEdgeClass::setLabel(const MPlex<FFEdgeLabel *>&l, BOOL doFree)
{
 int k=l.length();
 if(k ==0)
  return;
 
 if(l[l.low()]->edge->derGraph == derGraph)
  {
   for(int j=label->low(); j<label->fence(); label->nextInt(j))
    {
     if(doFree)
      delete (*label)[j];
     l[j]->index=j;
     (*label)[j] = l[j];
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

void FFEdgeClass::assignLabel(FFEdgeLabel *l, int i)
{
 if((label != NULL) && (i >= 0))
  {
   (*label)[i]->assign(l);
  }
 else
  {
   int k= getLabelIndex(l->type());
   if(k <0)
    k= derGraph->addEdgeLabel(l->proto());
   
   (*label)[k]->assign(l);
  }
}

void FFEdgeClass::setLabel(FFEdgeLabel *l, int i, BOOL doFree)
{
 if((label != NULL) && (i >= 0))
  {
   if(doFree)
    delete (*label)[i];
   l->index=i;
   (*label)[i] = l;
  }
 else
  {
   int k= getLabelIndex(l->type());
   if(k <0)
    k= derGraph->addEdgeLabel(l);
   
   if(doFree)
    delete (*label)[k];
   l->index=k;
   (*label)[k] = l;
  }
}

FFNode FFEdgeClass::source()
{
  return sourceNode;
}

FFNode FFEdgeClass::target()
{
  return destNode;
}

int FFEdgeClass::getLabelIndex(const String& s, FFSet<int> *set) const
{
 return derGraph->getEdgeLabelIndex(s, set);
}

void FFEdgeClass::setTarget(FFNodeClass *n)
{
	if(!derGraph->contains(n))
		return;
	
	FFSet<FFEdge>& rFrom1= (FFSet<FFEdge>&) destNode->transFromPred();
	FFSet<FFEdge>& rFrom2= (FFSet<FFEdge>&) n->transFromPred();
	labelEventStruct theEvent;
	theEvent.edge= this;
	
	rFrom1.del(this);
	rFrom2.add(this);
	destNode = n;
	_sendEvent(TARGET_CHANGE, &theEvent);
}

void FFEdgeClass::setSource(FFNodeClass *n)
{
 if(!derGraph->contains(n))
  return;
 
 FFSet<FFEdge>& rFrom1= (FFSet<FFEdge>&) sourceNode->transToSucc();
 FFSet<FFEdge>& rFrom2= (FFSet<FFEdge>&) n->transToSucc();
 labelEventStruct theEvent;
 theEvent.edge= this;
 
 rFrom1.del(this);
 rFrom2.add(this);
 sourceNode = n;
 _sendEvent(SOURCE_CHANGE, &theEvent);
}

void FFEdgeClass::revert()
{
 FFSet<FFEdge>& rTo = (FFSet<FFEdge>&) sourceNode->transToSucc();
 FFSet<FFEdge>& rFrom = (FFSet<FFEdge>&) destNode->transFromPred();
 
 labelEventStruct theEvent;
 theEvent.edge = this;
 
 rTo.del(this);
 rFrom.del(this);
 
 FFNode tmp= sourceNode;
 sourceNode=destNode;
 destNode = tmp;
 
 ((FFSet<FFEdge>&)sourceNode->transToSucc()).add(this);
 ((FFSet<FFEdge>&)destNode->transFromPred()).add(this);
 
 _sendEvent(REV_EDGE, &theEvent);
}

void FFEdgeClass::init(FFGraphClass *g)
{
 derGraph=g;
 if(label != NULL)
  {
   for(Pix i=label->first(); i!=0; label->next(i))
    {
     (*label)(i)->init(this);
    }
  }
}

FFEdgeClass::FFEdgeClass(MPlex<FFEdgeLabel *>*l, FFNodeClass *s, 
                         FFNodeClass *t) 
{
 value = 0;
 weight =1.0;
 sourceNode= s;
 destNode= t;
 label = 0;
 if(l != NULL)
  {
   label = new MPlex<FFEdgeLabel *>((*l));
   
   for(int i=label->low(); i < label->fence(); label->nextInt(i))
    {
     FFEdgeLabel *tmpL= (*label)[i]->proto();
     tmpL->index=i;
     (*label)[i]= tmpL;
    }
  }
}

FFEdgeClass::~FFEdgeClass()
{
 if(label != NULL)
  {	
   int i;
   
   while(( i=label->low()) < label->fence())
    {
     delLabel(i, YES);
    }
   //    for(Pix i=label->first(); i!=0; label->next(i))
   //     {
   //      delete (*label)(i);
   //     }
   delete label;
  }
}

void FFEdgeClass::show()
{

   int i = getLabelIndex("MCEdgeLabel");
   int j = getLabelIndex("SNIEdgeLabel");
   cerr << "\n\n\t[" << source()->getName() << "]";
   cerr << "--("
	<< getName();

   if (i!=-1) cerr << "/ MC: " << (*label)[i]->unparse();
   if (j!=-1) cerr << "/ SNI: " << (*label)[j]->unparse();

   cerr << ")->";
   cerr << "[" << target()->getName() << "]" << endl;
   return;
}

void FFEdgeClass::showGraphic()
{
	if((derGraph->getGraphicEdgeIndex() < 0) ||
		 ((*sourceNode)() == NULL) ||
		 ((*destNode)() == NULL))
		return;

	FFGRAPHIKEDGE->setWindow(derGraph->getWindow());
}
 
void FFEdgeClass::hideGraphic()
{
  if(derGraph->getGraphicEdgeIndex() >= 0)
	{
		FFGRAPHIKEDGE->setWindow(NULL);
	}
  return;  
}
			
