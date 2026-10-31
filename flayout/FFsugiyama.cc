/*
// Copyright 1994 Falk Schreiber, Universitaet Passau
//
*/

#include <ffgraph/FFsugiyama.h>

// Explicitly instantiate the list/stack adapters used by this translation unit.
// DLNode and SLNode were libg++ implementation details; the STL-backed
// compatibility adapters intentionally do not expose those node templates.
template class DLList <int>;
template class MFStack <int>;
template class SLList <int>;


void sdfsvisit (int k, SNode *nodes, int &dfs, int &comp)
///////////////////////////////////////////////////////////////////
//                                                               //
// Fuer die Tiefensuche mit zusaetzlicher Berechnung der         //
// Rueckwaertskanten.                                            //
//                                                               //
///////////////////////////////////////////////////////////////////
{
  Pix p;
  
  // Fuer alle Nachfolger
  for (p = nodes[k].succ.first (); p != 0; nodes[k].succ.next (p))
    if (nodes[nodes[k].succ (p)].dfs == 0)
      {
	dfs++;
	nodes[nodes[k].succ (p)].dfs = dfs;
	sdfsvisit (nodes[k].succ (p), nodes, dfs, comp);
	comp++;
	nodes[nodes[k].succ (p)].comp = comp;
      }
}

void sdfs (SNode *nodes, int anzN, FFGraph &graph)
///////////////////////////////////////////////////////////////////
//                                                               //
// Tiefensuche mit zusaetzlicher Berechnung der dfs- und comp-   //
// Nummern.                                                      //
// (Bisher nur rekursiv, besser waere mit Stack.)                //
//                                                               //
///////////////////////////////////////////////////////////////////
{
  int dfs = 0,
    comp = 0,
    i,
    startN;
  
  if (graph->getStartNode () != 0)
    {
      startN = (graph->getStartNode ())->getValue ();
      dfs++;
      nodes[startN].dfs = dfs;
      sdfsvisit (startN, nodes, dfs, comp);
      comp++;
      nodes[startN].comp = comp;
    }
    
  // Startknoten
  for (i = 1; i <= anzN; i++)
    if (nodes[i].indeg == 0)
      {	
	dfs++;
	nodes[i].dfs = dfs;
	sdfsvisit (i, nodes, dfs, comp);
	comp++;
	nodes[i].comp = comp;
      }

  for (i = 1; i <= anzN; i++)
    if (nodes[i].dfs == 0)
      {	
	dfs++;
	nodes[i].dfs = dfs;
	sdfsvisit (i, nodes, dfs, comp);
	comp++;
	nodes[i].comp = comp;
      }
}

void init_backEdges (SNode *nodes, int anzE, DLList <int> &backEdges,  
		     SEdge *edges)
///////////////////////////////////////////////////////////////////
//                                                               //
// Berechnet die Menge alle Rueckwaertskanten.                   //
//                                                               //
///////////////////////////////////////////////////////////////////
{
  int i;

  for (i = 1; i <= anzE; i++)
    if (edges[i].type == 0)
      if ((nodes[edges[i].source].dfs > nodes[edges[i].target].dfs) &&
	  (nodes[edges[i].source].comp < nodes[edges[i].target].comp))
	backEdges.append (i);
}

void slevelvisit (int k, SNode *nodes)
///////////////////////////////////////////////////////////////////
//                                                               //
// Fuer die Berechnung der Level der Knoten. (Hier lassen sich   //
// eventuell noch rekursive Aufrufe einsparen.)                  //
//                                                               //
///////////////////////////////////////////////////////////////////
{
  Pix p;
    
  for (p = nodes[k].succ.first (); p != 0; nodes[k].succ.next (p))
    if ((nodes[nodes[k].succ (p)].level) < nodes[k].level+1)
      {
	nodes[nodes[k].succ (p)].level = nodes[k].level+1;
	nodes[nodes[k].succ (p)].attrib = 1;
	slevelvisit (nodes[k].succ (p), nodes);
      }
}

