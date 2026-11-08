# FF import indexing compatibility audit — 3 November 2026

The next-step request included investigating repeated FF node-ID scans while
preserving duplicates, merge and callbacks. Existing profiling establishes
approximately quadratic growth. Production lookup/replay is **unchanged** here;
this audit supplies a concrete regression that a fast index must satisfy.

`ff_graph_adapter.cc` resolves each ID using `FFGraphClass::nodeWithValue`, which
returns the first match in the graph container's current traversal. Replay invokes
user-provided label parsers and callbacks. `FFNodeClass::setValue` writes directly
without notifying the graph, and callbacks can renumber an earlier node into a
duplicate ID. Graph membership and the formerly cached node's ID can both remain
unchanged while the correct first match changes.

## Permanent reproducer

Added `lookupMutationOracle` to
`tests/modern_ff/test_ff_graph_regression_pass12.cpp`. Two existing nodes begin
with IDs 1 and 2, in known container traversal order. Replay first resolves ID 2,
then a real node-label parser changes the earlier node from 1 to 2. A subsequent
target-2 reference must resolve to that earlier node, creating the expected
self-loop. The old cached node still has ID 2, so a cache that checks only its
pointer/value remains wrong. The test uses the existing resetExistingNodeIds=false
adapter contract and asserts actual object incidence, not parse success.

The production regression passes in Debug, Release and strict sanitizers as part
of the full **75/75**, **77/77** and **63/63** headless results. A separate local
control compiled the actual adapter with a per-import ID cache and pointer/value
validation. It exited **1 as expected**, failing specifically on the new
`callback must resolve current winner, not cached pointer` oracle. This deliberate
failure is not a failing production test and was not registered in CTest.

Control source, exact compile/link commands and failure log are retained in
`build/ff-import-cache-audit/`. The production adapter itself has no diff.

## Remaining work

A safe index needs mutation tracking for node membership and direct ID changes,
plus preservation of the graph's first-match policy for duplicates. Possible
future work is indexed unique IDs with traversal fallback for duplicates and
explicit invalidation hooks; it needs a separate ownership/container/API design
and regressions for removal, callback reentrancy, copies and merge. Graph-size
checks or cached-pointer ID checks alone do not satisfy this reproducer.

FF import acceleration therefore remains open. This checkpoint deliberately
does not ship a semantically incorrect cache or silently change callback behavior.
Rome optimization remains low priority for its deprecated format; its loader
and the modern parsers remain supported and tested. See the combined checkpoint
provenance in `build/remaining-steps-checkpoint/`. No commits or pushes were made.
