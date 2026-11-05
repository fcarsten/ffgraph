#include "test_check.h"
#include <ffgraph/FFGraph.h>
#include <ffgraph/FFTclWindow.h>
#include <ffgraph/FFTcl3DWindow.h>
#include <ffgraph/FFTclCompat.h>
#include <ffgraph/FFGraphicEdge.h>
#include <ffgraph/ffError.h>
#include <string>
#include <fstream>
#include <sstream>
#include <set>
#include <tuple>

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

// FF_I names from the A95 demographs refer to artwork absent from the corpus.
// Exercise both loading into a live window and attaching an already loaded graph.
void missingIcons(bool threeD, bool loadBefore, const std::string& source) {
  const int errorsBefore = errors;
  FFGraph graph(YES);
  graph->setNodeIndex(graph->addNodeLabel(new FFGraphicNode));
  graph->setEdgeIndex(graph->addEdgeLabel(new FFGraphicEdge));
  if (loadBefore) graph->load(source.c_str(), NO);
  auto* window = threeD ? static_cast<FFTclBasicWindow*>(new FFTcl3DWindow)
                        : static_cast<FFTclBasicWindow*>(new FFTclWindow);
  graph->createGraphic(window);
  if (!loadBefore) graph->load(source.c_str(), NO);
  FF_CHECK(graph->getNumNodes() > 0);
  auto* interp = window->getInterpreter();
  FFGraphicNode* missing = nullptr;
  std::set<std::pair<std::string, std::string>> missingNames;
  for (Pix p = graph->allNodes().first(); p; graph->allNodes().next(p)) {
    auto* graphic = graph->allNodes()(p)->getGraphicNode();
    FF_CHECK(!graphic->getId().empty() && !graphic->getLabelId().empty());
    FF_CHECK(FF_TclEvalObjv(interp, {".frame0.canvas2", "type", graphic->getLabelId().c_str()}) == TCL_OK);
    FF_CHECK(std::string(Tcl_GetStringResult(interp)) == "text");
    if (graphic->getIcon() == "__NoIcon__") continue;
    missing = graphic;
    missingNames.emplace(graphic->getIcon().c_str(), graphic->getSelIcon().c_str());
    FF_CHECK(FF_TclEvalObjv(interp, {".frame0.canvas2", "type", graphic->getId().c_str()}) == TCL_OK);
    FF_CHECK(std::string(Tcl_GetStringResult(interp)) == "image");
    const std::string flag = "::ffgraph_icons::placeholders(.frame0.canvas2," + std::string(graphic->getId().c_str()) + ")";
    FF_CHECK(FF_TclEvalObjv(interp, {"set", flag.c_str()}) == TCL_OK);
    FF_CHECK(std::string(Tcl_GetStringResult(interp)) == "1");
    FF_CHECK(FF_TclEvalObjv(interp, {".frame0.canvas2", "itemcget", graphic->getId().c_str(), "-image"}) == TCL_OK);
    const std::string picture = Tcl_GetStringResult(interp);
    FF_CHECK(FF_TclEvalObjv(interp, {"image", "width", picture.c_str()}) == TCL_OK);
    FF_CHECK(std::string(Tcl_GetStringResult(interp)) == "30");
  }
  FF_CHECK(missing != nullptr);
  // Rendering placeholders must not rewrite any stored graph references.
  String serialized; graph->saveToString(serialized);
  FFGraph copy(YES);
  copy->setNodeIndex(copy->addNodeLabel(new FFGraphicNode));
  copy->setEdgeIndex(copy->addEdgeLabel(new FFGraphicEdge));
  copy->load(serialized, NO);
  FF_CHECK(copy->getNumNodes() == graph->getNumNodes());
  FF_CHECK(copy->getNumEdges() == graph->getNumEdges());
  using References = std::multiset<std::tuple<std::string, std::string, std::string, std::string>>;
  auto references = [](FFGraph& g) {
    References result;
    for (Pix p = g->allNodes().first(); p; g->allNodes().next(p)) {
      auto* n = g->allNodes()(p)->getGraphicNode();
      result.emplace(n->getIcon().c_str(), n->getSelIcon().c_str(), n->getMask().c_str(), n->getSelMask().c_str());
    }
    return result;
  };
  FF_CHECK(references(graph) == references(copy));
  window->updateGraph();
  FF_CHECK(errors == errorsBefore);
  FF_CHECK(FF_TclEvalObjv(interp, {"array", "size", "::ffgraph_icons::warnings"}) == TCL_OK);
  FF_CHECK(std::stoul(Tcl_GetStringResult(interp)) == missingNames.size());
  // Replacing a placeholder with real artwork must re-enable its actual mask.
  missing->changeToIcon("redball.png", "blueball.png");
  missing->setMask("ballmask.xbm", "diamondmask.xbm");
  FF_CHECK(errors == errorsBefore);
  const std::string flag = "::ffgraph_icons::placeholders(.frame0.canvas2," + std::string(missing->getId().c_str()) + ")";
  FF_CHECK(FF_TclEvalObjv(interp, {"set", flag.c_str()}) == TCL_OK);
  FF_CHECK(std::string(Tcl_GetStringResult(interp)) == "0");
  FF_CHECK(FF_TclEvalObjv(interp, {".frame0.canvas2", "itemcget", missing->getId().c_str(), "-image"}) == TCL_OK);
  const std::string picture = Tcl_GetStringResult(interp);
  FF_CHECK(FF_TclEvalObjv(interp, {"image", "width", picture.c_str()}) == TCL_OK);
  FF_CHECK(std::string(Tcl_GetStringResult(interp)) == "14");
  graph->removeGraphic();
}
}
int main(int argc, char** argv) {
  Tcl_FindExecutable(argc ? argv[0] : "test_image_assets");
  userCharErrorHandler = unexpectedError;
  exercise(false); FFWindow::collectPendingInterpreters();
  exercise(true); FFWindow::collectPendingInterpreters();
  std::string source =
      "GRAPH \"missing A95 icons\" = DIRECTED\n"
      "1 {$ FF_I \"INSTRUCTION_BUS_INTERFACE\" \"diamondmask\" NP 40 40 $} \"MUX\"\n"
      " 2 \"\";\n"
      "2 {$ FF_I \"INSTRUCTION_CONTROL_UNIT\" \"ballmask\" NP 100 100 $} \"B STAGE\";\n"
      "3 {$ FF_I \"INSTRUCTION_CONTROL_UNIT\" \"ballmask\" NP 160 160 $} \"J REG\";\nEND";
  if (argc == 2) {
    std::ifstream input(argv[1]); FF_CHECK(input.good());
    std::ostringstream contents; contents << input.rdbuf(); source = contents.str();
  }
  for (bool threeD : {false, true}) for (bool loadBefore : {false, true}) {
    missingIcons(threeD, loadBefore, source);
    FFWindow::collectPendingInterpreters();
  }
  FF_CHECK(errors == 0);
  std::cout << "PNG pixels, legacy aliases, masks, graph metadata and 2D/3D icons verified\n";
}
