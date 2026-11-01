#ifndef FFGRAPH_MODERN_GML_GRAPH_ADAPTER_H
#define FFGRAPH_MODERN_GML_GRAPH_ADAPTER_H

#include "gml_events.h"

class FFGraphClass;

namespace ffgraph_gml {

// Experimental bridge for comparing the new parser with GTControl. Not
// selected by FFGraphClass::load; do not use on untrusted input until the
// semantic parity suite and error handling have been fully validated.
struct ApplyResult {
  bool ok = false;
  std::string error;
};

// Syntax failures and semantic validation failures leave the supplied graph
// untouched. The preflight simulates cursor and node-ID resolution before
// committing events. An empty successful event stream is valid.
// This is not a general strong exception guarantee: allocation failures or
// exceptions from graph callbacks during the commit phase are not rolled back.
ApplyResult applyToGraph(const Result& parsed, FFGraphClass& graph);

}  // namespace ffgraph_gml
#endif
