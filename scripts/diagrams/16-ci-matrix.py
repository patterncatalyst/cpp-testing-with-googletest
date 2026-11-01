#!/usr/bin/env python3
"""Figure 16.1 - the CI matrix a push triggers: the default lane and the two
sanitizer lanes run in parallel (ASan with UBSan in one binary, TSan in its own),
and a coverage gate runs alongside; all must be green. Run from the repo root:

    python scripts/diagrams/16-ci-matrix.py
"""
import sys
from pathlib import Path

sys.path.insert(0, "scripts")
import generate_diagram as g  # noqa: E402

g.OUT = "assets/diagrams"
Path(g.OUT).mkdir(parents=True, exist_ok=True)

g.emit(
    "16-ci-matrix", 940, 320,
    nodes=[
        {"x": 30, "y": 130, "w": 150, "h": 60, "style": "accent",
         "lines": ["push / PR", "triggers CI"]},

        {"x": 360, "y": 40, "w": 260, "h": 46, "style": "box",
         "lines": ["default lane: build + ctest"]},
        {"x": 360, "y": 104, "w": 260, "h": 46, "style": "box",
         "lines": ["asan-ubsan lane (one binary)"]},
        {"x": 360, "y": 168, "w": 260, "h": 46, "style": "box",
         "lines": ["tsan lane (its own binary)"]},
        {"x": 360, "y": 232, "w": 260, "h": 46, "style": "box",
         "lines": ["coverage gate: llvm-cov"]},

        {"x": 760, "y": 130, "w": 150, "h": 60, "style": "ink",
         "lines": ["all green", "merge allowed"]},
    ],
    edges=[
        {"x1": 180, "y1": 150, "x2": 360, "y2": 63, "amber": True},
        {"x1": 180, "y1": 158, "x2": 360, "y2": 127, "amber": True},
        {"x1": 180, "y1": 166, "x2": 360, "y2": 191, "amber": True},
        {"x1": 180, "y1": 172, "x2": 360, "y2": 255, "amber": True},
        {"x1": 620, "y1": 63, "x2": 760, "y2": 150},
        {"x1": 620, "y1": 127, "x2": 760, "y2": 155},
        {"x1": 620, "y1": 191, "x2": 760, "y2": 165},
        {"x1": 620, "y1": 255, "x2": 760, "y2": 172},
    ],
    notes=[
        {"x": 490, "y": 300,
         "text": "ASan and TSan never share a binary, so they are separate legs",
         "anchor": "middle"},
    ],
)
