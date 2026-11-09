"""Exercise release-source provenance and preview archive boundaries."""
import importlib.util
from pathlib import Path
import subprocess
import tarfile
import tempfile
import unittest

spec = importlib.util.spec_from_file_location("package_release", Path(__file__).resolve().parents[1] / "scripts/package-release.py")
packaging = importlib.util.module_from_spec(spec)
spec.loader.exec_module(packaging)


class ReleasePackagingTest(unittest.TestCase):
    def test_clean_commit_and_explicit_dirty_preview(self):
        with tempfile.TemporaryDirectory() as temporary:
            root = Path(temporary)
            source = root / "source"
            source.mkdir()
            def git(*args):
                return subprocess.check_output(["git", "-C", str(source), *args], stderr=subprocess.STDOUT)
            git("init")
            (source / ".gitignore").write_text("build/\n")
            # Byte assertions below require LF fixtures on Windows too,
            # independent of Python newline translation and Git autocrlf.
            (source / "VERSION").write_bytes(b"1.0.0-rc1\n")
            binary = b"\x00\xff\r\noriginal binary bytes\r\n"
            (source / "asset.bin").write_bytes(binary)
            git("add", ".")
            git("-c", "user.name=Release test", "-c", "user.email=test@example.invalid",
                "-c", "commit.gpgsign=false", "commit", "-m", "fixture")
            # Exercise Windows-style Git settings even in Linux CI.
            git("config", "core.autocrlf", "true")
            commit, state = packaging.source_state(source, False)
            self.assertEqual(state, "clean")
            target = root / "clean.tar.gz"
            packaging.archive_source(source, target, "ffgraph-test", False)
            with tarfile.open(target) as archive:
                self.assertEqual(archive.extractfile("ffgraph-test/VERSION").read(), b"1.0.0-rc1\n")
                self.assertEqual(archive.extractfile("ffgraph-test/asset.bin").read(), binary)
            (source / "VERSION").write_bytes(b"modified preview\n")
            (source / "new-notice.txt").write_text("uncommitted notice\n")
            (source / "build").mkdir()
            (source / "build/private.txt").write_text("must not ship\n")
            with self.assertRaisesRegex(ValueError, "clean checkout"):
                packaging.source_state(source, False)
            self.assertEqual(packaging.source_state(source, True), (commit, "uncommitted-preview"))
            packaging.archive_source(source, target, "ffgraph-preview", True)
            with tarfile.open(target) as archive:
                self.assertEqual(archive.extractfile("ffgraph-preview/VERSION").read(), b"modified preview\n")
                self.assertIn("ffgraph-preview/new-notice.txt", archive.getnames())
                self.assertNotIn("ffgraph-preview/build/private.txt", archive.getnames())
                self.assertFalse(any("/.git/" in name for name in archive.getnames()))


if __name__ == "__main__":
    unittest.main()
