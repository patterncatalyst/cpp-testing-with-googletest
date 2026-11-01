#!/usr/bin/env python3
"""Figure 5.1 — the per-test fixture lifecycle: for every TEST_F, GoogleTest
constructs a fresh fixture, runs SetUp, runs the test body, then destroys the
fixture, so no test can see another test's state. Run from the repo root:

    python scripts/diagrams/05-fixture-lifecycle.py
"""
import sys
from pathlib import Path

sys.path.insert(0, "scripts")
import generate_diagram as g  # noqa: E402

g.OUT = "assets/diagrams"
Path(g.OUT).mkdir(parents=True, exist_ok=True)

g.emit(
    "05-fixture-lifecycle", 820, 410,
    nodes=[
        {"x": 90, "y": 40, "w": 330, "h": 52, "style": "box",
         "lines": ["construct a fresh ManifestTest"]},
        {"x": 90, "y": 128, "w": 330, "h": 60, "style": "accent",
         "lines": ["SetUp()", "manifest_.add(1000); add(2500)"]},
        {"x": 90, "y": 224, "w": 330, "h": 52, "style": "box",
         "lines": ["the TEST_F body runs"]},
        {"x": 90, "y": 312, "w": 330, "h": 60, "style": "ink",
         "lines": ["destruct", "~Manifest frees the vector"]},
        {"x": 500, "y": 128, "w": 290, "h": 156, "style": "ghost",
         "lines": ["Repeats for every TEST_F.",
                   "Each test gets its own",
                   "instance, so no test sees",
                   "another test's state."]},
    ],
    edges=[
        {"x1": 255, "y1": 92, "x2": 255, "y2": 128, "amber": True},
        {"x1": 255, "y1": 188, "x2": 255, "y2": 224, "amber": True},
        {"x1": 255, "y1": 276, "x2": 255, "y2": 312, "amber": True},
    ],
)
