/*
// Copyright 1994 Carsten Friedrich, Universitaet Passau
//
// $Author: friedric $
// $Date: 1995/12/18 12:22:13 $
// $RCSfile: FFTclGraphInsp.h,v $
// $Revision: 1.2 $
// $State: Exp $
//
*/

#ifndef FF_TCL_GRAPH_INSPECTOR
#define FF_TCL_GRAPH_INSPECTOR

#include "FFTclInspector.h"
#include "FFGraphClass.h"
#include "FFGraph.h"
#include "FFWindow.h"
#include "FFNodeClass.h"
#include "FFEdgeClass.h"

class FFTclGraphInsp :  public FFTclInspector
{
  FFGraph graph;
  FFGraphicNode *currentN;
  FFGraphicEdge *currentE;
  int nodeIndex,
		 edgeIndex,
		 graphIndex;

  AVLSet<String> menus;
		 
	void _buildGraphMenu();
	void _buildNodeMenu();
	void _buildEdgeMenu();
	void _clearMenus();
public:
	void buildMenus();
  void inspect();
  void inspectGroup(const FFSet<FFGraphicNode *>&);
  void inspectNode(FFNode n);
  void inspectEdge(FFEdge e);
  int recall(Tcl_Interp *i, int k, const char **c);

  FFTclGraphInsp(FFGraphLabel *g);
  ~FFTclGraphInsp(){}
};

#endif
