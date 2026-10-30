#include <string.h>

#include <ffgraph/FFRandom.h>
#include <ffgraph/FFGraphClass.h>
#include <ffgraph/FFGraphWindow.h>
#include <ffgraph/FFLayoutLabel.h>
#include <ffgraph/FFTIEdgeLabel.h>
#include <ffgraph/FFTIGraphLabel.h>
#include <ffgraph/FFGraphicEdge.h>
#include <ffgraph/FFMatrix.h>

FFRandom::FFRandom ()
{
 _setType(ffRandomType);
}


FFRandom::~FFRandom ()
{
}

void FFRandom::init(const FFGraph& g)
{ 
 FFLayoutLabel::init(g);
}


void FFRandom::layout ()
{
 BOOL tmpUpdate=YES;
 if (graph->getWindow ())
  {
   graph->getWindow()->setBusy ();
   tmpUpdate = graph->getWindow()->setUpdate(NO);
  }
 
 doLayout();
 
 if (graph->getWindow ())
  {
   graph->getWindow()->unsetBusy ();
   graph->getWindow()->setUpdate(tmpUpdate);
   graph->getWindow()->adjustWindowSize();
  }
 cerr << "Ready" << endl;
}


void FFRandom::doLayout()
{
 eventStatus tmpStatus= graph->getEventStatus();
 graph->setEventStatus(EVENT_DISABLED);
 
 const FFSet<FFNode>& tmpS= graph->allNodes();
 for(Pix i= tmpS.first(); i!= 0; tmpS.next(i))
  {
   FFNode tmpN= tmpS(i);
   if(tmpN->getGraphicNode())
    tmpN->getGraphicNode()->moveTo(FFPoint(lrand48() % 400, lrand48() % 300));
  }
 
 if(graph->getWindow())
  graph->getWindow()->update();

 graph->setEventStatus(tmpStatus);
}


