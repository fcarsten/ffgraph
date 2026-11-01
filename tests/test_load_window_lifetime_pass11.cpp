// Regression for File > Load: the local FFGraph handle in the Tcl callback
// must not destroy the newly created graph window when the callback returns.
// Run under Xvfb with FF_LIBDIR set to the repository's lib/ffgraph directory.
#include <ffgraph/FFGraph.h>
#include <ffgraph/FFGraphClass.h>
#include <ffgraph/FFTclWindow.h>
#include <ffgraph/FFWindow.h>

#include <tk.h>

#include <iostream>

int main(int argc, char** argv)
{
  if (argc != 2) {
    std::cerr << "Usage: test_load_window_lifetime_pass11 graph.gml\n";
    return 1;
  }
  Tcl_FindExecutable(argv[0]);
  const int initial = Tk_GetNumMainWindows();
  FFGraphClass* graph = nullptr;
  {
    // Match the File > Load callback: the graph handle is local, but the
    // GUI window is expected to outlive that callback.
    FFGraph local;
    local->createGraphic(new FFTclWindow);
    graph = local.operator->();
    FFWindow::retainGraph(local);
    local->load(String(argv[1]), YES);
    if (!graph->getWindow()) {
      std::cerr << "Graph window missing after import\n";
      return 2;
    }
  }

  // The local FFGraph has gone out of scope. This is the original failure
  // point: without the retained reference, deleting it also closed the GUI.
  if (Tk_GetNumMainWindows() != initial + 1) {
    std::cerr << "New window disappeared after load callback\n";
    return 3;
  }
  FFWindow::collectPendingInterpreters();
  if (Tk_GetNumMainWindows() != initial + 1 || !graph->getWindow()) {
    std::cerr << "New window disappeared during deferred cleanup\n";
    return 4;
  }

  // Closing a graph's graphic must release its retained reference at the
  // next safe event-loop collection point (not inside the deletion callback).
  graph->removeGraphic();
  FFWindow::collectPendingInterpreters();
  if (Tk_GetNumMainWindows() != initial) {
    std::cerr << "Graph window not reclaimed after Close\n";
    return 5;
  }
  Tcl_Finalize();
  return 0;
}
