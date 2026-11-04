// Private geometry recovery around the existing publication operations.
#ifndef FFGRAPH_SUGI_PUBLICATION_H
#define FFGRAPH_SUGI_PUBLICATION_H
#include <ffgraph/FFGraphicNode.h>
#include <ffgraph/FFGraphicEdge.h>
#include <ffgraph/FFGraphWindow.h>
#include <vector>
#include "FFsugi_sizes.h"

namespace ffgraph_sugiyama {
struct Publication {
  struct Node { FFGraphicNode* graphic; FFPoint position; };
  struct Edge {
    FFGraphicEdge* graphic;
    MPlex<FFPoint> backup;
    BOOL spline;
    bool splineChanged = false;
    explicit Edge(FFGraphicEdge* g) : graphic(g), backup(g->getEdgePoints()), spline(g->spline) {}
  };
  std::vector<Node> nodes;
  std::vector<Edge> edges;
  bool active = false;
  Publication(const FFSet<FFNode>& ns,const FFSet<FFEdge>& es) {
    if (std::size_t(ns.length())>nodes.max_size() || std::size_t(es.length())>edges.max_size()) throw SizeError();
    nodes.reserve(ns.length()); edges.reserve(es.length());
    for (Pix p=ns.first();p;ns.next(p))
      if (auto* g=ns(p)->getGraphicNode()) nodes.push_back({g,g->getCoords()});
    for (Pix p=es.first();p;es.next(p))
      if (auto* g=es(p)->getGraphicEdge()) edges.emplace_back(g);
  }
  Publication(const Publication&) = delete;
  Publication& operator=(const Publication&) = delete;
  void begin() noexcept {
    // Put copies in the live graphics, retaining original storage and Pix
    // addresses in backups until publication has completely succeeded.
    for (auto& e:edges) e.graphic->edgePoints.swap(e.backup);
    active = true;
  }
  void rollback() noexcept {
    if (!active) return;
    for (auto& n:nodes) for (int i=0;i<FFPOINT_SIZE;++i) n.graphic->coords.v[i]=n.position.v[i];
    for (auto& e:edges) {
      e.graphic->edgePoints.swap(e.backup);
      e.splineChanged = e.graphic->spline != e.spline;
      e.graphic->spline=e.spline;
    }
    active = false;
  }
  void refresh(FFGraphWindow* window) {
    if (!window) return;
    for (auto& e:edges) if (e.splineChanged) window->setSpline(e.graphic,e.spline);
    window->updateGraph(); window->adjustWindowSize();
  }
  void commit() noexcept { active = false; }
  ~Publication() { rollback(); }
};
}
#endif
