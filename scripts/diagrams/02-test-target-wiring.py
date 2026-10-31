#!/usr/bin/env python3
"""Figure 2.1 — how the build files compose a running test suite: Conan provides
GoogleTest, CMake links it with the library under test into the unit_tests
executable, gtest_discover_tests registers one CTest case per TEST, and ctest
runs them and reports per case. Run from the repo root:

    python scripts/diagrams/02-test-target-wiring.py
"""
import sys
from pathlib import Path

sys.path.insert(0, "scripts")
import generate_diagram as g  # noqa: E402

g.OUT = "assets/diagrams"
Path(g.OUT).mkdir(parents=True, exist_ok=True)

g.emit(
    "02-test-target-wiring", 1060, 350,
    nodes=[
        {"x": 30, "y": 40, "w": 180, "h": 56, "style": "accent",
         "lines": ["conanfile.py", "gtest/1.18.0 (pinned)"]},
        {"x": 30, "y": 150, "w": 180, "h": 56, "style": "box",
         "lines": ["shipping library", "rate_quote.cpp"]},
        {"x": 30, "y": 260, "w": 180, "h": 56, "style": "box",
         "lines": ["test source", "rate_quote_test.cpp"]},
        {"x": 320, "y": 140, "w": 210, "h": 86, "style": "accent",
         "lines": ["unit_tests (executable)", "links shipping,",
                   "GTest::gtest_main, gmock"]},
        {"x": 630, "y": 150, "w": 190, "h": 66, "style": "box",
         "lines": ["gtest_discover_tests", "one CTest case per TEST"]},
        {"x": 890, "y": 150, "w": 150, "h": 66, "style": "ink",
         "lines": ["ctest", "per-case report"]},
    ],
    edges=[
        {"x1": 210, "y1": 68, "x2": 320, "y2": 160, "label": "gtest_main, gmock"},
        {"x1": 210, "y1": 178, "x2": 320, "y2": 183, "label": "link"},
        {"x1": 210, "y1": 288, "x2": 320, "y2": 206, "label": "TEST cases"},
        {"x1": 530, "y1": 183, "x2": 630, "y2": 183, "amber": True, "label": "register"},
        {"x1": 820, "y1": 183, "x2": 890, "y2": 183, "amber": True, "label": "run"},
    ],
)
