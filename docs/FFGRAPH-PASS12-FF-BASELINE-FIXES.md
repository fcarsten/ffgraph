# Pass 12 — baseline failure fixes

Date: 1 November 2026. Branch: `pass12-modern-ff-parser`.
Starting HEAD: `f074e8744a1efe6cca495596c7bea90e5b122bce`.
The checkout was clean; the user had committed the previous pilot checkpoint.
No commits or pushes were made during this follow-up.

## Reproduced failures and fixes

Read the pilot checkpoint, current source, and relevant Tcl/label lifetime
reports before editing. Reproduced both failing Release container tests and
both failing GUI sanitizer tests. The separate malformed-FF diagnostic again
failed with the original four-byte graph-name leak.

### Edge label ID lifetime

`FFGraphicEdge::getLabelId()` returns a `String` by value.
`FFTclBasicWindow::changeLabel(FFGraphicEdge*, ...)` converted that temporary
into a `const char*` in one statement and used it in the next. The temporary
had already expired. Both GUI tests reproduced `stack-use-after-scope`.

`fgraphic/FFTclBasicWindow.cc` now retains the returned `String` until
`FF_TclEvalObjv` finishes copying command arguments. The node overload's getter
returns a reference and does not have this defect. No window-ownership,
interpreter-deletion, callback, menu or Tcl command semantics were changed.

`tests/test_load_window_lifetime_pass11.cpp` now also renames a real imported
edge repeatedly and reads its canvas `-text` property back through Tcl. It
checks short, long, empty and multiline names containing quotes, braces,
semicolon, brackets and dollar signs. Both the original File/Load lifecycle
coverage and the actual canvas checks run in Debug, Release and sanitizers.

### Release tests removing operations

`test_mplex_stl` threw `MPlex: no unused index` and
`test_container_mplex_pass8` threw `MPlex: delete low from empty plex`.
Mutations such as `add_high`, `del_index` and `reset_low` were inside `assert()`;
Release's `-DNDEBUG` removed the operations as well as their checks.

Both test sources now use the small, test-only `tests/test_check.h` helper.
`FF_CHECK` evaluates its condition once in every configuration, prints the
failed expression and location, and aborts on failure. Exception-path checks
and randomized model comparisons stay active under Release too. No container
implementation, production assertion policy, compiler flags or test selection
was changed.

### Legacy FF error-token ownership

The old scanner allocated `IDENTIFIER` text with `strdup`; successful grammar
actions freed consumed tokens. Error recovery and the `parser_start: error`
abort discarded stack/lookahead pointers without freeing them. A graph name
on an incomplete graph was the simplest leak, but incomplete label arguments
and unexpected lookahead strings could leak too.

`FFScanner.l` and checked-in `FFScanner.{h,cc}` now retain identifier copies in
`std::list<String> tokenText`. Stable list elements supply borrowed `char*`
semantic values. `FFParser.y` and checked-in `FFParser.cc` remove the 21 token
`free` calls consistently. Scanner destruction frees all tokens, including
those discarded during parse errors. Each Parse entry clears storage from
an earlier invocation. The scanner's existing generated destructor still owns
its input buffer; no second buffer owner is added.

The grammar, scanner patterns, lexical acceptance, graph construction order,
label callbacks, diagnostics and legacy partial-mutation policy are unchanged.
In particular, malformed legacy `load()` still resets IDs and may create nodes.
This fix is **ownership cleanup**, not transactional loading or a backend switch.

`tests/test_graph_io_pass10.cpp` now repeats eight invalid FF inputs twenty
times through **both** FFControl APIs: **320 rejected parses**, followed by
20 successful fresh parses. Cases exercise retained graph names, incomplete
node colors/icons/masks and edge/graph label arguments, ignored internal strings,
unterminated text and unexpected lookahead after `END`. A test controller
captures syntax errors through the existing virtual error callback. Existing
file/string round trips and the historical 499-node/499-edge sample remain.
This coverage runs even when the modern FF pilot is OFF.

## Verification actually executed

Host: Ubuntu 24.04, GCC 13.3.0, CMake 3.28.3, Ninja 1.11.1, Tcl/Tk 8.6.14,
Bison 3.8.2, Flex 2.6.4, Xvfb. Used the existing preset build directories.
Debug has the FF pilot OFF; Release and ASan have it ON.

| Check | Result |
| --- | --- |
| Before fixes: two focused Release tests | 0/2 passed; both original exceptions reproduced |
| Before fixes: two focused GUI ASan tests | 0/2 passed; original stack-use-after-scope reproduced |
| Before fixes: `--legacy-malformed`, leak detection ON | FAIL; four bytes leaked in one `strdup` allocation |
| Debug configure/build, full CTest | PASS; **42/42 passed** |
| Release configure/build, full CTest | PASS; **45/45 passed** |
| ASan/UBSan configure/build | PASS |
| Full ASan/UBSan, `detect_leaks=1` | **43/45 passed**; two newly exposed Tk/fontconfig leak failures described below |
| Full ASan/UBSan, supplemental `detect_leaks=0` | **45/45 passed**; no ASan/UBSan runtime errors |
| `test_graph_io_pass10`, leak detection ON | PASS, including 320 rejected legacy parses and existing valid round trips |
| All three modern FF compatibility/oracle/rejection tests, leak detection ON | PASS |
| Original `--legacy-malformed`, leak detection ON after fixes | PASS; original four-byte leak gone; historical partial mutation preserved |
| `git diff --check` | PASS |

No CTest was disabled, skipped, marked as expected failure or given a sanitizer
suppression. The supplemental no-leak run is a separate runtime-check result,
**not** a claim that the full leak-enabled suite passes.

