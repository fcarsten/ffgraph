#ifndef SPRINGEMBEDDER
#define SPRINGEMBEDDER


#include <math.h>
#include <stream.h>
#include <string.h>
#include "globdefs.h"
#include "FFGraphClass.h"

#include <container/AVLSet.h>
#include <container/VOHSet.h>
#include <container/VHSet.h>

#include "ffError.h"
#include "FFlayout_algorithmen.h"
#include <container/AVLMap.h>
#include "FFGraphicNode.h"
#include "FFGraphWindow.h"
#include "FFPoint.h"

struct FSSpringInfo
{ 
 BOOL    animation, useEdgeWeight; 
 int     maxIteration, animationIntervals, dimension, quality;
 BOOL    stopBit;
 FFCoord optDistance;
 FFPoint offset;
};


void springembedder (FFGraph& Graph, const FSSpringInfo& info, FFSet<FFNode>);

#endif
