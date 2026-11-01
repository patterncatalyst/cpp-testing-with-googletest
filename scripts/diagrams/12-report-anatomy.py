#!/usr/bin/env python3
"""Figure 12.2 — how to read a sanitizer report, using an ASan heap-use-after-free:
the error kind and faulting access, then the stack that freed the memory, then the
stack that allocated it, then the summary. Run from the repo root:

    python scripts/diagrams/12-report-anatomy.py
"""
import sys
from pathlib import Path

sys.path.insert(0, "scripts")
import generate_diagram as g  # noqa: E402

g.OUT = "assets/diagrams"
Path(g.OUT).mkdir(parents=True, exist_ok=True)

g.emit(
    "12-report-anatomy", 760, 360,
    nodes=[
        {"x": 230, "y": 30, "w": 300, "h": 50, "style": "accent",
         "lines": ["heap-use-after-free, READ of size 4", "the error kind and the faulting access"]},
        {"x": 230, "y": 110, "w": 300, "h": 50, "style": "box",
         "lines": ["#0 total_weight ... weights.cpp", "where the bad access happened"]},
        {"x": 230, "y": 190, "w": 300, "h": 50, "style": "box",
         "lines": ["freed by thread T0 here", "the vector destructor, make_batch"]},
        {"x": 230, "y": 270, "w": 300, "h": 50, "style": "box",
         "lines": ["previously allocated here", "the vector constructor"]},
    ],
    edges=[
        {"x1": 380, "y1": 80, "x2": 380, "y2": 110, "amber": True},
        {"x1": 380, "y1": 160, "x2": 380, "y2": 190, "amber": True},
        {"x1": 380, "y1": 240, "x2": 380, "y2": 270, "amber": True},
    ],
    notes=[
        {"x": 115, "y": 60, "text": "what", "anchor": "middle"},
        {"x": 115, "y": 140, "text": "where", "anchor": "middle"},
        {"x": 115, "y": 220, "text": "when freed", "anchor": "middle"},
        {"x": 115, "y": 300, "text": "when made", "anchor": "middle"},
        {"x": 645, "y": 200, "text": "read these", "anchor": "middle"},
        {"x": 645, "y": 216, "text": "three together", "anchor": "middle"},
    ],
)
