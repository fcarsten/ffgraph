#ifndef IMPROVE_POSITION
#define IMPROVE_POSITION

#include <globdefs.h>
#include <math.h>
#include <DLList.h>
#include <SLList.h>
#include <stream.h>
#include <String.h>

#include <ffgraph/FFlayout_algorithmen.h>
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

int improve_positions (int *nodeArray, 
		       SNode *nodes, 
		       int maxL, 
		       int anzN, 
		       int dummies, 
		       FFCoord xDistance,
		       FFCoord yDistance,
		       FFPoint offset,
		       int orient,
		       BOOL grid,
		       int sameSize,
		       int newmaxS);


#endif

