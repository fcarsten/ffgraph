# Pass 11 — first modern parser/scanner pilot (GML)

**Patch baseline:** the latest `ffgraph-current(1).zip` supplied on 1 November
2026, with Pass 8, Pass 8 follow-up, Pass 9, and Pass 10 applied in that order.
This is an **experimental, opt-in first stage**, not a claim that all of Pass 11
is completed or the modern GML parser is drop-in-compatible yet.

## Inventory and compatibility boundary

| Format/path | Original grammar | Original scanner | Legacy generated files | Status |
|---|---|---|---|---|
| sgraph | `fgraph/FFParser.y` | `fgraph/FFScanner.l` | `FFParser.{cc,h}` and `FFScanner.{cc,h}` | unchanged |
| Graphlet/GML | `fgraph/GTParser.y` | `fgraph/GTScanner.l` | `GTParser.{cc,h}` and `GTScanner.{cc,h}` | unchanged, including the Pass 10 token-lifetime fix |
| Rome | `fgraph/RomeParser.y` | `fgraph/RomeScanner.l` | `RomeParser.{cc,h}` and `RomeScanner.{cc,h}` | unchanged |

The `.y` and `.l` files contain 1990s `bison++` and `flex++` class extensions
such as `%name`, `%define LEX`, `%header` and `%define MEMBERS`. They are not
valid modern GNU Bison/Flex inputs. The current application MUST continue
using the checked-in generated C++ code; do not point Bison or Flex directly
at these old grammars or delete the generated files.

## Changes

- `fgraph/modern/modern_gml.y` uses modern Bison C API with a pure, prefixed
  parser and explicit scanner/collector parameters. A `%destructor` releases
  scanner-produced string tokens on Bison error-recovery/discard paths.
- `fgraph/modern/modern_gml.l` uses modern reentrant Flex, with a separate
  symbol prefix. It recognises the old GML token vocabulary: signed decimal
  integers, signed decimal reals, keys, raw quoted strings, list delimiters
  (`[]` or `{}`), and column-1 `#` comments. Like the old scanner it ignores
  unrecognised characters. Integers within the `int` range and ordinary
  floating point values are the intended compatibility domain; overflow and
  non-ASCII/unterminated input need further parity work.
- `gml_events.h/.cc` provides a minimal independent ordered attribute-event
  sink and memory-safe scanner lifetime wrapper. It **does not modify graphs**:
  original GTParser's `current_node`, `current_edge`, nested-list and
  `gmlHash` side effects are not yet migrated. The application does not use
  it yet.
- `tests/modern_gml` is a **standalone** generator/compile/test project,
  independent of Tcl/Tk/Xpm; the root project can also include it with
  `-DFFGRAPH_ENABLE_MODERN_GML=ON` (default OFF).
- `test_gml_modern_pass11` exercises representative graph attributes, raw
  escaped quotes, mixed bracket kinds, loops, repeated parser construction,
  empty input, malformed/truncated input and recovery for the next parse.
- `.github/workflows/gml-parser-check.yml` is a **manually dispatched** Ubuntu
  generator+test workflow; it is not part of the already-proven default CI.

## Generator verification commands (Ubuntu 24.04)

```sh
sudo apt-get install --yes build-essential cmake ninja-build bison flex
cmake -S tests/modern_gml -B build/modern-gml -G Ninja \
  -DCMAKE_BUILD_TYPE=Debug -DBUILD_TESTING=ON
cmake --build build/modern-gml --parallel 2
ctest --test-dir build/modern-gml --output-on-failure
```

You can also trigger the **Modern GML generator pilot** workflow on a GitHub
branch. With full Tcl/Tk development dependencies, the root project can run
both sets of tests with `cmake --preset debug -DFFGRAPH_ENABLE_MODERN_GML=ON`.
The generator's `.cpp` and `.hpp` outputs live exclusively in the build tree;
no checked-in generated parser is replaced or rewritten.

## Actual verification in this environment

- `bison` and `flex` were **not installed**. A package-index update failed
  because Debian repository DNS resolution was unavailable. Thus **no modern
  generated scanner/parser was built or run here**, and the manually
  dispatched GitHub job has **not been executed**.
- The new `gml_events.cc` wrapper and test driver passed a C++17
  `-Wall -Wextra -Werror -fsyntax-only` compile using a clearly identified
  temporary **stub** for the Bison-generated header. This does **not** verify
  the actual generator output or its ABI.
- The existing Pass 10 GML/sgraph round-trip executable (already built from
  the legacy parser sources) exited successfully under ASan/UBSan with leak
  detection enabled. This **is not** a test of the modern pilot.
- The full main CMake build cannot configure here without Tk/Xpm development
  dependencies; it was not retested as a complete application.

## Remaining work / acceptance criteria for replacing the default GML parser

1. Run the real GNU Bison/Flex standalone generation and fix any warnings,
   compile errors or test failures. Make it green on both Debug and sanitizer
   jobs with `ASAN_OPTIONS=detect_leaks=1`.
2. Move the old GML semantic actions into a shared, tested event consumer and
   compare **actual FFGraphClass results**, not just token events, on a larger
   corpus (node IDs, edge resolution, names, position labels, parallel edges,
   loops, nested attributes, quoted escapes, malformed and truncated files).
3. Compare error recovery/diagnostics and failure behaviour; add limits for
   excessive nesting/input size and numeric out-of-range handling, with
   explicitly recorded deviations from the legacy format.
4. Only then opt in to using the new GML parser for real application imports;
   retain the old implementation as a fallback until parity is demonstrated.
5. Migrate the remaining FF/sgraph and Rome parsers independently, without
   changing Tcl/Tk, file formats, the parked inspector issue, or SPQR.

**Do not interpret this patch as a completed migration or as a successful
Bison/Flex compilation.** It is the conservative isolated first checkpoint.