void slevel (SNode *nodes, int anzN, FFGraph &graph)
///////////////////////////////////////////////////////////////////
//                                                               //
// Fuer jeden Knoten wird der laengste Pfad vom Startknoten      //
// errechnet. Der Knoten erhaelt diesen Wert als Level.          //
// Level beginnt mit 0!                                          //
//                                                               //
///////////////////////////////////////////////////////////////////
{
  int i,
    startN;
  
  if (graph->getStartNode () != 0)
    {
      startN = (graph->getStartNode ())->getValue ();
      nodes[startN].level = 0;
      nodes[startN].attrib = 1;
      slevelvisit (startN, nodes);
    }
  
  // Startknoten (Knoten mit indeg == 0)
  for (i = 1; i <= anzN; i++)
    if (nodes[i].indeg == 0)
      {
	nodes[i].level = 0;
	nodes[i].attrib = 1;
	slevelvisit (i, nodes);
      }

  for (i = 1; i <= anzN; i++)
    if (nodes[i].attrib == 0)
      {
	nodes[i].level = 0;
	nodes[i].attrib = 1;
	slevelvisit (i, nodes);
      }
}

void leaves_down (SNode *nodes, SEdge *edges, 
		  int anzN, DLList <int> &parEdges, int maxL)
///////////////////////////////////////////////////////////////////
//                                                               //
// Blaetter auf den untersten Level setzen.                      //
//                                                               //
///////////////////////////////////////////////////////////////////
{
  int i;

  for (i = 1; i <= anzN; i++)
    if (nodes[i].outdeg == 0)
      nodes[i].level = maxL;
}

void slevel_correct (SNode *nodes, SEdge *edges, 
		     int anzN, DLList <int> &parEdges)
///////////////////////////////////////////////////////////////////
//                                                               //
// Fuer jeden Knoten wird der laengste Pfad vom Startknoten      //
// errechnet. Der Knoten erhaelt diesen Wert als Level.          //
// Level beginnt mit 0!                                          //
//                                                               //
///////////////////////////////////////////////////////////////////
{
  int //i,
    tmpE;
  Pix p;
    
  for (p = parEdges.first (); p != 0; parEdges.next (p))
    {
      tmpE = parEdges (p);
      if (nodes[edges[tmpE].source].level == 
	  nodes[edges[tmpE].target].level-1)
	{
	  nodes[edges[tmpE].target].level = nodes[edges[tmpE].target].level+1;
	  slevelvisit (edges[tmpE].target, nodes);
	}
    }
}

void  change_edges (SNode *nodes, SEdge *edges, 
		    DLList <int> &backEdges)
///////////////////////////////////////////////////////////////////
//                                                               //
// Rueckwaertskanten werden umgedreht, die Strukturen "nodes"    //
// und "edges" werden aktualisiert.                              //
//                                                               //
///////////////////////////////////////////////////////////////////

{
  int h, 
    e, 
    source, // source-Knoten der Kante 
    target; // target-Knoten der Kante 
  Pix p, 
    q;
  
  for (p = backEdges.first (); p != 0; backEdges.next (p))
    {
      e = backEdges (p);
      source = edges[e].source;
      target = edges[e].target;

      // Succ-Knoten korrigieren (in nodes aktualisieren)
      q = nodes[source].succ.first ();
      while (q != 0)
	{
	  if (nodes[source].succ (q) == target)
	    {
	      nodes[source].succ.del (q);
	      q = 0;
	      (nodes[source].outdeg)--;
	    }
	  else
	    nodes[source].succ.next (q);
	}

      // Pred-Knoten korigieren
      q = nodes[target].pred.first ();
      while (q != 0)
	{
	  if (nodes[target].pred (q) == source)
	    {
	      nodes[target].pred.del (q);
	      q = 0;
	      (nodes[target].indeg)--;
	    }
	  else
	    nodes[target].pred.next (q);
	}

      nodes[source].pred.append (target);
      nodes[target].succ.append (source);
      (nodes[source].indeg)++;
      (nodes[target].outdeg)++;

      h = edges[e].source;
      edges[e].source = edges[e].target;
      edges[e].target = h;
    }
}

void rem_ref_edges (SNode *nodes, SEdge *edges, int anzE, int &anzRef, 
		    DLList <int> &refEdges)
