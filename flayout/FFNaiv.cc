#include <string.h>

#include <ffgraph/FFNaiv.h>
#include <ffgraph/FFGraphClass.h>
#include <ffgraph/FFGraphWindow.h>
#include <ffgraph/FFLayoutLabel.h>
#include <ffgraph/FFTIEdgeLabel.h>
#include <ffgraph/FFNaivInsp.h>
#include <ffgraph/FFTIGraphLabel.h>
#include <container/VOHSet.h>
#include <container/AVLSet.h>


FFNaiv::FFNaiv ()
{
 xOffSet = NAIV_X_OFFSET;
 yOffSet = NAIV_Y_OFFSET;

 _setType(ffNaivType);
}


FFNaiv::~FFNaiv ()
{
 tiIndex=-1;
}

void FFNaiv::init(const FFGraph& g)
{ 
 FFLayoutLabel::init(g);
 tiIndex= graph->getEdgeLabelIndex(ffTIEdgeLabel);
 inspector= new FFNaivInsp(this);
}

FFCoord FFNaiv::_layout (FFNode node)
{
 aktuX += xOffSet;
 int count =0;
//  FFCoord tmpX= aktuY;
 FFCoord tmpY= aktuX;
 FFCoord minX= aktuY;
 FFCoord maxX= aktuY;
 FFCoord tmpCoord= aktuY;
 const FFSet<FFEdge>& tmpES = node->transToSucc();

//  strstream tmpS;
//  tmpS << aktuY << " : ";
 
 for(Pix i= tmpES.first(); i != 0; tmpES.next(i))
  {
   FFTIEdgeLabel *el = (FFTIEdgeLabel *)tmpES(i)->getLabel(tiIndex);
   if(el->getTiType() == TI_TREE)
    {
     count ++;
     tmpCoord = _layout(tmpES(i)->target());
     if(count == 1)
      {
       minX = tmpCoord;
       maxX = tmpCoord;
      }
     else if(tmpCoord < minX)
      minX= tmpCoord;
     else if(tmpCoord > maxX)
      maxX= tmpCoord;
     
    }
//    else
//     tmpES(i)->hideGraphic();

  }

 // Sonderfall: Wir haben nur einen Nachfolger, also zentrieren ber 
 // diesem Nachfolger

 //  if(count == 1)
 //   {
 //    node->getGraphicNode()->setColor("blue");   
 //   }
 
 //  tmpS << minX << " : " << maxX << ends;
 
 node->getGraphicNode()->moveTo(FFPoint((minX + maxX)/2, tmpY));
 //  node->setName(tmpS.str());
 
 if(count == 0)
  aktuY += yOffSet;
 aktuX -= xOffSet;
 return (minX+maxX)/2;
}

void FFNaiv::layout ()
{
 if( (tiIndex == -1) &&
     (tiIndex= graph->getEdgeLabelIndex(ffTIEdgeLabel)) == -1)
   {
    graph->addGraphLabel(new FFTIGraphLabel);
    if((tiIndex= graph->getEdgeLabelIndex(ffTIEdgeLabel)) == -1)
     return;
   }

 FFNode tmpN=graph->getStartNode();
 aktuX = 0.0;
 aktuY = 0.0;
 FFCoord maxX, minX;
 FFCoord tmpX=0.0;
 int count=0;
 
 maxX= 0.0;
 minX= 0.0;
 BOOL tmpUpdate=YES;
 
 if (graph->getWindow ())
  {
   graph->getWindow()->setBusy ();
   tmpUpdate = graph->getWindow()->setUpdate(NO);
  }
 
 if(tmpN != NULL)
  {
   const FFSet<FFEdge>& tmpES = tmpN->transToSucc();
   for(Pix i= tmpES.first(); i != 0; tmpES.next(i))
    {
     FFTIEdgeLabel *el = (FFTIEdgeLabel *)tmpES(i)->getLabel(tiIndex);
     if(el->getTiType() == TI_TREE)
      {
       count ++;
       tmpX=_layout(tmpES(i)->target());
       if(tmpX < minX)
        minX = tmpX;
       else if(tmpX > maxX)
        maxX = tmpX;
      }
     // else
     // tmpES(i)->hideGraphic();
      
    }

   // Sonderfall: Wir haben nur einen Nachfolger, also zentrieren ber 
   // diesen Nachfolger
   if(count == 1)
    {
     minX = tmpX;
     maxX = tmpX;
    }
   
   tmpN->getGraphicNode()->moveTo(FFPoint((minX + maxX)/2, 0));
  }
 if (graph->getWindow ())
  {
   graph->getWindow()->unsetBusy ();
   graph->getWindow()->setUpdate(tmpUpdate);
   graph->getWindow()->adjustWindowSize();
  }
}

void FFNaiv::event(labelEvent event, labelEventStruct *)
{
 switch(event)
  {
  case END_LOAD:
   {
    if(graph->getStartNode() == NULL)
     {
      // cerr << "OK, we still don't have a startnode, "
      // << "let's look for one" << endl;
      
      const FFSet<FFNode>& tmpS= graph->allNodes();
      for(Pix i= tmpS.first(); i != 0; tmpS.next(i))
       if(tmpS(i)->indeg() == 0)
        {
         graph->setStartNode(tmpS(i));
         // cerr << "Hey, found one!"<< endl;
         
         break;
        }
     }
    break;
   }
  default:
   {
   }
  }
}
