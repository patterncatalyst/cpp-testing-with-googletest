#!/usr/bin/env python3
"""Figure 11.1 — the shape of a concurrent test: a latch releases N worker threads
together, each records M times into a mutex-protected ledger, and after the threads
join the test asserts the aggregate the concurrency must preserve. Run from the
repo root:

    python scripts/diagrams/11-concurrent-test-anatomy.py
"""
import sys
from pathlib import Path

sys.path.insert(0, "scripts")
import generate_diagram as g  # noqa: E402

g.OUT = "assets/diagrams"
Path(g.OUT).mkdir(parents=True, exist_ok=True)

g.emit(
    "11-concurrent-test-anatomy", 980, 300,
    nodes=[
        {"x": 30, "y": 110, "w": 150, "h": 80, "style": "accent",
         "lines": ["std::latch", "release every", "worker at once"]},

        {"x": 250, "y": 40, "w": 210, "h": 54, "style": "box",
         "lines": ["worker thread", "M x record(g)"]},
        {"x": 250, "y": 123, "w": 210, "h": 54, "style": "box",
         "lines": ["worker thread", "M x record(g)"]},
        {"x": 250, "y": 206, "w": 210, "h": 54, "style": "box",
         "lines": ["worker thread", "M x record(g)"]},

        {"x": 540, "y": 110, "w": 180, "h": 80, "style": "ink",
         "lines": ["ShipmentLedger", "mutex-protected", "count, total"]},

        {"x": 780, "y": 110, "w": 180, "h": 80, "style": "box",
         "lines": ["join, then assert", "count = N x M", "total = N x M x g"]},
    ],
    edges=[
        {"x1": 180, "y1": 140, "x2": 250, "y2": 67, "amber": True},
        {"x1": 180, "y1": 150, "x2": 250, "y2": 150, "amber": True},
        {"x1": 180, "y1": 160, "x2": 250, "y2": 233, "amber": True},
        {"x1": 460, "y1": 67, "x2": 540, "y2": 135},
        {"x1": 460, "y1": 150, "x2": 540, "y2": 150},
        {"x1": 460, "y1": 233, "x2": 540, "y2": 165},
        {"x1": 720, "y1": 150, "x2": 780, "y2": 150, "label": "after join", "amber": True},
    ],
    notes=[
        {"x": 355, "y": 285, "text": "N worker threads, M records each",
         "anchor": "middle"},
    ],
)
