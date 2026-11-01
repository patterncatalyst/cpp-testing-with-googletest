#!/usr/bin/env python3
"""Figure 13.1 — four common sources of a flaky test and the determinism technique
that removes each: inject a seeded engine, inject the time, pin the schedule, and
shuffle the order. Run from the repo root:

    python scripts/diagrams/13-flakiness-sources.py
"""
import sys
from pathlib import Path

sys.path.insert(0, "scripts")
import generate_diagram as g  # noqa: E402

g.OUT = "assets/diagrams"
Path(g.OUT).mkdir(parents=True, exist_ok=True)

g.emit(
    "13-flakiness-sources", 940, 340,
    nodes=[
        {"x": 40, "y": 70, "w": 300, "h": 50, "style": "box",
         "lines": ["randomness from a live seed"]},
        {"x": 40, "y": 140, "w": 300, "h": 50, "style": "box",
         "lines": ["wall-clock time, sleeps"]},
        {"x": 40, "y": 210, "w": 300, "h": 50, "style": "box",
         "lines": ["thread scheduling"]},
        {"x": 40, "y": 280, "w": 300, "h": 50, "style": "box",
         "lines": ["dependence on test order"]},

        {"x": 560, "y": 70, "w": 340, "h": 50, "style": "ink",
         "lines": ["inject a seeded engine"]},
        {"x": 560, "y": 140, "w": 340, "h": 50, "style": "ink",
         "lines": ["inject the time, never sleep"]},
        {"x": 560, "y": 210, "w": 340, "h": 50, "style": "ink",
         "lines": ["a latch, then repeat under TSan"]},
        {"x": 560, "y": 280, "w": 340, "h": 50, "style": "ink",
         "lines": ["run with --gtest_shuffle"]},
    ],
    edges=[
        {"x1": 340, "y1": 95, "x2": 560, "y2": 95, "amber": True},
        {"x1": 340, "y1": 165, "x2": 560, "y2": 165, "amber": True},
        {"x1": 340, "y1": 235, "x2": 560, "y2": 235, "amber": True},
        {"x1": 340, "y1": 305, "x2": 560, "y2": 305, "amber": True},
    ],
    notes=[
        {"x": 190, "y": 46, "text": "source of flakiness", "anchor": "middle"},
        {"x": 730, "y": 46, "text": "make it deterministic", "anchor": "middle"},
    ],
)
