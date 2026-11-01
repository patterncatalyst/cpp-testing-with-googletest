#!/usr/bin/env python3
"""Figure 10.2 — mock versus fake: a mock is programmed with expectations that are
verified during and after the call, while a fake is a working implementation whose
accumulated state the test inspects afterward. Run from the repo root:

    python scripts/diagrams/10-mock-vs-fake.py
"""
import sys
from pathlib import Path

sys.path.insert(0, "scripts")
import generate_diagram as g  # noqa: E402

g.OUT = "assets/diagrams"
Path(g.OUT).mkdir(parents=True, exist_ok=True)

g.emit(
    "10-mock-vs-fake", 900, 340,
    bands=[
        {"x": 30, "y": 50, "w": 400, "h": 250, "label": "mock", "fill": "#fff8ef"},
        {"x": 470, "y": 50, "w": 400, "h": 250, "label": "fake", "fill": "#f1f5f9"},
    ],
    nodes=[
        {"x": 60, "y": 96, "w": 340, "h": 52, "style": "box",
         "lines": ["EXPECT_CALL set up front", "expectations, not behavior"]},
        {"x": 60, "y": 168, "w": 340, "h": 52, "style": "box",
         "lines": ["asserts the interaction", "which calls, which arguments, order"]},
        {"x": 60, "y": 240, "w": 340, "h": 44, "style": "ink",
         "lines": ["verified when the mock is destroyed"]},

        {"x": 500, "y": 96, "w": 340, "h": 52, "style": "box",
         "lines": ["a working implementation", "real behavior, kept simple"]},
        {"x": 500, "y": 168, "w": 340, "h": 52, "style": "box",
         "lines": ["asserts the result", "inspect its state after the run"]},
        {"x": 500, "y": 240, "w": 340, "h": 44, "style": "ink",
         "lines": ["checked by ordinary EXPECT_EQ afterward"]},
    ],
)
