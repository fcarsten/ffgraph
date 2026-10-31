/*
// Copyright 1994-97 Falk Schreiber, Universitaet Passau
//
*/

#include <string.h>

#include <ffgraph/FFlayout_algorithmen.h>
#include <ffgraph/FFGraphicNode.h>

SNode::SNode ()
{
  dummy = NO;
  indeg = 0; 
  outdeg = 0;
  attrib = 1;
  pred.clear ();
  succ.clear ();
  dfs = 0;
  comp = 0;
  level = 0;
  elevel = 0;
  leaf = 0;
  anzDum = 0;
  vertUnit = 1;
  edgeNum = 0;
  maxPos = 0;
  height = 0.0;
  width = 0.0;
  point.v[0] = 0.0;  
  point.v[1] = 0.0;
  point.v[2] = 0.0;
}

SNode::~SNode ()
{
  node = 0;
  pred.clear ();
  succ.clear ();
};

SEdge::SEdge ()
{
  attrib = 1; 
  width = 0.0;
  type = 0;
  source = 0;
  target = 0;
  nodeNr1 = 0;    
  nodeNr2 = 0;
};

SEdge::~SEdge ()
{
  edge = 0;    
}

void init_snode (SNode *nodes, int anzN, FFSet <FFEdge> &allEdges)
///////////////////////////////////////////////////////////////////
//                                                               //
// Initialisierung der Datenstruktur "snode"                     //
//                                                               //
///////////////////////////////////////////////////////////////////
{
  FFGraphicNode *gn;
  FFSet <FFNode> nSet;
  Pix p;
  FFNode tmpN;
  int i,
    source,
    target;
  
  // Fuer jeden Knoten
  for (i = 1; i <= anzN; i++)
    {
      tmpN = nodes[i].node;

      gn = (*tmpN) ();
      // Falls eine Graphikrepraesentation existiert
      if (gn != 0)
	{
	  nodes[i].height = gn->getHeight ();
	  nodes[i].width = gn->getWidth ();
	}
    }

  // Jede Kante zuordnen
  for (p = allEdges.first (); p != 0; allEdges.next (p))
    {
      source = (allEdges (p)->source ())->getValue ();
      target = (allEdges (p)->target ())->getValue ();

      nodes[target].pred.append (source);
      (nodes[target].indeg)++;
      nodes[source].succ.append (target);
      (nodes[source].outdeg)++;
    }
}

void  init_sedge (SEdge *edges, int anzE)
///////////////////////////////////////////////////////////////////
//                                                               //
// Initialisierung der Datenstruktur "sedge"                     //
//                                                               //
///////////////////////////////////////////////////////////////////
{
  int i;

  for (i = 1; i <= anzE; i++)
    {
      edges[i].source = edges[i].edge->source ()->getValue ();
      edges[i].target = edges[i].edge->target ()->getValue ();
    }
}

void init_sgraph (FFGraph &graph, SNode *nodes, SEdge *edges, 
		  int anzN, int anzE, int anzEN,
		  FFSet <FFNode> &allNodes, FFSet <FFEdge> &allEdges)
