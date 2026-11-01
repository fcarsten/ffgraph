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

// Only successful parses are applied: an invalid/truncated parse leaves the
// supplied graph untouched. An empty successful event stream is valid.
ApplyResult applyToGraph(const Result& parsed, FFGraphClass& graph);

}  // namespace ffgraph_gml
#endif