///////////////////////////////////////////////////////////////////
//                                                               //
// Reflexive Kanten aus temporaerer Datenstruktur entfernen.     //
//                                                               //
///////////////////////////////////////////////////////////////////
{
  int e, 
    source, // source-Knoten der Kante 
    target; //target-Knoten der Kante
  Pix q;
  
  anzRef = 0;
  for (e = 1; e <= anzE; e++)
    if (edges[e].source == edges[e].target)
      {
	anzRef++;
	edges[e].type = 1;
	refEdges.append (e);
	source = edges[e].source;
	target = source;
	
	// Succ-Knoten loeschen
	q = nodes[source].succ.first ();
	while (q != 0)
	  {
	    if (nodes[source].succ (q) == target)
	      {
		nodes[source].succ.del (q);
		q = 0;
		(nodes[source].outdeg)--;
	      }
	    else
	      nodes[source].succ.next (q);
	  }
	
	// Pred-Knoten loeschen
	q = nodes[target].pred.first ();
	while (q != 0)
	  {
	    if (nodes[target].pred (q) == source)
	      {
		nodes[target].pred.del (q);
		q = 0;
		(nodes[target].indeg)--;
	      }
	    else
	      nodes[target].pred.next (q);
	  }
      }
}

void rem_par_edges (SNode *nodes, SEdge *edges, 
		    int anzE, int &anzPar, int type,
		    DLList <int> &parEdges)
///////////////////////////////////////////////////////////////////
//                                                               //
// Markiert (und loescht aus temporaerer Struktur) parallele     // 
// Kanten.                                                       //
//                                                               //
///////////////////////////////////////////////////////////////////
{
  int e, 
    j, 
    source, 
    target;
  Pix q;

  anzPar = 0;
  for (j = 1; j <= anzE; j++)
    if (edges[j].type == 0)
      for (e = 1; e <= anzE; e++)
	{

	  // Bereits geloeschte Kanten nicht noch einmal beruecksichtigen
	  if ((e != j) &&
	      (edges[e].type == 0) &&
	      (edges[e].source == edges[j].source) &&
	      (edges[e].target == edges[j].target))
	    {
	      anzPar++;
	      edges[e].type = type;
	      // type ist 2 oder 3, je nachdem, ob es sich um eine 
	      // echte parallele Kante handelt oder eine parallele Kante
	      // nach Umdrehen der Rueckwaertskanten
	      parEdges.append (e);
	      source = edges[e].source;
	      target = edges[e].target;

	      // Succ-Knoten loeschen (nodes aktualisieren)
	      q = nodes[source].succ.first ();
	      while (q != 0)
		{
		  if (nodes[source].succ (q) == target)
		    {
		      nodes[source].succ.del (q);
		      q = 0;
		      (nodes[source].outdeg)--;
		    }
		  else
		    nodes[source].succ.next (q);
		}
	      
	      // Pred-Knoten loeschen
	      q = nodes[target].pred.first ();
	      while (q != 0)
		{
		  if (nodes[target].pred (q) == source)
		    {
		      nodes[target].pred.del (q);
		      q = 0;
		      (nodes[target].indeg)--;
		    }
		  else
		    nodes[target].pred.next (q);
		}
	    }
	}
}  

int set_positions (int *nodeArray, SNode *nodes, int anzN, int dummies, 
		   FSSugiInfo info, int maxL)
///////////////////////////////////////////////////////////////////
//                                                               //
// Bestimmt die absolute vertikale Position aus der relativen    //
// vertikalen Position und verschiebt den Knoten an die neu      //
// berechnete Position.                                          //
// Rueckgabewert 1 bei Fehler.                                   //
//                                                               //
///////////////////////////////////////////////////////////////////
{
  FFPoint point;
  FFCoord max = 0.0;
  int n,
    i,
    *vertUnit;

  vertUnit = new int [maxL+1];
  if (vertUnit == 0)
    return (1);

  for (n = 1; n <= anzN+dummies; n++)
    vertUnit[nodes[n].level] = nodes[n].vertUnit;

  if ((info.sameSize == 1) || (info.sameSize == 2))
    for (n = 1; n <= anzN; n++)
      if (nodes[n].height > max)
	max = nodes[n].height;

  for (n = 1; n <= anzN+dummies; n++)
    {
      // in "vertUnit" zusaetzlicher Abstandsfaktor
      if (info.orient == 1)
	nodes[n].point.v[1] = (nodes[n].vertUnit - 1) * 
	  (info.yDistance + max);
      else
	nodes[n].point.v[1] = (nodes[n].vertUnit - 1) * 
	  (info.xDistance + max);  
    }

  for (n = anzN; n <= anzN+dummies; n++)
    {
      // in "vertUnit" zusaetzlicher Abstandsfaktor
      if (info.orient == 1)
	{
	  if (nodes[n].anzDum == 2)
	    for (i = nodes[n].level+1; i <= nodes[n].elevel; i++)
	      nodes[n].posList[i].v[1] = (vertUnit[i] - 1) * 
		(info.yDistance + max);
	}
      else
	if (nodes[n].anzDum == 2)
	  for (i = nodes[n].level+1; i <= nodes[n].elevel; i++)
	    nodes[n].posList[i].v[1] = (vertUnit[i] - 1) * 
	      (info.xDistance + max);
    }
  
  delete[] vertUnit;
  return (0);
}
  
