/*
// Copyright 1994 Carsten Friedrich, Universitaet Passau
//
// $Author: friedric $
// $Date: 1995/11/01 12:02:31 $
// $RCSfile: FFGraph.cc,v $
// $Revision: 1.2 $
// $State: Exp $
//
*/

#ifdef RCSID
#undef RCSID
#endif
#define RCSID "$Author: friedric $ $RCSfile: FFGraph.cc,v $ $Date: 1995/11/01 12:02:31 $ $Revision: 1.2 $ $State: Exp $"

#include <stream.h>
#include <ffgraph/FFGraph.h>
#include <ffgraph/FFGraphClass.h>

FFGraphClass *FFGraph::createGraph() const
{
  return new FFGraphClass(bare);
#ifdef DEBUG
  cerr << "createGraph()\n";
#endif
}

/////////////////////////////////////////////////////////////////////////////
//                                                                         //
// FFGraph::FFGraph(FFLabelClass *nl, FFLabelClass *el)                    //
//                                                                         //
// Funktionalitaet:                                                        //
//                                                                         //
// - Konstruktor fuer einen mit Knotenlabel nl und Kantenlabel el          //
//   gelabelten Graphen + Huelle.                                          //
//                                                                         //
// Bemerkungen:                                                            //
//                                                                         //
// - nl und el, koennen nach dem Aufruf vom Benutzer freigeben werden      //
//                                                                         //
// Vorbedingungen:                                                         //
//                                                                         //
// - nl, el muessen Subklasse von FFLabelClass sein und auf gueltige       //
//   Objekte zeigen                                                        //
//                                                                         //
// Nachbedingungen:                                                        //
//                                                                         //
// - keine                                                                 //
//                                                                         //
/////////////////////////////////////////////////////////////////////////////

FFGraph::FFGraph(BOOL b)
{
 bare = b;
 graph= 0;
}

/////////////////////////////////////////////////////////////////////////////
//                                                                         //
// FFGraph::FFGraph(FFGraphClass *g)                                       //
//                                                                         //
// Funktionalitaet:                                                        //
//                                                                         //
// - Konstruktor fuer eine Huelle fuer den Graphen g                       //
//                                                                         //
// Bemerkungen:                                                            //
//                                                                         //
// - nl und el, koennen nach dem Aufruf vom Benutzer freigeben werden      //
//                                                                         //
// Vorbedingungen:                                                         //
//                                                                         //
// - nl, el muessen Subklasse von FFLabelClass sein und auf gueltige       //
//   Objekte zeigen                                                        //
//                                                                         //
// Nachbedingungen:                                                        //
//                                                                         //
// - keine                                                                 //
//                                                                         //
/////////////////////////////////////////////////////////////////////////////

FFGraph::FFGraph(FFGraphClass *g)
{
 bare= NO;

 graph= g;
 if(g)
  g->incNumOfReps();  
}

/////////////////////////////////////////////////////////////////////////////
//                                                                         //
// FFGraph::FFGraph(FFGraph& g)                                            //
//                                                                         //
// Funktionalitaet:                                                        //
//                                                                         //
// - Copy-Konstruktor fuer Graphhuelle g.                                  //
//                                                                         //
// Bemerkungen:                                                            //
//                                                                         //
// - Kopiert nur die Huelle, nicht aber den Graphen! (vgl. FFGraph::copy() //
//                                                                         //
// Vorbedingungen:                                                         //
//                                                                         //
// - keine                                                                 //
//                                                                         //
// Nachbedingungen: //
//                                                                         //
// - keine                                                                 //
//                                                                         //
/////////////////////////////////////////////////////////////////////////////

FFGraph::FFGraph(const FFGraph& g)
{
 bare=NO;
 graph= 0;
 assign(g);
}

/////////////////////////////////////////////////////////////////////////////
//                                                                         //
// FFGraph& FFGraph::operator = (FFGraph& g)                               //
//                                                                         //
// Funktionalitaet:                                                        //
//                                                                         //
// - Weist der Huelle den Graphen der Huelle g zu                          //
//                                                                         //
// Bemerkungen:                                                            //
//                                                                         //
// - keine                                                                 //
//                                                                         //
// Vorbedingungen:                                                         //
//                                                                         //
// - keine                                                                 //
//                                                                         //
// Nachbedingungen:                                                        //
//                                                                         //
// - keine                                                                 //
//                                                                         //
/////////////////////////////////////////////////////////////////////////////

