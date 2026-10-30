#ifndef FF_SPRING
#define FF_SPRING

#include <globdefs.h>
#include <ffgraph/FFGraph.h>
#include <ffgraph/FFNodeClass.h>
#include <container/AVLSet.h>
#include <container/VOHSet.h>
#include <container/VHSet.h>

#include "FFlayout_algorithmen.h"
#include <container/AVLMap.h>
#include "FFGraphicNode.h"
#include "FFGraphWindow.h"

// #include <ffgraph/FFspringembedder.h>
#include <ffgraph/FFLayoutLabel.h>
#include <String.h>

#define ffSpringType String("FFSpringembedder")

struct FSSpringNodes
///////////////////////////////////////////////////////////////////
//                                                               //
// Struktur zur Speicherung der Knoten + zusaetzlicher           //
// Information zwecks schnellem Zugriff fuer Springembedder.     //
//                                                               //
///////////////////////////////////////////////////////////////////
{
  FFNode         node;
  int            attrib;
  FFPoint        coord;
  BOOL           move;
 //  FFSet <int>    succ;     // nur Knotennummern
 //  FFSet <int>    pred;     // nur Knotennummenr
  FFSet <int>    conNodes; // nur Knotennummern
  FFSet <FFEdge> toSucc;   // Kanten zu Vorgaenger und Nachfolger
  FFSet <FFEdge> fromPred;
};

class FFSpring : public FFLayoutLabel
{
 BOOL    animation, useEdgeWeight; 
 int     maxIteration, animationIntervals, dimension, quality;
 BOOL    stopBit;
 FFCoord optDistance;
 FFPoint offset;
 FSSpringNodes  *nodes;
 
public:
 FFSpring ();
 virtual ~FFSpring ();
 
 virtual BOOL hasOwnInspector() const {return YES;} 
 
 virtual void init(const FFGraph& g);
 virtual BOOL parse(const String&) {return YES;}
 virtual String unparse() {return String("");}
 virtual FFGraphLabel* proto() const {return new FFSpring;}
 virtual BOOL isInspectable() const {return YES;}
 virtual FFGraphWindow *getWindow() const {return graph->getWindow();}
 
 BOOL    getAnimation ();
 void    setAnimation (BOOL);
 
 int     getQuality ();
 BOOL    setQuality (int);
 
 BOOL    getUseEdgeWeight ();
 void    setUseEdgeWeight (BOOL);
 
 int     getMaxIteration ();
 BOOL    setMaxIteration (int);
 
 int     getAnimationIntervals ();
 BOOL    setAnimationIntervals (int);
 
 FFCoord getOptDistance ();
 BOOL    setOptDistance (FFCoord);
 
 FFCoord getxoff ();
 BOOL    setxoff (FFCoord);
 FFCoord getyoff ();
 BOOL    setyoff (FFCoord);
 FFCoord getzoff ();
 BOOL    setzoff (FFCoord);
 
 BOOL    getDimension ();
 void    setDimension (BOOL);
 
 void stop();
 
 void    layout (FFSet <FFNodeClass *>&);
 void    layout ();
 
 // Layout Routinen
 
 void init_FSSpringNodes (const FFSet <FFNode> &);
 void init_forces (FFPoint *, int);
 void compute_forces (FFCoord, FFPoint *, int);
 void compute_forces_edges (FFCoord , FFPoint *, int);
 FFCoord compute_maxForce (FFPoint *, int , FFCoord *);
 void compute_nodePosition (FFPoint *, int, FFCoord, FFCoord *);
 
 FFCoord cool (FFCoord , FFCoord , FFCoord &, FFCoord &, FFCoord &, 
               FFCoord &, int &);
 void write_nodePosition (int);
 void change_optDist (FFCoord &);
 void springembedder_layout (const FFSet <FFNode> &);
 void springembedder(const FFSet <FFNode>&);
};

#endif