///////////////////////////////////////////////////////////////////
//                                                               //
// Initialisierung der Datenstruktur "sgraph" durch den Original-//
// graphen.                                                      //
//                                                               //
///////////////////////////////////////////////////////////////////
{
  Pix p;
  int i;
    
  // Knoten nach ihrem bisherigen "value"-Wert sortieren
  // attrib == 0 gibt an, dass dieser Eintrag schon belegt ist
  for (p = allNodes.first (); p != 0; allNodes.next (p))
    {
      if (((allNodes (p)->getValue ()) > 0) &&
	  ((allNodes (p)->getValue ()) <= anzN))
      {
	if (nodes[allNodes (p)->getValue ()].attrib == 1)
	  {
	    nodes[allNodes (p)->getValue ()].node = allNodes (p);
	    nodes[allNodes (p)->getValue ()].attrib = 0;
	  } 
	else
	  allNodes (p)->setValue (0);
      }
    }
  
  // alle anderen Knoten (kein sinnvoller "value"-Wert)
  // (Suche freies Feld in nodes)
  i = 1;
  while ((i < anzN) && (nodes[i].attrib == 0))
    i++;
  for (p = allNodes.first (); p != 0; allNodes.next (p))
    {
      if ((allNodes (p)->getValue () <= 0) ||
	  (allNodes (p)->getValue () > anzN))
	{
	  nodes[i].node = allNodes (p);
	  nodes[i].attrib = 0;
	  allNodes (p)->setValue (i);

	  while ((i < anzN) && (nodes[i].attrib == 0))
	    i++;
	}
    }

  // Kanten nach ihrem bisherigen value-Wert sortieren
  for (p = allEdges.first (); p != 0; allEdges.next (p))
    {
      if (((allEdges (p)->getValue ()) > 0) &&
	  ((allEdges (p)->getValue ()) <= anzE))
      {
	if (edges[allEdges (p)->getValue ()].attrib == 1)
	  {
	    edges[allEdges (p)->getValue ()].edge = allEdges (p);
	    edges[allEdges (p)->getValue ()].attrib = 0;
	  }
	else
	  allEdges (p)->setValue (0);
      }
    }
  
  // alle anderen Kanten (ohne sinnvollen "value"-Wert)
  i = 1;
  while ((i < anzE) && (edges[i].attrib == 0))
    i++;
  
  for (p = allEdges.first (); p != 0; allEdges.next (p))
    {
      if (((allEdges (p)->getValue ()) <= 0) ||
	  ((allEdges (p)->getValue ()) > anzE))
	{
	  edges[i].edge = allEdges (p);
	  edges[i].attrib = 0;
	  allEdges (p)->setValue (i);

	  while ((i < anzE) && (edges[i].attrib == 0))
	    i++;
	}
    }

  init_snode (nodes, anzN, allEdges);
  init_sedge (edges, anzE);
}

void finish_sgraph (FFGraph &graph, SNode *nodes, SEdge *edges, 
		    int anzN, int anzE, int dummies,   
		    DLList <int> &backEdges, 
		    FFCoord xDistance, FFCoord yDistance,
		    int orient)
