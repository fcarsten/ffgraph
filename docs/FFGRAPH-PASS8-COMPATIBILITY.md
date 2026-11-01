# Pass 8: container correctness and compatibility notes

This incremental patch is based on the user's **1 November 2026** `ffgraph-current(1).zip` archive. It does not change graph formats, the Tcl/Tk user interface, or the SPQR build exclusion. The inspector/WSLg issue is not part of this work.

## Contracts retained / clarified

- `MPlex` uses a half-open allocated range `[lo, fence)` and retains logically deleted interior slots. `length()` counts allocated slots (including holes); `count()` counts present elements. `low()` and `high()` identify the first/last **present** index. `Pix` is an opaque address of a slot value; `del_index` does not deallocate a slot, so a deleted slot's `Pix_to_index` still works for `undel_Pix`. `owns` is false for deleted slots.
- `MPlex::reset_low` preserves existing value addresses (and therefore existing live `Pix`) by extracting and reinserting `std::map` nodes. Growing or reindexing past representable `int` boundaries throws `std::out_of_range` **before** changing the object, rather than invoking signed-overflow undefined behaviour. An empty plex at the minimum `int` low index has no representable `high() == low()-1` sentinel; `high()` now throws. The same applies to `MFPlex::ecnef()` in this case.
- `MPlex` insertion and copy assignment commit metadata only after successful element copies, so an exception does not leave bounds inconsistent. `DLList` copy assignment now provides the strong guarantee for element-copy failures; successful assignments invalidate handles to the replaced elements as before.
- `VOHSet` (and `VHSet`) `Pix` are addresses of entries, stable across `rehash`/`resize` but invalid after erase/clear/assignment. A live-address index validates a `Pix` **without dereferencing an erased node**; `owns` returns false and `next` clears a stale handle. `operator()(invalid Pix)` now throws `std::out_of_range` rather than dereferencing freed memory. Copy/set-algebra/erase operations keep the address index in sync.
- Historical AVLSet code remains available through its existing compilation flags. Its tree-rebuild buffer is released with `delete[]`; self-union/intersection avoid rebuilding, and self-difference clears the set.

## Remaining API risks and limits

- The historical `Type&` / mutable `key(Pix)` interfaces in the set/map/bag classes expose keys stored inside standard hash tables. **Clients must not mutate a key while it is in a hash container.** Doing so violates lookup invariants. Changing these signatures would break existing APIs, so they are retained. The searched in-tree `VHMap::key(Pix)` call sites only inspect keys; this is not a proof about downstream users.
- No opaque-pointer compatibility layer can unambiguously distinguish an expired `Pix` from a *new* element allocated at exactly the same address (ABA/reuse). Do not use a `Pix` after its element is erased; invalid handles are still an error. The AVL containers retain their historical invalid-handle restrictions and mutable keys.
- `MPlex` still uses linear scans for `Pix`-to-slot lookup. `VHMap` and `VHBag` also scan addresses when resolving a Pix; this patch deliberately does not redesign them. These paths should be profiled under a representative large graph before optimisation.
- The AVL templates are legacy implementations, not a replacement for standard maps or sets. The new tests cover a selected functional subset, not every implementation path or every pathological tree shape.
- `XPlex` and legacy scanner/parser implementations are not changed; further independent review is appropriate if they become active build targets.

## Tests and environment evidence

Environment used for this pass: Debian 13, GCC 14.2.0, CMake 3.31, Ninja, `xvfb-run` available. The baseline `cmake --preset debug` configuration stopped at `pkg_check_modules(tk REQUIRED)` because Tk development metadata/headers are missing. An attempted apt package setup did not complete in this environment. Consequently **no full ffgraph CMake build, project CTest run, Xvfb/Tk test, installation test, or graphical smoke test is claimed**.

The following 14 **standalone** no-Tcl/Tk tests compile and pass with `c++ -std=gnu++17 -Iinclude -I.` in each mode: Debug (`-O0 -g`), optimised (`-O2 -UNDEBUG`, keeping test assertions enabled), and ASan/UBSan (`-O1 -g -fno-omit-frame-pointer -fsanitize=address,undefined`, `ASAN_OPTIONS=detect_leaks=1:halt_on_error=1`, `UBSAN_OPTIONS=halt_on_error=1`):

`test_dllist_stl`, `test_hash_pointer_compat`, `test_hashset_stl`, `test_mplex_stl`, `test_mplex_priority1`, `test_string_compat`, `test_strstream_compat`, `test_vhbag_stl`, `test_vhmap_stl`, `test_container_mplex_pass8`, `test_container_hash_pix_pass8`, `test_container_avl_pass8`, `test_container_avlmap_pass8`, `test_container_dllist_pass8`.

The five new tests plus previously existing but unregistered `test_mplex_priority1` are registered in the project `CMakeLists.txt` and need confirmation through **actual project CTest** on a machine with Tcl/Tk development dependencies installed.

Before fixing, focused ASan runs demonstrated `VOHSet::owns` use-after-free on erased Pix and AVLSet `operator new[]`/`operator delete` mismatch; the DLList assignment and MPlex integer-boundary tests also failed in the unpatched baseline. The new tests additionally exercise a deterministic 3,000-step sparse-plex model and a 1,200-step hash-set model, aliasing, rehash/copy/Pix validity, exception injection, AVL set/map operations, and integer limits.

**Illustrative Pix traversal benchmark:** 20,000 `VOHSet<int>` elements, four complete traversals with element reads, GCC `-O2` on this Debian container; original baseline: **0.68–0.88 ms**, patched: **0.80–1.00 ms** over three samples each. This only measures the tested path and machine. The address-membership index preserves expected constant-time `Pix` lookup but consumes additional memory; remeasure with realistic graph sizes if needed.

## Suggested verification on Ubuntu 24.04

```sh
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

If these pass, continue Pass 8 with a separate audit of mutable hashed keys and the historical AVL/XPlex lifetime paths; alternatively proceed to Pass 9 (graph invariants) with this patch as a tested incremental checkpoint.
