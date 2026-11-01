#!/usr/bin/env python3
"""Figure 9.1 — the seam a mock plugs into: the unit under test depends on an
abstract collaborator, so a real implementation serves production while a mock
serves the test, which controls what the mock returns and observes how it is
called. Run from the repo root:

    python scripts/diagrams/09-mock-seam.py
"""
import sys
from pathlib import Path

sys.path.insert(0, "scripts")
import generate_diagram as g  # noqa: E402

g.OUT = "assets/diagrams"
Path(g.OUT).mkdir(parents=True, exist_ok=True)

g.emit(
    "09-mock-seam", 900, 360,
    nodes=[
        {"x": 40, "y": 150, "w": 220, "h": 70, "style": "accent",
         "lines": ["Dispatcher", "unit under test", "holds a CarrierGateway&"]},
        {"x": 340, "y": 150, "w": 220, "h": 70, "style": "box",
         "lines": ["CarrierGateway", "abstract interface", "the seam"]},

        {"x": 640, "y": 48, "w": 230, "h": 64, "style": "sub",
         "lines": ["RealCarrier", "talks to the network", "(production)"]},
        {"x": 640, "y": 258, "w": 230, "h": 64, "style": "ink",
         "lines": ["MockCarrierGateway", "MOCK_METHOD per virtual", "(test)"]},
    ],
    edges=[
        {"x1": 260, "y1": 185, "x2": 340, "y2": 185, "label": "calls", "amber": True},
        {"x1": 560, "y1": 170, "x2": 640, "y2": 92, "label": "implements"},
        {"x1": 560, "y1": 200, "x2": 640, "y2": 284, "label": "implements", "amber": True},
    ],
    notes=[
        {"x": 755, "y": 340,
         "text": "the test sets return values and observes the calls",
         "anchor": "middle"},
    ],
)
