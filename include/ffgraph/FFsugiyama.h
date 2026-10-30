#ifndef SUGIYAMA
#define SUGIYAMA

#include <globdefs.h>
#include <stream.h>
#include <string.h>
#include <math.h>
#include <DLList.h>

#include <ffgraph/FFNodeClass.h>
#include <ffgraph/FFGraphClass.h>
#include <ffgraph/FFGraph.h>
#include <ffgraph/ffError.h>
#include <ffgraph/FFlayout_algorithmen.h>
#include <ffgraph/FFreduce_crossing.h>
#include <ffgraph/FFimprove_position.h>
#include <ffgraph/FFGraphWindow.h>
#include <ffgraph/FFTclWindow.h>

#include <container/AVLSet.h>
#include <container/VOHSet.h>
#include <container/VHSet.h>
#include <container/AVLMap.h>
#include <container/SLStack.h>

struct FSSugiInfo
{ 
    int crossingAlg, 
      maxSize, 
      iteration1, 
      iteration2, 
      sameSize, 
      orient,
      dimension;
    FFPoint offset;
    FFCoord xDistance, 
      yDistance, 
      zDistance, 
      dummyOffset;
    BOOL useStartNode, 
      useEndNodes, 
      keepEdgePoints,
      grid, 
      leavesDown,
      sortLeaves,
      deterministic;
};

void sugiyama (FFGraph& graph, 
	       struct FSSugiInfo info, 
	       FFSet <FFNode> locNodes);
                     
#endif








