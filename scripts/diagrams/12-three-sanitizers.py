#!/usr/bin/env python3
"""Figure 12.1 — the three sanitizers, what each instruments, and the bug each one
catches in this example. ASan and UBSan share one binary; TSan needs its own. Run
from the repo root:

    python scripts/diagrams/12-three-sanitizers.py
"""
import sys
from pathlib import Path

sys.path.insert(0, "scripts")
import generate_diagram as g  # noqa: E402

g.OUT = "assets/diagrams"
Path(g.OUT).mkdir(parents=True, exist_ok=True)

g.emit(
    "12-three-sanitizers", 960, 320,
    bands=[
        {"x": 20, "y": 44, "w": 600, "h": 200, "label": "one binary: asan-ubsan",
         "fill": "#fff8ef"},
        {"x": 640, "y": 44, "w": 300, "h": 200, "label": "a separate binary: tsan",
         "fill": "#f1f5f9"},
    ],
    nodes=[
        {"x": 40, "y": 84, "w": 180, "h": 64, "style": "box",
         "lines": ["AddressSanitizer", "memory errors"]},
        {"x": 250, "y": 84, "w": 350, "h": 64, "style": "ink",
         "lines": ["use-after-free, buffer overflow, leaks", "catches: a dangling span"]},

        {"x": 40, "y": 164, "w": 180, "h": 64, "style": "box",
         "lines": ["UndefinedBehavior", "Sanitizer"]},
        {"x": 250, "y": 164, "w": 350, "h": 64, "style": "ink",
         "lines": ["overflow, bad shift, misalignment", "catches: 32-bit sum overflow"]},

        {"x": 660, "y": 124, "w": 260, "h": 70, "style": "box",
         "lines": ["ThreadSanitizer", "data races", "catches: a shared accumulator"]},
    ],
    notes=[
        {"x": 320, "y": 272,
         "text": "ASan and UBSan combine in one build; TSan cannot join them",
         "anchor": "middle"},
    ],
)
