#!/usr/bin/env python3
"""Figure 9.2 — the anatomy of an EXPECT_CALL: which mock, which method and its
argument matchers, how many times it should be called, and what it should do when
it is. Run from the repo root:

    python scripts/diagrams/09-expect-call-anatomy.py
"""
import sys
from pathlib import Path

sys.path.insert(0, "scripts")
import generate_diagram as g  # noqa: E402

g.OUT = "assets/diagrams"
Path(g.OUT).mkdir(parents=True, exist_ok=True)

g.emit(
    "09-expect-call-anatomy", 900, 300,
    nodes=[
        {"x": 40, "y": 70, "w": 180, "h": 56, "style": "accent",
         "lines": ["EXPECT_CALL(carrier,", "which mock"]},
        {"x": 250, "y": 70, "w": 230, "h": 56, "style": "box",
         "lines": ["quote_cents(Ge(0), _))", "method + arg matchers"]},
        {"x": 510, "y": 70, "w": 150, "h": 56, "style": "user",
         "lines": [".Times(1)", "cardinality"]},
        {"x": 690, "y": 70, "w": 180, "h": 56, "style": "ink",
         "lines": [".WillOnce(Return(800))", "action"]},
    ],
    edges=[
        {"x1": 220, "y1": 98, "x2": 250, "y2": 98, "amber": True},
        {"x1": 480, "y1": 98, "x2": 510, "y2": 98, "amber": True},
        {"x1": 660, "y1": 98, "x2": 690, "y2": 98, "amber": True},
    ],
    notes=[
        {"x": 130, "y": 170, "text": "the mock object", "anchor": "middle"},
        {"x": 365, "y": 170, "text": "match this call and its arguments",
         "anchor": "middle"},
        {"x": 585, "y": 190, "text": "how many times", "anchor": "middle"},
        {"x": 780, "y": 170, "text": "what to return", "anchor": "middle"},
        {"x": 450, "y": 250,
         "text": "omit .Times and WillOnce implies exactly one call",
         "anchor": "middle"},
    ],
)