int make_dummies (SNode *nodes, SEdge *edges, int anzN, int anzE,
		  DLList <int> &refEdges, DLList <int> &parEdges)
///////////////////////////////////////////////////////////////////
//                                                               //
// Dummy-Knoten bestimmen.                                       //
// Wichtig: Kanten in edges zeigen noch die alte Verbindung, nur //
// in nodes sind die durch dummies entstehenden Verbindungen     // 
// aktualisiert.                                                 // 
// Rueckgabewert: Anzahl dummies                                 //
//                                                               //
///////////////////////////////////////////////////////////////////
{
  int num=0, 
    i, 
    source, 
    target,
    tmpE;
  Pix p;
  
  for (i = 1; i <= anzE; i++)
    if (edges[i].type == 0)
      {
	source = edges[i].source;
	target = edges[i].target;
	
	// Falls genau ein Level zwischen source und target
	if (nodes[source].level == (nodes[target].level-2))
	  {
	    num++;
	    nodes[anzN+num].edgeNum = i;
	    nodes[anzN+num].level = nodes[source].level+1;
	    nodes[anzN+num].point = nodes[source].point;
	    nodes[anzN+num].dummy = 1;
	    nodes[anzN+num].anzDum = 1;
      
	    nodes[anzN+num].indeg = 1;
	    nodes[anzN+num].outdeg = 1;
	    nodes[anzN+num].succ.append (target);
	    nodes[anzN+num].pred.append (source);

	    // Succ-Knoten loeschen (In nodes die betreffenden Knoten 
	    // aktualisieren) 
	    p = nodes[source].succ.first ();
	    while (p != 0)
	      {
		if (nodes[source].succ (p) == target)
		  {
		    nodes[source].succ.del (p);
		    p = 0;
		  }
		else
		  nodes[source].succ.next (p);
	      }
  
	    // Pred-Knoten loeschen
	    p = nodes[target].pred.first ();
	    while (p != 0)
	      {
		if (nodes[target].pred (p) == source)
		  {
		    nodes[target].pred.del (p);
		    p = 0;
		  }
		else
		  nodes[target].pred.next (p);
	      }
	  
	    // Dummy eintragen
	    nodes[source].succ.append (anzN+num);
	    nodes[target].pred.append (anzN+num);
	    edges[i].nodeNr1 = anzN+num;
	  }
	else
	  {
	    if (nodes[source].level < (nodes[target].level-1))
	      {
		// also mehr als zwei Level zwischen den Knoten
		num++;
		nodes[anzN+num].edgeNum = i;
		nodes[anzN+num].level = nodes[source].level+1;
		nodes[anzN+num].point = nodes[source].point;
		nodes[anzN+num].dummy = 1;
		nodes[anzN+num].anzDum = 2;
		// Hier auch Endlevel (elevel) eintragen
		nodes[anzN+num].elevel = nodes[target].level-2;
		
		nodes[anzN+num].indeg = 1;
		nodes[anzN+num].outdeg = 1;
		nodes[anzN+num].succ.append (anzN+num+1);
		nodes[anzN+num].pred.append (source);
		
		nodes[anzN+num+1].edgeNum = i;
		nodes[anzN+num+1].level = nodes[target].level-1;
		nodes[anzN+num+1].point = nodes[target].point;
		nodes[anzN+num+1].dummy = 1;
		nodes[anzN+num+1].anzDum = 2;
		
		nodes[anzN+num+1].indeg = 1;
		nodes[anzN+num+1].outdeg = 1;
		nodes[anzN+num+1].succ.append (target);
		nodes[anzN+num+1].pred.append (anzN + num);
	      
		// Succ-Knoten loeschen 
		p = nodes[source].succ.first ();
		while (p != 0)
		  {
		    if (nodes[source].succ (p) == target)
		      {
			nodes[source].succ.del (p);
			p = 0;
		      }
		    else
		      nodes[source].succ.next (p);
		  }
		
		// Pred-Knoten loeschen
		p = nodes[target].pred.first ();
		while (p != 0)
		  {
		    if (nodes[target].pred (p) == source)
		      {
			nodes[target].pred.del (p);
			p = 0;
		      }
		    else
		      nodes[target].pred.next (p);
		  }
	      
		// Dummy eintragen
		nodes[source].succ.append (anzN+num);
		nodes[target].pred.append (anzN+num+1);
		edges[i].nodeNr1 = anzN+num;
		edges[i].nodeNr2 = anzN+num+1;
		// nun noch "num" aktualisieren
		num++;
	      }
	  }
      }
  
  // Nun fuer reflexive und parallele Kanten
  for (p = refEdges.first (); p != 0; refEdges.next (p))
    {
      num++;
      tmpE = refEdges (p);
      source = edges[tmpE].source;
      nodes[anzN+num].edgeNum = tmpE;
      nodes[anzN+num].level = nodes[source].level+1;
      nodes[anzN+num].point = nodes[source].point;
      nodes[anzN+num].dummy = 1;
      nodes[anzN+num].anzDum = 1;
      
      nodes[anzN+num].indeg = 1;
      nodes[anzN+num].outdeg = 0;
      nodes[anzN+num].pred.append (source);

      nodes[source].succ.append (anzN+num);
      (nodes[source].outdeg)++;
      edges[tmpE].nodeNr1 = anzN+num;
    }

  for (p = parEdges.first (); p != 0; parEdges.next (p))
    {
      tmpE = parEdges (p);
      source = edges[tmpE].source;
      target = edges[tmpE].target;
	
      // Falls genau ein Level zwischen source und target
      if (nodes[source].level == (nodes[target].level-2))
	{
	  num++;
	  nodes[anzN+num].edgeNum = tmpE;
	  nodes[anzN+num].level = nodes[source].level+1;
	  nodes[anzN+num].point = nodes[source].point;
	  nodes[anzN+num].dummy = 1;
	  nodes[anzN+num].anzDum = 1;
	  
	  nodes[anzN+num].indeg = 1;
	  nodes[anzN+num].outdeg = 1;
	  nodes[anzN+num].succ.append (target);
	  nodes[anzN+num].pred.append (source);

	  // Dummy eintragen
	  nodes[source].succ.append (anzN+num);
	  nodes[source].outdeg++;
	  nodes[target].pred.append (anzN+num);
	  nodes[target].indeg++;
	  edges[tmpE].nodeNr1 = anzN+num;
	}
      else
	{
	  if (nodes[source].level < (nodes[target].level-1))
	    {
	      num++;
	      nodes[anzN+num].edgeNum = tmpE;
	      nodes[anzN+num].level = nodes[source].level+1;
	      nodes[anzN+num].point = nodes[source].point;
	      nodes[anzN+num].dummy = 1;
	      nodes[anzN+num].anzDum = 2;
	      // Hier auch Endlevel (elevel) eintragen
	      nodes[anzN+num].elevel = nodes[target].level-2;
	      
	      nodes[anzN+num].indeg = 1;
	      nodes[anzN+num].outdeg = 1;
	      nodes[anzN+num].succ.append (anzN+num+1);
	      nodes[anzN+num].pred.append (source);
	      
	      nodes[anzN+num+1].edgeNum = tmpE;
	      nodes[anzN+num+1].level = nodes[target].level-1;
	      nodes[anzN+num+1].point = nodes[target].point;
	      nodes[anzN+num+1].dummy = 1;
	      nodes[anzN+num+1].anzDum = 2;
		
	      nodes[anzN+num+1].indeg = 1;
	      nodes[anzN+num+1].outdeg = 1;
	      nodes[anzN+num+1].succ.append (target);
	      nodes[anzN+num+1].pred.append (anzN + num);
	      
	      // Dummy eintragen
	      nodes[source].succ.append (anzN+num);
	      nodes[source].outdeg++;
	      nodes[target].pred.append (anzN+num+1);
	      nodes[target].indeg++;
	      edges[tmpE].nodeNr1 = anzN+num;
	      edges[tmpE].nodeNr2 = anzN+num+1;
	      // nun noch "num" aktualisieren
	      num++;
	    }
	}
    }

  return (num);
}

