"""GDB Python script: count actual Sugiyama matrix calls and capture three
largest distinct matrix geometries. This is diagnostic, never a timing run.

FFGRAPH_PROFILE_OUTPUT must name a new output directory, outside source data.
Run with gdb -batch -x tools/profile_sugi_matrices.py --args profile_graphs ...
"""
import collections
import json
import os
from pathlib import Path
import sys
import gdb


output = Path(os.environ["FFGRAPH_PROFILE_OUTPUT"])
output.mkdir(parents=True, exist_ok=False)
histogram = collections.Counter()
captured = {}
errors = []
exit_codes = []
reductions = []


class ReductionCalls(gdb.Breakpoint):
    def stop(self):
        try:
            frame = gdb.newest_frame()
            reductions.append({v: int(frame.read_var(v)) for v in
                               ("maxL", "maxS", "newmaxS", "anzND", "cross_alg")})
            reductions[-1]["counter_calls"] = 0
        except Exception as error:
            errors.append(str(error))
            self.enabled = False
        return False


class MatrixCalls(gdb.Breakpoint):
    def stop(self):
        try:
            frame = gdb.newest_frame()
            rows, cols = (int(frame.read_var(v)) for v in ("r", "c"))
            pitch = frame.read_var("maxS")
            # Optimized fast returns need neither stride nor a matrix pointer.
            # reduce_crossings increments its incoming maxS before calling us.
            stride = reductions[-1]["maxS"] + 1 if pitch.is_optimized_out else int(pitch)
            key = (rows, cols, stride)
            histogram[key] += 1
            if reductions:
                reductions[-1]["counter_calls"] += 1
            if key in captured or rows < 2 or cols < 2:
                return False
            if len(captured) == 3 and rows * cols <= min(r * c for r, c, _ in captured):
                return False
            if not (stride > max(rows, cols) and (rows + 1) * stride <= 262144):
                raise RuntimeError("capture memory limit or invalid matrix geometry")
            pointer = int(frame.read_var("matrix"))
            data = bytes(gdb.selected_inferior().read_memory(pointer, (rows + 1) * stride * 4))
            values = []
            for row in range(1, rows + 1):
                values.append([int.from_bytes(data[(row * stride + col) * 4:
                                                   (row * stride + col + 1) * 4],
                                              sys.byteorder, signed=True)
                               for col in range(1, cols + 1)])
            captured[key] = values
            if len(captured) > 3:
                del captured[min(captured, key=lambda k: k[0] * k[1])]
        except Exception as error:
            errors.append(str(error) + "; use a Debug build if arguments were optimized out")
            self.enabled = False
        return False


gdb.execute("set pagination off")
gdb.execute("set debuginfod enabled off")
gdb.events.exited.connect(lambda event: exit_codes.append(getattr(event, "exit_code", None)))
counter_symbol = ("ffgraph_sugiyama::countCrossings"
                  if gdb.lookup_global_symbol("ffgraph_sugiyama::countCrossings")
                  else "number_of_crossings")
breakpoint = MatrixCalls(counter_symbol)
reduction_breakpoint = ReductionCalls("reduce_crossings")
gdb.execute("run")
breakpoint.delete()
reduction_breakpoint.delete()
summaries = []
for (rows, cols, stride), values in sorted(captured.items()):
    name = f"matrix-{rows}-{cols}-{stride}.txt"
    (output / name).write_text(f"{rows} {cols} {stride}\n" +
                               "".join(" ".join(map(str, row)) + "\n" for row in values))
    summaries.append(dict(file=name, rows=rows, cols=cols, stride=stride,
                          edges=sum(v == 1 for row in values for v in row)))
(output / "matrices.json").write_text(json.dumps(dict(
    calls=sum(histogram.values()),
    histogram=[dict(rows=r, cols=c, stride=s, calls=n)
               for (r, c, s), n in sorted(histogram.items())],
    captured=summaries, reductions=reductions, counter_symbol=counter_symbol,
    exit_codes=exit_codes, errors=errors), indent=2) + "\n")
if errors or exit_codes != [0]:
    raise gdb.GdbError(f"matrix capture failed: {errors}, exits={exit_codes}")