## Remaining toolkit leaks, independently isolated

After the GUI tests reached normal shutdown, LeakSanitizer reported font
allocations that had been masked by the earlier ASan abort:

- `test_load_window_lifetime_pass11`: **9,367 bytes / 304 allocations**.
- `test_gui_file_workflow_pass11`: **59,697 bytes / 2,208 allocations**.

The stacks originate in `libfontconfig`, through Xft and Tk font allocation
while constructing menus. A dependency-only reproducer,
`tests/repro_tk_menu_font_leak.cpp`, links **only Tcl/Tk**, creates a menu and
menubutton, explicitly destroys its root, processes idle work, confirms its
interpreter-deletion callback, and calls `Tcl_Finalize()`. It reproduces
**9,359 bytes / 304 allocations** on this host, with no ffgraph code linked.
It is a standalone diagnostic; the two registered GUI tests continue to
report their leak failures normally.

Additional isolated experiments: a button without a menu left only a
320-byte / three-allocation fontconfig configuration leak. Calling `FcFini()`
after `Tcl_Finalize()` removed those 320 bytes; it did not fix the menu font
leaks. A menu-only reproducer still leaked 9,029 bytes / 301 allocations even
with `FcFini()`. Disabling tearoff, explicitly destroying the root first and
draining idle cleanup also did not eliminate the menu leak. No experimental
`FcFini`, dynamic-loader hooks, menu workarounds or shutdown changes were kept
in the application/tests.

The attribution to the installed Tk/Xft/fontconfig dependency stack is based
on these local reproductions. Relevant primary references consulted:
[fontconfig's finalization contract](https://fontconfig.pages.freedesktop.org/fontconfig/fontconfig-devel/#FcFini),
[Tk 8.6.14 menu cleanup implementation](https://github.com/tcltk/tk/blob/core-8-6-14/generic/tkMenu.c),
and [Tk 8.6.14 Xft font implementation](https://github.com/tcltk/tk/blob/core-8-6-14/unix/tkUnixRFont.c).
No specific upstream fix or newer library version is claimed to solve this
host's leak; that requires testing another dependency build.

## Commands

Before-fix reproduction:

```sh
ctest --test-dir build/release \
  -R '^test_(mplex_stl|container_mplex_pass8)$' --output-on-failure
ASAN_OPTIONS=detect_leaks=1:halt_on_error=1 \
UBSAN_OPTIONS=print_stacktrace=1:halt_on_error=1 \
ctest --preset asan \
  -R '^test_(load_window_lifetime|gui_file_workflow)_pass11$' --output-on-failure
```

Build and full verification (all executed):

```sh
cmake --preset debug
cmake --build --preset debug --parallel 2
ctest --preset debug --output-on-failure

cmake --preset release
cmake --build --preset release --parallel 2
ctest --test-dir build/release --output-on-failure

cmake --preset asan
cmake --build --preset asan --parallel 2
ASAN_OPTIONS=detect_leaks=1:halt_on_error=1 \
UBSAN_OPTIONS=print_stacktrace=1:halt_on_error=1 \
ctest --preset asan --output-on-failure

# Separate runtime check; does not replace the strict leak-enabled run.
ASAN_OPTIONS=detect_leaks=0:halt_on_error=1 \
UBSAN_OPTIONS=print_stacktrace=1:halt_on_error=1 \
ctest --preset asan --output-on-failure

ASAN_OPTIONS=detect_leaks=1:halt_on_error=1 \
UBSAN_OPTIONS=print_stacktrace=1:halt_on_error=1 \
build/asan/tests/modern_ff/test_ff_compatibility_pass12 --legacy-malformed
```

Dependency-only reproducer (compiled/run independently):

```sh
c++ -std=c++17 -g -fsanitize=address,undefined -fno-omit-frame-pointer \
  tests/repro_tk_menu_font_leak.cpp -I/usr/include/tcl8.6 \
  -ltk8.6 -ltcl8.6 -o /tmp/ff-pass12-tk-menu-dependency
ASAN_OPTIONS=detect_leaks=1:halt_on_error=1 \
UBSAN_OPTIONS=print_stacktrace=1:halt_on_error=1 \
xvfb-run -a /tmp/ff-pass12-tk-menu-dependency
```

GUI/LSan execution needed approval outside sandbox display/tracing restrictions.
Logs are ephemeral `/tmp/ff-pass12-fixes-*`, including
`release-before.log`, `asan-before.log`, `legacy-before.log`, `legacy-after.log`,
`debug-ctest.log`, `release-ctest.log`, `asan-ctest.log`, `asan-runtime.log` and
`tk-dependency.log`.

## Remaining risks and scope

- Full leak-enabled sanitizer CTest is still **not green** due to the reproduced
  dependency leaks. This patch fixes the original application lifetime error,
  Release test defects and FF token leak; it does not patch or upgrade system Tk.
- The legacy token pool retains all parsed identifier text until controller
  destruction (or the next Parse invocation), increasing peak memory relative
  to freeing successful tokens immediately. Large-scale memory limits and
  allocation failure/callback exception injection were not tested.
- The legacy scanner object's private layout changes: rebuild all consumers
  of `FFScanner.h` together. All three full CMake builds did so. Historical
  Bison++/flex++ regeneration was not run; source definitions and checked-in
  outputs were updated together and the latter were compiled/tested.
- No changes to production backend selection, modern FF/GML parsers, Rome,
  containers, graph ownership APIs, excluded SPQR or parked WSLg inspector work.
- Interactive desktop use and alternative Tk/fontconfig versions remain
  unverified. Existing Xvfb GUI Load/Merge/Close tests pass their functional
  checks in every configuration tested.
