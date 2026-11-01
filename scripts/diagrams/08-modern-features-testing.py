#!/usr/bin/env python3
"""Figure 8.1 — three modern C++ features and how each is tested: a concept
checked when the template compiles, a lazy ranges pipeline materialized to a
value, and a generator coroutine whose yielded values are collected. Run from
the repo root:

    python scripts/diagrams/08-modern-features-testing.py
"""
import sys
from pathlib import Path

sys.path.insert(0, "scripts")
import generate_diagram as g  # noqa: E402

g.OUT = "assets/diagrams"
Path(g.OUT).mkdir(parents=True, exist_ok=True)

g.emit(
    "08-modern-features-testing", 980, 320,
    nodes=[
        {"x": 40, "y": 64, "w": 250, "h": 56, "style": "box",
         "lines": ["concept Weighable", "constrains a template"]},
        {"x": 350, "y": 64, "w": 250, "h": 56, "style": "accent",
         "lines": ["checked when it compiles", "a bad type is rejected"]},
        {"x": 660, "y": 64, "w": 290, "h": 56, "style": "ink",
         "lines": ["static_assert, constexpr bool"]},

        {"x": 40, "y": 144, "w": 250, "h": 56, "style": "box",
         "lines": ["ranges pipeline", "filter then transform"]},
        {"x": 350, "y": 144, "w": 250, "h": 56, "style": "accent",
         "lines": ["lazy, fold_left pulls once", "nothing runs until folded"]},
        {"x": 660, "y": 144, "w": 290, "h": 56, "style": "ink",
         "lines": ["materialize, EXPECT_EQ the value"]},

        {"x": 40, "y": 224, "w": 250, "h": 56, "style": "box",
         "lines": ["generator coroutine", "co_yield each value"]},
        {"x": 350, "y": 224, "w": 250, "h": 56, "style": "accent",
         "lines": ["lazy, resumed on demand", "may never terminate"]},
        {"x": 660, "y": 224, "w": 290, "h": 56, "style": "ink",
         "lines": ["collect yielded values, or take(n)"]},
    ],
    edges=[
        {"x1": 290, "y1": 92, "x2": 350, "y2": 92, "amber": True},
        {"x1": 600, "y1": 92, "x2": 660, "y2": 92, "amber": True},
        {"x1": 290, "y1": 172, "x2": 350, "y2": 172, "amber": True},
        {"x1": 600, "y1": 172, "x2": 660, "y2": 172, "amber": True},
        {"x1": 290, "y1": 252, "x2": 350, "y2": 252, "amber": True},
        {"x1": 600, "y1": 252, "x2": 660, "y2": 252, "amber": True},
    ],
    notes=[
        {"x": 165, "y": 40, "text": "feature", "anchor": "middle"},
        {"x": 475, "y": 40, "text": "when it runs", "anchor": "middle"},
        {"x": 805, "y": 40, "text": "how you test it", "anchor": "middle"},
    ],
)