FFGraph& FFGraph::operator = (const FFGraph& g)
{
  return assign(g);
}

FFGraph& FFGraph::assign (const FFGraph& g)
{
  // Releasing the old reference before acquiring the new one could destroy
  // the graph when assigning a handle to itself (or to another handle for
  // the same graph). Keep the source alive throughout the assignment.
  if (this == &g)
    return *this;

  FFGraph &source = const_cast<FFGraph&>(g);
  if (source.graph == NULL)
   {
    source.graph = source.createGraph();
    source.graph->incNumOfReps();
   }

  FFGraphClass *newGraph = source.graph;
  if (newGraph == graph)
    return *this;

  // Acquire the new ownership before releasing the old graph. This also
  // leaves *this unchanged if constructing a lazy source graph throws.
  newGraph->incNumOfReps();
  FFGraphClass *oldGraph = graph;
  graph = newGraph;
  if (oldGraph && !oldGraph->decNumOfReps())
    delete oldGraph;

  return *this;
}

/////////////////////////////////////////////////////////////////////////////
//                                                                         //
// FFGraph& FFGraph::operator == (FFGraph& g)                              //
//                                                                         //
// Funktionalitaet:                                                        //
//                                                                         //
// - Vergleicht die Huelle mit der Huelle g                                //
//                                                                         //
// Bemerkungen:                                                            //
//                                                                         //
// - Zwei Huellen sind gleich wenn sie Huellen fuer den selben Graph sind  //
//   Zwei ungueltige Huellen sind nie gleich !                             //
//   ( g ungueltig => ((g == g) -> False))                                  //
//                                                                         //
// Vorbedingungen:                                                         //
//                                                                         //
// - keine                                                                 //
//                                                                         //
// Nachbedingungen:                                                        //
//                                                                         //
// - keine                                                                 //
//                                                                         //
/////////////////////////////////////////////////////////////////////////////

int FFGraph::operator == (const FFGraph& g) const
{
  if(!graph)
    {
      return NO;
    }
  
  return (graph== g.graph);
}

int FFGraph::operator != (const FFGraph& g) const
{
  if(!graph)
	{
		return YES;
	}
  
  return (graph != g.graph);
}

////////////////////////////////////////////////////////////////////////////
//                                                                         //
// FFGraphClass *FFGraph::operator ()()                                    //
//                                                                         //
// Funktionalitaet:                                                        //
//                                                                         //
// - Liefert den der Huelle zugrunde liegenden Graphen                     //
//                                                                         //
// Vorbedingungen:                                                         //
//                                                                         //
// - keine                                                                 //
//                                                                         //
// Nachbedingungen:                                                        //
//                                                                         //
// - keine                                                                 //
//                                                                         //
/////////////////////////////////////////////////////////////////////////////

// FFGraphClass *FFGraph::operator->()
// {
//   if(!graph)
//     {
//       graph= createGraph();
//       graph->incNumOfReps();
//     }
  
//   return graph;
// }

/////////////////////////////////////////////////////////////////////////////
//                                                                         //
// FFGraph::~FFGraph()                                                     //
//                                                                         //
// Funktionalitaet:                                                        //
//                                                                         //
// - Destruktor. Loescht eine Huelle des Graphen. ist diese Huelle die     //
//   letzte Referenz auf den Graphen, so wird der Graph geloescht.         //
//                                                                         //
// Vorbedingungen:                                                         //
//                                                                         //
// - keine                                                                 //
//                                                                         //
// Nachbedingungen:                                                        //
//                                                                         //
// - keine                                                                 //
//                                                                         //
/////////////////////////////////////////////////////////////////////////////

FFGraph::~FFGraph()
{ 
  if(graph)
	{
		if(!graph->decNumOfReps())
		{
			delete graph;
		}
	}
}

