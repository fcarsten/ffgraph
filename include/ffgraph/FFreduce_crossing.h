#ifndef REDUCE_CROSSINGS
#define REDUCE_CROSSINGS


#include <stream.h>
#include <string.h>
#include <SLList.h>

#include <ffgraph/FFGraphClass.h>
#include <ffgraph/FFNodeClass.h>
#include <ffgraph/FFGraph.h>
#include <ffgraph/FFlayout_algorithmen.h>
#include <ffgraph/ffError.h>
#include <ffgraph/FFsugiyama.h>

#include <container/AVLSet.h>
#include <container/VOHSet.h>
#include <container/VHSet.h>
#include <container/AVLMap.h>

int reduce_crossings (SNode *nodes, 
		      int maxL, 
		      int maxS, 
		      int newmaxS, 
		      int anzN,
		      int cross_alg, 
		      int *nodeArray,
		      int iteration1, 
		      int iteration2);
#endif

