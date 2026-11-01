#!/usr/bin/env python3
"""Figure 14.2 — the GDB commands that cover most test debugging, each paired with
what it does. Run from the repo root:

    python scripts/diagrams/14-gdb-commands.py
"""
import sys
from pathlib import Path

sys.path.insert(0, "scripts")
import generate_diagram as g  # noqa: E402

g.OUT = "assets/diagrams"
Path(g.OUT).mkdir(parents=True, exist_ok=True)

rows = [
    ("break fn if cond", "stop in fn only when the condition holds"),
    ("run --gtest_filter=...", "start, running just the failing test"),
    ("print expr", "show a variable or evaluate an expression"),
    ("next / step", "over the next line / into the call"),
    ("finish", "run to the end of the current function"),
    ("backtrace", "the call stack that reached here"),
]

nodes = []
edges = []
y = 54
for cmd, meaning in rows:
    nodes.append({"x": 40, "y": y, "w": 250, "h": 40, "style": "box", "lines": [cmd]})
    nodes.append({"x": 360, "y": y, "w": 480, "h": 40, "style": "ink", "lines": [meaning]})
    edges.append({"x1": 290, "y1": y + 20, "x2": 360, "y2": y + 20, "amber": True})
    y += 52

g.emit(
    "14-gdb-commands", 880, 54 + 52 * len(rows) + 10,
    nodes=nodes,
    edges=edges,
    notes=[{"x": 165, "y": 36, "text": "command", "anchor": "middle"},
           {"x": 600, "y": 36, "text": "what it does", "anchor": "middle"}],
)
