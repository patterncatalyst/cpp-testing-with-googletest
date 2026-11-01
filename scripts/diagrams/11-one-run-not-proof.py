#!/usr/bin/env python3
"""Figure 11.2 — why one green concurrent run is not proof: a single run exercises
one interleaving out of many, so a race can hide. ThreadSanitizer checks the memory
model on the schedule it sees, and repetition samples more schedules. Run from the
repo root:

    python scripts/diagrams/11-one-run-not-proof.py
"""
import sys
from pathlib import Path

sys.path.insert(0, "scripts")
import generate_diagram as g  # noqa: E402

g.OUT = "assets/diagrams"
Path(g.OUT).mkdir(parents=True, exist_ok=True)

g.emit(
    "11-one-run-not-proof", 920, 300,
    nodes=[
        {"x": 40, "y": 110, "w": 210, "h": 80, "style": "box",
         "lines": ["one test run", "one interleaving", "of very many"]},

        {"x": 330, "y": 50, "w": 250, "h": 56, "style": "ghost",
         "lines": ["green", "this schedule happened to work"]},
        {"x": 330, "y": 194, "w": 250, "h": 56, "style": "accent",
         "lines": ["a race still hides", "in a schedule not sampled"]},

        {"x": 660, "y": 50, "w": 230, "h": 56, "style": "ink",
         "lines": ["ThreadSanitizer", "flags the race on any run"]},
        {"x": 660, "y": 194, "w": 230, "h": 56, "style": "ink",
         "lines": ["repetition", "samples more schedules"]},
    ],
    edges=[
        {"x1": 250, "y1": 130, "x2": 330, "y2": 86},
        {"x1": 250, "y1": 170, "x2": 330, "y2": 214, "amber": True},
        {"x1": 580, "y1": 78, "x2": 660, "y2": 78, "amber": True},
        {"x1": 580, "y1": 222, "x2": 660, "y2": 222, "amber": True},
    ],
    notes=[
        {"x": 460, "y": 290,
         "text": "a passing run narrows the doubt; the sanitizer and repetition close it",
         "anchor": "middle"},
    ],
)
