// Test/profiling snapshots keyed by imported IDs, before layout scratch IDs change.
#ifndef FFGRAPH_TEST_SUGIYAMA_LAYOUT_SNAPSHOT_H
#define FFGRAPH_TEST_SUGIYAMA_LAYOUT_SNAPSHOT_H
#include <ffgraph/FFGraph.h>
#include <ffgraph/FFGraphicNode.h>
#include <ffgraph/FFGraphicEdge.h>
#include <ffgraph/FFNodeClass.h>
#include <ffgraph/FFEdgeClass.h>
#include <algorithm>
#include <array>
#include <cmath>
#include <iomanip>
#include <limits>
#include <map>
#include <stdexcept>
#include <string>
#include <tuple>
#include <vector>

namespace ffgraph_test {
using LayoutPoint = std::array<FFCoord, 4>;
using LayoutNode = std::tuple<int, std::string, LayoutPoint>;
using LayoutEdge = std::tuple<int, int, std::string, FFCoord, std::vector<LayoutPoint>>;
struct LayoutSnapshot {
  std::string name;
  bool directed;
  std::vector<LayoutNode> nodes;
  std::vector<LayoutEdge> edges;
};
inline LayoutPoint layoutPoint(const FFPoint& p) {
  LayoutPoint result{};
  for (int i = 0; i < 4; ++i) {
    if (!std::isfinite(p.v[i])) throw std::runtime_error("nonfinite layout coordinate");
    result[i] = p.v[i];
  }
  return result;
}
inline std::map<FFNode, int> layoutIds(FFGraphClass& graph) {
  std::map<FFNode, int> result;
  std::vector<int> ids;
  for (Pix p = graph.allNodes().first(); p; graph.allNodes().next(p)) {
    auto* node = graph.allNodes()(p);
    result.emplace(node, node->getValue());
    ids.push_back(node->getValue());
  }
  std::sort(ids.begin(), ids.end());
  if (std::adjacent_find(ids.begin(), ids.end()) != ids.end())
    throw std::runtime_error("layout snapshots require unique imported node IDs");
  return result;
}
inline LayoutSnapshot captureLayout(FFGraphClass& graph, const std::map<FFNode, int>& ids) {
  LayoutSnapshot out{graph.getName().c_str(), bool(graph.getDirected()), {}, {}};
  if (graph.allNodes().length() != static_cast<int>(ids.size()))
    throw std::runtime_error("layout changed node count");
  for (Pix p = graph.allNodes().first(); p; graph.allNodes().next(p)) {
    auto* node = graph.allNodes()(p);
    if (!node->getGraphicNode()) throw std::runtime_error("missing node graphics");
    out.nodes.emplace_back(ids.at(node), node->getName().c_str(),
                           layoutPoint(node->getGraphicNode()->getCoords()));
  }
  for (Pix p = graph.allEdges().first(); p; graph.allEdges().next(p)) {
    auto* edge = graph.allEdges()(p);
    auto* graphic = edge->getGraphicEdge();
    if (!graphic || graphic->anzPoints() < 2) throw std::runtime_error("invalid edge route");
    std::vector<LayoutPoint> route;
    for (int k = 0; k < graphic->anzPoints(); ++k) route.push_back(layoutPoint(graphic->getPointAt(k)));
    out.edges.emplace_back(ids.at(edge->source()), ids.at(edge->target()),
                           edge->getName().c_str(), edge->getWeight(), std::move(route));
  }
  std::sort(out.nodes.begin(), out.nodes.end());
  std::sort(out.edges.begin(), out.edges.end());
  return out;
}
inline void writeLayout(std::ostream& out, const LayoutSnapshot& s) {
  out << std::setprecision(std::numeric_limits<FFCoord>::max_digits10);
  out << "graph " << std::quoted(s.name) << ' ' << s.directed << ' '
      << s.nodes.size() << ' ' << s.edges.size() << '\n';
  for (const auto& [id, name, point] : s.nodes) {
    out << "node " << id << ' ' << std::quoted(name);
    for (auto v : point) out << ' ' << v;
    out << '\n';
  }
  for (const auto& [source, target, name, weight, route] : s.edges) {
    out << "edge " << source << ' ' << target << ' ' << std::quoted(name) << ' ' << weight
        << ' ' << route.size();
    for (const auto& point : route) for (auto v : point) out << ' ' << v;
    out << '\n';
  }
}
inline LayoutSnapshot readLayout(std::istream& in) {
  LayoutSnapshot s{};
  std::string tag;
  std::size_t nodes, edges;
  if (!(in >> tag >> std::quoted(s.name) >> s.directed >> nodes >> edges) || tag != "graph")
    throw std::runtime_error("invalid layout snapshot header");
  for (std::size_t i = 0; i < nodes; ++i) {
    int id; std::string name; LayoutPoint point;
    in >> tag >> id >> std::quoted(name);
    for (auto& v : point) in >> v;
    if (!in || tag != "node") throw std::runtime_error("invalid snapshot node");
    s.nodes.emplace_back(id, name, point);
  }
  for (std::size_t i = 0; i < edges; ++i) {
    int source, target; std::string name; FFCoord weight; std::size_t count;
    in >> tag >> source >> target >> std::quoted(name) >> weight >> count;
    if (!in || tag != "edge" || count < 2) throw std::runtime_error("invalid snapshot edge");
    std::vector<LayoutPoint> route(count);
    for (auto& point : route) for (auto& v : point) in >> v;
    s.edges.emplace_back(source, target, name, weight, route);
  }
  if (!in || in >> tag) throw std::runtime_error("invalid/trailing snapshot data");
  return s;
}
inline bool sameLayout(const LayoutSnapshot& a, const LayoutSnapshot& b) {
  if (a.name != b.name || a.directed != b.directed || a.nodes.size() != b.nodes.size() ||
      a.edges.size() != b.edges.size()) return false;
  const auto samePoint = [](const LayoutPoint& x, const LayoutPoint& y) {
    for (int k = 0; k < 4; ++k)
      if (!std::isfinite(y[k]) || std::abs(x[k] - y[k]) > 1e-4f + 2e-6f * std::abs(x[k])) return false;
    return true;
  };
  for (std::size_t i = 0; i < a.nodes.size(); ++i) {
    if (std::get<0>(a.nodes[i]) != std::get<0>(b.nodes[i]) ||
        std::get<1>(a.nodes[i]) != std::get<1>(b.nodes[i]) ||
        !samePoint(std::get<2>(a.nodes[i]), std::get<2>(b.nodes[i]))) return false;
  }
  for (std::size_t i = 0; i < a.edges.size(); ++i) {
    const auto& [as, at, an, aw, ar] = a.edges[i];
    const auto& [bs, bt, bn, bw, br] = b.edges[i];
    if (as != bs || at != bt || an != bn || aw != bw || ar.size() != br.size()) return false;
    for (std::size_t k = 0; k < ar.size(); ++k) if (!samePoint(ar[k], br[k])) return false;
  }
  return true;
}
} // namespace ffgraph_test
#endif