///////////////////////////////////////////////////////////////////
//                                                               //
// Rueckschreiben der Datenstruktur "sgraph" in den Original-    //
// graphen.                                                      //
//                                                               //
///////////////////////////////////////////////////////////////////
{ 
  FFGraphicNode *gn;
  FFGraphicEdge *ge;
  FFEdge tmpE;
  int i,
    j,
    k;
  String tmpS;
  FFPoint point, oldpoint;
  FFSet <int> backEdgesSet;
  Pix p;

  // Rueckwaertskanten bestimmen
  for (p = backEdges.first (); p != 0; backEdges.next (p))
    backEdgesSet.add (backEdges (p));

  if (orient == 1)
    {
      // Positioniere die Knoten an die richtige Stelle 
      for (i = 1; i <= anzN; i++)
	{
	  gn = (*(nodes[i].node)) ();
	  
	  if (gn)
	    gn->moveTo (nodes[i].point);
	}
      
      // Kopiere die Kantenstuetzpunkte
      i = anzN+1;
      while (i <= anzN+dummies)
	{
	  tmpE = edges[nodes[i].edgeNum].edge;
	  ge = (*tmpE) ();
	  
	  if (nodes[i].anzDum == 1)	    
	    {
	      if (ge)
		{
		  if (edges[nodes[i].edgeNum].type != 1)
		    ge->addPointAt (1, nodes[i].point);
		  else
		    {
		      nodes[i].point.v[0] = nodes[i].point.v[0] - 
			(xDistance/4);
		      ge->addPointAt (1, nodes[i].point);
		      nodes[i].point.v[0] = nodes[i].point.v[0] + 
			(xDistance/2);
		      ge->addPointAt (2, nodes[i].point);
		    }
		  i++;
		}
	    }
	  else
	    {
	      if (backEdgesSet.contains (nodes[i].edgeNum))
		{
		  if (ge)
		    {
		      ge->addPointAt (1, nodes[i+1].point);
		      k = 2;
		      oldpoint = nodes[i+1].point;
		      for (j = nodes[i].elevel; j > nodes[i].level; j--) 
			{
			  if ((j > nodes[i].level+1) && 
			      (oldpoint.v[0] == nodes[i].posList[j].v[0]) &&
			      (oldpoint.v[0] == nodes[i].posList[j-1].v[0]))
			    {
			    }
			  else
			    {
			      ge->addPointAt (k, nodes[i].posList[j]);
			      k++;
			      oldpoint = nodes[i].posList[j];
			    }
			}
		      
		      ge->addPointAt (k, nodes[i].point);
		    }
		  i = i+2;
		}
	      else  
		{
		  if (ge)
		    {
		      ge->addPointAt (1, nodes[i].point);
		      k = 2;
		      oldpoint = nodes[i].point;
		      for (j = nodes[i].level+1; j <= nodes[i].elevel; j++)
			{
			  if ((j < nodes[i].elevel) && 
			      (oldpoint.v[0] == nodes[i].posList[j].v[0]) &&
			      (oldpoint.v[0] == nodes[i].posList[j+1].v[0]))
			    {
			    }
			  else
			    {
			      ge->addPointAt (k, nodes[i].posList[j]);
			      k++;
			      oldpoint = nodes[i].posList[j];
			    }
			}
		      ge->addPointAt (k, nodes[i+1].point);
		    }
		  i = i+2;
		}
	    }
	}
    }
  else
    {
      // orient == 2
      // Positioniere die Knoten an die richtige Stelle 
      for (i = 1; i <= anzN; i++)
	{
	  gn = (*(nodes[i].node)) ();
	  point = nodes[i].point;
	  point.v[0] = nodes[i].point.v[1];
	  point.v[1] = nodes[i].point.v[0];
	  
	  if (gn)
	    gn->moveTo (point);
	}
      
      // Kopiere die Kantenstuetzpunkte
      i = anzN+1;
      while (i <= anzN+dummies)
	{
	  tmpE = edges[nodes[i].edgeNum].edge;
	  ge = (*tmpE) ();
	  
	  if (nodes[i].anzDum == 1)	    
	    {
	      if (ge)
		{
		  if (edges[nodes[i].edgeNum].type != 1)
		    {
		      point = nodes[i].point;
		      point.v[0] = nodes[i].point.v[1];
		      point.v[1] = nodes[i].point.v[0];
		      
		      ge->addPointAt (1, point);
		    }
		  else
		    {
		      nodes[i].point.v[0] = nodes[i].point.v[0] - 
			(yDistance/4);
		      point = nodes[i].point;
		      point.v[0] = nodes[i].point.v[1];
		      point.v[1] = nodes[i].point.v[0];

		      ge->addPointAt (1, point);
		      nodes[i].point.v[0] = nodes[i].point.v[0] + 
			(yDistance/2);
		      point = nodes[i].point;
		      point.v[0] = nodes[i].point.v[1];
		      point.v[1] = nodes[i].point.v[0];
		      
		      ge->addPointAt (2, point);
		    }
		  i++;
		}
	    }
	  else
	    {
	      if (backEdgesSet.contains (nodes[i].edgeNum))
		{
		  if (ge)
		    {
		      point = nodes[i+1].point;
		      point.v[0] = nodes[i+1].point.v[1];
		      point.v[1] = nodes[i+1].point.v[0];

		      ge->addPointAt (1, point);
		      k = 2;
		      oldpoint = point;
		      for (j = nodes[i].elevel; j > nodes[i].level; j--) 
			{
			  point = nodes[i].posList[j];
			  point.v[0] = nodes[i].posList[j].v[1];
			  point.v[1] = nodes[i].posList[j].v[0];

			  if ((j > nodes[i].level+1) && 
			      (oldpoint.v[0] == nodes[i].posList[j].v[0]) &&
			      (oldpoint.v[0] == nodes[i].posList[j-1].v[0]))
			    {
			    }
			  else
			    {			  
			      ge->addPointAt (k, point);
			      k++;
			      oldpoint = point;
			    }
			}

		      point = nodes[i].point;
		      point.v[0] = nodes[i].point.v[1];
		      point.v[1] = nodes[i].point.v[0];

		      ge->addPointAt (k, point);
		    }
		  i = i+2;
		}
	      else  
		{
		  if (ge)
		    {
		      point = nodes[i].point;
		      point.v[0] = nodes[i].point.v[1];
		      point.v[1] = nodes[i].point.v[0];

		      ge->addPointAt (1, point);
		      k = 2;
		      oldpoint = point;
		      for (j = nodes[i].level+1; j <= nodes[i].elevel; j++)
			{
			  point = nodes[i].posList[j];
			  point.v[0] = nodes[i].posList[j].v[1];
			  point.v[1] = nodes[i].posList[j].v[0];
			  
			  if ((j < nodes[i].elevel) && 
			      (oldpoint.v[0] == nodes[i].posList[j].v[0]) &&
			      (oldpoint.v[0] == nodes[i].posList[j+1].v[0]))
			    {
			    }
			  else  
			    {
			      ge->addPointAt (k, point);
			      k++;
			      oldpoint = point;
			    }
			}
		      point = nodes[i+1].point;
		      point.v[0] = nodes[i+1].point.v[1];
		      point.v[1] = nodes[i+1].point.v[0];

		      ge->addPointAt (k, point);
		    }
		  i = i+2;
		}
	    }
	}
    }
}

