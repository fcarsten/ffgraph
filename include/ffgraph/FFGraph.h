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

// A graph-owned label (or its inspector) must not hold an owning FFGraph
// handle: the graph owns the label, so that would create a reference cycle.
// This deliberately non-copyable view preserves the FFGraph interface for
// legacy layout algorithms that accept FFGraph&. Any FFGraph copied from the
// view becomes a normal, reference-counted owning handle.
class FFGraphObserver : public FFGraph
{
public:
  FFGraphObserver() : FFGraph(YES) {}

  void observe(const FFGraph& owner)
  {
    graph = owner.operator->();
  }

  // Unlike a normal FFGraph, an unbound observer must not manufacture a
  // graph that it cannot own or release.
  FFGraphClass* operator->() const { return graph; }

  // FFGraph's destructor would normally decrement the graph's reference
  // count. The observed graph is not owned by this handle.
  ~FFGraphObserver() override { graph = nullptr; }

  FFGraphObserver(const FFGraphObserver&) = delete;
  FFGraphObserver& operator=(const FFGraphObserver&) = delete;
  FFGraphObserver& operator=(const FFGraph&) = delete;
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
