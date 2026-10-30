/*
// Copyright 1994 Carsten Friedrich, Universitaet Passau
//
// $Author: friedric $
// $Date: 1996/05/06 14:18:00 $
// $RCSfile: FFGraphicEdge.cc,v $
// $Revision: 1.2 $
// $State: Exp $
//
*/

#ifdef RCSID
#undef RCSID
#endif
#define RCSID "$Author: friedric $ $RCSfile: FFGraphicEdge.cc,v $ $Date: 1996/05/06 14:18:00 $ $Revision: 1.2 $ $State: Exp $"


extern "C"{
#include <math.h>
}

#include <stream.h>

#include <ffgraph/FFGraphClass.h>
#include <ffgraph/FFEdgeClass.h>
#include <ffgraph/FFNodeClass.h>
#include <ffgraph/FFGraph.h>
#include <ffgraph/ffError.h>
#include <ffgraph/FFGraphWindow.h>
#include <ffgraph/FFGraphicEdge.h>
#include <ffgraph/FFGraphicNode.h>


FFCoord FFGraphicEdge::dist(const FFPoint& p1, const FFPoint& p2)
{
  return (p1 - p2).abs();
}

FFCoord FFGraphicEdge::dist(const FFPoint& P, const FFPoint& A, const FFPoint& B)
{
  FFPoint R = B-A;
  FFCoord x =  - R*(A-P) / (R*R) ;
  
  if(x < 0.0 || x > 1.0)
    {
      return  HUGE_VAL;
    }
  
  return  dist(A + R * x, P);
}

void FFGraphicEdge::setColor(const String& s)
{
  farbe=s;
  
  if(fenster && tkEdge.length())
    {
      fenster->changeColor(this, farbe);
    }
  return;
}

String FFGraphicEdge::getColor()
{
 return farbe;
}

void FFGraphicEdge::adjustEdge()
{
 int i= edgePoints.length();
 if(i >= 2)
  {
   FFPoint ref1,ref2;
   
   if(i == 2)
    {
     ref1 = (*edge->target())()->getCoords();
     ref2 = (*edge->source())()->getCoords();
    }
   else
    {
     ref1 = edgePoints[1];
     ref2 = edgePoints[i-2];
    }
   
   if(fenster)
    {
     edgePoints[0] =fenster->adjust((*edge->source())(), ref1);
     edgePoints[i-1]=fenster->adjust((*edge->target())(), ref2);
     fenster->updateGraph(this);
     
    }
  }
 
 return;
}

int FFGraphicEdge::getPointNear(const FFPoint& tmpP)
{
 if(!fenster)
  return -1;
 
 int aktP = -1;    
 FFCoord minDist = 1000.0;
 
 //  cerr << "distance to " << tmpP() << endl;
 
 for(int i=edgePoints.low(); i< edgePoints.fence(); edgePoints.nextInt(i))
  {
   // cerr << "Point " << i << " dist: " << dist(tmpP, fenster->proj(edgePoints[i])) << endl;
   if(dist(tmpP, fenster->proj(edgePoints[i])) < minDist)
    {
     minDist= dist(tmpP, fenster->proj(edgePoints[i]));
     aktP=i;
    }
  }
 //	cerr << "Point " << aktP << " won" << endl
 return aktP;
}      

void FFGraphicEdge::setName(const String& l)
{
  if(fenster && tkLabel.length())   
    fenster->changeLabel(this, l);
}

void FFGraphicEdge::assign(const FFLabelClass *l)
{
 if(l->type() != type())
  return;
 FFGraphicEdge *nl = (FFGraphicEdge *)l;
 
 setColor(nl->getColor());
 edgePoints = nl->getEdgePoints();
}

void FFGraphicEdge::event(labelEvent e, labelEventStruct *theEvent)
{
	switch(e)
	{
		case NAME_CHANGE:
		{
			setName(edge->getName());
			break;
		}
		case SOURCE_CHANGE:
		{
			sourceChanged();
			break;
		}
		case TARGET_CHANGE:
		{
			targetChanged();
			break;
		}
		case REV_EDGE:
		{
			revert();
			break;
		}
      case ADD_GRAPHLABEL:
      {
        if(!fenster)
        {
          fenster = edge->getGraph()->getWindow();

          if(fenster)
          { 
            adjustEdge();
            tkEdge = fenster->addEdge(this);
            tkLabel = fenster->addLabel(this);
          }
        }
        break;
      }
      case DEL_GRAPHLABEL:
      {
        if(fenster && 
          (theEvent->index == edge->getGraph()->getGraphWindowIndex()))
        {
          fenster->remove(this);
          fenster = 0;

          tkEdge = "";
          tkLabel = "";
        }
        break;
      }
		default:
			break;
	}
}

void FFGraphicEdge::revert()
{
  edgePoints.reverse();
  
  if(fenster)
    updateGraphic();
  return;
}

void FFGraphicEdge::setWindow(FFGraphWindow *i)
{
  if((source == NULL) || (target == NULL))
    {
      ffError(RUNTIME_WARNING, NO_GRAPHIK_CONTEXT);
      return;
    }
  
  if(fenster == i)
    return;
  
  if(fenster != NULL)
    fenster->remove(this);      
  
  fenster=i;
  
	if(fenster)
	{ 
	  tkEdge = fenster->addEdge(this);
		tkLabel = fenster->addLabel(this);
  }
  else
  {	
		tkEdge="";
		tkLabel="";
	}
  return;
}

