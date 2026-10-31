#!/usr/bin/env python3
"""Figure 1.1 — the local build-and-test loop every example in this book runs,
and the three sanitizer lanes that share it. Run from the repo root:

    python scripts/diagrams/01-build-and-test-loop.py
"""
import sys
from pathlib import Path

sys.path.insert(0, "scripts")
import generate_diagram as g  # noqa: E402

g.OUT = "assets/diagrams"
Path(g.OUT).mkdir(parents=True, exist_ok=True)

g.emit(
    "01-build-and-test-loop", 980, 430,
    bands=[
        {"x": 30, "y": 286, "w": 930, "h": 120,
         "label": "The same loop, three lanes (separate build dirs)",
         "fill": "#fafafa"},
    ],
    nodes=[
        {"x": 30,  "y": 56, "w": 140, "h": 64, "style": "box",
         "lines": ["Edit source"]},
        {"x": 210, "y": 56, "w": 160, "h": 64, "style": "accent",
         "lines": ["conan install", "resolve gtest/1.18.0"]},
        {"x": 410, "y": 56, "w": 160, "h": 64, "style": "box",
         "lines": ["cmake --preset", "configure (Ninja)"]},
        {"x": 610, "y": 56, "w": 160, "h": 64, "style": "box",
         "lines": ["cmake --build", "clang, -Werror"]},
        {"x": 810, "y": 56, "w": 150, "h": 64, "style": "accent",
         "lines": ["ctest", "gtest_discover_tests"]},
        {"x": 810, "y": 190, "w": 150, "h": 56, "style": "ink",
         "lines": ["observe", "green: done"]},
        {"x": 55,  "y": 330, "w": 270, "h": 58, "style": "box",
         "lines": ["default preset", "build/ · fast feedback"]},
        {"x": 355, "y": 330, "w": 280, "h": 58, "style": "box",
         "lines": ["asan-ubsan preset", "build-asan/ · ASan + UBSan"]},
        {"x": 665, "y": 330, "w": 270, "h": 58, "style": "box",
         "lines": ["tsan preset", "build-tsan/ · TSan (own binary)"]},
    ],
    edges=[
        {"x1": 170, "y1": 88, "x2": 210, "y2": 88},
        {"x1": 370, "y1": 88, "x2": 410, "y2": 88},
        {"x1": 570, "y1": 88, "x2": 610, "y2": 88},
        {"x1": 770, "y1": 88, "x2": 810, "y2": 88},
        {"x1": 885, "y1": 120, "x2": 885, "y2": 190, "amber": True},
        {"x1": 810, "y1": 218, "x2": 100, "y2": 218, "amber": True,
         "label": "red: fix, rerun"},
        {"x1": 100, "y1": 218, "x2": 100, "y2": 120, "amber": True},
    ],
)
