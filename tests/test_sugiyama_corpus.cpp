#include "sugiyama_layout_snapshot.h"
#include "sugiyama_corpus_verification.h"
#include <ffgraph/FFSugi.h>
#include <ffgraph/ffError.h>
#include <cstdint>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <map>
#include <set>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

extern kindErrorHandler* userKindErrorHandler;
extern charErrorHandler* userCharErrorHandler;

namespace {
std::uint64_t totalVerified = 0;
void require(bool condition, const std::string& message) {
  if (!condition) throw std::runtime_error(message);
}
struct ErrorGuard {
  kindErrorHandler* kind = userKindErrorHandler;
  charErrorHandler* text = userCharErrorHandler;
  ErrorGuard() {
    userKindErrorHandler = [](errGrade grade, errKind problem, const char*, int) {
      if (grade >= USER_ERROR) throw std::runtime_error("graph error " + std::to_string(problem));
    };
    userCharErrorHandler = [](errGrade grade, const char* problem, const char*, int) {
      if (grade >= USER_ERROR) throw std::runtime_error(problem);
    };
  }
  ~ErrorGuard() { userKindErrorHandler = kind; userCharErrorHandler = text; }
};
struct HeadlessSugi : FFSugi {
  void init(const FFGraph& graph) override { FFLayoutLabel::init(graph); }
};
void graphics(FFGraph& graph) {
  graph->_setDirected(NO);
  graph->setNodeIndex(graph->addNodeLabel(new FFGraphicNode));
  graph->setEdgeIndex(graph->addEdgeLabel(new FFGraphicEdge));
}
struct Synthetic {
  std::string name;
  int nodes;
  bool directed = true, unusualIds = false;
  std::vector<std::pair<int,int>> edges; // zero-based construction indices
};
std::vector<Synthetic> syntheticCases() {
  std::vector<Synthetic> cases = {{"empty",0}, {"singleton",1}, {"isolated",17},
                                 {"self-loops",9}};
  for (int i = 0; i < 9; ++i) cases.back().edges.emplace_back(i,i);
  Synthetic chain{"deep-long-edges",96};
  for (int i = 1; i < chain.nodes; ++i) chain.edges.emplace_back(i-1,i);
  chain.edges.insert(chain.edges.end(),{{0,2},{0,3},{0,95},{1,93},{5,89}});
  cases.push_back(chain);
  for (bool inward : {false,true}) {
    Synthetic fan{inward ? "wide-fanin" : "wide-fanout",129};
    for (int i = 1; i < fan.nodes; ++i) fan.edges.emplace_back(inward ? i : 0,inward ? 0 : i);
    cases.push_back(fan);
  }
  Synthetic alternating{"alternating-widths",105};
  const std::vector<int> widths = {2,31,3,29,2,37,1};
  int base = 0;
  for (std::size_t layer = 1; layer < widths.size(); ++layer) {
    const int next = base+widths[layer-1];
    for (int i = 0; i < widths[layer]; ++i) {
      alternating.edges.emplace_back(base+i%widths[layer-1],next+i);
      alternating.edges.emplace_back(base+(i*7+1)%widths[layer-1],next+i);
    }
    base = next;
  }
  alternating.edges.insert(alternating.edges.end(),{{0,104},{1,103},{5,100}});
  cases.push_back(alternating);
  Synthetic matching{"crossed-matching",24};
  for (int i = 0; i < 12; ++i) matching.edges.emplace_back(i,23-i);
  cases.push_back(matching);
  Synthetic ties{"ties-and-zero-slots",27};
  for (int i = 0; i < 12; ++i) if (i%4) {
    ties.edges.emplace_back(i,12+i%3);
    ties.edges.emplace_back(i,23-i%3);
  }
  cases.push_back(ties);
  Synthetic dense{"dense-bipartite",48};
  for (int i = 0; i < 24; ++i) for (int j = 24; j < 48; ++j) dense.edges.emplace_back(i,j);
  cases.push_back(dense);
  Synthetic multi{"parallel-antiparallel-loops",9};
  for (int i = 1; i < 9; ++i) {
    multi.edges.insert(multi.edges.end(),{{i-1,i},{i-1,i},{i,i-1},{i,i}});
  }
  multi.edges.insert(multi.edges.end(),{{0,8},{0,8},{8,0}});
  cases.push_back(multi);
  Synthetic cyclic{"strong-components",25};
  for (int i = 0; i < 24; ++i) {
    cyclic.edges.emplace_back(i,(i+1)%24);
    cyclic.edges.emplace_back(i,(i+7)%24);
  }
  cases.push_back(cyclic);
  Synthetic disconnected{"disconnected-components",38};
  disconnected.directed = false;
  for (int i = 1; i < 12; ++i) disconnected.edges.emplace_back(i-1,i);
  for (int i = 12; i < 23; ++i) disconnected.edges.emplace_back(i,12+(i-11)%11);
  for (int i = 25; i < 34; ++i) disconnected.edges.emplace_back(24,i);
  cases.push_back(disconnected);
  for (unsigned seed : {7u,71u,701u}) {
    Synthetic random{"seeded-layered-"+std::to_string(seed),80};
    std::uint32_t state = seed;
    for (int layer = 0; layer < 4; ++layer) for (int i = 0; i < 16; ++i) {
      random.edges.emplace_back(layer*16+i,(layer+1)*16+i);
      for (int edge = 0; edge < 2; ++edge) {
        state = state*1664525u+1013904223u;
        random.edges.emplace_back(layer*16+i,(layer+1)*16+(state>>16)%16);
      }
    }
    random.edges.insert(random.edges.end(),{{0,79},{3,73},{17,71}});
    cases.push_back(random);
  }
  auto ids = multi; ids.name = "duplicate-negative-sparse-ids"; ids.unusualIds = true;
  cases.push_back(ids);
  return cases;
}
void construct(FFGraph& graph, const Synthetic& fixture) {
  graph->setName(String(fixture.name)); graph->_setDirected(fixture.directed);
  std::vector<FFNode> nodes;
  for (int i = 0; i < fixture.nodes; ++i) {
    auto* node = graph->addNode(FFPoint(i*11,i%7*13,i%5*17));
    node->setValue(fixture.unusualIds ? (i%3 == 0 ? -7 : i%3 == 1 ? 2 : 100000+i) : i+1);
    node->setName(String("n"+std::to_string(i)));
    node->getGraphicNode()->setWidth(12+i%5*17);
    node->getGraphicNode()->setHeight(9+i%7*13);
    nodes.push_back(node);
  }
  int index = 0;
  for (auto [source,target] : fixture.edges) {
    auto* edge = graph->addEdge(nodes.at(source),nodes.at(target));
    ++index;
    edge->setValue(fixture.unusualIds ? 0 : index);
    edge->setWeight(float((index%5)-1));
    edge->setName(String("e"+std::to_string(index)));
  }
  if (!nodes.empty()) {
    graph->setStartNode(nodes[nodes.size()/2]);
    graph->addEndNode(nodes.back());
  }
}

struct NodeState {
  FFNode node;
  int value;
  std::string name, color;
  FFPoint position, range;
  std::set<FFEdge> outgoing, incoming;
};
struct EdgeState {
  FFEdge edge;
  FFNode source, target;
  int value;
  FFCoord weight;
  std::string name;
  FFGraphicEdge* graphic;
};
std::set<FFEdge> edgeSet(const FFSet<FFEdge>& edges) {
  std::set<FFEdge> result;
  for (Pix p = edges.first(); p; edges.next(p)) result.insert(edges(p));
  return result;
}
bool samePoint(const FFPoint& a, const FFPoint& b) {
  return std::equal(a.v,a.v+4,b.v);
}
void exercise(FFGraph& graph, const std::string& name, int expectedNodes, int expectedEdges) {
  require(graph->getNumNodes() == expectedNodes && graph->getNumEdges() == expectedEdges,
          name+": input counts differ from baseline");
  std::vector<NodeState> nodes;
  std::vector<EdgeState> edges;
  std::map<FFNode,int> identities;
  for (Pix p = graph->allNodes().first(); p; graph->allNodes().next(p)) {
    auto* node = graph->allNodes()(p); auto* graphic = node->getGraphicNode();
    require(graphic != nullptr,name+": missing node graphics");
    identities[node] = int(identities.size())+1; // Independent of imported/scratch IDs.
    nodes.push_back({node,node->getValue(),node->getName().c_str(),graphic->getColor().c_str(),
                     graphic->getCoords(),graphic->getRange(),edgeSet(node->transToSucc()),edgeSet(node->transFromPred())});
  }
  for (Pix p = graph->allEdges().first(); p; graph->allEdges().next(p)) {
    auto* edge = graph->allEdges()(p);
    edges.push_back({edge,edge->source(),edge->target(),edge->getValue(),edge->getWeight(),
                     edge->getName().c_str(),edge->getGraphicEdge()});
  }
  const auto* start = graph->getStartNode();
  std::set<FFNode> endNodes;
  for (Pix p = graph->getEndNodes().first(); p; graph->getEndNodes().next(p))
    endNodes.insert(graph->getEndNodes()(p));
  const bool directed = graph->getDirected();
  const std::string graphName = graph->getName().c_str();
  auto* layout = new HeadlessSugi; graph->addGraphLabel(layout);
  for (int variant = 0; variant < 6; ++variant) {
    layout->setDimension(variant == 1 || variant == 4 ? 3 : 2);
    layout->setOrient(variant == 2 || variant == 4 ? 2 : 1);
    layout->setUseGrid(variant != 2 && variant != 3);
    layout->setSameSize(variant >= 2 ? 2 : 1);
    layout->setLeavesDown(variant == 4);
    layout->setUseStartNode(variant != 3);
    require(layout->setCrossingAlg(variant == 5 ? 2 : 0),"crossing mode rejected");
    require(layout->setxoff(variant == 2 ? 31 : 0) && layout->setyoff(variant == 2 ? 47 : 0),"offset rejected");
    ffgraph_test::LayoutSnapshot reference;
    ffgraph_test::SugiVerification baseline{};
    for (bool referenceOnly : {true,false}) {
      // Use identical graph objects/traversal and restore incoming node/edge IDs
      // and node coordinates. Global Sugiyama straightens/rebuilds all routes.
      for (const auto& state : nodes) { state.node->setValue(state.value); state.node->getGraphicNode()->moveTo(state.position); }
      for (const auto& state : edges) state.edge->setValue(state.value);
      srand48(12345); std::srand(12345);
      ffgraph_test::resetSugiVerification(referenceOnly);
      layout->layout();
      const auto metrics = ffgraph_test::sugiVerification();
      require(graph->getNumNodes() == expectedNodes && graph->getNumEdges() == expectedEdges &&
              graph->getDirected() == directed && graph->getStartNode() == start &&
              graph->getName() == graphName.c_str(),name+": graph metadata/membership changed");
      std::set<FFNode> ends;
      for (Pix p = graph->getEndNodes().first(); p; graph->getEndNodes().next(p))
        ends.insert(graph->getEndNodes()(p));
      require(ends == endNodes,name+": end-node set changed");
      for (const auto& state : nodes) {
        auto* graphic = state.node->getGraphicNode();
        require(graph->allNodes().contains(state.node) && state.node->getName() == state.name.c_str() &&
                graphic->getColor() == state.color.c_str() && samePoint(graphic->getRange(),state.range) &&
                edgeSet(state.node->transToSucc()) == state.outgoing &&
                edgeSet(state.node->transFromPred()) == state.incoming,name+": node identity/attributes/incidence changed");
      }
      for (const auto& state : edges)
        require(graph->allEdges().contains(state.edge) && state.edge->source() == state.source &&
                state.edge->target() == state.target && state.edge->getWeight() == state.weight &&
                state.edge->getName() == state.name.c_str() && state.edge->getGraphicEdge() == state.graphic,
                name+": edge identity/attributes/incidence changed");
      const auto snapshot = ffgraph_test::captureLayout(*graph.operator->(),identities);
      require(metrics.finalCrossings <= metrics.initialCrossings,name+": reduction increased adjacent-layer crossings");
      if (variant == 5) require(metrics.finalCrossings == metrics.initialCrossings && metrics.permutations == 0,
                                name+": no-reduction control changed crossings or sorted a matrix");
      if (name == "crossed-matching")
        require(metrics.initialCrossings == 66 && metrics.finalCrossings == (variant == 5 ? 66u : 0u),
                "crossed matching differs from independently known 66-to-zero result");
      if (name == "dense-bipartite")
        require(metrics.initialCrossings == 76176 && metrics.finalCrossings == 76176,
                "complete bipartite crossings differ from choose(24,2)^2");
      if (referenceOnly) { reference = snapshot; baseline = metrics; }
      else {
        totalVerified += metrics.permutations;
        require(reference.name == snapshot.name && reference.directed == snapshot.directed &&
                reference.nodes == snapshot.nodes && reference.edges == snapshot.edges,
                name+": complete geometry differs from reference, variant "+std::to_string(variant));
        require(metrics.initialCrossings == baseline.initialCrossings && metrics.finalCrossings == baseline.finalCrossings &&
                metrics.levels == baseline.levels && metrics.width == baseline.width,name+": reference crossing/order dimensions differ");
        std::cout << "{\"case\":" << std::quoted(name) << ",\"variant\":" << variant
                  << ",\"nodes\":" << expectedNodes << ",\"edges\":" << expectedEdges
                  << ",\"levels\":" << metrics.levels << ",\"width\":" << metrics.width
                  << ",\"initial_crossings\":" << metrics.initialCrossings << ",\"final_crossings\":" << metrics.finalCrossings
                  << ",\"permutations\":" << metrics.permutations << ",\"checked_matrices\":" << metrics.checkedMatrices << "}\n";
      }
    }
  }
}
}

