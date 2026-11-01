#!/usr/bin/env python3
"""Figure 16.2 - one script, two places: ci.sh runs the same stages on a developer's
machine and in GitHub Actions, over a pinned toolchain, so a local pass and a CI pass
mean the same thing. Run from the repo root:

    python scripts/diagrams/16-local-equals-ci.py
"""
import sys
from pathlib import Path

sys.path.insert(0, "scripts")
import generate_diagram as g  # noqa: E402

g.OUT = "assets/diagrams"
Path(g.OUT).mkdir(parents=True, exist_ok=True)

g.emit(
    "16-local-equals-ci", 900, 300,
    nodes=[
        {"x": 330, "y": 40, "w": 240, "h": 56, "style": "accent",
         "lines": ["ci.sh", "the four stages, in order"]},

        {"x": 60, "y": 170, "w": 300, "h": 56, "style": "box",
         "lines": ["developer runs ./ci.sh", "before pushing"]},
        {"x": 540, "y": 170, "w": 300, "h": 56, "style": "box",
         "lines": ["GitHub Actions runs it", "on every push"]},

        {"x": 330, "y": 240, "w": 240, "h": 46, "style": "ink",
         "lines": ["pinned toolchain, same result"]},
    ],
    edges=[
        {"x1": 400, "y1": 96, "x2": 210, "y2": 170, "amber": True},
        {"x1": 500, "y1": 96, "x2": 690, "y2": 170, "amber": True},
        {"x1": 210, "y1": 226, "x2": 400, "y2": 262},
        {"x1": 690, "y1": 226, "x2": 500, "y2": 262},
    ],
    notes=[
        {"x": 450, "y": 150, "text": "same stages, same order, same gate",
         "anchor": "middle"},
    ],
)
