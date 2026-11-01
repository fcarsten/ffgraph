#include "rome_graph_adapter.h"
#include <ffgraph/FFGraphClass.h>
#include <ffgraph/FFNodeClass.h>
#include <ffgraph/FFEdgeClass.h>

namespace ffgraph_rome {
ApplyResult applyToGraph(const Result& parsed, FFGraphClass& graph, bool resetIds) {
  if (!parsed.ok) return {false, parsed.error, {}};
  ApplyResult result;
  if (resetIds)
    for (Pix p = graph.allNodes().first(); p; graph.allNodes().next(p))
      graph.allNodes()(p)->setValue(-1);
  graph._setDirected(NO);
  for (const auto& row : parsed.records) {
    if (row.kind == Kind::Node) graph.addNode()->setValue(row.id);
    else {
      // Use the same live lookup as legacy: duplicates, existing merge nodes
      // and callback mutations must not be resolved by an independent ID map.
      auto* source = graph.nodeWithValue(row.source);
      auto* target = graph.nodeWithValue(row.target);
      if (!source || !target)
        result.missing.push_back({row.id, row.source, row.target, row.line});
      else graph.addEdge(source, target)->setValue(row.id);
    }
  }
  result.ok = true;
  return result;
}
} // namespace ffgraph_rome