void init_positions (int *nodeArray, SNode *nodes, int anzN, int maxL, 
		     int maxS, int newmaxS, int dummies)
///////////////////////////////////////////////////////////////////
//                                                               //
// Initialisierung von "nodeArray".                              //
//                                                               //
///////////////////////////////////////////////////////////////////

{
  int n,
    m;
    
  for (n = 1; n <= anzN; n++)
    {
      // Nullte Spalte jeder Zeile enthaelt Zahl der Knoten im aktuellen 
      // Level.
      (nodeArray[(newmaxS+1)*nodes[n].level])++;
      nodeArray[(newmaxS+1)*nodes[n].level+ 
	       nodeArray[(newmaxS+1)*nodes[n].level]] = n;
    }  
  
  for (n = anzN + 1; n <= anzN + dummies; n++)
    {
      if (nodes[n].anzDum == 1)
	{
	  (nodeArray[(newmaxS+1)*nodes[n].level])++;
	  nodeArray[(newmaxS+1)*nodes[n].level+ 
		   nodeArray[(newmaxS+1)*nodes[n].level]] = n;
	}
      else
	{
	  for (m = nodes[n].level; m <= (nodes[n+1].level)-1; m++)
	    {
	      (nodeArray[(newmaxS+1)*m])++;
	      nodeArray[(newmaxS+1)*m + 
		       nodeArray[(newmaxS+1)*m]] = n;
	    }
	  
	  n++;
	  (nodeArray[(newmaxS+1)*nodes[n].level])++;
	  nodeArray[(newmaxS+1)*nodes[n].level+ 
		   nodeArray[(newmaxS+1)*nodes[n].level]] = n;	   
	}
    }  
}

