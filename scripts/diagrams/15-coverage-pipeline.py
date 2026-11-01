#!/usr/bin/env python3
"""Figure 15.1 - the llvm-cov pipeline: compile instrumented, run to emit a raw
profile, merge it, and report line, region, and branch coverage. Run from the repo
root:

    python scripts/diagrams/15-coverage-pipeline.py
"""
import sys
from pathlib import Path

sys.path.insert(0, "scripts")
import generate_diagram as g  # noqa: E402

g.OUT = "assets/diagrams"
Path(g.OUT).mkdir(parents=True, exist_ok=True)

g.emit(
    "15-coverage-pipeline", 980, 210,
    nodes=[
        {"x": 30, "y": 80, "w": 200, "h": 64, "style": "accent",
         "lines": ["compile instrumented", "-fprofile-instr-generate", "-fcoverage-mapping"]},
        {"x": 280, "y": 80, "w": 180, "h": 64, "style": "box",
         "lines": ["run the tests", "LLVM_PROFILE_FILE", "-> .profraw"]},
        {"x": 510, "y": 80, "w": 180, "h": 64, "style": "box",
         "lines": ["llvm-profdata merge", "-> .profdata"]},
        {"x": 740, "y": 80, "w": 210, "h": 64, "style": "ink",
         "lines": ["llvm-cov report / show", "line, region, branch"]},
    ],
    edges=[
        {"x1": 230, "y1": 112, "x2": 280, "y2": 112, "amber": True},
        {"x1": 460, "y1": 112, "x2": 510, "y2": 112, "amber": True},
        {"x1": 690, "y1": 112, "x2": 740, "y2": 112, "amber": True},
    ],
    notes=[
        {"x": 490, "y": 56, "text": "one build, one run, one report", "anchor": "middle"},
    ],
)
