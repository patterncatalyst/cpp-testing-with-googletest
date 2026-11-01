#!/usr/bin/env python3
"""Figure 13.2 — a stress run: repeating the suite many times, each in a shuffled
order, turns a rare order- or timing-dependent failure from a once-in-a-while
surprise into a reproducible one. Run from the repo root:

    python scripts/diagrams/13-stress-repeat.py
"""
import sys
from pathlib import Path

sys.path.insert(0, "scripts")
import generate_diagram as g  # noqa: E402

g.OUT = "assets/diagrams"
Path(g.OUT).mkdir(parents=True, exist_ok=True)

g.emit(
    "13-stress-repeat", 940, 250,
    nodes=[
        {"x": 40, "y": 90, "w": 250, "h": 70, "style": "accent",
         "lines": ["--gtest_repeat=N", "--gtest_shuffle", "N runs, each reordered"]},
        {"x": 370, "y": 50, "w": 230, "h": 54, "style": "ink",
         "lines": ["most iterations pass", "the common schedules"]},
        {"x": 370, "y": 150, "w": 230, "h": 54, "style": "box",
         "lines": ["one iteration fails", "a rare order or timing"]},
        {"x": 680, "y": 150, "w": 230, "h": 54, "style": "ink",
         "lines": ["now reproducible", "a seed to debug from"]},
    ],
    edges=[
        {"x1": 290, "y1": 110, "x2": 370, "y2": 77},
        {"x1": 290, "y1": 140, "x2": 370, "y2": 177, "amber": True},
        {"x1": 600, "y1": 177, "x2": 680, "y2": 177, "amber": True},
    ],
    notes=[
        {"x": 470, "y": 230,
         "text": "repetition turns a once-in-a-while failure into one you can catch",
         "anchor": "middle"},
    ],
)
