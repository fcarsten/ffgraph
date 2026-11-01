#ifndef FFGRAPH_MODERN_FF_GRAPH_ADAPTER_H
#define FFGRAPH_MODERN_FF_GRAPH_ADAPTER_H
#include "ff_events.h"
class FFGraphClass;
namespace ffgraph_ff {
// Shared by the selectable modern FF importer and compatibility tests.
// Replay only successful parse() results. Like legacy load(), optional ID reset
// is for merge imports. Syntax failures leave the destination unchanged.
// Callback/allocation exceptions during replay are not transactional.
bool applyToGraph(const Result&, FFGraphClass&, bool resetExistingNodeIds = true);
} // namespace ffgraph_ff
#endif