void FFGraphicEdge::targetChanged()
{
 if(edgePoints.length() >= 2 )
  {
   edgePoints[edgePoints.high()] = (*edge->target())()->getCoords();
   adjustEdge();
   
   if(fenster)
    {
     fenster->updateGraph(this);
    }
  }
 return;
}

void FFGraphicEdge::sourceChanged()
{
 if(edgePoints.length() >= 2 )
  {
   edgePoints[0] = (*edge->source())()->getCoords();
   adjustEdge();
   
   if(fenster)
    {
     fenster->updateGraph(this);
    }
  }
 return;
}

void FFGraphicEdge::updateGraphic()
{
  if(fenster != NULL)
    fenster->updateGraph(this);
      
  return;
}

  
BOOL FFGraphicEdge::addPointAt(int pos, const FFPoint& tmpP)
{  
  int i;
  int next=0;  
  FFCoord minDist= HUGE_VAL;
  
  if(pos != -1)
   {
    next=pos;
   }
  else
   {
    for(i= 0; i< edgePoints.length() -1 ; i++)
     {
#ifdef DEBUG
      // cerr << "Distance is :" << 
      // dist(tmpP, edgePoints[i], edgePoints[i+1]) << "\n";
#endif
      if(dist(tmpP, edgePoints[i], edgePoints[i+1]) < minDist)
       {
        minDist= dist(tmpP, edgePoints[i], edgePoints[i+1]);
        next=i+1;
       }
     }
   }
  
  int zahl=next;
  
  edgePoints.add(FFPoint());
  
  for(i= edgePoints.length()-2; i>= zahl; i--)
   {
    edgePoints[i+1]= edgePoints[i];
   }
  edgePoints[zahl]= tmpP;
  
  if(pos != 0)             // kein adjust beim laden eines graphen
   adjustEdge();	
  return YES;	  
}

BOOL FFGraphicEdge::deletePointAt(const FFPoint& tmpP)
{
 if(edgePoints.length() > 2)
  {
   int next =1;
   
   FFCoord minDist= dist(tmpP, edgePoints[0]);
   
   for(int i=2; i < edgePoints.length(); i++)
    {
     if(dist(tmpP, edgePoints[i]) < minDist)
      {
       minDist= dist(tmpP,edgePoints[i]);
       next=i;
      }
    }
   return deletePointAt(next);
  }
 return NO;
}
                    
BOOL FFGraphicEdge::deletePointAt(int p)
{
  int i;
  //  cerr << "Deleting point at " << p << endl;
  
  if(!edgePoints.valid(p))
    return NO;

  if((edgePoints.length() <= 3) && spline)
    toggleSpline();
  
  for(i=p; i< edgePoints.length()-1; i++)
    {
      edgePoints[i]= edgePoints[i+1];
    }
  
	edgePoints.del_high();
  if(p != edgePoints.length() -2)     // kein adjust beim Laden eines Graphen
	  adjustEdge();
  
  return YES;
}

BOOL FFGraphicEdge::modifyPointAt(int p, FFPoint& tmpP )
{
  if(!edgePoints.valid(p))
    return NO;
  
  edgePoints[p] = tmpP;
  adjustEdge();
  
  return YES;
}

String FFGraphicEdge::getId()
{
  return tkEdge;
}

String FFGraphicEdge::getLabelId()
{
  return tkLabel;
}

FFGraphicEdge::FFGraphicEdge() : farbe("black") 
{
  fenster= 0;
  spline= NO;
  source = target = 0;
  _setType(ffEdgeWindowType);
}

void FFGraphicEdge::init(FFEdgeClass *e)
{
  FFEdgeLabel::init(e);

  target = (*edge->target())();
  source = (*edge->source())();
	
	if((target == NULL) || (source == NULL))
	{
		target=source= 0;
		return;
	}
		
	fenster = edge->getGraph()->getWindow();
	if(edge->getGraph()->getGraphicEdgeIndex() <= 0)
	edge->getGraph()->setEdgeIndex(
			edge->getGraph()->getEdgeLabelIndex(type()));
	

  if(edgePoints.length() < 2)
  {
	  edgePoints.add(source->getCoords());
	  edgePoints.add(target->getCoords());
	}
	else
	{
		edgePoints[0]=	source->getCoords();
		edgePoints[1]=	target->getCoords();
	}
  adjustEdge();
	
	if(fenster)
	{ 
	  tkEdge = fenster->addEdge(this);
		tkLabel = fenster->addLabel(this);
  }
  return;  
}

void FFGraphicEdge::toggleSpline()
{
  if(spline)
    spline = NO;
  else
    spline = YES;
  
  if((edgePoints.length() > 2) && tkEdge.length() && fenster)
    {
      fenster->setSpline(this, spline);
    }
  return;
      
}

void FFGraphicEdge::rmoveTo(const FFPoint& tmpP)
{
  for(int i=1; i< edgePoints.length()-1; i++)
    {
      edgePoints[i] += tmpP;
    }
  adjustEdge();
  return;
}

void FFGraphicEdge::rmovePoint(int p, const FFPoint& tmpP)
{
  if(edgePoints.valid(p))
    {
      edgePoints[p] += tmpP;
      adjustEdge();
    }
  return;
}

void FFGraphicEdge::movePoint(int p, const FFPoint& tmpP)
{
  if(edgePoints.valid(p))
    {
      edgePoints[p] = tmpP;
      adjustEdge();
    }
  return;
}

FFGraphicEdge::~FFGraphicEdge()
{
	if(fenster)
		fenster->remove(this);
}


