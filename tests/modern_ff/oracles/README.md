# Frozen FF graph oracles

Captured before legacy FF retirement, from the actual `FFControl` parser at
`65e5498c40116c0ec85bccb3bbab5d060f098f57` plus a temporary snapshot-output hook.
The capture hook was removed after recording; expected data is never generated
from the production parser during a build or test.

Each graph had `FFGraphicNode`, `FFGraphicEdge`, and the test `Probe` label
prototypes installed. Snapshots sort node/edge records, preserve multiplicity,
and include names, node IDs, directedness, start/end nodes, Probe payloads,
coordinates, ranges, colors, icon/mask variants, shapes, weights and routes.
Floating values use the existing full-precision snapshot formatter. Quoted
names may contain raw newlines. The historical oracle describes `contrib/out.g`.

Independent hand-written assertions and source-graph writer round trips remain
in the test driver in addition to these frozen historical observations.

| Input | Input SHA-256 | Snapshot SHA-256 |
| --- | --- | --- |
| `tests/modern_ff/fixtures/attributes.g` | `8b673c0d676ff0c60c6f5e35c3100d26d107b3889cc1c651ecb29bd489d606e3` | `2e73d8ff5aa6ccaf15652fb87f64c805368b875c27941e1be212069e1a656ebc` |
| `tests/modern_ff/fixtures/comments.g` | `c648ebf6eac8300894ef65644b548377bee8934e18df16930bd69243619c4cd6` | `0201d6d46c34a280fccd30c6d6abcf672f2d243a8046d040dd19147bcd2183c4` |
| `tests/modern_ff/fixtures/empty.g` | `487f4d96dbccc4c9ba10ed7c70e931950b06f371202c4c95e60c2a7cfcc52045` | `6c3e56dd74cdfa0cfb9c95b44d2cf3b101e0a67652354c0cebb7d30f43a8c01d` |
| `tests/modern_ff/fixtures/gui.g` | `18f11d41184ef081b074e8e62aa2a46a5f60c41082ba407ad9e27e588f8a355b` | `b1d69dbbb03da22d83866764499598dcee30269e29638b3e605b3343e08d1f1f` |
| `contrib/out.g` | `d0f7b2dd760afec05cacf5205c5e7772338dae88285d773d9f59a8b9a57eda16` | `3d827e2804da6518d6dad18a29db1ccfa8535544652357d5cc646dec3e8e7f58` |
| `tests/modern_ff/fixtures/one-point.g` | `6ca83952b24834745e561ec40ad3316bce0c7b88bb3bc9fe2834716cdd31d087` | `9bef22d8ae13e8527179e40d11686769c4a6b0bc4774228451933e264f56e163` |
| `tests/modern_ff/fixtures/signed-ids.g` | `d69f69ed9c5613209635c1fdef812b1fc9f7265c82a31463f20efceb253b7604` | `5a4ccd22a51b7cb989a22f8c8715f8183fc8912f0f92bbeeab7cc20a41346ca1` |
| `tests/modern_ff/fixtures/string-comment.g` | `733a309569b3c4a0c93123ac19dc13785124752c941aae5eab2c7b8ca991d9b7` | `ac225d5f57acfd22ccdde19b1e77caa9629b3ebfe447bfb74c7ff857c93e2a82` |
| `tests/modern_ff/fixtures/strings.g` | `68d3bc4ef2f7af61557d6b2d2411a1c2bd58245ca967fa4e6775d114e2cc5548` | `f532b28b7d700483170c2d43b4cf01b771fd5c5f7af5f65ed2fa8e87b90efe7f` |
| `tests/modern_ff/fixtures/structure.g` | `f9ea1a4bd19c460c543d657a960eb508f9501ac90c7bd2139af4996c6a6c1f0d` | `02171cd3c6a6df4ced9e13c0169d49a8f4e187101f203cdcdaecc8ab79713b1c` |
