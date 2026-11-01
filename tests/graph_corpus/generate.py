#!/usr/bin/env python3
"""Reproducible GML grammar + graph-topology corpus for ffgraph.

Uses the KEY (INTEGER | REAL | STRING | list) grammar and lexical vocabulary
of fgraph/modern/modern_gml.{y,l}; does not try to emulate undocumented
error recovery of the historical scanner. Standard library only.

Generated data is disposable: the generator and golden metadata are the
source of truth. Never overwrite an existing directory without --force.
"""
from __future__ import annotations

import argparse
import hashlib
import json
import math
import random
import re
import shutil
from collections import Counter, deque
from dataclasses import dataclass
from pathlib import Path

VERSION = 1
PROFILES = ("smoke", "standard", "stress")
# Curated user-supplied historical examples: test syntax independently from
# the generated graph semantic oracle. The AGD options file is NOT a graph.
EXAMPLE_EXPECTATIONS = {
    "10dm.gml": "syntax", "fuenfer.gml": "syntax", "nase.gml": "syntax",
    "nase2.gml": "syntax", "nase3.gml": "syntax", "options.gml": "invalid",
}
# The grammar treats braces and brackets as the same token; no exponentials,
# plus-prefixed numerals, or escaped-string interpretation is assumed.
TOKEN = re.compile(r'''(?P<space>[ \t\r\n]+)|(?P<comment>\#[^\n]*(?=\n|\Z))|(?P<string>"(?:[^"\\]|\\.|\\\n)*")|(?P<real>-?[0-9]+\.[0-9]+)|(?P<int>-?[0-9]+)|(?P<key>[A-Za-z_][A-Za-z_0-9:]*)|(?P<open>[\[{])|(?P<close>[\]}])''')


def parse_attributes(source: str) -> list[tuple[str, object]]:
    """Strict independent grammar oracle for generated files, NOT legacy GTControl.

    Rejects unknown bytes rather than silently skipping them, to ensure our
    fixtures avoid reliance on legacy scanner recovery or tolerance.
    """
    tokens: list[tuple[str, str]] = []
    pos = 0
    while pos < len(source):
        found = TOKEN.match(source, pos)
        if not found:
            raise ValueError(f"unrecognised lexical input at offset {pos}")
        kind = found.lastgroup
        if kind == "comment" and (pos != 0 and source[pos - 1] != "\n"):
            raise ValueError("comments must start in column 1")
        if kind not in ("space", "comment"):
            tokens.append((kind, found.group()))
        pos = found.end()
    index = 0

    def parse_list(nested: bool = False) -> list[tuple[str, object]]:
        nonlocal index
        result: list[tuple[str, object]] = []
        while index < len(tokens):
            kind, word = tokens[index]
            if kind == "close":
                if not nested:
                    raise ValueError("unexpected list close")
                index += 1
                return result
            if kind != "key":
                raise ValueError(f"expected key, found {kind}")
            key = word
            index += 1
            if index == len(tokens):
                raise ValueError("missing attribute value")
            kind, word = tokens[index]
            index += 1
            if kind == "open":
                value: object = parse_list(True)
            elif kind == "int":
                value = int(word)
                if not -(2**31) <= value < 2**31:
                    raise ValueError("out-of-range integer")
            elif kind == "real":
                value = float(word)
            elif kind == "string":
                value = word[1:-1]  # Scanner preserves escapes literally.
            else:
                raise ValueError(f"missing value for {key}")
            result.append((key, value))
        if nested:
            raise ValueError("unclosed list")
        return result

    result = parse_list()
    if index != len(tokens):
        raise ValueError("unconsumed tokens")
    return result


@dataclass(frozen=True)
class Spec:
    name: str
    family: str
    n: int
    edges: tuple[tuple[int, int], ...]
    coords: str = "spread"
    features: tuple[str, ...] = ()
    id_start: int = 1
    id_stride: int = 1


def edge_path(n: int) -> list[tuple[int, int]]:
    return [(i - 1, i) for i in range(1, n)]


def edge_cycle(n: int) -> list[tuple[int, int]]:
    return edge_path(n) + ([(n - 1, 0)] if n >= 3 else [])


def edge_clique(n: int) -> list[tuple[int, int]]:
    return [(i, j) for i in range(n) for j in range(i + 1, n)]


