# Pass 11 follow-up: graph window closes immediately after File > Load

## Defect and scope

The Tcl `File > Load (M-l)` menu calls `FFTclBasicWindow::recall()` with
`modPress l`. That path creates a local `FFGraph tmpG`, attaches a new
`FFTclBasicWindow` graph label, imports a file, and returns. `FFGraph` is
reference-counted but the window label is not an owning reference to its
graph. Consequently, destruction of `tmpG` deletes the graph and its new
window at the end of the Tcl command. This explains an apparently successful
import followed by immediate closure. It is independent of the selected GML
backend; the modern parser is not changed by this patch.

`File > Merge (M-m)` imports into the current graph and does not take this
path. Command-line file loading already keeps a vector of `FFGraph` handles
alive until the main Tk event loop ends.

## Implementation

* `FFWindow::retainGraph()` keeps a reference-counted handle for graphs opened
  into new top-level windows. The `l` callback registers the new graph before
  importing it, so its lifetime is not tied to the callback stack.
* At the existing safe event-loop cleanup point,
  `collectPendingInterpreters()`, graph handles with no attached graphic are
  removed. Releases and deferred Tcl interpreter deletion are drained until
  stable; interpreter destruction does not run inside active Tcl callbacks.
* After the last Tk main window closes, `mainLoop()` releases remaining
  retained graphs and collects any newly queued interpreter deletions. This
  also handles a Tcl root being destroyed without first removing its graph
  label. Unlike a permanent global pool, this does not retain loaded graphs
  indefinitely after the GUI quits.
* Added Xvfb CTest `test_load_window_lifetime_pass11` which exercises a real
  GML load into a new `FFTclWindow`, destruction of the local graph handle,
  subsequent safe collection, and explicit `removeGraphic()` teardown.

The GUI's current Tcl menu bindings, public graph format and modern/legacy
parser selections are unchanged. SPQR and the inspector remain out of scope.

## Verification in this environment

* Reproduced the lifetime problem from source inspection: the `l` branch had
  a local `FFGraph` whose destructor was guaranteed to run immediately after
  loading; no surviving `FFGraph` reference was created there.
* Confirmed that the Tcl `File > Load` menu is bound to `modPress l` and
  `File > Merge` to `modPress m`.
* Confirmed with the archived executable under Xvfb that command-line loading
  of `nase.gml` leaves a Tk process running until the test's timeout. This is
  **not** a test of the patched executable.
* GCC C++17 syntax checks of changed `FFWindow.cc`, `FFTclBasicWindow.cc` and
  the new regression driver, using the supplied Tcl/Tk 8.6 headers: passed.
* Git whitespace and clean application validation: passed.
* **Not run here**: compilation/linking of the full modern ffgraph executable
  and execution of the new Xvfb regression. The environment lacks complete
  Tk/Xpm development packages and Bison/Flex. The user's Ubuntu build is
  required for runtime confirmation of the fix.

## Ubuntu build and verification

After applying this incremental patch on top of the latest Pass 11 source:

```sh
cmake -S . -B build/gml-modern-default -G Ninja \
  -DCMAKE_BUILD_TYPE=Debug -DBUILD_TESTING=ON \
  -DFFGRAPH_GML_BACKEND=modern \
  -DFFGRAPH_ENABLE_MODERN_GML=ON
cmake --build build/gml-modern-default --target \
  ffgraph test_load_window_lifetime_pass11 --parallel 2
ctest --test-dir build/gml-modern-default \
  -R '^test_load_window_lifetime_pass11$' \
  --output-on-failure -V
```

Then launch `./build/gml-modern-default/ffgraph` and use **File > Load**
with `nase.gml` and `nase2.gml`; the newly opened window should remain open
until explicitly closed. Repeat for multiple windows and check **File > Merge**.
Finally, run the regular GML differential tests and the full CTest suite.

A sanitizer build using the project's `-DFFGRAPH_SANITIZERS=ON` option is
recommended, especially when opening and closing many windows. The new test
is registered only when Xvfb is found.

## Known limitations

The owner registry is cleaned after a Tcl event finishes, not during the
callback that deletes a window. A Tk root destroyed directly without removing
its graph label can keep its graph resident until the last Tk root closes.
This mirrors the pre-existing main-program owner lifetime and avoids
re-entrant deletion of C++ graphics during active Tcl evaluation.
