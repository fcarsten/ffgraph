"""Standalone, dependency-free tests of graph-corpus grammar and oracles."""
import hashlib
import json
import tempfile
import unittest
from collections import Counter
from pathlib import Path

from generate import (EXAMPLE_EXPECTATIONS, PROFILES, SYNTAX_INVALID, SYNTAX_VALID, build_corpus,
                      graph_metrics, parse_attributes, write_corpus)


class CorpusTests(unittest.TestCase):
    def test_all_profiles_and_graph_manifest(self):
        sizes = []
        for profile in PROFILES:
            files, manifest = build_corpus(20261101, profile)
            cases = manifest["cases"]
            self.assertEqual(len({c["name"] for c in cases}), len(cases))
            self.assertEqual(len({c["file"] for c in cases}), len(cases))
            graph_cases = [c for c in cases if c["category"] == "graph"]
            sizes.append(len(graph_cases))
            self.assertEqual(len([c for c in cases if c["category"] == "syntax"]), len(SYNTAX_VALID))
            self.assertEqual(len([c for c in cases if c["category"] == "invalid"]), len(SYNTAX_INVALID))
            for case in cases:
                raw = files[case["file"]]
                self.assertEqual(hashlib.sha256(raw).hexdigest(), case["sha256"])
                if case["category"] == "invalid":
                    with self.assertRaises(ValueError):
                        parse_attributes(raw.decode())
                    continue
                ast = parse_attributes(raw.decode())
                if case["category"] != "graph":
                    continue
                self.assertEqual(len(ast), 1)
                self.assertEqual(ast[0][0], "graph")
                root = ast[0][1]
                nodes = [val for key, val in root if key == "node"]
                edges = [val for key, val in root if key == "edge"]
                self.assertEqual(len(nodes), case["metrics"]["nodes"])
                self.assertEqual(len(edges), case["metrics"]["edges"])
                self.assertEqual(len({node["id"] for node in case["nodes"]}), len(nodes))
                for item, expected in zip(nodes, case["nodes"]):
                    flat = dict(item)
                    self.assertEqual(flat["id"], expected["id"])
                    self.assertEqual(flat["label"], expected["label"])
                    graphics = dict(flat["graphics"])
                    self.assertEqual(graphics["x"], expected["x"])
                    self.assertEqual(graphics["y"], expected["y"])
                for item, expected in zip(edges, case["edges"]):
                    for field in ("id", "source", "target", "label"):
                        self.assertEqual(dict(item)[field], expected[field])
                oracle = files[case["oracle"]]
                self.assertEqual(hashlib.sha256(oracle).hexdigest(), case["oracle_sha256"])
                expected_lines = [f'graph\tcorpus_{case["name"]}']
                expected_lines += [f'node\t{n["id"]}\t{n["label"]}\t{n["x"]:.3f}\t{n["y"]:.3f}'
                                   for n in case["nodes"]]
                expected_lines += [f'edge\t{e["id"]}\t{e["source"]}\t{e["target"]}\t{e["label"]}'
                                   for e in case["edges"]]
                self.assertEqual(oracle.decode().splitlines()[1:], expected_lines)
                metrics = graph_metrics(len(nodes), case["edges"],
                                        [node["id"] for node in case["nodes"]])
                self.assertEqual(metrics, case["metrics"])
                self.assertEqual(sum(metrics["degree_sequence"]), 2 * len(edges))
                self.assertEqual(sum(metrics["component_sizes"]), len(nodes))
                self.assertTrue(all(e["source"] in {n["id"] for n in case["nodes"]}
                                    and e["target"] in {n["id"] for n in case["nodes"]}
                                    for e in case["edges"]))
            self.assertEqual(json.loads(files["manifest.json"]), manifest)
        self.assertEqual(sizes, sorted(sizes))
        self.assertEqual(len(set(sizes)), 3)

    def test_curated_historical_examples(self):
        sample_dir = Path(__file__).with_name("samples")
        files, manifest = build_corpus(20261101, "smoke", examples_dir=sample_dir)
        examples = [c for c in manifest["cases"] if "historical_source" in c]
        self.assertEqual(len(examples), len(EXAMPLE_EXPECTATIONS))
        for c in examples:
            raw = files[c["file"]]
            self.assertEqual(raw, (sample_dir / c["historical_source"]).read_bytes())
            self.assertEqual(c["category"], EXAMPLE_EXPECTATIONS[c["historical_source"]])
            if c["category"] == "invalid":
                with self.assertRaises(ValueError):
                    parse_attributes(raw.decode("latin-1"))
            else:
                parse_attributes(raw.decode("latin-1"))

    def test_seed_reproducibility_and_independence(self):
        a, _ = build_corpus(12345, "smoke")
        b, _ = build_corpus(12345, "smoke")
        c, _ = build_corpus(12346, "smoke")
        self.assertEqual(a, b)
        self.assertNotEqual(a["graph/path_8.gml"], c["graph/path_8.gml"])
        # Changing seed does not change the topology of a nonrandom family.
        _, ma = build_corpus(12345, "smoke")
        _, mb = build_corpus(12346, "smoke")
        self.assertEqual(next(x for x in ma["cases"] if x["name"] == "path_8")["edges"],
                         next(x for x in mb["cases"] if x["name"] == "path_8")["edges"])

    def test_atomic_check_behavior_and_integrity(self):
        files, _ = build_corpus(12, "smoke")
        with tempfile.TemporaryDirectory() as tmp:
            root = Path(tmp) / "corpus"
            write_corpus(root, files, check=False, force=False)
            write_corpus(root, files, check=True, force=False)
            with self.assertRaises(SystemExit):
                write_corpus(root, files, check=False, force=False)
            target = root / "graph" / "single_edge.gml"
            target.write_text("changed")
            with self.assertRaises(SystemExit):
                write_corpus(root, files, check=True, force=False)
            write_corpus(root, files, check=False, force=True)
            write_corpus(root, files, check=True, force=False)
            (root / "personal_notes.txt").write_text("do not remove")
            with self.assertRaises(SystemExit):
                write_corpus(root, files, check=False, force=True)
            self.assertEqual((root / "personal_notes.txt").read_text(), "do not remove")
            (root / "personal_notes.txt").unlink()
            write_corpus(root, files, check=False, force=True)
            other = Path(tmp) / "important"
            other.mkdir()
            (other / "data.txt").write_text("important")
            with self.assertRaises(SystemExit):
                write_corpus(other, files, check=False, force=True)
            self.assertEqual((other / "data.txt").read_text(), "important")

    def test_force_allows_precreated_empty_build_directory(self):
        files, _ = build_corpus(20261101, "smoke")
        with tempfile.TemporaryDirectory() as tmp:
            root = Path(tmp) / "generated-layout-corpus"
            root.mkdir()  # A build system may create it before invoking us.
            with self.assertRaises(SystemExit):
                write_corpus(root, files, check=False, force=False)
            write_corpus(root, files, check=False, force=True)
            write_corpus(root, files, check=True, force=False)
            # Rebuilding must safely replace the previously generated corpus.
            write_corpus(root, files, check=False, force=True)
            write_corpus(root, files, check=True, force=False)
            # An unrecognised nonempty directory must still be protected.
            unknown = Path(tmp) / "unknown"
            unknown.mkdir()
            (unknown / "notes.txt").write_text("keep me")
            with self.assertRaises(SystemExit):
                write_corpus(unknown, files, check=False, force=True)
            self.assertEqual((unknown / "notes.txt").read_text(), "keep me")

    def test_grammar_boundaries(self):
        for valid in ('', 'root []', 'root { inner [ leaf -0.25 ] }',
                      '# top-level comment\nroot [ number -2147483648 ]'):
            parse_attributes(valid)
        for invalid in ('root', 'root [', 'root [ n ]', 'root [ n 4 ] ]',
                        'root [ n 1e3 ]', 'root [ n +1 ]',
                        'root [ n 2147483648 ]', 'root [ n 1 # inline comment\n ]'):
            with self.subTest(invalid=invalid), self.assertRaises(ValueError):
                parse_attributes(invalid)


if __name__ == '__main__':
    unittest.main()
