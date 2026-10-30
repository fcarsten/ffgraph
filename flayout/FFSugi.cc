/*
// Copyright 1994-97 Falk Schreiber, Universitaet Passau
//
*/

#include <ffgraph/FFSugi.h>
#include <ffgraph/FFGraphClass.h>
#include <ffgraph/FFGraphWindow.h>
#include <ffgraph/FFSugiInsp.h>

FFSugi::FFSugi ()
{
  _setType (ffSugiType);
  info.crossingAlg    = 0;     // gewaehlter Algorithmus zur Kreuzungs-
                               // reduzierung 
                               // 0 - mittels Adjazenzmatrizen
                               // 1 - mittels Bubbling (nicht mehr in dieser
                               // Version, da nur unwesentlich schneller und
			       // viel schlechter)
                               // 2 - keine Kreuzungsreduzierung
  info.iteration1     = 8;     // fuer Kreuzungsreduzierung 
  info.iteration2     = 3;     //         - " -
  info.xDistance      = 100;   // Abstaende zwischen Knoten in den drei
  info.yDistance      = 70;    // Raumrichtungen 
  info.zDistance      = 50;
  info.useStartNode   = YES;   // Startknoten-Information benutzen
  info.keepEdgePoints = NO;    // Fuer lokales Layout:
                               // Kantenstuetzpunkte von Knoten ausserhalb des 
                               // lok. Layouts erhalten 
                               // (lokales Layout zur Zeit nicht verfuegbar)
  info.useEndNodes    = NO;    // Endknoten-Information benutzen
                               // (zur Zeit nicht verfuegbar)
  info.sameSize       = 1;     // Alle Icons als Icons mit der Groesse
                               // 1 - des groessten Icons
                               // 2 - des eigenen Icons
                               // betrachten
  info.orient         = 1;     // 1 - Layout von oben nach unten
                               // 2 - Layout von links nach rechts
  info.grid           = YES;   // Gitter unterlegen
  info.dummyOffset    = 100;   // Abstaende zwischen Kanten in x-Richtung
                               // in % des Knotenabstandes 
                               // (Dieser Wert wird zur Zeit nicht 
                               // beruecksichtigt)
  info.offset.v[0]    = 0;     // Offset vom Koordinaten-Nullpunkt
  info.offset.v[1]    = 0;     // in den drei Raumrichtungen
  info.offset.v[2]    = 0;
  info.dimension      = 2;     // verwendete Dimension (2 oder 3)
                               // (3D-Layout zur Zeit nicht verfuegbar) 
  info.leavesDown     = NO;    // Blaetter in die unterste Ebene setzen
  info.deterministic  = YES;   // deterministisches oder nichtdeterministisches
                               // Layout 
}


FFSugi::~FFSugi ()
{
}

void FFSugi::init (const FFGraph& g) 
{ 
  FFLayoutLabel::init (g); 
  inspector= new FFSugiInsp (this);
}

// BOOL FFSugi::setGraph (FFGraph &g)
// {
//   graph = g;
//   return (YES);
// }


BOOL FFSugi::getUseStartNode ()
{
  return (info.useStartNode);
}


void FFSugi::setUseStartNode (BOOL value)
{
  info.useStartNode = value;
}


BOOL FFSugi::getUseEndNodes ()
{
  return (info.useEndNodes);
}


void FFSugi::setUseEndNodes (BOOL value)
{
  // info.useEndNodes = value;
  info.useEndNodes = NO;
}


BOOL FFSugi::getKeepEdgePoints ()
{
  return (info.keepEdgePoints);
}


void FFSugi::setKeepEdgePoints (BOOL value)
{
  // info.keepEdgePoints = value;
  info.keepEdgePoints = NO;
}


int FFSugi::getSameSize ()
{
  return (info.sameSize);
}


void FFSugi::setSameSize (int value)
{
  // value 1 oder 2
  if ((value == 1) || (value == 2))
    info.sameSize = value;
 
  if (info.sameSize == 2)
    info.grid = NO;
}


int FFSugi::getOrient ()
{
  return (info.orient);
}


void FFSugi::setOrient (int value)
{
  info.orient = value;
}


BOOL FFSugi::getUseGrid ()
{
  return (info.grid);
}


void FFSugi::setUseGrid (BOOL value)
{
  info.grid = value;
  if ((info.grid != NO) && (info.sameSize == 2))
    info.sameSize = 1;
}


BOOL FFSugi::getLeavesDown ()
{
  return (info.leavesDown);
}


void FFSugi::setLeavesDown (BOOL value)
{
  info.leavesDown = value;
}


