#!/usr/bin/env python3
"""Bound each stress case independently; a crash, timeout or missing case fails."""
import argparse
import os
import signal
import subprocess
import sys


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--binary", required=True)
    parser.add_argument("--mode", choices=("controls", "limits", "gui"), required=True)
    args = parser.parse_args()
    if args.mode == "gui":
        cases = [("wide2", "limited", "1", "gui", updates) for updates in ("off", "on")]
    else:
        cases = [(shape, "limited" if args.mode == "limits" else "control", str(orientation),
                  "headless", "off")
                 for shape in ("wide2", "wide3", "routes") for orientation in (1, 2)]
        if args.mode == "limits":
            cases += [(shape, "limited", "1", "headless", "off")
                      for shape in ("matrix-square", "matrix-slabs", "matrix-partial")]
    for case in cases:
        command = [args.binary, *case]
        print("RUN", " ".join(command), flush=True)
        process = subprocess.Popen(command, stdout=subprocess.PIPE, stderr=subprocess.STDOUT,
                                   text=True, start_new_session=True)
        try:
            output, _ = process.communicate(timeout=45)
        except subprocess.TimeoutExpired:
            os.killpg(process.pid, signal.SIGKILL)
            output, _ = process.communicate()
            print(output, end="", flush=True)
            print("FAIL: subprocess exceeded 45 seconds", file=sys.stderr)
            return 1
        print(output, end="", flush=True)
        if process.returncode:
            print(f"FAIL: subprocess exited {process.returncode}", file=sys.stderr)
            return 1
    print(f"Passed all {len(cases)} {args.mode} subprocesses", flush=True)
    return 0


if __name__ == "__main__":
    sys.exit(main())
