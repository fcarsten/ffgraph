#ifndef FFGRAPH_MODERN_GML_GRAPH_ADAPTER_H
#define FFGRAPH_MODERN_GML_GRAPH_ADAPTER_H

#include "gml_events.h"

class FFGraphClass;

namespace ffgraph_gml {

// Shared event-to-graph bridge for the production modern GML importer and
// differential comparison against the retained GTControl implementation.
struct ApplyResult {
  bool ok = false;
  std::string error;
};

// Syntax failures and semantic validation failures leave the supplied graph
// untouched. The preflight simulates cursor and node-ID resolution before
// committing events. An empty successful event stream is valid.
// This is not a general strong exception guarantee: allocation failures or
// exceptions from graph callbacks during the commit phase are not rolled back.
// resetExistingNodeIds matches FFGraphClass::load()'s historical behaviour
// on successful file loads. The reset is deferred until after semantic
// preflight, so a rejected import leaves existing node IDs untouched.
ApplyResult applyToGraph(const Result& parsed, FFGraphClass& graph,
                         bool resetExistingNodeIds = false);

}  // namespace ffgraph_gml
#endif