int get_max_level (SNode *nodes, int anzND)
///////////////////////////////////////////////////////////////////
//                                                               //
// Liefert Anzahl der Ebenen (groesster Wert, welcher in "level" //
// von SNode gespeichert ist).                                   //
//                                                               //
///////////////////////////////////////////////////////////////////
{
  int maxL = 0, 
    i;
  
  for (i = 1; i <= anzND; i++)
    if (nodes[i].level > maxL)
      maxL = nodes[i].level;

  return (maxL);
}

int get_max_size (SNode *nodes, int anzN, int dummies, int maxL)
///////////////////////////////////////////////////////////////////
//                                                               //
// Liefert maximale Breite einer Ebene.                          //
//                                                               //
///////////////////////////////////////////////////////////////////
{
  int maxS = 0, 
    i,
    j,
    *nodesOfLevel;
  
  nodesOfLevel = new int [maxL+1];
  if (nodesOfLevel == 0)
    return (anzN + dummies);

  for (i = 0; i <= maxL; i++)
    nodesOfLevel[i] = 0;
    
  for (i = 1; i <= anzN; i++)
    (nodesOfLevel[nodes[i].level])++;
  
  for (i = anzN+1; i <= anzN+dummies; i++)
    {
      if (nodes[i].anzDum == 1)
	(nodesOfLevel[nodes[i].level])++;
      else
	{
	  for (j = nodes[i].level; j <= nodes[i+1].level; j++)
	    (nodesOfLevel[j])++;
	  i++;
	}
    }

  for (i = 0; i <= maxL; i++)
    if (nodesOfLevel[i] > maxS)
      maxS = nodesOfLevel[i];
  
  delete[] nodesOfLevel;

  return (maxS);
}

void compute_edges_of_level (int *nodeArray, SNode *nodes, int anzN, 
			     int dummies, int maxL, int orient)
///////////////////////////////////////////////////////////////////
//                                                               //
// Berechnet Kanten von Level i nach Level i+1. Aktualisiert     //
// "vertUnit", also den Faktor des Abstandes zwischen Level i    //
// Level i+1.                                                    //
// Zusaetzlich: Berechnet die Hoehe des groessten Icons pro Level//
// und aktualisiert "vertUnit" entsprechend.                     //
//                                                               //
///////////////////////////////////////////////////////////////////
{
  int max, 
    n, 
    tmp, 
    lev, 
    abs = 1;

  // Fuer alle Level
  for (lev = 0; lev <= maxL; lev++)
    {
      max = 0;

      // Max. Zahl ein- bzw. ausgehender Kanten zwischen Knoten des Levels
      // und des naechsten Levels bestimmen.
      for (n = 1; n <= anzN; n++)
        {
          if (nodes[n].level == lev)
            if (nodes[n].outdeg > max)
              max = nodes[n].outdeg;
          if (nodes[n].level == lev + 1)
            if (nodes[n].indeg > max)
              max = nodes[n].indeg;	
        }

      tmp = 0;
      for (n = 1; n <= anzN+dummies; n++)
        if (nodes[n].level == lev + 1)
          {
            // zusaetzlicher Abstand zw. zwei Level: 1/3 der Anzahl maximal ein
	    // bzw. ausgehender Kanten zwischen Knoten von Level l und l+1.
	    // (Jedoch nur, falls die Orientierung nicht vertauscht wurde.)
	    if (orient == 2)
	      nodes[n].vertUnit = abs + 1;
	    else
	      nodes[n].vertUnit = abs + 1 + (int) (max / 3);

            nodes[n].vertUnit = nodes[n].vertUnit;
            tmp = nodes[n].vertUnit;
         }

      abs = tmp;
    }
}