BOOL FFSugi::getDeterministic ()
{
  return (info.deterministic);
}


void FFSugi::setDeterministic (BOOL value)
{
  info.deterministic = value;
}


int FFSugi::getCrossingAlg ()
{
  return (info.crossingAlg);
}


BOOL FFSugi::setCrossingAlg (int value)
{
  if ((value == 0) || (value == 1))
    {
      info.crossingAlg = value;
      return (YES);
    }
  else
    return (NO); 
}


int FFSugi::getIteration1 ()
{
  return (info.iteration1);
}


BOOL FFSugi::setIteration1 (int value)
{   
  if (value < 0)
    return (NO);
  else
    {
      info.iteration1 = value;
      return (YES);
    }
}


FFCoord FFSugi::getxoff () 
{
  return (info.offset.v[0]);
}


BOOL FFSugi::setxoff (FFCoord value)
{
  if (value < 0)
    return (NO);
  else
    {
      info.offset.v[0] = value;
      return (YES);
    }
}


FFCoord FFSugi::getyoff ()
{
  return (info.offset.v[1]);
}


BOOL FFSugi::setyoff (FFCoord value)
{
  if (value < 0)
    return (NO);
  else
    {
      info.offset.v[1] = value;
      return (YES);
    }
}


FFCoord FFSugi::getzoff () 
{
  return (info.offset.v[2]);
}


BOOL FFSugi::setzoff (FFCoord value)
{
  if (value < 0)
    return (NO);
  else
    {
      info.offset.v[2] = value;
      return (YES);
    }
}


FFCoord FFSugi::getxDistance ()
{
  return (info.xDistance);
}


BOOL FFSugi::setxDistance (FFCoord value)
{
  if (value < 1)
    return (NO);
  else
    {
      info.xDistance = value;
      return (YES);
    }
}


FFCoord FFSugi::getyDistance ()
{
  return (info.yDistance);
}


BOOL FFSugi::setyDistance (FFCoord value)
{
  if (value < 1)
    return (NO);
  else
    {
      info.yDistance = value;
      return (YES);
    }
}


FFCoord FFSugi::getzDistance ()
{
  return (info.zDistance);
}


BOOL FFSugi::setzDistance (FFCoord value)
{
  if (value < 1)
    return (NO);
  else
    {
      info.zDistance = value;
      return (YES);
    }
}


FFCoord FFSugi::getDummyOffset()
{
  return (info.dummyOffset);
}


void FFSugi::setDummyOffset (FFCoord xValue)
{
  //  info.dummyOffset = xValue;
  info.dummyOffset = 100;
}


int FFSugi::getDimension ()
{
  return (info.dimension);
}


void FFSugi::setDimension (int value)
{   
  if (value == 2 || value == 3)
    info.dimension = value;
}


/////////////////////////////////////////////////////////////////////////////
//                                                                         //
// void FFSugi::layout () bzw. layout (FFSet <FFNode> &locNodes)           //
//                                                                         //
// Funktionalitaet:                                                        //
// - die uebergebenen Knoten des betreffenden Graphen werden mittels des   //
//   Sugiyama-Algorithmus layouted                                         //
// (lokales Layout ist zur Zeit nicht verfuegbar)                          //
//                                                                         //
// Bemerkungen:                                                            //
// - Veraendert die x und y Koordinaten der Knoten		           //
// - Veraendert "value" und "attribut" der Knoten und                      //
//   "value" der Kanten                                                    // 
// - Stuetzpunkte der Kanten werden entfernt und neue eingefuegt	   //
//							                   //
// Vorbedingungen:                                                         //
// - es muss ein Graph mit setGraph() uebergeben wurden sein               //
// (- die zu layoutenden Knoten muessen uebergeben werden)                 //
//                                                                         //
// Nachbedingungen:                                                        //
// - die Koordinaten der Knoten sind auf die entsprechenden Werte gesetzt  //
// - "value" von Knoten und Kanten und "attribut" der Knoten haben ge-     //
//   aenderte Werte					                   //
//                                                                         //
/////////////////////////////////////////////////////////////////////////////

void FFSugi::layout (FFSet <FFNode> &locNodes)
{
  if (graph->getWindow ())
    graph->getWindow ()->setBusy ();

  sugiyama (graph, info, locNodes);

  if (graph->getWindow ())
    graph->getWindow ()->unsetBusy ();
}


void FFSugi::layout ()
{
  if (graph->getWindow ())
    graph->getWindow ()->setBusy ();

  sugiyama (graph, info, graph->allNodes ());

  if (graph->getWindow ())
    graph->getWindow ()->unsetBusy ();
}


