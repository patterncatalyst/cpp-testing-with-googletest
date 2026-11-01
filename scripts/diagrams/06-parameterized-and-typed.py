#!/usr/bin/env python3
"""Figure 6.1 — write the body once, let the suite generate the cases: TEST_P
runs one body over each row of INSTANTIATE_TEST_SUITE_P, and TYPED_TEST runs one
body over each type in the type list. Run from the repo root:

    python scripts/diagrams/06-parameterized-and-typed.py
"""
import sys
from pathlib import Path

sys.path.insert(0, "scripts")
import generate_diagram as g  # noqa: E402

g.OUT = "assets/diagrams"
Path(g.OUT).mkdir(parents=True, exist_ok=True)

g.emit(
    "06-parameterized-and-typed", 880, 360,
    nodes=[
        {"x": 50, "y": 56, "w": 330, "h": 58, "style": "accent",
         "lines": ["TEST_P body", "GetParam() gives one row"]},
        {"x": 50, "y": 152, "w": 330, "h": 58, "style": "box",
         "lines": ["INSTANTIATE_TEST_SUITE_P", "Values: 5 weight rows"]},
        {"x": 50, "y": 250, "w": 330, "h": 62, "style": "ink",
         "lines": ["5 generated cases", "/w1 /w500 /w501 /w30000 /w30001"]},
        {"x": 500, "y": 56, "w": 330, "h": 58, "style": "accent",
         "lines": ["TYPED_TEST body", "TypeParam gives one type"]},
        {"x": 500, "y": 152, "w": 330, "h": 58, "style": "box",
         "lines": ["TYPED_TEST_SUITE", "Types<int, long, double>"]},
        {"x": 500, "y": 250, "w": 330, "h": 62, "style": "ink",
         "lines": ["3 generated cases per body", "int, long, double"]},
    ],
    edges=[
        {"x1": 215, "y1": 114, "x2": 215, "y2": 152, "amber": True},
        {"x1": 215, "y1": 210, "x2": 215, "y2": 250, "amber": True},
        {"x1": 665, "y1": 114, "x2": 665, "y2": 152, "amber": True},
        {"x1": 665, "y1": 210, "x2": 665, "y2": 250, "amber": True},
    ],
    notes=[
        {"x": 440, "y": 32,
         "text": "write the body once; the suite generates a case per input row or per type",
         "anchor": "middle"},
    ],
)