void init_nondet (FFSet <FFNode> &allNodes, 
		  FFSet <FFEdge> &allEdges)
///////////////////////////////////////////////////////////////////
//                                                               //
// Value aller Knoten und Kanten (Sortierung selbiger!) auf 0    //
// setzen. (also nichtdeterministische Behandlung des Graphen)   //
//                                                               //
///////////////////////////////////////////////////////////////////
{  
  Pix p;
  
  for (p = allNodes.first (); p != 0; allNodes.next (p))
    allNodes (p)->setValue (0);
  for (p = allEdges.first (); p != 0; allEdges.next (p))
    allEdges (p)->setValue (0);
}

void sugiyama (FFGraph &graph, struct FSSugiInfo info, 
	       FFSet <FFNode> locNodes)
///////////////////////////////////////////////////////////////////
//                                                               //
//                                                               //
///////////////////////////////////////////////////////////////////
{
  int anzN, 
    anzE, 
    anzEN, // Gesamtzahl Knoten und Kanten, Kanten dreifach (fuer dummies) 
    anzRef, 
    anzPar1, 
    anzPar2, // Par2: parallele Kanten nach Umdrehen von Rueckwaertskanten
    maxL, //maxLevel 
    maxS, //maxSize
    newmaxS, 
    newmaxL,
    i, 
    j,
    dummies = 0, 
    test;
  FFSet <FFNode> allNodes;
  FFSet <FFEdge> allEdges;
  DLList <int> backEdges, 
    refEdges, 
    parEdges;
//  Pix p;
  FFGraphWindow *window;
  SNode *nodes;
  SEdge *edges;
  int *nodeArray; // Feld fuer relative Positionen der Knoten
  FFPoint point;

  window = graph->getWindow ();
  if (window)
    window->setUpdate (NO);

  graph->allNodes (allNodes);
  graph->allEdges (allEdges);
  anzN = allNodes.length ();
  anzE = allEdges.length ();
  anzEN = anzN + 3*anzE;

  // Falls Graph leer ist
  if (anzN == 0)
    {
      if (window)
	window->setUpdate (YES);
      return;
    }

  // Entfernung aller Kantenstuetzpunkte
  graph->straightenEdges (); 

  nodes = new SNode [anzEN+1];
  if (nodes == 0)
    {
      if (window)
	window->setUpdate (YES);
      return;
    }

  edges = new SEdge [anzE+1];
  if (edges == 0)
    {
      delete[] nodes;
      if (window)
	window->setUpdate (YES);
      return;
    }

  if (! info.deterministic)
    init_nondet (allNodes, allEdges);

  // Strukturen SNode und SEdge (nodes, edges) aufbauen.
  init_sgraph (graph, nodes, edges, anzN, anzE, anzEN, allNodes, allEdges); 

  // Paralelle und reflexive Kanten vorerst entfernen,
  // diese muessen erst bei der Erstellung der Dummies 
  // wieder beruecksichtigt werden.
  rem_ref_edges (nodes, edges, anzE, anzRef, refEdges);
  rem_par_edges (nodes, edges, anzE, anzPar1, 2, parEdges);
 
  sdfs (nodes, anzN, graph);
  init_backEdges (nodes, anzE, backEdges, edges);
  change_edges (nodes, edges, backEdges); 

  // Neu entstandene parallele Kanten temporaer beseitigen
  // (solche koennen durch  das Umkehren der Rueckwaertskanten 
  // enstehen).
  rem_par_edges (nodes, edges, anzE, anzPar2, 3, parEdges);

  slevel (nodes, anzN, graph);
  slevel_correct (nodes, edges, anzN, parEdges);

  // Falls Blaetter nach unten gesetzt werden sollen
  if (info.leavesDown != 0)
    {
      maxL = get_max_level (nodes, anzN+dummies);
      leaves_down (nodes, edges, anzN, parEdges, maxL);
    }

  dummies = make_dummies (nodes, edges, anzN, anzE, refEdges, parEdges); 
  maxL = get_max_level (nodes, anzN+dummies);
  maxS = get_max_size (nodes, anzN, dummies, maxL);  
  newmaxS = maxS+(2*anzRef)+anzPar1+anzPar2;
  newmaxL = maxL+1;

  // Fuer ueberlange Dummy-Kanten "posList" (Liste bzw. hier Array fuer
  // Positionen anlegen
  for (i = anzN; i <= anzN+dummies; i++)
    {
      nodes[i].posList = new FFPoint [maxL+1];
      if (nodes[i].posList == 0)
	{
	  delete[] edges;
	  delete[] nodes;
	  
	  if (window)
	    window->setUpdate (YES);
	  return;
	}
      
      for (j = 0; j <= maxL; j++)
	nodes[i].posList[j] = point;
    }

  nodeArray = new int [(newmaxL+1)*(newmaxS+1)];
  if (nodeArray == 0)
    {
      delete[] edges;
      delete[] nodes;

      if (window)
	window->setUpdate (YES);
      return;
    }

  for (i = 0; i <= (maxL+1)*(newmaxS+1)-1; i++)
    nodeArray[i] = 0;

  // "nodeArray" initialisieren
  init_positions (nodeArray, nodes, anzN, maxL, maxS, newmaxS, dummies);
  
  // Kreuzungsreduzierung
  test = reduce_crossings (nodes, maxL, maxS, newmaxS, anzN+dummies, 
			   info.crossingAlg, nodeArray, 
			   info.iteration1, info.iteration2);
  if (test != 0)
    {
      delete[] nodeArray;
      delete[] edges;
      delete[] nodes;

      if (window)
	window->setUpdate (YES);
      return;
    }
    
  // absolute Positionen der Knoten setzen
  test = improve_positions (nodeArray, nodes, maxL, anzN, dummies, 
			    info.xDistance, info.yDistance, info.offset,
			    info.orient, info.grid, info.sameSize, newmaxS); 
	
  if (test != 0)
    {
      delete[] nodeArray;
      delete[] edges;
      delete[] nodes;

      if (window)
	window->setUpdate (YES);
      return;
    }
  
  compute_edges_of_level (nodeArray, nodes, anzN, dummies, maxL, info.orient);
  test = set_positions (nodeArray, nodes, anzN, dummies, info, maxL);
  if (test != 0)
    {
      delete[] nodeArray;
      delete[] edges;
      delete[] nodes;

      if (window)
	window->setUpdate (YES);
      return;
    }

  delete[] nodeArray;

  finish_sgraph (graph, nodes, edges, anzN, anzE, dummies, backEdges, 
		 info.xDistance, info.yDistance, info.orient);

  for (i = anzN; i <= anzN+dummies; i++)
    delete[] nodes[i].posList;
  
  delete[] edges;
  delete[] nodes;
  
  if (window)
    {
      window->setUpdate (YES);
      window->updateGraph ();
      window->adjustWindowSize ();
    }
}
