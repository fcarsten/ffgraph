"""Metamorphic construction, invariants and regeneration-safety tests."""
import hashlib
import json
import tempfile
import unittest
from collections import Counter
from pathlib import Path

from generate import parse_attributes, write_corpus
from metamorphic import (DX, DY, SCALE, TRANSFORMS, build_metamorphic)


def semantic(raw):
    root = dict(parse_attributes(raw.decode("ascii")))["graph"]
    nodes = [dict(fields) for key, fields in root if key == "node"]
    edges = [dict(fields) for key, fields in root if key == "edge"]
    positions = {n["label"]: (n["id"], dict(n["graphics"])["x"],
                                dict(n["graphics"])["y"]) for n in nodes}
    id_to_label = {v[0]: name for name, v in positions.items()}
    topology = Counter((e["id"], e["label"],
                        *sorted((id_to_label[e["source"]], id_to_label[e["target"]])))
                       for e in edges)
    return positions, topology


class MetamorphicTests(unittest.TestCase):
    def test_profiles_and_metamorphic_relations(self):
        for profile, pair_count in (("smoke", 55), ("standard", 75), ("stress", 85)):
            with self.subTest(profile=profile):
                files, manifest = build_metamorphic(20261101, profile)
                self.assertEqual(len([c for c in manifest["cases"]
                                      if c["category"] == "variant"]), pair_count)
                self.assertEqual(json.loads(files["manifest.json"]), manifest)
                for c in manifest["cases"]:
                    self.assertEqual(hashlib.sha256(files[c["file"]]).hexdigest(), c["sha256"])
                    if c["category"] != "variant":
                        continue
                    original, edges = semantic(files[c["base"]])
                    mutated, transformed_edges = semantic(files[c["file"]])
                    self.assertEqual(edges, transformed_edges)
                    self.assertEqual(original.keys(), mutated.keys())
                    kind = c["transform"]
                    original_ids = {n: p[0] for n, p in original.items()}
                    mutated_ids = {n: p[0] for n, p in mutated.items()}
                    if kind == "relabel_ids" and original_ids:
                        self.assertNotEqual(original_ids, mutated_ids)
                    elif kind != "relabel_ids":
                        self.assertEqual(original_ids, mutated_ids)
                    for name, old in original.items():
                        new = mutated[name]
                        expected = ((old[1] + DX, old[2] + DY) if kind == "translate_xy" else
                                    (old[1] * SCALE, old[2] * SCALE) if kind == "scale_xy" else
                                    (old[1], old[2]))
                        self.assertAlmostEqual(new[1], expected[0], places=3)
                        self.assertAlmostEqual(new[2], expected[1], places=3)

    def test_determinism_and_safe_initial_build(self):
        a, _ = build_metamorphic(42, "smoke")
        b, _ = build_metamorphic(42, "smoke")
        c, _ = build_metamorphic(43, "smoke")
        self.assertEqual(a, b)
        self.assertNotEqual(a["base/path_8.gml"], c["base/path_8.gml"])
        with tempfile.TemporaryDirectory() as temp:
            root = Path(temp) / "metamorphic"
            root.mkdir()   # First CMake generation may see a pre-created directory.
            write_corpus(root, a, check=False, force=True)
            write_corpus(root, a, check=True, force=False)
            write_corpus(root, a, check=False, force=True)
            (root / "unknown.txt").write_text("preserve")
            with self.assertRaises(SystemExit):
                write_corpus(root, a, check=False, force=True)
            self.assertEqual((root / "unknown.txt").read_text(), "preserve")


if __name__ == "__main__":
    unittest.main()
