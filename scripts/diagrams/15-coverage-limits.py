#!/usr/bin/env python3
"""Figure 15.2 - what coverage does and does not tell you: it shows which lines and
branches a run executed, not whether the test asserted the result or whether an
untested input would break. Run from the repo root:

    python scripts/diagrams/15-coverage-limits.py
"""
import sys
from pathlib import Path

sys.path.insert(0, "scripts")
import generate_diagram as g  # noqa: E402

g.OUT = "assets/diagrams"
Path(g.OUT).mkdir(parents=True, exist_ok=True)

g.emit(
    "15-coverage-limits", 900, 280,
    bands=[
        {"x": 30, "y": 50, "w": 410, "h": 200, "label": "coverage tells you", "fill": "#f1f5f9"},
        {"x": 470, "y": 50, "w": 400, "h": 200, "label": "coverage cannot tell you", "fill": "#fff8ef"},
    ],
    nodes=[
        {"x": 55, "y": 92, "w": 360, "h": 48, "style": "box",
         "lines": ["which lines and branches ran"]},
        {"x": 55, "y": 156, "w": 360, "h": 48, "style": "box",
         "lines": ["where a whole branch went untested"]},

        {"x": 495, "y": 92, "w": 350, "h": 48, "style": "box",
         "lines": ["whether the test asserted the result"]},
        {"x": 495, "y": 156, "w": 350, "h": 48, "style": "box",
         "lines": ["whether an untested input breaks it"]},
    ],
    notes=[
        {"x": 450, "y": 268,
         "text": "100% coverage with weak assertions still passes a broken program",
         "anchor": "middle"},
    ],
)
