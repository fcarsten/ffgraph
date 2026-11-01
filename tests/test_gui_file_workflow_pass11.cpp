// Exercise the actual Tcl File > Load / Merge / Close commands, not merely
// FFGraphClass::load() or a manually retained window. Run under Xvfb.
#include <ffgraph/FFGraph.h>
#include <ffgraph/FFGraphClass.h>
#include <ffgraph/FFNodeClass.h>
#include <ffgraph/FFTclWindow.h>
#include <ffgraph/FFWindow.h>

#include <tcl.h>
#include <tk.h>

#include <algorithm>
#include <cstdlib>
#include <iostream>
#include <string>
#include <vector>

namespace {
class TrackedWindow final : public FFTclWindow {
public:
  TrackedWindow() { instances().push_back(this); }
  ~TrackedWindow() override {
    auto& windows = instances();
    windows.erase(std::remove(windows.begin(), windows.end(), this), windows.end());
  }
  FFGraphLabel* proto() const override { return new TrackedWindow; }
  static std::vector<TrackedWindow*>& instances() {
    static std::vector<TrackedWindow*> windows;
    return windows;
  }
};

void require(bool good, const char* reason) {
  if (!good) { std::cerr << "GUI workflow: " << reason << '\n'; std::exit(1); }
}

void command(TrackedWindow* window, const char* text) {
  Tcl_Interp* interp = window->getInterpreter();
  if (Tcl_Eval(interp, text) != TCL_OK) {
    std::cerr << "Tcl command '" << text << "' failed: "
              << Tcl_GetStringResult(interp) << '\n';
    std::exit(2);
  }
}

void choose(TrackedWindow* window, const char* filename) {
  Tcl_Interp* interp = window->getInterpreter();
  require(Tcl_SetVar(interp, "ffgraph_test_file", filename, TCL_GLOBAL_ONLY),
          "setting test filename");
  // Replace only the file-selection dialog; use the real menu dispatch.
  command(window, "proc FileSelectorBox {args} {return $::ffgraph_test_file}");
}

std::size_t nodes(TrackedWindow* window) {
  FFGraph owner = window->getGraph();
  return owner->allNodes().length();
}

std::size_t edges(TrackedWindow* window) {
  FFGraph owner = window->getGraph();
  return owner->allEdges().length();
}

void close(TrackedWindow* window) {
  // 'w' removes the C++ label in an active Tcl callback. Do not touch the
  // pointer again; interpreter collection must happen *after* Tcl_Eval.
  command(window, "recall modPress w");
  FFWindow::collectPendingInterpreters();
}
} // namespace

int main(int argc, char** argv) {
  if (argc != 4 && argc != 5) {
    std::cerr << "Usage: test_gui_file_workflow_pass11 valid.gml merge.gml invalid.gml [large.gml]\n";
    return 2;
  }
  Tcl_FindExecutable(argv[0]);
  const int initial = Tk_GetNumMainWindows();

  // Each cycle starts with a real GUI root. New windows must survive the
  // Tcl callback that opens them, even after deferred cleanup has run.
  for (int cycle = 0; cycle != (argc == 5 ? 1 : 4); ++cycle) {
    FFGraph root;
    auto* initialWindow = new TrackedWindow;
    root->createGraphic(initialWindow);
    require(Tk_GetNumMainWindows() == initial + 1, "initial Tk root");
    require(nodes(initialWindow) == 0, "new graph starts empty");

    // Cancelling the file picker must not manufacture a second window.
    choose(initialWindow, "");
    command(initialWindow, "recall modPress l");
    require(Tk_GetNumMainWindows() == initial + 1,
            "cancelled File > Load created a window");
    choose(initialWindow, argc == 5 ? argv[4] : argv[1]);
    command(initialWindow, "recall modPress l");
    FFWindow::collectPendingInterpreters();
    require(TrackedWindow::instances().size() == 2,
            "File > Load created one independent tracked window");
    require(Tk_GetNumMainWindows() == initial + 2,
            "loaded window persists after Tcl returns");
    auto* loaded = TrackedWindow::instances().back();
    require(loaded != initialWindow, "new window differs from source");
    const auto oldNodes = nodes(loaded);
    const auto oldEdges = edges(loaded);
    require(oldNodes > 0 && oldEdges > 0, "GML actually imported");
    if (argc == 5)
      require(oldNodes >= 100, "large-corpus fixture was not loaded");

    // File > Merge uses the same Tcl entry point as the File menu. It must
    // modify the selected graph, not create or close any top-level window.
    choose(loaded, argv[2]);
    command(loaded, "recall modPress m");
    FFWindow::collectPendingInterpreters();
    require(nodes(loaded) > oldNodes, "merge adds nodes");
    require(edges(loaded) >= oldEdges, "merge retains prior edges");
    require(Tk_GetNumMainWindows() == initial + 2, "merge preserves windows");

    // The modern importer rejects an unresolved edge endpoint during its
    // semantic preflight. Repeated rejection must preserve the graph and
    // leave both GUI windows operational.
    const auto mergedNodes = nodes(loaded);
    const auto mergedEdges = edges(loaded);
    FFGraph snapshotOwner = loaded->getGraph();
    const auto* originalGraph = snapshotOwner.operator->();
    struct NodeState { FFNodeClass* pointer; int id; String name; };
    std::vector<NodeState> savedNodes;
    for (Pix p = snapshotOwner->allNodes().first(); p;
         snapshotOwner->allNodes().next(p)) {
      auto* node = snapshotOwner->allNodes()(p);
      savedNodes.push_back({node, node->getValue(), node->getName()});
    }
    choose(loaded, argv[3]);
    for (int rejected = 0; rejected != 3; ++rejected) {
      command(loaded, "recall modPress m");
      require(nodes(loaded) == mergedNodes && edges(loaded) == mergedEdges,
              "invalid merge changed existing graph");
      require(loaded->getGraph().operator->() == originalGraph,
              "rejected import replaced graph object");
      for (const auto& saved : savedNodes)
        require(snapshotOwner->allNodes().contains(saved.pointer) &&
                saved.pointer->getValue() == saved.id &&
                saved.pointer->getName() == saved.name,
                "rejected import changed node identity or contents");
    }
    command(loaded, "update idletasks");
    require(Tk_GetNumMainWindows() == initial + 2,
            "failed merge destroyed a graph window");

    // First close the child, then continue issuing commands in the parent.
    snapshotOwner = FFGraph(YES);  // release test-only graph reference
    close(loaded);
    require(Tk_GetNumMainWindows() == initial + 1, "child close");
    require(TrackedWindow::instances().size() == 1, "child label reclaimed");
    command(initialWindow, "update idletasks");
    close(initialWindow);
    require(Tk_GetNumMainWindows() == initial, "parent close");
    require(TrackedWindow::instances().empty(), "all graph labels reclaimed");
    FFWindow::collectPendingInterpreters();
  }

  // File > Close is also exercised as a nested Tcl callback, where eager
  // interpreter deletion would invalidate the still-active Tcl_Eval stack.
  {
    FFGraph root;
    auto* window = new TrackedWindow;
    root->createGraphic(window);
    command(window, "after idle {recall modPress w}; update idletasks");
    FFWindow::collectPendingInterpreters();
    require(Tk_GetNumMainWindows() == initial, "nested close cleaned Tk root");
    require(TrackedWindow::instances().empty(), "nested close reclaimed label");
  }

  FFWindow::collectPendingInterpreters();
  require(Tk_GetNumMainWindows() == initial, "no Tk roots leaked");
  Tcl_Finalize();
  return 0;
}
