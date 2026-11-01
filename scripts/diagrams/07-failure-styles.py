#!/usr/bin/env python3
"""Figure 7.1 — three kinds of failure and how each is signalled and tested: a
recoverable error as a std::expected value, a broken contract as an exception,
and a programming bug as an assert that aborts. Run from the repo root:

    python scripts/diagrams/07-failure-styles.py
"""
import sys
from pathlib import Path

sys.path.insert(0, "scripts")
import generate_diagram as g  # noqa: E402

g.OUT = "assets/diagrams"
Path(g.OUT).mkdir(parents=True, exist_ok=True)

g.emit(
    "07-failure-styles", 960, 320,
    nodes=[
        {"x": 40, "y": 64, "w": 240, "h": 56, "style": "box",
         "lines": ["recoverable input error", "bad weight text"]},
        {"x": 340, "y": 64, "w": 260, "h": 56, "style": "accent",
         "lines": ["std::expected<int, E>", "return a value"]},
        {"x": 660, "y": 64, "w": 280, "h": 56, "style": "ink",
         "lines": ["has_value(), *result, error()"]},

        {"x": 40, "y": 144, "w": 240, "h": 56, "style": "box",
         "lines": ["caller broke a contract", "bad argument at the API"]},
        {"x": 340, "y": 144, "w": 260, "h": 56, "style": "accent",
         "lines": ["throw std::invalid_argument", "with a message"]},
        {"x": 660, "y": 144, "w": 280, "h": 56, "style": "ink",
         "lines": ["EXPECT_THROW, then what()"]},

        {"x": 40, "y": 224, "w": 240, "h": 56, "style": "box",
         "lines": ["programming bug", "an impossible state"]},
        {"x": 340, "y": 224, "w": 260, "h": 56, "style": "accent",
         "lines": ["assert, then abort", "terminate the process"]},
        {"x": 660, "y": 224, "w": 280, "h": 56, "style": "ink",
         "lines": ["EXPECT_DEATH(stmt, regex)"]},
    ],
    edges=[
        {"x1": 280, "y1": 92, "x2": 340, "y2": 92, "amber": True},
        {"x1": 600, "y1": 92, "x2": 660, "y2": 92, "amber": True},
        {"x1": 280, "y1": 172, "x2": 340, "y2": 172, "amber": True},
        {"x1": 600, "y1": 172, "x2": 660, "y2": 172, "amber": True},
        {"x1": 280, "y1": 252, "x2": 340, "y2": 252, "amber": True},
        {"x1": 600, "y1": 252, "x2": 660, "y2": 252, "amber": True},
    ],
    notes=[
        {"x": 160, "y": 40, "text": "situation", "anchor": "middle"},
        {"x": 470, "y": 40, "text": "how it signals", "anchor": "middle"},
        {"x": 800, "y": 40, "text": "how you test it", "anchor": "middle"},
    ],
)
