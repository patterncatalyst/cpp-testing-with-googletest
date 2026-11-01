#!/usr/bin/env python3
"""Figure 3.1 — the billable-weight rule and how the three test cases cover its
branches: volumetric weight is computed from the dimensions, billable weight is
the greater of volumetric and actual, and each test exercises one outcome.
Run from the repo root:

    python scripts/diagrams/03-billable-weight-branches.py
"""
import sys
from pathlib import Path

sys.path.insert(0, "scripts")
import generate_diagram as g  # noqa: E402

g.OUT = "assets/diagrams"
Path(g.OUT).mkdir(parents=True, exist_ok=True)

g.emit(
    "03-billable-weight-branches", 900, 290,
    nodes=[
        {"x": 30, "y": 46, "w": 180, "h": 56, "style": "box",
         "lines": ["Dimensions{L, W, H}", "centimetres"]},
        {"x": 30, "y": 160, "w": 180, "h": 56, "style": "box",
         "lines": ["actual_grams", "scale weight"]},
        {"x": 280, "y": 46, "w": 200, "h": 56, "style": "box",
         "lines": ["volumetric grams", "L x W x H / 5"]},
        {"x": 550, "y": 90, "w": 170, "h": 66, "style": "accent",
         "lines": ["max(volumetric,", "actual)"]},
        {"x": 760, "y": 90, "w": 110, "h": 66, "style": "ink",
         "lines": ["billable", "grams"]},
    ],
    edges=[
        {"x1": 210, "y1": 74, "x2": 280, "y2": 74},
        {"x1": 480, "y1": 74, "x2": 550, "y2": 112, "amber": True,
         "label": "bulky wins"},
        {"x1": 210, "y1": 188, "x2": 550, "y2": 136, "amber": True,
         "label": "heavy wins"},
        {"x1": 720, "y1": 123, "x2": 760, "y2": 123, "amber": True},
    ],
    notes=[
        {"x": 450, "y": 262,
         "text": "three tests: bulky bills volumetric, heavy bills actual, a tie bills either",
         "anchor": "middle"},
    ],
)
