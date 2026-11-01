#include "gml_graph_adapter.h"

#include <ffgraph/FFEdgeClass.h>
#include <ffgraph/FFGraphClass.h>
#include <ffgraph/FFGraphicNode.h>
#include <ffgraph/FFNodeClass.h>

#include <deque>
#include <optional>
#include <unordered_map>

namespace ffgraph_gml {

namespace {
// Model only the parts of GTParser's cursor state that can cause semantic
// rejection.  In particular, new nodes start with value -1, and the ID map
// retains old keys when a node receives a second `id` attribute.  Neither
// graph members nor graph labels are touched during this pass.
struct ShadowNode { int value; };
struct ShadowEdge { ShadowNode* source; ShadowNode* target; };

ApplyResult preflight(const Events& events, FFGraphClass& graph,
                      bool resetExistingNodeIds) {
  // deque keeps node references stable while the synthetic graph grows.
  std::deque<ShadowNode> nodes;
  for (Pix p = graph.allNodes().first(); p; graph.allNodes().next(p))
    nodes.push_back({resetExistingNodeIds ? -1 : graph.allNodes()(p)->getValue()});

  std::unordered_map<int, ShadowNode*> by_id;
  ShadowNode* current_node = nullptr;
  std::optional<ShadowEdge> current_edge;

  const auto resolve = [&](int value) -> ShadowNode* {
    const auto mapped = by_id.find(value);
    if (mapped != by_id.end()) return mapped->second;
    // This models graph.nodeWithValue(), including existing destination
    // nodes and as-yet-uncommitted nodes.  Duplicate IDs are not newly
    // rejected, to retain the legacy importer's permissive behaviour.
    for (ShadowNode& node : nodes)
      if (node.value == value) return &node;
    return nullptr;
  };

  for (const Event& event : events.values) {
    if (event.kind == Event::Kind::Begin) {
      if (event.key == "graph") {
        current_node = nullptr;
        current_edge.reset();
      } else if (event.key == "node") {
        current_edge.reset();
        nodes.push_back({-1});
        current_node = &nodes.back();
      } else if (event.key == "edge") {
        if (!current_node)
          return {false, "GML edge without a preceding node"};
        current_edge = ShadowEdge{current_node, current_node};
      }
      continue;
    }
    if (event.kind != Event::Kind::Integer) continue;

    if (event.key == "id" && !current_edge && current_node) {
      current_node->value = event.integer;
      by_id[event.integer] = current_node;
    }
    if (!current_edge) continue;

    if (event.key == "source" &&
        current_edge->source->value != event.integer) {
      if (current_node && current_node->value != event.integer)
        current_node = resolve(event.integer);
      if (!current_node)
        return {false, "Unknown GML edge source ID"};
      current_edge->source = current_node;
    } else if (event.key == "target" &&
               current_edge->target->value != event.integer) {
      ShadowNode* target = resolve(event.integer);
      if (!target)
        return {false, "Unknown GML edge target ID"};
      current_edge->target = target;
    }
  }
  return {true, {}};
}
} // namespace

ApplyResult applyToGraph(const Result& parsed, FFGraphClass& graph,
                         bool resetExistingNodeIds) {
  if (!parsed.ok)
    return {false, parsed.events.error.empty() ? "GML syntax error"
                                               : parsed.events.error};

  // Semantic failures must be detected before touching the destination.
  // The commit phase below intentionally retains the established cursor
  // semantics.  This is a semantic-atomicity guarantee, not rollback of
  // allocation failures or exceptions thrown by graph event listeners.
  const ApplyResult validated = preflight(parsed.events, graph,
                                          resetExistingNodeIds);
  if (!validated.ok) return validated;

  // The historical load() API clears old numeric node IDs for each import.
  // Do it only after complete syntax and semantic validation, not on errors.
  if (resetExistingNodeIds) {
    for (Pix p = graph.allNodes().first(); p; graph.allNodes().next(p))
      graph.allNodes()(p)->setValue(-1);
  }

  // GTParser has one current node and one current edge, not a scope stack.
  // A nested graphics/unknown list leaves those cursors unchanged. Mimic
  // this intentionally for the parity pilot; don't infer nesting semantics
  // from the event stack until the legacy behaviour has been characterised.
  FFNodeClass* current_node = nullptr;
  FFEdgeClass* current_edge = nullptr;
  std::unordered_map<int, FFNodeClass*> by_id;

  for (const Event& event : parsed.events.values) {
    if (event.kind == Event::Kind::Begin) {
      if (event.key == "graph") {
        current_node = nullptr;
        current_edge = nullptr;
      } else if (event.key == "node") {
        current_edge = nullptr;
        current_node = graph.addNode();
      } else if (event.key == "edge") {
        // GTParser reports a consistency error and skips edges that appear
        // before the first node. Return a diagnostic rather than dereference
        // a null current_node; parity corpus uses defined/valid graph files.
        if (!current_node)
          return {false, "GML edge without a preceding node"};
        current_edge = graph.addEdge(current_node, current_node);
        if (!current_edge)
          return {false, "Cannot allocate GML edge"};
      }
      continue;
    }
    if (event.kind == Event::Kind::End)
      continue;  // Matches GTParser's empty END_LIST action.

    if (event.kind == Event::Kind::String) {
      if (event.key == "label") {
        if (current_edge) current_edge->setName(event.text.c_str());
        else if (current_node) current_node->setName(event.text.c_str());
        else graph.setName(event.text.c_str());
      }
      continue;
    }

    const bool integer = event.kind == Event::Kind::Integer;
    const double numeric = integer ? static_cast<double>(event.integer) : event.real;

    if (integer && event.key == "id") {
      if (current_edge) current_edge->setValue(event.integer);
      else if (current_node) {
        current_node->setValue(event.integer);
        by_id[event.integer] = current_node;
      }
    }

    if (!current_edge && current_node && current_node->getGraphicNode()) {
      FFGraphicNode* graphic = current_node->getGraphicNode();
      if (event.key == "x")
        graphic->moveTo(FFPoint(numeric, graphic->gety()));
      else if (event.key == "y")
        graphic->moveTo(FFPoint(graphic->getx(), numeric));
      else if (event.key == "z")
        graphic->moveTo(FFPoint(graphic->getx(), graphic->gety(), numeric));
    }

    if (!integer || !current_edge)
      continue;

    if (event.key == "source" &&
        current_edge->source()->getValue() != event.integer) {
      if (current_node && current_node->getValue() != event.integer) {
        const auto found = by_id.find(event.integer);
        current_node = found != by_id.end()
                           ? found->second : graph.nodeWithValue(event.integer);
      }
      // The old importer emits an error and keeps the current edge source
      // if the ID cannot be resolved. We return a diagnostic for that case.
      if (!current_node)
        return {false, "Unknown GML edge source ID"};
      current_edge->setSource(current_node);
    } else if (event.key == "target" &&
               current_edge->target()->getValue() != event.integer) {
      const auto found = by_id.find(event.integer);
      FFNodeClass* target = found != by_id.end()
                              ? found->second : graph.nodeWithValue(event.integer);
      if (!target)
        return {false, "Unknown GML edge target ID"};
      current_edge->setTarget(target);
    }
  }
  return {true, {}};
}

}  // namespace ffgraph_gml
