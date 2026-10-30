#include <ffgraph/FFSpring.h>
#include <ffgraph/FFGraphClass.h>
#include <ffgraph/FFGraphWindow.h>
#include <ffgraph/FFLayoutLabel.h>
#include <ffgraph/FFSpringInsp.h>

#include <container/VOHSet.h>
#include <container/AVLSet.h>

FFSpring::FFSpring ()
{
  _setType(ffSpringType);
  animation           = NO;   // Animation waehrend der Berechnung
  useEdgeWeight       = YES;  // Beruecksichtigung der Kantengewichte
  maxIteration        = 1000; 
  animationIntervals  = 10;   // fuer Animation
  optDistance         = 100;
  offset.v[0]         = 100;
  offset.v[1]         = 100; 
  offset.v[2]         = 100;
  quality             = 2;
  stopBit             = NO;
  dimension           = 2;    // Berechnung fuer 2D oder 3D

  nodes = 0;
}


FFSpring::~FFSpring ()
{
}

void FFSpring::init(const FFGraph& g)
{ 
  FFLayoutLabel::init(g);
  inspector= new FFSpringInsp(this);
}

//BOOL FFSpring::setGraph (FFGraph &g)
//{
//  graph = g;
//  return (YES);
//}


BOOL FFSpring::getAnimation ()
{
  return (animation);
}


void FFSpring::setAnimation (BOOL value)
{
  animation = value;
}


BOOL FFSpring::getUseEdgeWeight ()
{
  return (useEdgeWeight);
}


void FFSpring::setUseEdgeWeight (BOOL value)
{
  useEdgeWeight = value;
}


int FFSpring::getMaxIteration ()
{
  return (maxIteration);
}


BOOL FFSpring::setMaxIteration (int value)
{
  if (value <= 0)
    return (NO);
  else
    {
      maxIteration = value;
      return (YES);
    }
}


int FFSpring::getAnimationIntervals ()
{
  return (animationIntervals);
}


BOOL FFSpring::setAnimationIntervals (int value)
{
  if ((value <= 0) || (value > maxIteration))
    return (NO);
  else
    {
      animationIntervals = value;
      return (YES);
    }
}


int FFSpring::getQuality ()
{
  return (quality);
}

void FFSpring::stop()
{
 // cerr << "Stopping as well ...." << endl;
 
 stopBit = YES;
}

BOOL FFSpring::setQuality (int value)
{
  if ((value < 0) || (value > 10))
    return (NO);
  else
    {
      quality = value;
      return (YES);
    }
}


FFCoord FFSpring::getOptDistance ()
{
  return (optDistance);
}


BOOL FFSpring::setOptDistance (FFCoord value)
{
  if (value <= 0)
    return (NO);
  else
    {
      optDistance = value;
      return (YES);
    }
}


FFCoord FFSpring::getxoff ()
{
  return (offset.v[0]);
}


BOOL FFSpring::setxoff (FFCoord value)
{
  if (value < 0)
    return (NO);
  else
    {
      offset.v[0] = value;
      return (YES);
    }
}


FFCoord FFSpring::getyoff ()
{
  return (offset.v[1]);
}


BOOL FFSpring::setyoff (FFCoord value)
{
  if (value < 0)
    return (NO);
  else
    {
      offset.v[1] = value;
      return (YES);
    }
}


FFCoord FFSpring::getzoff ()
{
  return (offset.v[2]);
}


BOOL FFSpring::setzoff (FFCoord value)
{
  if (value < 0)
    return (NO);
  else
    {
      offset.v[2] = value;
      return (YES);
    }
}


int FFSpring::getDimension ()
{
  return (dimension);
}


void FFSpring::setDimension (BOOL value)
{
  if (value == 2 || value == 3)
    dimension = value;
}


/////////////////////////////////////////////////////////////////////////////
//                                                                         //
// void FFSpring::layout () 					        //
//                                                                         //
// Funktionalitaet:                                                        //
// - der betreffende Graph wird mittels des springembedder-Algorithmus     //
//   layouted                                                              //
//                                                                         //
// Bemerkungen:                                                            //
// - Veraendert die x und y Koordinaten der Knoten                         // 
// - Veraendert value der Knoten und der Kanten 	        	        // 
// - Stuetzpunkte der Kanten werden entfernt		                 //
//	                                                                 //
// Vorbedingungen:                                                         //
// - es muss ein Graph mit setGraph() uebergeben wurden sein               //
//                                                                         //
// Nachbedingungen:                                                        //
// - die Koordinaten der Knoten sind auf die entsprechenden Werte gesetzt, //
//   value von Knoten und Kanten enthaelt andere Werte                     //
//							        //
/////////////////////////////////////////////////////////////////////////////

void FFSpring::layout ()
{
 FFSet <FFNode> locNode;
 
 BOOL tmpUpdate=YES;
 
 stopBit = NO;
 
 if (graph->getWindow ())
  {
   graph->getWindow()->setBusy ();
   tmpUpdate = graph->getWindow()->setUpdate(NO);
  }
 
 springembedder (locNode);
 
 if (graph->getWindow ())
  {
   graph->getWindow()->unsetBusy ();
   graph->getWindow()->setUpdate(tmpUpdate);
   graph->getWindow()->adjustWindowSize();
  }
}

void FFSpring::layout (FFSet <FFNode>& locNodes)
{
 BOOL tmpUpdate=YES;
 
 if (graph->getWindow ())
  {
   graph->getWindow()->setBusy ();
   tmpUpdate = graph->getWindow()->setUpdate(NO);
  }
  
 stopBit = NO;

 springembedder (locNodes);
 
 if (graph->getWindow ())
  {
   graph->getWindow()->unsetBusy ();
   graph->getWindow()->setUpdate(tmpUpdate);
   graph->getWindow()->adjustWindowSize();
  }
}

