"""Launch the installed app outside the checkout with isolated shutdown hooks."""
import argparse
import os
from pathlib import Path
import shutil
import subprocess
import tempfile


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("--build", type=Path, required=True)
    parser.add_argument("--libdir", required=True)
    args = parser.parse_args()
    work = Path(tempfile.mkdtemp(prefix="runtime-check-", dir=args.build))
    prefix = work / "prefix with spaces"
    subprocess.run(["cmake", "--install", str(args.build), "--prefix", str(prefix)], check=True,
                   stdout=(work / "install.log").open("w"), stderr=subprocess.STDOUT)
    binary = prefix / "bin/ffgraph"
    link = prefix / "bin/ffgraph3D"
    assert binary.is_file() and link.is_symlink() and link.resolve() == binary
    resources = prefix / args.libdir / "ffgraph"
    custom = work / "custom resources"
    shutil.copytree(resources, custom)
    # These private staged scripts exercise the genuine app startup, inspectors
    # and icon lookup, then terminate the GUI process without production hooks.
    hook = r'''
after idle {
  if {![file exists $env(FF_LIBDIR)/tcl/xfmisc.tcl]} {exit 11}
  if {![file isdirectory $env(FF_ICONDIR)]} {exit 12}
  set out [open $env(FFGRAPH_RUNTIME_RESULT) w]
  puts $out $env(FF_LIBDIR)
  puts $out $env(FF_ICONDIR)
  close $out
  exit 0
}
'''
    for directory in (resources, custom):
        for filename in ("plainGraph.tcl", "fgraph.tcl"):
            with (directory / "tcl" / filename).open("a") as script:
                script.write(hook)
    for override in (False, True):
        for executable in (binary, link):
            env = os.environ.copy()
            env.pop("FF_LIBDIR", None)
            env.pop("FF_ICONDIR", None)
            chosen = custom if override else resources
            if override:
                env["FF_LIBDIR"] = str(custom)
                env["FF_ICONDIR"] = str(custom / "icons")
            result = work / f"{executable.name}-{override}.result"
            env["FFGRAPH_RUNTIME_RESULT"] = str(result)
            run = subprocess.run([str(executable)], cwd=work, env=env, text=True,
                                 capture_output=True, timeout=20)
            (work / f"{executable.name}-{override}.log").write_text(run.stdout + run.stderr)
            assert run.returncode == 0, run.stderr
            assert "Error:" not in run.stderr, run.stderr
            assert result.read_text().splitlines() == [str(chosen), str(chosen / "icons")]
    print("Installed 2D/3D runtime and explicit resource overrides passed:", work)


if __name__ == "__main__":
    main()
