#!/usr/bin/env python3
"""Figure 8.2 — why a test over an infinite generator terminates: views::take
pulls a fixed number of values on demand, so the coroutine's unbounded loop body
runs only as many times as the consumer asks. Run from the repo root:

    python scripts/diagrams/08-lazy-generator-pull.py
"""
import sys
from pathlib import Path

sys.path.insert(0, "scripts")
import generate_diagram as g  # noqa: E402

g.OUT = "assets/diagrams"
Path(g.OUT).mkdir(parents=True, exist_ok=True)

g.emit(
    "08-lazy-generator-pull", 960, 300,
    nodes=[
        {"x": 40, "y": 96, "w": 250, "h": 80, "style": "accent",
         "lines": ["running_total(500)", "while (true) co_yield", "an infinite source"]},
        {"x": 370, "y": 96, "w": 220, "h": 80, "style": "box",
         "lines": ["views::take(3)", "pulls on demand,", "stops after three"]},
        {"x": 670, "y": 96, "w": 250, "h": 80, "style": "ink",
         "lines": ["{500, 1000, 1500}", "the test compares this"]},
    ],
    edges=[
        {"x1": 290, "y1": 136, "x2": 370, "y2": 136, "label": "value on demand", "amber": True},
        {"x1": 590, "y1": 136, "x2": 670, "y2": 136, "label": "collected", "amber": True},
    ],
    notes=[
        {"x": 165, "y": 212, "text": "the loop body resumes 3 times, never a 4th",
         "anchor": "middle"},
        {"x": 480, "y": 72, "text": "laziness is what makes the test terminate",
         "anchor": "middle"},
    ],
)
