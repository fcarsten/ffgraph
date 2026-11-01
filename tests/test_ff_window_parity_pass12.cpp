// Compare production FF loads to independently invoked FFControl in real
// 2D/3D Tk windows. Window state and graph semantics must both agree.
#include "test_check.h"
#include "graph_corpus/graph_snapshot.h"
#include "FFParser.h"
#include "FFScanner.h"
#include <ffgraph/FFTclWindow.h>
#include <ffgraph/FFTcl3DWindow.h>
#include <ffgraph/FFGraphicEdge.h>
#include <ffgraph/ffError.h>
#include <fstream>
#include <sstream>
#include <vector>
#include <cmath>

extern charErrorHandler* userCharErrorHandler;
namespace {
int errors = 0;
void error(errGrade, const char* message, const char*, int) {
  std::cerr << message << '\n'; ++errors;
}
template<class Base> class Window final : public Base {
public:
  std::vector<double> state() const {
    std::vector<double> result{this->scale, this->alpha, this->beta, this->gamma,
                               this->labelXoff, this->labelYoff};
    for (const auto* matrix : {&this->ctm, &this->invCtm})
      for (const auto& row : matrix->row)
        for (double coordinate : row.v) result.push_back(coordinate);
    return result;
  }
};
void compare(FFGraph& a, FFGraph& b) {
  FF_CHECK(ffgraph_test::difference(ffgraph_test::capture(*a.operator->()),
                                   ffgraph_test::capture(*b.operator->())).empty());
  FF_CHECK(a->getDirected() == b->getDirected());
  FF_CHECK(a->getStartNode()->getValue() == b->getStartNode()->getValue());
  FF_CHECK(a->getEndNodes().length() == b->getEndNodes().length());
  for (Pix p = a->allEdges().first(); p; a->allEdges().next(p)) {
    auto* edge = a->allEdges()(p);
    FFEdgeClass* other = nullptr;
    for (Pix q = b->allEdges().first(); q; b->allEdges().next(q))
      if (b->allEdges()(q)->getName() == edge->getName()) other = b->allEdges()(q);
    FF_CHECK(other && edge->getWeight() == other->getWeight());
    auto* ga = edge->getGraphicEdge(); auto* gb = other->getGraphicEdge();
    FF_CHECK(ga->getColor() == gb->getColor() && ga->anzPoints() == gb->anzPoints());
    for (int i = 0; i < ga->anzPoints(); ++i)
      for (int d = 0; d < 3; ++d) FF_CHECK(ga->getPointAt(i).v[d] == gb->getPointAt(i).v[d]);
  }
}
template<class Base> void exercise(const char* path, const std::string& contents) {
  const int initial = Tk_GetNumMainWindows();
  for (bool file : {false, true})
    for (bool update : {false, true}) {
      {
        FFGraph selected(YES), legacy(YES);
        auto* production = new Window<Base>; auto* oracle = new Window<Base>;
        selected->createGraphic(production); legacy->createGraphic(oracle);
        production->setUpdate(update); oracle->setUpdate(NO);
        const int before = errors;
        if (file) selected->load(String(path), YES);
        else selected->load(String(contents), NO);
        FFControl control;
        if (file) { std::ifstream input(path); control.Parse(input, legacy.operator->()); }
        else control.Parse(String(contents), legacy.operator->());
        oracle->setUpdate(update); oracle->adjustWindowSize();
        if (update) oracle->update();
        FF_CHECK(errors == before && production->getUpdate() == update);
        FF_CHECK(Tk_GetNumMainWindows() == initial + 2);
        compare(selected, legacy);
        FF_CHECK(production->state() == oracle->state());
        // Independent window oracle in addition to differential comparisons.
        const auto state = production->state();
        FF_CHECK(std::abs(state[0] - 1.25) < 1e-6 && std::abs(state[1] - 0.5) < 1e-6 &&
                 std::abs(state[2] + 0.75) < 1e-6 && std::abs(state[3] - 1.0) < 1e-6 &&
                 std::abs(state[4] - 1.2) < 1e-6 && std::abs(state[5] + 0.8) < 1e-6);
        FF_CHECK(selected->getNumNodes() == 3 && selected->getNumEdges() == 3);
        FF_CHECK(selected->nodeWithValue(1)->getName() == "source" &&
                 selected->nodeWithValue(2)->getGraphicNode()->getz() == 10);
#if FFGRAPH_TEST_MODERN_FF
        auto* original = selected->nodeWithValue(1);
        const auto snapshot = ffgraph_test::capture(*selected.operator->());
        selected->load("GRAPH \"bad\" = {$ FF_SC 200 FF_LO 99 99 $} 888 \"partial\";", NO);
        FF_CHECK(errors == before + 1 && selected->nodeWithValue(1) == original);
        FF_CHECK(ffgraph_test::difference(snapshot, ffgraph_test::capture(*selected.operator->())).empty());
        FF_CHECK(production->state() == state && production->getUpdate() == update);
#endif
        selected->removeGraphic(); legacy->removeGraphic();
      }
      FFWindow::collectPendingInterpreters();
      FF_CHECK(Tk_GetNumMainWindows() == initial);
    }
}
} // namespace
int main(int argc, char** argv) {
  FF_CHECK(argc == 2);
  Tcl_FindExecutable(argv[0]); userCharErrorHandler = error;
  std::ifstream input(argv[1]); FF_CHECK(input.good());
  std::ostringstream contents; contents << input.rdbuf();
  exercise<FFTclWindow>(argv[1], contents.str());
  exercise<FFTcl3DWindow>(argv[1], contents.str());
  FFWindow::collectPendingInterpreters(); Tcl_Finalize();
  std::cout << "FF 2D/3D window, graph and transform parity passed\n";
}
