#!/usr/bin/env python3
"""Figure 7.2 — how a death test runs: EXPECT_DEATH forks a child that runs the
statement and is expected to abort; the parent waits and checks that the child
died and its stderr matched the regex. Run from the repo root:

    python scripts/diagrams/07-death-test-fork.py
"""
import sys
from pathlib import Path

sys.path.insert(0, "scripts")
import generate_diagram as g  # noqa: E402

g.OUT = "assets/diagrams"
Path(g.OUT).mkdir(parents=True, exist_ok=True)

g.emit(
    "07-death-test-fork", 820, 400,
    nodes=[
        {"x": 290, "y": 36, "w": 240, "h": 54, "style": "accent",
         "lines": ["EXPECT_DEATH(stmt, regex)"]},
        {"x": 330, "y": 128, "w": 160, "h": 48, "style": "box",
         "lines": ["fork()"]},
        {"x": 60, "y": 226, "w": 300, "h": 70, "style": "box",
         "lines": ["child process", "runs stmt, assert fires, abort()",
                   "writes the message to stderr"]},
        {"x": 460, "y": 226, "w": 300, "h": 70, "style": "box",
         "lines": ["parent process", "waits for the child"]},
        {"x": 460, "y": 328, "w": 300, "h": 54, "style": "ink",
         "lines": ["pass: child died AND stderr matched;",
                   "a clean exit is a failure"]},
    ],
    edges=[
        {"x1": 410, "y1": 90, "x2": 410, "y2": 128, "amber": True},
        {"x1": 360, "y1": 176, "x2": 240, "y2": 226, "amber": True, "label": "fork"},
        {"x1": 460, "y1": 176, "x2": 590, "y2": 226, "amber": True},
        {"x1": 360, "y1": 261, "x2": 460, "y2": 261, "amber": True,
         "label": "exit + stderr"},
        {"x1": 610, "y1": 296, "x2": 610, "y2": 328, "amber": True},
    ],
)
