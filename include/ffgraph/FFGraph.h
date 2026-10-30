/*
// Copyright 1994 Carsten Friedrich, Universitaet Passau
//
// $Author: friedric $
// $Date: 1995/11/01 12:02:04 $
// $RCSfile: FFGraph.h,v $
// $Revision: 1.1 $
// $State: Exp $
//
*/

#ifndef FF_GRAPH
#define FF_GRAPH

#include "globdefs.h"

#include <String.h>
#include "FFGraphClass.h"

class FFGraphClass;

class FFGraph
{
protected:
 FFGraphClass *graph;
 BOOL bare;
 
 virtual FFGraphClass *createGraph() const;
 FFGraph& assign (const FFGraph&);

 friend class FFGraphClass;
 public:

  FFGraph(FFGraphClass *);
  FFGraph(BOOL = NO);
  FFGraph(const FFGraph&);
  virtual ~FFGraph();

  FFGraph& operator = (const FFGraph&);
  int operator == (const FFGraph&) const;
  int operator != (const FFGraph&) const;
  
  FFGraphClass * operator -> () const;
};

inline FFGraphClass *FFGraph::operator->() const
{
 FFGraph *self = const_cast<FFGraph *>(this);
 
 if(!self->graph)
  {
   self->graph= createGraph();
   self->graph->incNumOfReps();
  }
 
 return graph;
}

#endif