def edge_grid(cols: int, rows: int) -> list[tuple[int, int]]:
    return [(r * cols + c, r * cols + c + 1)
            for r in range(rows) for c in range(cols - 1)] + [
            (r * cols + c, (r + 1) * cols + c)
            for r in range(rows - 1) for c in range(cols)]


def edge_tree(n: int) -> list[tuple[int, int]]:
    return [((i - 1) // 2, i) for i in range(1, n)]


def edge_bipartite(a: int, b: int) -> list[tuple[int, int]]:
    return [(i, a + j) for i in range(a) for j in range(b)]


def edge_ladder(length: int) -> list[tuple[int, int]]:
    return edge_path(length) + [(a + length, b + length) for a, b in edge_path(length)] + [
        (i, length + i) for i in range(length)]


def rng_for(seed: int, name: str) -> random.Random:
    digest = hashlib.sha256(f"{seed}:{name}".encode("utf-8")).digest()
    return random.Random(int.from_bytes(digest[:8], "big"))


def random_edges(name: str, seed: int, n: int, count: int) -> list[tuple[int, int]]:
    rng = rng_for(seed, name)
    seen: set[tuple[int, int]] = set()
    while len(seen) < count:
        a, b = rng.randrange(n), rng.randrange(n)
        if a != b:
            seen.add((min(a, b), max(a, b)))
    return sorted(seen)


def catalog(seed: int, profile: str) -> list[Spec]:
    specs = [
        Spec("empty", "empty", 0, ()),
        Spec("singleton", "isolated", 1, ()),
        Spec("two_isolates", "isolated", 2, ()),
        Spec("single_edge", "path", 2, ((0, 1),)),
        Spec("path_8", "path", 8, tuple(edge_path(8))),
        Spec("cycle_8", "cycle", 8, tuple(edge_cycle(8))),
        Spec("star_16", "star", 16, tuple((0, i) for i in range(1, 16))),
        Spec("clique_6", "clique", 6, tuple(edge_clique(6))),
        Spec("grid_4x4", "grid", 16, tuple(edge_grid(4, 4)), "grid"),
        Spec("binary_tree_15", "tree", 15, tuple(edge_tree(15))),
        Spec("two_components", "disconnected", 10,
             tuple(edge_path(4) + [(i + 4, j + 4) for i, j in edge_cycle(4)])),
        Spec("bipartite_4x5", "bipartite", 9, tuple(edge_bipartite(4, 5))),
        Spec("loop_singleton", "loop", 1, ((0, 0),), features=("self_loop",)),
        Spec("parallel_and_loops", "multigraph", 4,
             ((0, 1), (0, 1), (1, 0), (1, 1), (2, 2), (3, 0)),
             features=("parallel_edges", "self_loop")),
        Spec("coincident_positions", "geometry", 8, tuple(edge_cycle(8)), "coincident"),
        Spec("collinear_positions", "geometry", 12, tuple(edge_path(12)), "collinear"),
        Spec("negative_coordinates", "geometry", 8, tuple(edge_cycle(8)), "negative"),
        Spec("nonconsecutive_ids", "identifiers", 8, tuple(edge_path(8)),
             "spread", ("nonconsecutive_ids",), 101, 17),
    ]
    if profile in ("standard", "stress"):
        specs += [
            Spec("ladder_20", "ladder", 20, tuple(edge_ladder(10)), "grid"),
            Spec("wheel_16", "wheel", 16, tuple(edge_cycle(15) + [(15, i) for i in range(15)])),
            Spec("binary_tree_63", "tree", 63, tuple(edge_tree(63))),
            Spec("grid_8x8", "grid", 64, tuple(edge_grid(8, 8)), "grid"),
            Spec("sparse_random_64", "random_sparse", 64,
                 tuple(random_edges("sparse_random_64", seed, 64, 90))),
            Spec("dense_random_32", "random_dense", 32,
                 tuple(random_edges("dense_random_32", seed, 32, 240))),
            Spec("disconnected_isolates", "disconnected", 32, tuple(edge_cycle(12))),
            Spec("double_star", "articulation", 18,
                 tuple([(0, i) for i in range(2, 10)] + [(1, i) for i in range(10, 18)] + [(0, 1)])),
        ]
    if profile == "stress":
        specs += [
            Spec("path_512", "path", 512, tuple(edge_path(512)), "collinear"),
            Spec("grid_20x20", "grid", 400, tuple(edge_grid(20, 20)), "grid"),
            Spec("random_256_1024", "random", 256,
                 tuple(random_edges("random_256_1024", seed, 256, 1024))),
            Spec("tree_1023", "tree", 1023, tuple(edge_tree(1023))),
        ]
    return specs


def coordinates(n: int, mode: str, rng: random.Random) -> list[tuple[float, float]]:
    result = []
    for i in range(n):
        if mode == "coincident":
            x, y = (0.0, 0.0)
        elif mode == "collinear":
            x, y = (i * 3.5, 0.0)
        elif mode == "grid":
            width = max(1, math.ceil(math.sqrt(n)))
            x, y = ((i % width) * 42.5, (i // width) * 42.5)
        elif mode == "negative":
            x, y = (-200.25 - i * 13.5, -18.75 - i * 7.25)
        else:
            x, y = (round(rng.uniform(-250, 250), 3), round(rng.uniform(-250, 250), 3))
        result.append((x, y))
    return result


def fmt(x: float) -> str:
    # Always emits a legal REAL token (not a Python exponent or -0.0).
    return f"{x:.3f}"


def gml_for(spec: Spec, seed: int) -> tuple[str, list[dict], list[dict]]:
    rng = rng_for(seed, spec.name)
    ids = [spec.id_start + i * spec.id_stride for i in range(spec.n)]
    positions = coordinates(spec.n, spec.coords, rng)
    nodes = [dict(id=ids[i], label=f"vertex_{i}", x=x, y=y)
             for i, (x, y) in enumerate(positions)]
    edges = [dict(id=10001 + i, source=ids[u], target=ids[v], label=f"edge_{i}")
             for i, (u, v) in enumerate(spec.edges)]
    open_list, close_list = ("{", "}") if spec.name in ("nonconsecutive_ids", "negative_coordinates") else ("[", "]")
    lines = ["# generated by tests/graph_corpus/generate.py; DO NOT EDIT",
             f"graph {open_list}", f'  label "corpus_{spec.name}"',
             '  directed 0', '  corpus_marker "deterministic"',
             '  meta [ generator "ffgraph" version 1 ratio -1.25 ]']
    for node in nodes:
        lines += [f'  node {open_list}', f'    id {node["id"]}',
                  f'    label "{node["label"]}"',
                  '    graphics [', f'      x {fmt(node["x"])}',
                  f'      y {fmt(node["y"])}',
                  '      w 10.0', '      h 10.0',
                  '      fill "#e6e6e6"', '    ]',
                  f'  {close_list}']
    for edge in edges:
        lines += [f'  edge {open_list}', f'    id {edge["id"]}',
                  f'    source {edge["source"]}', f'    target {edge["target"]}',
                  f'    label "{edge["label"]}"',
                  f'  {close_list}']
    lines += [close_list]
    return "\n".join(lines) + "\n", nodes, edges


def render_oracle(spec: Spec, nodes: list[dict], edges: list[dict]) -> bytes:
    """Plain TSV graph oracle for independent C++ and Python consumers.

    Exact logical node/edge IDs, labels and initial XY positions; this is an
    import oracle, NOT a reference spring-layout coordinate output.
    """
    lines = ["#ffgraph-corpus-oracle-v1", f"graph\tcorpus_{spec.name}"]
    for node in nodes:
        lines.append(f'node\t{node["id"]}\t{node["label"]}\t{fmt(node["x"])}\t{fmt(node["y"])}')
    for edge in edges:
        lines.append(f'edge\t{edge["id"]}\t{edge["source"]}\t{edge["target"]}\t{edge["label"]}')
    return ("\n".join(lines) + "\n").encode("ascii")


def graph_metrics(n: int, edges: list[dict], ids: list[int]) -> dict:
    adj: dict[int, set[int]] = {key: set() for key in ids}
    degree = Counter({key: 0 for key in ids})
    pairs = Counter()
    loops = 0
    for e in edges:
        u, v = e["source"], e["target"]
        if u == v:
            loops += 1
            degree[u] += 2
        else:
            degree[u] += 1
            degree[v] += 1
            adj[u].add(v)
            adj[v].add(u)
        pairs[(min(u, v), max(u, v))] += 1
    visited: set[int] = set()
    sizes = []
    for root in ids:
        if root in visited:
            continue
        visited.add(root)
        queue = deque([root])
        size = 0
        while queue:
            current = queue.popleft()
            size += 1
            for other in adj[current] - visited:
                visited.add(other)
                queue.append(other)
        sizes.append(size)
    return {"nodes": n, "edges": len(edges),
            "components": len(sizes), "component_sizes": sorted(sizes, reverse=True),
            "isolated_nodes": sum(1 for value in degree.values() if value == 0),
            "self_loops": loops,
            "parallel_edge_groups": sum(1 for count in pairs.values() if count > 1),
            "degree_sequence": sorted(degree.values()),
            "max_degree": max(degree.values(), default=0)}


SYNTAX_VALID = {
    "empty_document": "",
    "empty_attributes": 'graph []\n',
    "nested_unknown_keys": 'AGD_option_list [ function "layout" parameters [ alpha -4 beta -3.25 details { state "ok" } ] ]\n',
    "mixed_list_delimiters": 'root { first [ second { n -5 f 0.001 } ] }\n',
    "repeated_keys": 'root [ n 1 n 2 n 3 label "literal \\\"quote\\\" and \\\\ slash" ]\n',
    "colon_keys_and_comment": '# column-one scanner comment\nroot [ name: "text" xyz_2 -1 ]\n',
    "integer_extremes": 'root [ min -2147483648 max 2147483647 negative -0 ]\n',
}
SYNTAX_INVALID = {
    "missing_value": "root [ label ]\n",
    "unclosed_root": "graph [ node [ id 1 ]\n",
    "unclosed_nested": "graph [ node [ id 1\n",
    "orphan_scalar": "graph [ 17 ]\n",
    "orphan_close": "graph [] ]\n",
    "missing_edge_target": "graph [ node [ id 1 ] edge [ source 1 target ] ]\n",
    "unclosed_quoted_string": 'graph [ label "unterminated ]\n',
    "key_without_scalar": "graph [ node [ id id 2 ] ]\n",
    "dangling_open": "root [ inner [ leaf 5 ] other [\n",
}


def build_corpus(seed: int, profile: str, *,
                 examples_dir: Path | None = None) -> tuple[dict[str, bytes], dict]:
    files: dict[str, bytes] = {}
    cases = []
    def add(name: str, category: str, source: str, *, spec: Spec | None = None,
            nodes: list[dict] | None = None, edges: list[dict] | None = None) -> None:
        relative = f"{category}/{name}.gml"
        if category != "invalid":
            parse_attributes(source)
        else:
            try:
                parse_attributes(source)
            except ValueError:
                pass
            else:
                raise AssertionError(f"negative fixture unexpectedly parses: {name}")
        raw = source.encode("utf-8")
        files[relative] = raw
        entry = {"name": name, "category": category, "file": relative,
                 "sha256": hashlib.sha256(raw).hexdigest()}
        if spec is not None:
            assert nodes is not None and edges is not None
            oracle_path = f"oracle/{name}.tsv"
            oracle = render_oracle(spec, nodes, edges)
            files[oracle_path] = oracle
            entry.update({"family": spec.family, "coordinate_mode": spec.coords,
                          "features": list(spec.features), "nodes": nodes, "edges": edges,
                          "oracle": oracle_path,
                          "oracle_sha256": hashlib.sha256(oracle).hexdigest(),
                          "metrics": graph_metrics(spec.n, edges, [node["id"] for node in nodes])})
        cases.append(entry)
    for spec in catalog(seed, profile):
        source, nodes, edges = gml_for(spec, seed)
        add(spec.name, "graph", source, spec=spec, nodes=nodes, edges=edges)
    for name, source in SYNTAX_VALID.items():
        add(name, "syntax", source)
    for name, source in SYNTAX_INVALID.items():
        add(name, "invalid", source)
    if examples_dir is not None:
        for original, category in sorted(EXAMPLE_EXPECTATIONS.items()):
            raw = (examples_dir / original).read_bytes()
            # Preserve the original bytes (including CRLF or legacy encodings).
            # Latin-1 decoding is reversible for byte-level syntax checking.
            source = raw.decode("latin-1")
            name = "historical_" + Path(original).stem
            relative = f"{category}/{name}.gml"
            if category == "invalid":
                try:
                    parse_attributes(source)
                except ValueError:
                    pass
                else:
                    raise ValueError(f"historical negative case now parses: {original}")
            else:
                parse_attributes(source)
            files[relative] = raw
            cases.append({"name": name, "category": category, "file": relative,
                          "historical_source": original,
                          "sha256": hashlib.sha256(raw).hexdigest()})
    manifest = {"format_version": VERSION, "seed": seed, "profile": profile,
                "grammar": "fgraph/modern/modern_gml.y and modern_gml.l",
                "notes": "graph=semantic + layout candidate; syntax=grammar only; invalid=must reject",
                "cases": cases}
    files["manifest.json"] = (json.dumps(manifest, indent=2, ensure_ascii=True) + "\n").encode()
    files["cases.tsv"] = ("#category\tname\tfile\tnodes\tedges\n" + "".join(
        f'{case["category"]}\t{case["name"]}\t{case["file"]}\t'
        f'{case.get("metrics", {}).get("nodes", "-")}\t{case.get("metrics", {}).get("edges", "-")}\n'
        for case in cases)).encode()
    return files, manifest


def write_corpus(root: Path, files: dict[str, bytes], *, check: bool, force: bool) -> None:
    if check:
        actual = {p.relative_to(root).as_posix(): p.read_bytes()
                  for p in root.rglob("*") if p.is_file()} if root.exists() else {}
        if actual != files:
            missing = sorted(files.keys() - actual.keys())
            unexpected = sorted(actual.keys() - files.keys())
            different = sorted(k for k in files.keys() & actual.keys() if files[k] != actual[k])
            raise SystemExit(f"Corpus mismatch: missing={missing} unexpected={unexpected} changed={different}")
        return
    if root.exists() or root.is_symlink():
        if not force:
            raise SystemExit(f"Refusing to overwrite {root} (use --force)")
        if not root.is_dir() or root.is_symlink():
            raise SystemExit("Refusing to replace non-directory or symlink")
        # CMake/Ninja can pre-create the output directory before the first
        # generation. An empty directory has no prior corpus to replace.
        # For nonempty directories, still require our complete prior footprint:
        # `--force` must never remove unrelated user files.
        if any(root.iterdir()):
            try:
                previous = json.loads((root / "manifest.json").read_text(encoding="utf-8"))
                if previous.get("format_version") != VERSION or \
                   previous.get("grammar") != "fgraph/modern/modern_gml.y and modern_gml.l":
                    raise ValueError("unrecognised manifest")
                owned = {"manifest.json", "cases.tsv"}
                for case in previous["cases"]:
                    owned.add(case["file"])
                    if case.get("oracle"):
                        owned.add(case["oracle"])
                present = {p.relative_to(root).as_posix() for p in root.rglob("*")
                           if p.is_file() or p.is_symlink()}
                if owned != present:
                    raise ValueError("unknown or missing files in corpus directory")
            except (OSError, ValueError, KeyError, TypeError) as exc:
                raise SystemExit(f"Refusing unsafe --force replacement of {root}: {exc}") from exc
            shutil.rmtree(root)
    for path, data in files.items():
        destination = root / path
        destination.parent.mkdir(parents=True, exist_ok=True)
        destination.write_bytes(data)


def main() -> None:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--output-dir", type=Path, required=True)
    parser.add_argument("--profile", choices=PROFILES, default="smoke")
    parser.add_argument("--seed", type=int, default=20261101)
    parser.add_argument("--examples-dir", type=Path,
                        help="optional directory containing six curated historical examples")
    parser.add_argument("--force", action="store_true")
    parser.add_argument("--check", action="store_true", help="verify exact reproduction, do not modify files")
    args = parser.parse_args()
    if args.force and args.check:
        parser.error("--force and --check are mutually exclusive")
    files, manifest = build_corpus(args.seed, args.profile, examples_dir=args.examples_dir)
    write_corpus(args.output_dir, files, check=args.check, force=args.force)
    graph = [c for c in manifest["cases"] if c["category"] == "graph"]
    print(f'{args.profile} seed={args.seed}: {len(manifest["cases"])} fixtures; '
          f'{len(graph)} graph fixtures, {sum(c["metrics"]["nodes"] for c in graph)} nodes, '
          f'{sum(c["metrics"]["edges"] for c in graph)} edges; '
          f'{"verified" if args.check else "generated"} at {args.output_dir}')


if __name__ == "__main__":
    main()
