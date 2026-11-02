#ifndef FFGRAPH_MODERN_ROME_GRAPH_ADAPTER_H
#define FFGRAPH_MODERN_ROME_GRAPH_ADAPTER_H

#include "rome_records.h"
class FFGraphClass;
namespace ffgraph_rome {
struct MissingEndpoint {
  int edge, source, target;
  std::size_t line;
};
struct ApplyResult {
  bool ok = false;
  std::string error;
  std::vector<MissingEndpoint> missing;
};
// Internal production Rome adapter. Rejected parses leave the graph unchanged; replay can
// still be partial if allocation or application label callbacks throw.
// resetIds=true models the successful public file-import ID reset.
ApplyResult applyToGraph(const Result& parsed, FFGraphClass& graph, bool resetIds = false);
} // namespace ffgraph_rome
#endif
