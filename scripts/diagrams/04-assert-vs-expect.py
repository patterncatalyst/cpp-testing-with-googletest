#!/usr/bin/env python3
"""Figure 4.1 — how ASSERT and EXPECT differ inside one test body: a failed
ASSERT stops the test immediately (fatal), so the lines below never run; a failed
EXPECT records the failure and the test keeps going. Run from the repo root:

    python scripts/diagrams/04-assert-vs-expect.py
"""
import sys
from pathlib import Path

sys.path.insert(0, "scripts")
import generate_diagram as g  # noqa: E402

g.OUT = "assets/diagrams"
Path(g.OUT).mkdir(parents=True, exist_ok=True)

g.emit(
    "04-assert-vs-expect", 820, 420,
    bands=[
        {"x": 30, "y": 30, "w": 450, "h": 366,
         "label": "one TEST body (runs top to bottom)", "fill": "#fafafa"},
    ],
    nodes=[
        {"x": 70, "y": 70, "w": 340, "h": 58, "style": "accent",
         "lines": ["ASSERT_EQ(shipments.size(), 3)", "check the precondition"]},
        {"x": 70, "y": 176, "w": 340, "h": 46, "style": "box",
         "lines": ["EXPECT_EQ(shipments[0], 1000)"]},
        {"x": 70, "y": 252, "w": 340, "h": 46, "style": "box",
         "lines": ["EXPECT_EQ(shipments[1], 1000)"]},
        {"x": 70, "y": 328, "w": 340, "h": 46, "style": "box",
         "lines": ["EXPECT_EQ(shipments[2], 500)"]},
        {"x": 560, "y": 70, "w": 230, "h": 58, "style": "ink",
         "lines": ["stop the test (fatal)", "the lines below never run"]},
        {"x": 560, "y": 252, "w": 230, "h": 46, "style": "ink",
         "lines": ["record failure, keep going"]},
    ],
    edges=[
        {"x1": 410, "y1": 99, "x2": 560, "y2": 99, "amber": True, "label": "fail"},
        {"x1": 240, "y1": 128, "x2": 240, "y2": 176, "label": "pass"},
        {"x1": 240, "y1": 222, "x2": 240, "y2": 252},
        {"x1": 240, "y1": 298, "x2": 240, "y2": 328},
        {"x1": 410, "y1": 275, "x2": 560, "y2": 275, "amber": True, "label": "fail"},
    ],
)
