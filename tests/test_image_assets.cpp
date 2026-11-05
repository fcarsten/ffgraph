#include "test_check.h"
#include <ffgraph/FFGraph.h>
#include <ffgraph/FFTclWindow.h>
#include <ffgraph/FFTcl3DWindow.h>
#include <ffgraph/FFTclCompat.h>
#include <ffgraph/FFGraphicEdge.h>
#include <ffgraph/ffError.h>
#include <string>

extern charErrorHandler* userCharErrorHandler;
namespace {
int errors = 0;
void unexpectedError(errGrade, const char* message, const char*, int) {
  ++errors; std::cerr << message << '\n';
}
void exercise(bool threeD) {
  FFGraph graph(YES);
  // Load before attaching a window to exercise legacy FF_NI / FF_NM metadata.
  graph->setNodeIndex(graph->addNodeLabel(new FFGraphicNode));
  graph->setEdgeIndex(graph->addEdgeLabel(new FFGraphicEdge));
  graph->load("GRAPH \"legacy icons\" = DIRECTED\n"
              "1 {$ NP 40 40 FF_NI \"redball.xpm\" \"blueball.xpm\" "
              "FF_NM \"ballmask.xbm\" \"diamondmask.xbm\" $} \"one\";\nEND", NO);
  FF_CHECK(graph->getNumNodes() == 1);
  auto node = graph->allNodes()(graph->allNodes().first());
  auto* window = threeD ? static_cast<FFTclBasicWindow*>(new FFTcl3DWindow)
                        : static_cast<FFTclBasicWindow*>(new FFTclWindow);
  graph->createGraphic(window);
  auto* interp = window->getInterpreter();
  auto* graphic = node->getGraphicNode();
  FF_CHECK(FF_TclEvalObjv(interp, {".frame0.canvas2", "type", graphic->getId().c_str()}) == TCL_OK);
  FF_CHECK(std::string(Tcl_GetStringResult(interp)) == "image");
  FF_CHECK(graphic->getIcon() == "redball.xpm" && graphic->getMask() == "ballmask.xbm");
  String serialized; graph->saveToString(serialized);
  FFGraph again(YES);
  again->setNodeIndex(again->addNodeLabel(new FFGraphicNode));
  again->load(serialized, NO);
  auto* copy = again->allNodes()(again->allNodes().first())->getGraphicNode();
  FF_CHECK(copy->getIcon() == "redball.xpm" && copy->getSelIcon() == "blueball.xpm");
  FF_CHECK(copy->getMask() == "ballmask.xbm" && copy->getSelMask() == "diamondmask.xbm");
  Tcl_SetVar(interp, "ffgraph_test_source", FFGRAPH_TEST_SOURCE_DIR, TCL_GLOBAL_ONLY);
#if defined(_WIN32) || defined(FFGRAPH_NO_XPM)
  Tcl_SetVar(interp, "ffgraph_test_xpm", "0", TCL_GLOBAL_ONLY);
#else
  Tcl_SetVar(interp, "ffgraph_test_xpm", "1", TCL_GLOBAL_ONLY);
#endif
  if (!threeD) {
    const std::string script = std::string(FFGRAPH_TEST_SOURCE_DIR) + "/tests/test_image_assets.tcl";
    const int status = FF_TclEvalObjv(interp, {"source", script.c_str()});
    if (status != TCL_OK) std::cerr << Tcl_GetStringResult(interp) << '\n';
    FF_CHECK(status == TCL_OK);
  }
  const int errorsBeforeCreation = errors;
  const std::string creationScript = std::string(FFGRAPH_TEST_SOURCE_DIR) + "/tests/test_icon_node_creation.tcl";
  const int creationStatus = FF_TclEvalObjv(interp, {"source", creationScript.c_str()});
  if (creationStatus != TCL_OK) std::cerr << Tcl_GetStringResult(interp) << '\n';
  FF_CHECK(creationStatus == TCL_OK);
  FF_CHECK(errors == errorsBeforeCreation);
  FF_CHECK(graph->getNumNodes() == 4);
  int images = 0, figures = 0;
  for (Pix p = graph->allNodes().first(); p; graph->allNodes().next(p)) {
    auto* created = graph->allNodes()(p)->getGraphicNode();
    FF_CHECK(!created->getId().empty() && !created->getLabelId().empty());
    FF_CHECK(FF_TclEvalObjv(interp, {".frame0.canvas2", "type", created->getLabelId().c_str()}) == TCL_OK);
    FF_CHECK(std::string(Tcl_GetStringResult(interp)) == "text");
    FF_CHECK(FF_TclEvalObjv(interp, {".frame0.canvas2", "type", created->getId().c_str()}) == TCL_OK);
    const std::string type = Tcl_GetStringResult(interp);
    if (created->getIcon() == "__NoIcon__") {
      FF_CHECK(type == "rectangle"); ++figures;
    } else {
      FF_CHECK(type == "image"); ++images;
      if (created != graphic) {
        FF_CHECK(created->getShape() == FF_IMAGE);
        FF_CHECK(created->getIcon() == "redball.png" && created->getSelIcon() == "blueball.png");
        FF_CHECK(created->getMask() == "ballmask.xbm" && created->getSelMask() == "diamondmask.xbm");
      }
    }
  }
  FF_CHECK(images == 3 && figures == 1);
  // Repainting a loaded icon must retain an image item in both projections.
  window->updateGraph();
  FF_CHECK(FF_TclEvalObjv(interp, {".frame0.canvas2", "type", graphic->getId().c_str()}) == TCL_OK);
  FF_CHECK(std::string(Tcl_GetStringResult(interp)) == "image");
  graph->removeGraphic();
}
}
int main(int argc, char** argv) {
  Tcl_FindExecutable(argc ? argv[0] : "test_image_assets");
  userCharErrorHandler = unexpectedError;
  exercise(false); FFWindow::collectPendingInterpreters();
  exercise(true); FFWindow::collectPendingInterpreters();
  FF_CHECK(errors == 0);
  std::cout << "PNG pixels, legacy aliases, masks, graph metadata and 2D/3D icons verified\n";
}