void number_nodes (FFGraph &graph, int *oldValues)
///////////////////////////////////////////////////////////////////
//                                                               //
// Numeriert die Knoten des Graphen "graph" beginnend mit 1      //
// durch, verwendet "value"-Wert des Knotens, um die Knotennummer//
// zu speichern. Im Feld "oldValues" koennen die alten Werte     //
// gemerkt (und am Ende des Layouts zurueckgeschrieben) wer-     //
// den. Falls die Knoten bereits eine fortlaufende Numerierung   //
// besitzen, so bleibt diese erhalten.                           //
//                                                               //
///////////////////////////////////////////////////////////////////
{    
  Pix p;
  int i = 1;
  FFSet <FFNode> allNodes;
  FFNode tmpN;

  allNodes |= graph->allNodes ();

  // Falls alte Werte in "value" nicht gespeichert werden sollen.
  if (oldValues == 0)
    {
      p = allNodes.first ();
      
      // Existiert bereits ein Knoten mit "value == i"?
      while (p != 0)
        {
          tmpN = graph->nodeWithValue (i);
          // Sonst aktuellen Knoten verwenden.
	  if (tmpN == NULL)
            tmpN = allNodes (p);
          
          tmpN->setValue (i);
          i++;
          allNodes.del (tmpN);
          p = allNodes.first ();
        }
    }

  else
    {
      // Speichere Inhalt von "value"
      p = allNodes.first ();

      while (p != 0)
        {
          tmpN = graph->nodeWithValue (i);
          if (tmpN == NULL)
            tmpN = allNodes (p);
          
          oldValues[i] = tmpN->getValue ();
          tmpN->setValue (i);
          i++;
          allNodes.del (tmpN);
          p = allNodes.first ();
        }
    }
}

void number_edges (FFGraph &graph, int *oldValues)
///////////////////////////////////////////////////////////////////
//                                                               //
// Numeriert die Kanten des Graphen "graph" beginnend mit 1      //
// durch, verwendet "value"-Wert der Kantens, um die Kantennummer//
// zu speichern. Im Feld "oldValues" koennen die alten Werte     //
// gemerkt (und am Ende des Layouts zurueckgeschrieben) wer-     //
// den. Falls die Kanten bereits eine fortlaufende Numerierung   //
// besitzen, so bleibt diese erhalten.                           //
//                                                               //
///////////////////////////////////////////////////////////////////
{    
  Pix p, 
    q;
  int i = 1;
  FFSet <FFEdge> allEdges;
  FFEdge tmpE;

  allEdges |= graph->allEdges ();

  // Falls alte Werte in "value" nicht gespeichert werden sollen.
  if (oldValues == 0)
    {
      p = allEdges.first ();
      while (p != 0)
        {
	  // Existiert bereits eine Kante mit "value == i"?
	  q = allEdges.first ();
	  while ((q != 0) && (allEdges (q)->getValue () != i))
	    allEdges.next (q);
	  
          // Sonst aktuelle Kante verwenden.
	  if (q == 0)
            tmpE = allEdges (p);
          else
	    tmpE = allEdges (q);

          tmpE->setValue (i);
          i++;
          allEdges.del (tmpE);
          p = allEdges.first ();
        }
    }

  else
    {
      // Speichere Inhalt von "value"
      p = allEdges.first ();
      while (p != 0)
        {
	  // Existiert bereits eine Kante mit "value == i"?
	  q = allEdges.first ();
	  while ((q != 0) && (allEdges (q)->getValue () != i))
	    allEdges.next (q);
	  
          if (q == 0)
            tmpE = allEdges (p);
          else
	    tmpE = allEdges (q);

          oldValues[i] = tmpE->getValue ();
          tmpE->setValue (i);
          i++;
          allEdges.del (tmpE);
          p = allEdges.first ();
        }
    }
}