int main(int argc, char** argv) {
  std::string current;
  try {
    require(argc == 3,"usage: test_sugiyama_corpus real|synthetic CORPUS_DIR");
    ErrorGuard errors;
    const std::string mode = argv[1];
    if (mode == "real") {
      const std::filesystem::path root(argv[2]);
      std::ifstream listing(root/"cases.tsv"); require(bool(listing),"cannot open corpus manifest");
      std::string line; int cases = 0;
      while (std::getline(listing,line)) {
        if (line.empty() || line[0] == '#') continue;
        std::istringstream row(line); std::string file; int nodes,edges;
        require(bool(row >> file >> nodes >> edges),"invalid corpus manifest");
        current = file; FFGraph graph(YES); graphics(graph);
        graph->load(String((root/file).string()),YES);
        exercise(graph,file,nodes,edges); ++cases;
      }
      require(cases >= 13,"real corpus unexpectedly incomplete");
    } else {
      require(mode == "synthetic","unknown corpus mode");
      for (const auto& fixture : syntheticCases()) {
        current = fixture.name; FFGraph graph(YES); graphics(graph); construct(graph,fixture);
        exercise(graph,fixture.name,fixture.nodes,int(fixture.edges.size()));
      }
    }
    require(totalVerified > 0,"verified reduction object was not exercised");
  } catch (const std::exception& error) {
    std::cerr << "Sugiyama corpus " << current << ": " << error.what() << '\n'; return 1;
  }
}
