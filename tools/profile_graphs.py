#!/usr/bin/env python3
"""Generate matched FF/GML/Rome workloads and measure the real ffgraph core.

No timing thresholds or generated inputs belong in normal CTest. Output must
be a new directory: incomplete runs and their diagnostics remain inspectable.
"""
import argparse
import csv
import hashlib
import json
import math
import os
from pathlib import Path
import platform
import statistics
import subprocess
import sys


def command(*args):
    return subprocess.run(args, text=True, capture_output=True, check=True).stdout.strip()


def edges_for(shape, size):
    if shape == "chain":
        return [(i, i + 1) for i in range(1, size)]
    if shape == "star":
        return [(1, i) for i in range(2, size + 1)]
    width = math.isqrt(size)
    return [(i, target) for i in range(1, size + 1)
            for target in (i + width, i + width + 1)
            if target <= size and (target - 1) // width == (i - 1) // width + 1]


def write_graphs(directory, shape, size):
    edges = edges_for(shape, size)
    name = f"{shape}-{size}"
    oracle = directory / f"{name}.tsv"
    oracle.write_text("".join(f"node {i}\n" for i in range(1, size + 1)) +
                      "".join(f"edge {a} {b}\n" for a, b in edges))
    outgoing = [[] for _ in range(size + 1)]
    for a, b in edges:
        outgoing[a].append(b)
    ff = [f'GRAPH "{name}" = UNDIRECTED\n']
    gml = ['graph [\n directed 0\n']
    rome = []
    for i in range(1, size + 1):
        # Match Rome's default (100,100,0): setup is identical in all formats.
        ff.append(f'{i} "" ' + " ".join(f'{b} ""' for b in outgoing[i]) + ';\n')
        gml.append(f' node [ id {i} ]\n')
        rome.append(f'{i} 0\n')
    ff.append('END\n')
    gml.extend(f' edge [ source {a} target {b} ]\n' for a, b in edges)
    gml.append(']\n')
    rome.append('#\n')
    rome.extend(f'{i} 0 {a} {b}\n' for i, (a, b) in enumerate(edges, 1))
    result = []
    for ext, lines in (("g", ff), ("gml", gml), ("rome", rome)):
        path = directory / f"{name}.{ext}"
        path.write_text("".join(lines))
        result.append(dict(name=name, format=ext, path=str(path), oracle=str(oracle),
                           expected_nodes=size, expected_edges=len(edges)))
    return result


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--binary", type=Path, required=True)
    parser.add_argument("--output", type=Path, required=True)
    parser.add_argument("--sizes", type=int, nargs="+", default=[100, 1000])
    parser.add_argument("--shapes", choices=["chain", "star", "layered"], nargs="+",
                        default=["chain", "star", "layered"])
    parser.add_argument("--phases", choices=["import", "parse", "replay", "layout"],
                        nargs="+", default=["import", "parse", "replay", "layout"])
    parser.add_argument("--layouts", choices=["random", "spring", "sugi"], nargs="+",
                        default=["random", "spring", "sugi"])
    parser.add_argument("--layout-max-nodes", type=int, default=1000)
    parser.add_argument("--quality", type=int, choices=range(1, 11), default=2)
    parser.add_argument("--samples", type=int, default=5)
    parser.add_argument("--timeout", type=float, default=180)
    for fmt in ("ff", "gml", "rome"):
        parser.add_argument(f"--real-{fmt}", type=Path, action="append", default=[])
    args = parser.parse_args()
    if (any(n < 2 or n > 100000 for n in args.sizes) or args.samples < 1 or
            args.samples > 10000 or args.layout_max_nodes < 1 or args.timeout <= 0):
        parser.error("invalid workload limits")
    binary = args.binary.resolve(strict=True)
    root = Path(__file__).resolve().parents[1]
    output = args.output.resolve()
    output.mkdir(parents=True, exist_ok=False)
    inputs = output / "inputs"
    inputs.mkdir()
    workloads = []
    for size in args.sizes:
        for shape in args.shapes:
            workloads.extend(write_graphs(inputs, shape, size))
    for fmt, paths in (("g", args.real_ff), ("gml", args.real_gml), ("rome", args.real_rome)):
        for index, source in enumerate(paths):
            source = source.resolve(strict=True)
            path = inputs / f"real-{fmt}-{index}.{fmt}"
            path.write_bytes(source.read_bytes())
            workloads.append(dict(name=f"real-{fmt}-{index}", format=fmt,
                                  path=str(path), source=str(source)))
    for work in workloads:
        data = Path(work["path"]).read_bytes()
        work.update(bytes=len(data), sha256=hashlib.sha256(data).hexdigest())
    metadata = dict(
        command=sys.argv, platform=platform.platform(), machine=platform.machine(),
        cpu=next((line.split(":", 1)[1].strip() for line in Path("/proc/cpuinfo").read_text()
                  .splitlines() if line.startswith("model name")), "unknown"),
        affinity=sorted(os.sched_getaffinity(0)),
        git_head=command("git", "-C", str(root), "rev-parse", "HEAD"),
        git_status=command("git", "-C", str(root), "status", "--short"),
        binary=str(binary), binary_sha256=hashlib.sha256(binary.read_bytes()).hexdigest(),
        source_sha256={str(p.relative_to(root)): hashlib.sha256(p.read_bytes()).hexdigest()
                       for p in (root / "tools/profile_graphs.cpp", Path(__file__).resolve(),
                                 root / "fgraph/modern/modern_gml.y")},
        cmake_cache=(binary.parent / "CMakeCache.txt").read_text(),
        workloads=workloads)
    (output / "manifest.json").write_text(json.dumps(metadata, indent=2) + "\n")
    summaries = []
    failures = 0
    with (output / "results.jsonl").open("w") as results:
        for work in workloads:
            for phase in args.phases:
                layouts = args.layouts if phase == "layout" else [""]
                for layout in layouts:
                    name = f'{work["name"]}-{work["format"]}-{phase}-{layout or "none"}'
                    row = dict(case=name, name=work["name"], format=work["format"],
                               phase=phase, layout=layout)
                    if phase == "layout" and work.get("expected_nodes", 0) > args.layout_max_nodes:
                        row.update(status="skipped", reason="layout node limit")
                        summaries.append(row)
                        continue
                    cmd = [str(binary), "--input", work["path"], "--phase", phase,
                           "--repeat", str(args.samples), "--quality", str(args.quality)]
                    if work.get("oracle"):
                        cmd.extend(["--oracle", work["oracle"]])
                    if layout:
                        cmd.extend(["--layout", layout, "--layout-max-nodes", str(args.layout_max_nodes)])
                    try:
                        run = subprocess.run(cmd, capture_output=True, text=True, timeout=args.timeout)
                        (output / f"{name}.stderr").write_text(run.stderr)
                        (output / f"{name}.stdout").write_text(run.stdout)
                        if run.returncode:
                            raise RuntimeError(f"exit {run.returncode}: {run.stderr[-500:]}")
                        samples = [json.loads(line) for line in run.stdout.splitlines()]
                        if len(samples) != args.samples:
                            raise RuntimeError("unexpected sample count")
                        if len({(s["nodes"], s["edges"]) for s in samples}) != 1:
                            raise RuntimeError("inconsistent graph counts")
                        # Establish real-file size before scheduling its layouts.
                        work.setdefault("expected_nodes", samples[0]["nodes"])
                        work.setdefault("expected_edges", samples[0]["edges"])
                        row.update(status="passed", nodes=samples[0]["nodes"], edges=samples[0]["edges"],
                                   median_ms=statistics.median(s["wall_ms"] for s in samples),
                                   min_ms=min(s["wall_ms"] for s in samples),
                                   max_ms=max(s["wall_ms"] for s in samples),
                                   peak_rss_kib=max(s["peak_rss_kib"] for s in samples),
                                   spring_iteration_cap=samples[0]["spring_iteration_cap"])
                        results.write(json.dumps(dict(**row, command=cmd, samples=samples)) + "\n")
                        results.flush()
                        print(f'{name}: {row["median_ms"]:.3f} ms', flush=True)
                    except (subprocess.TimeoutExpired, RuntimeError, ValueError) as error:
                        row.update(status="failed", reason=str(error))
                        failures += 1
                        results.write(json.dumps(row) + "\n")
                        results.flush()
                        print(f'{name}: FAILED: {error}', file=sys.stderr, flush=True)
                    summaries.append(row)
    (output / "summary.json").write_text(json.dumps(summaries, indent=2) + "\n")
    fields = list(dict.fromkeys(key for row in summaries for key in row))
    with (output / "summary.csv").open("w", newline="") as stream:
        writer = csv.DictWriter(stream, fieldnames=fields)
        writer.writeheader()
        writer.writerows(summaries)
    return 1 if failures else 0


if __name__ == "__main__":
    sys.exit(main())
