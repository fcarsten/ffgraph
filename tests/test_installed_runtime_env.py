"""Check native Windows runtime isolation even when Python runs on Linux."""
import os
from pathlib import Path
import unittest

from test_installed_runtime import isolate_windows_environment


class InstalledRuntimeEnvironmentTest(unittest.TestCase):
    def test_uppercase_windows_environment(self):
        # os.environ.copy() on Windows returns a plain dict with uppercase keys.
        env = {
            "SYSTEMROOT": "C:/Windows",
            "PATH": "development-toolchain",
            "TCL_LIBRARY": "development-tcl",
            "TK_LIBRARY": "development-tk",
            "TEMP": "temporary files",
        }
        prefix = Path("staged prefix with spaces")
        isolate_windows_environment(env, prefix)
        self.assertEqual(env, {
            "SYSTEMROOT": "C:/Windows",
            "PATH": str(prefix / "bin") + os.pathsep
                    + str(Path("C:/Windows") / "System32"),
            "TEMP": "temporary files",
        })


if __name__ == "__main__":
    unittest.main()
