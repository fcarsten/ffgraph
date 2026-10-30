//-*-Mode: C++;-*-
#ifndef _FF_TI_GRAPH_LABEL_
#define _FF_TI_GRAPH_LABEL_

#include <ffgraph/FFGraph.h>
#include <ffgraph/FFNodeClass.h>
#include <ffgraph/FFEdgeClass.h>
#include <ffgraph/FFGraphLabel.h>
#include <container/SLStack.h>

//---- newclass -----------------------------------------------------------

#define ffTIGraphLabelType String("FFTIGraphLabel")

class FFTIGraphLabel : public FFGraphLabel
{
 BOOL isLoading;
 int tiNodeLabel;
 int tiEdgeLabel;
 int aktuDfs;
 int aktuComp;
 int aktuLow;

 FFNode sNode;
 FFNode tNode;
 
 FFSet<FFNode> todo;
 
 void _dfsDir(FFNode);
 void _dfsUndir(FFNode);
 void _markArtPkt();
 void _adjEdgeDir();
 void _adjEdgeDirST();
 void _stNumbering();
 void _pushPath(SLStack<FFNode>&, FFNode, FFEdge);
 
public:
 FFTIGraphLabel();
 void dfs();
 
 virtual FFGraphLabel *proto() const {return new FFTIGraphLabel;}
 virtual void init(const FFGraph&);
 
 virtual void event(labelEvent, labelEventStruct *);
 
 virtual ~FFTIGraphLabel() {}
};

#endif
