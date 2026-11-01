#!/usr/bin/env python3
"""Figure 14.1 — the debug loop for a failing test: isolate it with a filter, break
in the function, run, inspect the state, find the cause, fix, and re-run green. Run
from the repo root:

    python scripts/diagrams/14-debug-loop.py
"""
import sys
from pathlib import Path

sys.path.insert(0, "scripts")
import generate_diagram as g  # noqa: E402

g.OUT = "assets/diagrams"
Path(g.OUT).mkdir(parents=True, exist_ok=True)

g.emit(
    "14-debug-loop", 940, 230,
    nodes=[
        {"x": 30, "y": 90, "w": 170, "h": 60, "style": "accent",
         "lines": ["isolate", "--gtest_filter"]},
        {"x": 240, "y": 90, "w": 170, "h": 60, "style": "box",
         "lines": ["break + run", "stop in the function"]},
        {"x": 450, "y": 90, "w": 180, "h": 60, "style": "box",
         "lines": ["inspect", "print, next, backtrace"]},
        {"x": 670, "y": 90, "w": 170, "h": 60, "style": "ink",
         "lines": ["find the cause", "then fix"]},
    ],
    edges=[
        {"x1": 200, "y1": 120, "x2": 240, "y2": 120, "amber": True},
        {"x1": 410, "y1": 120, "x2": 450, "y2": 120, "amber": True},
        {"x1": 630, "y1": 120, "x2": 670, "y2": 120, "amber": True},
        # loop back: re-run to confirm green
        {"x1": 755, "y1": 188, "x2": 115, "y2": 188, "label": "re-run until green", "amber": True},
    ],
    notes=[
        {"x": 470, "y": 50, "text": "a failing test is a lead, not a verdict",
         "anchor": "middle"},
    ],
)
