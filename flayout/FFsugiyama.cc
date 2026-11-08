/*
// Copyright 1994 Falk Schreiber, Universitaet Passau
//
*/

#include <ffgraph/FFsugiyama.h>
#include <map>
#include <vector>
#include <memory>
#include "FFsugi_sizes.h"
#include "FFsugi_numeric.h"
#include "FFsugi_publication.h"

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
  struct Frame { int node; Pix next; };
  std::vector<Frame> stack{{k,nodes[k].succ.first()}};
  while (!stack.empty()) {
    auto& frame = stack.back();
    if (!frame.next) {
      const int finished = frame.node;
      stack.pop_back();
      // The caller historically assigns the root's completion number.
      if (!stack.empty()) nodes[finished].comp = ++comp;
      continue;
    }
    const int child = nodes[frame.node].succ(frame.next);
    nodes[frame.node].succ.next(frame.next);
    if (nodes[child].dfs == 0) {
      nodes[child].dfs = ++dfs;
      stack.push_back({child,nodes[child].succ.first()});
    }
  }
}

static void sdfs_from (SNode *nodes, int anzN, FFNode start)
///////////////////////////////////////////////////////////////////
//                                                               //
// Tiefensuche mit zusaetzlicher Berechnung der dfs- und comp-   //
// Nummern.                                                      //
// Expliziter Stack erhaelt die rekursive Besuchsreihenfolge.    //
//                                                               //
///////////////////////////////////////////////////////////////////
{
  int dfs = 0,
    comp = 0,
    i,
    startN;
  
  if (start != nullptr)
    {
      startN = start->getValue ();
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

// Retain the historical helper API for callers that always honor graph start.
void sdfs(SNode* nodes, int count, FFGraph& graph) {
  sdfs_from(nodes,count,graph->getStartNode());
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
  struct Frame { int node; Pix next; };
  std::vector<Frame> stack{{k,nodes[k].succ.first()}};
  while (!stack.empty()) {
    auto& frame = stack.back();
    if (!frame.next) { stack.pop_back(); continue; }
    const int child = nodes[frame.node].succ(frame.next);
    nodes[frame.node].succ.next(frame.next);
    const int level = ffgraph_sugiyama::checkedAdd(nodes[frame.node].level,1);
    if (nodes[child].level < level) {
      nodes[child].level = level;
      nodes[child].attrib = 1;
      stack.push_back({child,nodes[child].succ.first()});
    }
  }
}

static void slevel_from (SNode *nodes, int anzN, FFNode start)
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
  
  if (start != nullptr)
    {
      startN = start->getValue ();
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

void slevel(SNode* nodes, int count, FFGraph& graph) {
  slevel_from(nodes,count,graph->getStartNode());
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

  // Build candidate lists in original edge-index order; retain the outer
  // traversal so group append order and the earliest representative match.
  std::map<std::pair<int,int>,std::vector<int>> candidates;
  for (int i = 1; i <= anzE; ++i) if (edges[i].type == 0)
    candidates[{edges[i].source,edges[i].target}].push_back(i);
  anzPar = 0;
  for (j = 1; j <= anzE; j++)
    if (edges[j].type == 0)
      for (int candidate : candidates.at({edges[j].source,edges[j].target}))
	{
          e = candidate;

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

  std::unique_ptr<int[]> vertOwner(new int[ffgraph_sugiyama::arrayCount<int>(
      ffgraph_sugiyama::checkedAdd(maxL,1))]);
  vertUnit = vertOwner.get();

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

namespace {
struct UpdateScope {
  FFGraphWindow* window;
  BOOL previous;
  bool active = true;
  explicit UpdateScope(FFGraphWindow* w) : window(w), previous(w ? w->setUpdate(NO) : YES) {}
  void restore() { if (active && window) window->setUpdate(previous); active = false; }
  ~UpdateScope() { restore(); }
};
struct ScratchIds {
  std::vector<std::pair<FFNode,int>> nodes;
  std::vector<std::pair<FFEdge,int>> edges;
  bool published = false;
  bool restored = false;
  ScratchIds(const FFSet<FFNode>& ns, const FFSet<FFEdge>& es) {
    nodes.reserve(ns.length()); edges.reserve(es.length());
    for (Pix p = ns.first(); p; ns.next(p)) nodes.emplace_back(ns(p),ns(p)->getValue());
    for (Pix p = es.first(); p; es.next(p)) edges.emplace_back(es(p),es(p)->getValue());
  }
  void restore() noexcept {
    if (!published && !restored) {
      for (auto [node,id] : nodes) node->setValue(id);
      for (auto [edge,id] : edges) edge->setValue(id);
      restored = true;
    }
  }
  ~ScratchIds() { restore(); }
};
}

void sugiyama(FFGraph& graph, FSSugiInfo info, FFSet<FFNode> locNodes)
{
  using namespace ffgraph_sugiyama;
  FFGraphWindow* window = graph->getWindow();
  UpdateScope updates(window);
  try {
    FFSet<FFNode> allNodes;
    FFSet<FFEdge> allEdges;
    graph->allNodes(allNodes); graph->allEdges(allEdges);
    const int anzN = allNodes.length(), anzE = allEdges.length();
    if (anzN == 0) return;
    for (auto distance : {info.xDistance,info.yDistance,info.zDistance})
      if (!std::isfinite(distance) || distance < 1) throw NumericError();
    coordinateInt(info.xDistance); coordinateInt(info.yDistance);
    for (auto value:info.offset.v) if (!std::isfinite(value)) throw NumericError();
    for (Pix p=allNodes.first();p;allNodes.next(p)) if (auto* g=allNodes(p)->getGraphicNode()) {
      finitePoint(g->getCoords());
      for (int i=0;i<3;++i)
        if (!std::isfinite(g->getRange().v[i]) || g->getRange().v[i]<0) throw NumericError();
    }
    for (Pix p=allEdges.first();p;allEdges.next(p)) if (auto* g=allEdges(p)->getGraphicEdge())
      for (int i=0;i<g->anzPoints();++i) finitePoint(g->getPointAt(i));
    const int nodeCount = arrayCount<SNode>(scratchNodes(anzN,anzE));
    const int edgeCount = arrayCount<SEdge>(checkedAdd(anzE,1));
    ScratchIds originalIds(allNodes,allEdges);
    std::unique_ptr<SNode[]> nodeOwner(new SNode[nodeCount]);
    std::unique_ptr<SEdge[]> edgeOwner(new SEdge[edgeCount]);
    auto* nodes = nodeOwner.get(); auto* edges = edgeOwner.get();
    DLList<int> backEdges, refEdges, parEdges;
    if (!info.deterministic) init_nondet(allNodes,allEdges);
    init_sgraph(graph,nodes,edges,anzN,anzE,nodeCount-1,allNodes,allEdges);
    int anzRef, anzPar1, anzPar2;
    rem_ref_edges(nodes,edges,anzE,anzRef,refEdges);
    rem_par_edges(nodes,edges,anzE,anzPar1,2,parEdges);
    sdfs_from(nodes,anzN,info.useStartNode ? graph->getStartNode() : nullptr);
    init_backEdges(nodes,anzE,backEdges,edges);
    change_edges(nodes,edges,backEdges);
    rem_par_edges(nodes,edges,anzE,anzPar2,3,parEdges);
    slevel_from(nodes,anzN,info.useStartNode ? graph->getStartNode() : nullptr);
    slevel_correct(nodes,edges,anzN,parEdges);
    if (info.leavesDown) leaves_down(nodes,edges,anzN,parEdges,get_max_level(nodes,anzN));
    const int dummies = make_dummies(nodes,edges,anzN,anzE,refEdges,parEdges);
    const int count = checkedAdd(anzN,dummies);
    const int maxL = get_max_level(nodes,count);
    const int maxS = get_max_size(nodes,anzN,dummies,maxL);
    const int newmaxS = checkedAdd(checkedAdd(maxS,checkedProduct(2,anzRef)),checkedAdd(anzPar1,anzPar2));
    const int entries = arrayCount<int>(levelEntries(maxL,newmaxS));
    const int pointCount = arrayCount<FFPoint>(checkedAdd(maxL,1));
    // Check every int-indexed reduction slab before allocating route/level arrays.
    if (info.crossingAlg == 0 && maxL > 0)
      arrayCount<int>(reductionEntries(maxL,checkedAdd(maxS,1)));
    std::vector<std::unique_ptr<FFPoint[]>> positions(checkedAdd(dummies,1));
    for (int i = anzN; i <= count; ++i) {
      positions[i-anzN].reset(new FFPoint[pointCount]);
      nodes[i].posList = positions[i-anzN].get();
    }
    std::unique_ptr<int[]> levelOwner(new int[entries]());
    auto* nodeArray = levelOwner.get();
    init_positions(nodeArray,nodes,anzN,maxL,maxS,newmaxS,dummies);
    if (reduce_crossings(nodes,maxL,maxS,newmaxS,count,info.crossingAlg,nodeArray,info.iteration1,info.iteration2))
      throw SizeError();
    if (improve_positions(nodeArray,nodes,maxL,anzN,dummies,info.xDistance,info.yDistance,
                          FFPoint(),info.orient,info.grid,info.sameSize,newmaxS)) throw std::bad_alloc();
    compute_edges_of_level(nodeArray,nodes,anzN,dummies,maxL,info.orient);
    if (set_positions(nodeArray,nodes,anzN,dummies,info,maxL)) throw std::bad_alloc();
    const int crossAxis = info.orient == 1 ? 0 : 1, layerAxis = 1-crossAxis;
    for (int n = 1; n <= count; ++n) {
      nodes[n].point.v[0] += info.offset.v[crossAxis];
      nodes[n].point.v[1] += info.offset.v[layerAxis];
      if (nodes[n].anzDum == 2)
        for (int level = nodes[n].level+1; level <= nodes[n].elevel; ++level) {
          nodes[n].posList[level].v[0] += info.offset.v[crossAxis];
          nodes[n].posList[level].v[1] += info.offset.v[layerAxis];
        }
    }
    for (int n=1;n<=count;++n) {
      finitePoint(nodes[n].point);
      if (nodes[n].anzDum==2)
        for (int level=nodes[n].level+1;level<=nodes[n].elevel;++level) finitePoint(nodes[n].posList[level]);
    }
    Publication publication(allNodes,allEdges);
    publication.begin();
    try {
      graph->straightenEdges();
      finish_sgraph(graph,nodes,edges,anzN,anzE,dummies,backEdges,info.xDistance,info.yDistance,info.orient);
      updates.restore();
      if (window) { window->updateGraph(); window->adjustWindowSize(); }
      publication.commit();
      originalIds.published = true; // Preserve historical successful scratch-ID behavior.
    } catch (...) {
      publication.rollback(); // No allocation or callback needed to restore model geometry.
      originalIds.restore();
      updates.restore();
      try { publication.refresh(window); } catch (...) { /* Model is restored even if repaint fails. */ }
      if (window) window->setUpdate(updates.previous);
      throw;
    }
  } catch (const NumericError& error) {
    updates.restore();
    ffError(USER_ERROR,error.what());
  } catch (const SizeError&) {
    updates.restore();
    ffError(USER_ERROR,GRAPH_TOO_LARGE);
  } catch (const std::bad_alloc&) {
    updates.restore();
    ffError(USER_ERROR,NO_MEMORY);
  }
}
