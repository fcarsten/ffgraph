#ifndef LAYOUT_ALGORITHMEN
#define LAYOUT_ALGORITHMEN

#include <globdefs.h>
#include <math.h>
#include <DLList.h>
#include <SLList.h>
#include <stream.h>
#include <String.h>

#include <ffgraph/FFGraphClass.h>
#include <ffgraph/FFGraph.h>
#include <ffgraph/FFNodeClass.h>
#include <ffgraph/FFEdgeClass.h>
#include <ffgraph/ffError.h>
#include <ffgraph/FFPoint.h>
#include <ffgraph/FFGraphicEdge.h>
#include <ffgraph/FFGraphicNode.h>

#include <container/AVLSet.h>
#include <container/VOHSet.h>
#include <container/VHSet.h>
#include <container/AVLMap.h>
#include <container/MFStack.h>


////////////////////////////////////////////////////////////////////////////
// Datenstruktur                     
// Knoten -> interne Informationen 
//        -> Liste der Vorgaenger-/Nachfolgerknoten
//        -> Dummy-Knoten fuer ueberlange Kanten
////////////////////////////////////////////////////////////////////////////
class SNode
{
  public:

  // Folgendes fuer alle Knoten:
  BOOL dummy; //ja, falls Knoten ein dummy ist
  int indeg; 
  int outdeg;
  int attrib; // zur internen Verwendung
  FFPoint point;
  FFCoord height; // Hoehe eines Knotens
  FFCoord width; // Breite eines Knotens
  DLList <int> pred; // Verweis auf Vorgaenger
  DLList <int> succ; // Verweis auf Nachfolger
  int vertUnit; // zusaetzlicher vertikaler Abstand fuer Layout
		// aufgrund der Anzahl der Kanten von/zu einem Knoten
  
  // folgendes nur fuer echte Knoten
  FFNode node; // Verweis auf Originalknoten
  int dfs; // dfs- und 
  int comp; // comp-Nummer fuer die Tiefensuche
  int level; 
  int anzDum; // Anzahl der dummies fuer eine Kante
	      // (1 oder 2)
  BOOL leaf; // Knoten ist Blatt eines Baumes oder nicht

  // folgendes nur fuer Dummy-Knoten
  int edgeNum; // Verweis auf Originalkante 
  int maxPos;
  int elevel;
  FFPoint *posList;

  SNode ();
  ~SNode ();
};


////////////////////////////////////////////////////////////////////////////
// Datenstruktur                     
// Kanten -> interne Informationen 
//        -> Source-/Destination-Knoten
////////////////////////////////////////////////////////////////////////////
class SEdge
{
  public:
  FFEdge edge; // Verweis auf Originalkante
  int attrib; // zur internen Verwendung
  FFCoord width; // Breite einer Kante 
  int type; // normale Kante: 0, reflexive Kante: 1, 
	    // parallele Kante: 2, parallele Kante nach 
	    // Umdrehen der Rueckwaertskanten: 3 
  int source;
  int target;
  int nodeNr1; // falls dummy-Knoten noetig, seine Nummer;
  int nodeNr2;

  SEdge ();
  ~SEdge ();
};


void init_sgraph (FFGraph &graph, 
		  SNode *nodes, 
		  SEdge *edges, 
		  int anzN, 
		  int anzE, 
		  int anzEN,
		  FFSet <FFNode> &allNodes, 
		  FFSet <FFEdge> &allEdges);
void finish_sgraph (FFGraph &graph, 
		    SNode *nodes, 
		    SEdge *edges,
		    int anzN, 
		    int anzE, 
		    int dummies,
		    DLList <int> &backEdges,
		    FFCoord xDistance,
		    FFCoord yDistance,
		    int orient);
void compute_edges_of_level (int *nodeArray, 
			     SNode *nodes, 
			     int anzN, 
			     int dummies, 
			     int maxL, 
			     int orient);
int get_max_level (SNode *nodes, 
		   int anzND);
int get_max_size (SNode *nodes, 
		  int anzN, 
		  int dummies,
		  int maxL);
void    number_nodes (FFGraph& graph, 
		      int* oldValues);
void    number_edges (FFGraph& graph, 
		      int* oldValues);

#endif




