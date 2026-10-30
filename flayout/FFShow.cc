#include <string.h>

#include <ffgraph/FFShow.h>
#include <ffgraph/FFGraphClass.h>
#include <ffgraph/FFGraphWindow.h>
#include <ffgraph/FFLayoutLabel.h>
#include <ffgraph/FFTIEdgeLabel.h>
#include <ffgraph/FFTIGraphLabel.h>
#include <ffgraph/FFGraphicEdge.h>
#include <ffgraph/FFMatrix.h>

FFShow::FFShow ()
{
 _setType(ffShowType);
}


FFShow::~FFShow ()
{
}

void FFShow::init(const FFGraph& g)
{ 
 FFLayoutLabel::init(g);
}


void FFShow::layout ()
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


void FFShow::doLayout()
{
 const FFSet<FFNode>& tmpS= graph->allNodes();
 for(Pix i= tmpS.first(); i!= 0; tmpS.next(i))
  {
   FFNode tmpN= tmpS(i);
   FFPoint tmpP= tmpN->getGraphicNode()->getCoords();
   
   if(tmpN->getGraphicNode())
    cerr << tmpN->getName() << " : " << tmpP() << endl;
  }
}


