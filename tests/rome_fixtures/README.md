# Legacy Rome baseline

These eight small fixtures are hand-written characterization inputs, not an
external Rome graph collection. `test_rome_baseline_pass12` checks independently
specified node IDs and edge `(id, source ID, target ID)` multisets against the
real checked-in Rome parser. Expectations were established before migration.

Each input is loaded through public filename dispatch, direct stream Parse,
direct string Parse with CRLF, and direct string Parse without a final newline.
Two further cases check merging through the public and direct entry points.
Checks remain enabled in Release. No display is required.

| Fixture | Observed contract |
|---|---|
| structure | Ignored second column, disconnected node, parallel edges, loop, comments and separator |
| duplicates | Duplicate/negative node IDs produce distinct nodes; loops remain |
| forward-missing | Forward references and missing endpoints skip edges with stderr warnings, without a user error |
| punctuation | Unrecognized punctuation is silently skipped by the scanner |
| partial-error | Three-column row rejects after earlier nodes/edge were constructed |
| keyword-error | KEY rejects, preserving the preceding node |
| string-error | STRING rejects, preserving the preceding node |
| empty | Comments/blank lines/separator produce no nodes or edges |

All imports force undirectedness and preserve the graph name. Public file loads
send START_LOAD/END_LOAD and reset existing node IDs to -1; direct Parse retains
existing IDs and does not send those events. Syntax errors currently report
line 1 regardless of their actual location. Duplicate-ID endpoint choice is
ambiguous; these tests check endpoint values/membership without prescribing
which duplicate node the legacy container iteration finds first.

The baseline exposed scanner buffer and rejected-text leaks. The accompanying
cleanup changes scanner ownership only; no Rome grammar or loading policy was
changed. See `docs/FFGRAPH-PASS12-ROME-BASELINE.md` for evidence and limits.
