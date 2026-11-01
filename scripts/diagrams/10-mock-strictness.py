#!/usr/bin/env python3
"""Figure 10.1 — the mock strictness spectrum: how a NiceMock, a default (naggy)
mock, and a StrictMock each treat a call that no EXPECT_CALL covers. Run from the
repo root:

    python scripts/diagrams/10-mock-strictness.py
"""
import sys
from pathlib import Path

sys.path.insert(0, "scripts")
import generate_diagram as g  # noqa: E402

g.OUT = "assets/diagrams"
Path(g.OUT).mkdir(parents=True, exist_ok=True)

g.emit(
    "10-mock-strictness", 920, 320,
    nodes=[
        {"x": 40, "y": 64, "w": 230, "h": 56, "style": "box",
         "lines": ["NiceMock<T>", "an uninteresting call is"]},
        {"x": 350, "y": 64, "w": 300, "h": 56, "style": "ink",
         "lines": ["silent", "allowed, no warning"]},

        {"x": 40, "y": 144, "w": 230, "h": 56, "style": "accent",
         "lines": ["default mock (naggy)", "an uninteresting call is"]},
        {"x": 350, "y": 144, "w": 300, "h": 56, "style": "ink",
         "lines": ["a warning", "allowed, logs GMOCK WARNING"]},

        {"x": 40, "y": 224, "w": 230, "h": 56, "style": "box",
         "lines": ["StrictMock<T>", "an uninteresting call is"]},
        {"x": 350, "y": 224, "w": 300, "h": 56, "style": "ink",
         "lines": ["a failure", "fails the test immediately"]},
    ],
    edges=[
        {"x1": 270, "y1": 92, "x2": 350, "y2": 92, "amber": True},
        {"x1": 270, "y1": 172, "x2": 350, "y2": 172, "amber": True},
        {"x1": 270, "y1": 252, "x2": 350, "y2": 252, "amber": True},
    ],
    notes=[
        {"x": 500, "y": 40,
         "text": "a call no EXPECT_CALL covers", "anchor": "middle"},
        {"x": 460, "y": 305,
         "text": "a matching EXPECT_CALL always takes priority on any of the three",
         "anchor": "middle"},
    ],
)
