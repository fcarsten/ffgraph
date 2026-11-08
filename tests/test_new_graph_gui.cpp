#include "test_check.h"
#include <ffgraph/FFGraph.h>
#include <ffgraph/FFTclWindow.h>
#include <ffgraph/FFTcl3DWindow.h>
#include <ffgraph/FFTclCompat.h>

namespace {
struct Child {
  FFTclBasicWindow* window = nullptr;
  bool alive = false;
};
class GraphLifetime : public FFGraphLabel {
  bool& destroyed;
public:
  explicit GraphLifetime(bool& flag) : destroyed(flag) { _setType("TestGraphLifetime"); }
  FFGraphLabel* proto() const override { return new GraphLifetime(destroyed); }
  ~GraphLifetime() override { destroyed = true; }
};
template<class Base> class TrackedWindow : public Base {
  Child& child;
  bool isChild;
public:
  explicit TrackedWindow(Child& c, bool owned = false) : child(c), isChild(owned) {
    if (isChild) { child.window = this; child.alive = true; }
  }
  FFGraphLabel* proto() const override { return new TrackedWindow(child, true); }
  ~TrackedWindow() override { if (isChild) child.alive = false; }
};
template<class Base> void exercise() {
  const int before = Tk_GetNumMainWindows();
  Child child;
  FFGraph parent(YES);
  auto* window = new TrackedWindow<Base>(child);
  parent->createGraphic(window);
  for (const char* command : {".frame1.menubutton1.m invoke {New (M-n)}", "recall modPress n"}) {
    bool graphDestroyed = false;
    FF_CHECK(Tcl_Eval(window->getInterpreter(), command) == TCL_OK);
    FF_CHECK(child.alive); // Check before dereferencing a possibly deleted label.
    FFWindow::collectPendingInterpreters();
    FF_CHECK(child.alive && Tk_GetNumMainWindows() == before + 2);
    FF_CHECK(dynamic_cast<Base*>(child.window) != nullptr);
    {
      // Acquire an owner only after the callback has returned and collection
      // has run, so this cannot conceal the original missing-reference bug.
      FFGraph created = child.window->getGraph();
      FF_CHECK(created.operator->() != parent.operator->());
      FF_CHECK(created->getNumNodes() == 0 && created->getNumEdges() == 0);
      created->addGraphLabel(new GraphLifetime(graphDestroyed));
      created->addNode(FFPoint(20, 30));
      FF_CHECK(created->getNumNodes() == 1 && parent->getNumNodes() == 0);
    }
    auto* interp = child.window->getInterpreter();
    FF_CHECK(Tcl_Eval(interp, "recall modPress w") == TCL_OK);
    FF_CHECK(!child.alive);
    FFWindow::collectPendingInterpreters();
    FF_CHECK(Tk_GetNumMainWindows() == before + 1);
    FF_CHECK(graphDestroyed); // The registry must release the closed graph too.
  }
  parent->removeGraphic();
  FFWindow::collectPendingInterpreters();
  FF_CHECK(Tk_GetNumMainWindows() == before);
}
}
int main(int argc, char** argv) {
  Tcl_FindExecutable(argc ? argv[0] : "test_new_graph_gui");
  exercise<FFTclWindow>();
  exercise<FFTcl3DWindow>();
  std::cout << "New Graph menu/command windows survive and close correctly in 2D/3D\n";
}
