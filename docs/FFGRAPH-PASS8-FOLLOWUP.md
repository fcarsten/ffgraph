# ffgraph — Pass 8 follow-up: AVL exception safety, Pix audit and local verification

**Date:** 1 November 2026
**Baseline:** `ffgraph-current(1).zip` plus `pass8-container-correctness.patch`.
**Patch scope:** This follow-up only; apply it **after** the previous Pass 8 patch.
**Deliberately excluded:** Inspector/WSLg troubleshooting, SPQR, changes to Tcl/Tk GUI behaviour and graph formats.

## Code changes

1. `AVLSet` copy constructor cleans up already cloned nodes if an element copy throws. Previously the constructor failed without running its destructor, leaking those nodes.
2. `AVLSet::operator=` uses copy-then-swap of the tree and count. A failure leaves the destination unchanged, while self-assignment retains existing behaviour.
3. `AVLSet::add` and `AVLMap::operator[]` increment the element count **after** successful node allocation, both for first-root and threaded-tree insertions. Previously failed allocation / element construction could leave `count` ahead of the tree.
4. `AVLMap` copy constructor clears its partially built tree before rethrowing an exception, avoiding leaks from failed copies.
5. Added `test_container_avl_exception_followup`, registered in the existing CMake/CTest container loop. It injects element-copy exceptions to reproduce the old leaks/corruption and checks assignment preservation, root/child insertions, map copying, invariants and retained element counts.
6. Clarified the **existing** mutable-key interface warning in `VHMap`, `VHBag`, and `VOHSet` without changing method signatures or normal operations.

## Verification performed in this environment

- Environment: Debian 13, GCC 14.2, C++17, installed Tcl/Tk 8.6.16 runtimes, supplied Tcl/Tk 8.6 public headers, Xvfb.
- **Reproduced the failing baseline**: the new AVL exception test aborts on `Tracked::alive` mismatch before the fix, confirming a leak in a throwing AVLSet copy constructor. The test then passes with the fix under address and undefined-behaviour sanitizers.
- **45/45 standalone test runs passed**: 15 container/string tests in Debug (`-O0 -g`), 15 in optimised mode (`-O2 -g -UNDEBUG`), and 15 with ASan/UBSan. For sanitizer runs, the first four tests completed with `-O1 -g -fsanitize=address,undefined` before the initial matrix execution timeout; the remaining tests were compiled with `-O0 -g -fsanitize=address,undefined`. ASan used `detect_leaks=1:halt_on_error=1`; UBSan used `halt_on_error=1`.
- Compiled and passed the real `tests/test_tcl_compat.cpp` test against the **user-supplied Tcl 8.6 public headers** and locally installed `/usr/lib/x86_64-linux-gnu/libtcl8.6.so`.
- `xvfb-run -a tclsh` successfully executed `package require Tk` (Tk 8.6.16) and destroyed the root window. This is only a runtime smoke check, **not** a ffgraph window-lifecycle test.
- **Full CMake/CTest build is NOT verified.** `cmake --preset debug`, `release`, and `asan` each stop on `pkg_check_modules(TK REQUIRED ...)` because `tk.pc` (the Tk development metadata) is missing here. Also absent are the Xpm development header `X11/xpm.h` and unversioned link library. Tcl/Tk header files alone cannot supply these dependencies. No full ffgraph executable, project CTest, installed application or GUI lifecycle test is claimed.

## Container compatibility audit and remaining limitations

- **Hash map/set/bag keys:** `VHMap::key(Pix)`, `VHBag::operator()(Pix)`, and `VOHSet::operator()(Pix)` expose mutable legacy references even though the underlying C++17 hash containers require immutable keys. In-place key changes may break hashing and are **unsupported**. Existing callers reviewed use mapped values or read keys; no demonstrated mutation requiring API changes was found. For changing a key, delete and reinsert the element. A different, safe API would require explicit design and call-site migration; not done in this conservative patch.
- **Pix lifetime:** `VOHSet` has address-membership validation from Pass 8; `VHMap`/`VHBag` validate addresses through scans; AVLSet's `owns` compares addresses without dereferencing invalid handles. After erase, clear or assignment a Pix must be treated as invalid. Address reuse can cause a *stale Pix* to match a newly allocated node; these legacy pointer-based handles cannot guarantee generational identity. Do not retain Pix across structural mutations unless API semantics specifically support that case. AVL `next`, `prev`, `key`/`contents`, and `operator()` assume valid Pix: passing a stale Pix may dereference freed memory. We did not add O(N) validation to their hot traversal paths.
- **XPlex:** `include/container/XPlex.h` is a separate, older, presently unused implementation, unlike the STL-backed MPlex. An isolated strict C++17 compilation probe did **not** compile: numerous dependent-base names lack `this->` and the class does not override required `MFPlex::nextInt`/`prevInt` virtuals. Its pointer/empty-range operations therefore received **no runtime or ASan validation**. It was excluded rather than silently modernised without known users or semantics. If XPlex is needed, treat its rebuild and Pix contract as a separately scoped compatibility project.
- **Remaining AVL exception cases:** Rebuilding set algebra (`|=`, `&=`, `-=`) and comparator-throw paths still use legacy direct tree manipulation and have not been shown strongly exception-safe. The current patch targets demonstrable copy and insertion bugs, not a replacement of the AVL algorithms.
- **Other checks needed:** Ubuntu 24.04 full CMake Debug/Release/ASan builds and CTest, Tk/Xvfb teardown, staging install, representative graph open/layout/save, performance with real workloads.

## Reproduction commands on Ubuntu 24.04

Install dependencies using your normal package manager (e.g. `sudo apt install build-essential cmake ninja-build pkg-config tcl-dev tk-dev libx11-dev libxpm-dev xvfb`). From a checkout matching the user's original uploaded archive, apply the original Pass 8 patch and **then** this follow-up patch:

```sh
git apply --check pass8-container-correctness.patch
git apply pass8-container-correctness.patch
git apply --check ffgraph-pass8-followup.patch
git apply ffgraph-pass8-followup.patch
cmake --preset debug
cmake --build --preset debug --parallel
ctest --preset debug --output-on-failure
cmake --preset release
cmake --build --preset release --parallel
ctest --test-dir build/release --output-on-failure
cmake --preset asan
cmake --build --preset asan --parallel
ctest --preset asan --output-on-failure
```

To run the new regression independently of CMake/Tk:

```sh
c++ -std=gnu++17 -O0 -g -fno-omit-frame-pointer -fsanitize=address,undefined \
  -Iinclude -I. tests/test_container_avl_exception_followup.cpp -o /tmp/test_avl_exception
ASAN_OPTIONS=detect_leaks=1:halt_on_error=1 UBSAN_OPTIONS=halt_on_error=1 /tmp/test_avl_exception
```

The patch was generated as a Git unified diff and was checked for whitespace, clean application and no generated build outputs.
