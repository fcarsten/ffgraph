#include "ff_graph_adapter.h"

#include <ffgraph/FFGraphClass.h>
#include <ffgraph/FFNodeClass.h>
#include <ffgraph/FFEdgeClass.h>
#include <ffgraph/FFGraphWindow.h>
#include <ffgraph/FFGraphicNode.h>
#include <ffgraph/FFGraphicEdge.h>
#include <ffgraph/FFGraphLabel.h>
#include <ffgraph/FFNodeLabel.h>
#include <ffgraph/FFEdgeLabel.h>

#include <iostream>
#include <set>

namespace ffgraph_ff {
bool applyToGraph(const Result& parsed, FFGraphClass& graph, bool resetExistingNodeIds) {
  if (!parsed.ok) return false;
  if (resetExistingNodeIds)
    for (Pix p = graph.allNodes().first(); p; graph.allNodes().next(p))
      graph.allNodes()(p)->setValue(-1);

  FFNodeClass* node = nullptr;
  FFEdgeClass* edge = nullptr;
  std::set<std::string> unmatched;
  const auto resolve = [&](int id) {
    FFNodeClass* found = graph.nodeWithValue(id);
    if (!found) { found = graph.addNode(); found->setValue(id); }
    return found;
  };
  const auto warn = [&](const char* scope, const std::string& type) {
    if (unmatched.insert(type).second)
      std::cerr << scope << " has no Label of Type: " << type << '\n';
  };
  for (const Event& e : parsed.events) {
    const auto& n = e.numbers;
    const auto& s = e.strings;
    FFGraphicNode* gn = node ? (*node)() : nullptr;
    FFGraphicEdge* ge = edge ? (*edge)() : nullptr;
    FFGraphWindow* window = graph.getWindow();
    switch (e.op) {
    case Op::GraphName:
      if (!s[0].empty() && graph.getName() == "") graph.setName(s[0].c_str());
      break;
    case Op::Directed: graph._setDirected(n[0]); break;
    case Op::Node: node = resolve(n[0]); edge = nullptr; break;
    case Op::Target: edge = graph.addEdge(node, resolve(n[0])); break;
    case Op::NodeName: node->setName(s[0].c_str()); break;
    case Op::EdgeName:
      // Legacy prepends explicit route points to the two default endpoints,
      // then removes those endpoints at the edge-label reduction.
      if (ge && ge->anzPoints() > 2) {
        ge->deletePointAt(ge->anzPoints() - 1);
        ge->deletePointAt(ge->anzPoints() - 1);
      }
      edge->setName(s[0].c_str());
      break;
    case Op::GraphLabel: {
      FFSet<int> types;
      const int k = graph.getGraphLabelIndex(s[0].c_str(), &types);
      if (k >= 0) graph.getGraphLabel(k)->parse(s[1].c_str());
      else warn("Graph", s[0]);
      break;
    }
    case Op::NodeLabel: {
      FFSet<int> types;
      const int k = node->getLabelIndex(s[0].c_str(), &types);
      if (k >= 0) node->getLabel(k)->parse(s[1].c_str());
      else warn("Node", s[0]);
      break;
    }
    case Op::EdgeLabel: {
      FFSet<int> types;
      const int k = edge->getLabelIndex(s[0].c_str(), &types);
      if (k >= 0) edge->getLabel(k)->parse(s[1].c_str());
      else warn("Edge", s[0]);
      break;
    }
    case Op::LabelOffset: if (window) window->setLabelOffset(n[0], n[1]); break;
    case Op::Scale:
      if (window) window->scaleXYZ(n[0]/100.0, n[0]/100.0, n[0]/100.0);
      break;
    case Op::RotateX: if (window) window->rotateX(n[0]/100.0); break;
    case Op::RotateY: if (window) window->rotateY(n[0]/100.0); break;
    case Op::RotateZ: if (window) window->rotateZ(n[0]/100.0); break;
    case Op::NodePosition: if (gn) gn->moveTo(FFPoint(n[0], n[1], n[2])); break;
    case Op::Start: graph.setStartNode(node); break;
    case Op::End: graph.addEndNode(node); break;
    case Op::NodeColor: if (gn) gn->setColor(s[0].c_str()); break;
    case Op::Radius: if (gn) gn->setRange(FFPoint(n[0], n[0], n[0])); break;
    case Op::Icon:
      if (gn) {
        gn->changeToIcon(s[0].c_str());
        if (s.size() == 2) gn->setMask(s[1].c_str());
      }
      break;
    case Op::NewIcon:
      if (gn) gn->changeToIcon(s[0].c_str(), s.size() == 2 ? s[1].c_str() : "__NoIcon__");
      break;
    case Op::Mask:
      if (gn) gn->setMask(s[0].c_str(), s.size() == 2 ? s[1].c_str() : "__NoMask__");
      break;
    case Op::EdgePoint: if (ge) ge->addPointAt(0, FFPoint(n[0], n[1], n[2])); break;
    case Op::EdgeColor: if (ge) ge->setColor(s[0].c_str()); break;
    case Op::Weight: edge->setWeight(n[0]/100.0); break;
    }
  }
  return true;
}
} // namespace ffgraph_ff
