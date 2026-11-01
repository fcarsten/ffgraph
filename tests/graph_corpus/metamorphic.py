#!/usr/bin/env python3
"""Deterministic, independent GML metamorphic cases for import/layout testing.

The source graph oracles remain unchanged. For each selected graph, emit a
series of transformations preserving its *undirected labelled multigraph*:
node-ID bijection, declaration reorder, edge orientation reversal, coordinate
translation, coordinate scaling. Output is not an expected layout image.
"""
from __future__ import annotations

import argparse
import hashlib
import json
from pathlib import Path

from generate import build_corpus, fmt, parse_attributes, write_corpus

TRANSFORMS = ("relabel_ids", "reorder_declarations", "reverse_edges",
              "translate_xy", "scale_xy")
DX = 64.125
DY = -32.750
SCALE = 2.0
SMOKE = ("empty", "singleton", "path_8", "cycle_8", "star_16",
         "grid_4x4", "two_components", "parallel_and_loops",
         "coincident_positions", "negative_coordinates", "nonconsecutive_ids")
STANDARD = ("ladder_20", "sparse_random_64", "dense_random_32", "disconnected_isolates")
STRESS = ("grid_20x20", "random_256_1024")


def encode_graph(name: str, nodes: list[dict], edges: list[dict]) -> bytes:
    """Use a deliberately simple legal subset of modern_gml.y.

    Retain labels and edge IDs: these provide stable independent identities for
    structure comparisons even when numeric node IDs have been permuted.
    """
    lines = ["# metamorphic GML fixture; generated, do not edit", "graph [",
             f'  label "corpus_{name}"', "  directed 0"]
    for n in nodes:
        lines.extend(("  node [", f'    id {n["id"]}',
                      f'    label "{n["label"]}"',
                      "    graphics [", f'      x {fmt(n["x"])}',
                      f'      y {fmt(n["y"])}', "      w 10.0", "      h 10.0",
                      "    ]", "  ]"))
    for e in edges:
        lines.extend(("  edge [", f'    id {e["id"]}',
                      f'    source {e["source"]}', f'    target {e["target"]}',
                      f'    label "{e["label"]}"', "  ]"))
    lines.append("]")
    result = ("\n".join(lines) + "\n").encode("ascii")
    parse_attributes(result.decode("ascii"))
    return result


def transform(nodes: list[dict], edges: list[dict], kind: str) -> tuple[list[dict], list[dict]]:
    assert kind in TRANSFORMS
    new_nodes, new_edges = ([n.copy() for n in nodes], [e.copy() for e in edges])
    if kind == "relabel_ids":
        mapping = {n["id"]: 500000 + (len(nodes) - index) * 19
                   for index, n in enumerate(nodes)}
        for n in new_nodes:
            n["id"] = mapping[n["id"]]
        for e in new_edges:
            e["source"] = mapping[e["source"]]
            e["target"] = mapping[e["target"]]
    elif kind == "reorder_declarations":
        new_nodes.reverse()
        new_edges.reverse()
    elif kind == "reverse_edges":
        for e in new_edges:
            e["source"], e["target"] = e["target"], e["source"]
    elif kind == "translate_xy":
        for n in new_nodes:
            n["x"] = round(n["x"] + DX, 3)
            n["y"] = round(n["y"] + DY, 3)
    elif kind == "scale_xy":
        for n in new_nodes:
            n["x"] = round(n["x"] * SCALE, 3)
            n["y"] = round(n["y"] * SCALE, 3)
    return new_nodes, new_edges


def build_metamorphic(seed: int, profile: str) -> tuple[dict[str, bytes], dict]:
    _, source_manifest = build_corpus(seed, profile)
    source_graphs = {c["name"]: c for c in source_manifest["cases"] if c["category"] == "graph"}
    selected = SMOKE + (STANDARD if profile in ("standard", "stress") else ()) + (
        STRESS if profile == "stress" else ())
    files: dict[str, bytes] = {}
    cases = []
    listing = ["#case\ttransform\tbase\tvariant\tnodes\tedges"]

    for name in selected:
        case = source_graphs[name]
        base = f"base/{name}.gml"
        original = encode_graph(name, case["nodes"], case["edges"])
        files[base] = original
        cases.append({"name": name, "category": "base", "file": base,
                      "sha256": hashlib.sha256(original).hexdigest()})
        for kind in TRANSFORMS:
            nodes, edges = transform(case["nodes"], case["edges"], kind)
            variant = f"variant/{name}__{kind}.gml"
            data = encode_graph(name, nodes, edges)
            files[variant] = data
            cases.append({"name": name + "__" + kind, "category": "variant",
                          "file": variant, "sha256": hashlib.sha256(data).hexdigest(),
                          "base": base, "transform": kind})
            listing.append(f'{name}\t{kind}\t{base}\t{variant}\t'
                           f'{len(nodes)}\t{len(edges)}')
    files["cases.tsv"] = ("\n".join(listing) + "\n").encode("ascii")
    manifest = {"format_version": 1, "seed": seed, "profile": profile,
                "grammar": "fgraph/modern/modern_gml.y and modern_gml.l",
                "notes": "metamorphic graph import/layout; compare by labels and undirected edge multiplicity",
                "transforms": list(TRANSFORMS), "cases": cases}
    files["manifest.json"] = (json.dumps(manifest, indent=2, ensure_ascii=True) + "\n").encode()
    return files, manifest


def main() -> None:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--output-dir", type=Path, required=True)
    parser.add_argument("--profile", choices=("smoke", "standard", "stress"), default="smoke")
    parser.add_argument("--seed", type=int, default=20261101)
    parser.add_argument("--force", action="store_true")
    parser.add_argument("--check", action="store_true")
    args = parser.parse_args()
    if args.force and args.check:
        parser.error("--force and --check are mutually exclusive")
    files, manifest = build_metamorphic(args.seed, args.profile)
    write_corpus(args.output_dir, files, check=args.check, force=args.force)
    print(f'{args.profile}: {len(manifest["cases"])} metamorphic files; '
          f'{sum(c["category"] == "variant" for c in manifest["cases"])} transformation pairs; '
          f'{"verified" if args.check else "generated"} at {args.output_dir}')


if __name__ == "__main__":
    main()
