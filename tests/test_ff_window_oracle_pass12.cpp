// Production FF imports in real 2D/3D Tk windows, with independent graph and
// transform expectations. No second parser supplies expected values.
#include "test_check.h"
#include "graph_corpus/graph_snapshot.h"
#include <ffgraph/FFTclWindow.h>
#include <ffgraph/FFTcl3DWindow.h>
#include <ffgraph/FFGraphicEdge.h>
#include <ffgraph/ffError.h>
#include <array>
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
using Matrix = std::array<std::array<double, 4>, 4>;
Matrix identity() {
  Matrix result{};
  for (int i = 0; i < 4; ++i) result[i][i] = 1;
  return result;
}
Matrix multiply(const Matrix& a, const Matrix& b) {
  Matrix result{};
  for (int i = 0; i < 4; ++i)
    for (int j = 0; j < 4; ++j)
      for (int k = 0; k < 4; ++k) result[i][j] += a[i][k] * b[k][j];
  return result;
}
Matrix extract(const std::vector<double>& state, int offset) {
  Matrix result{};
  for (int i = 0; i < 4; ++i)
    for (int j = 0; j < 4; ++j) result[i][j] = state[offset + 4*i + j];
  return result;
}
void transformOracle(const std::vector<double>& before, const std::vector<double>& after) {
  const double expected[] = {1.25, 0.5, -0.75, 1.0, 1.2, -0.8};
  for (int i = 0; i < 6; ++i) FF_CHECK(std::abs(after[i] - expected[i]) < 1e-6);
  // Independent double-precision matrix arithmetic checks the parsed values,
  // axis signs and operation order; do not call the window transform methods.
  Matrix scale = identity(), rx = identity(), ry = identity(), rz = identity();
  for (int i = 0; i < 3; ++i) scale[i][i] = 1.25;
  const double radians = std::acos(-1.0) / 180.0;
  const double x = 0.5*radians, y = -0.75*radians, z = radians;
  rx[1][1] = rx[2][2] = std::cos(x); rx[1][2] = std::sin(x); rx[2][1] = -std::sin(x);
  ry[0][0] = ry[2][2] = std::cos(y); ry[0][2] = -std::sin(y); ry[2][0] = std::sin(y);
  rz[0][0] = rz[1][1] = std::cos(z); rz[0][1] = std::sin(z); rz[1][0] = -std::sin(z);
  const Matrix wanted = multiply(rz, multiply(ry, multiply(rx, multiply(scale, extract(before, 6)))));
  const Matrix forward = extract(after, 6);
  const Matrix product = multiply(forward, extract(after, 22));
  for (int i = 0; i < 4; ++i)
    for (int j = 0; j < 4; ++j) {
      FF_CHECK(std::abs(forward[i][j] - wanted[i][j]) < 1e-5);
      FF_CHECK(std::abs(product[i][j] - (i == j ? 1 : 0)) < 1e-5);
    }
}
void graphOracle(FFGraph& graph) {
  FF_CHECK(graph->getName() == "FF GUI integration" && graph->getDirected());
  FF_CHECK(graph->getNumNodes() == 3 && graph->getNumEdges() == 3);
  auto* source = graph->nodeWithValue(1); auto* target = graph->nodeWithValue(2);
  auto* isolated = graph->nodeWithValue(3);
  FF_CHECK(source && target && isolated);
  FF_CHECK(source->getName() == "source" && target->getName() == "target" && isolated->getName() == "isolated");
  FF_CHECK(graph->getStartNode() == source && graph->getEndNodes().length() == 1 &&
           graph->getEndNodes().contains(target));
  FF_CHECK(source->getGraphicNode()->getx() == 20 && source->getGraphicNode()->gety() == 40 &&
           source->getGraphicNode()->getWidth() == 8 && source->getGraphicNode()->getColor() == "blue");
  FF_CHECK(target->getGraphicNode()->getx() == 100 && target->getGraphicNode()->gety() == 80 &&
           target->getGraphicNode()->getz() == 10 && isolated->getGraphicNode()->getx() == 160);
  int bends = 0, parallels = 0, loops = 0;
  for (Pix p = graph->allEdges().first(); p; graph->allEdges().next(p)) {
    auto* edge = graph->allEdges()(p);
    if (edge->getName() == "bend") {
      ++bends;
      FF_CHECK(edge->source() == source && edge->target() == target && edge->getWeight() == 1.75);
      auto* graphic = edge->getGraphicEdge();
      FF_CHECK(graphic->getColor() == "green" && graphic->anzPoints() == 3);
      FF_CHECK(graphic->getPointAt(1).v[0] == 55 && graphic->getPointAt(1).v[1] == 65 &&
               graphic->getPointAt(1).v[2] == 5);
    } else if (edge->getName() == "parallel") {
      ++parallels;
      FF_CHECK(edge->source() == source && edge->target() == target && edge->getWeight() == -0.5);
    } else if (edge->getName() == "loop") {
      ++loops; FF_CHECK(edge->source() == target && edge->target() == target);
    } else FF_CHECK(false);
  }
  FF_CHECK(bends == 1 && parallels == 1 && loops == 1);
}
template<class Base> void exercise(const char* path, const std::string& contents) {
  const int initial = Tk_GetNumMainWindows();
  for (bool file : {false, true})
    for (bool update : {false, true}) {
      {
        FFGraph graph(YES);
        auto* window = new Window<Base>; graph->createGraphic(window);
        window->setUpdate(update);
        const auto beforeState = window->state();
        const int before = errors;
        if (file) graph->load(String(path), YES);
        else graph->load(String(contents), NO);
        FF_CHECK(errors == before && window->getUpdate() == update);
        FF_CHECK(Tk_GetNumMainWindows() == initial + 1);
        graphOracle(graph);
        const auto state = window->state(); transformOracle(beforeState, state);
        auto* original = graph->nodeWithValue(1);
        const auto snapshot = ffgraph_test::capture(*graph.operator->());
        graph->load("GRAPH \"bad\" = {$ FF_SC 200 FF_LO 99 99 $} 888 \"partial\";", NO);
        FF_CHECK(errors == before + 1 && graph->nodeWithValue(1) == original);
        FF_CHECK(ffgraph_test::difference(snapshot, ffgraph_test::capture(*graph.operator->())).empty());
        FF_CHECK(window->state() == state && window->getUpdate() == update);
        graph->removeGraphic();
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
  std::cout << "FF 2D/3D graph, transform and rejection oracles passed\n";
}
