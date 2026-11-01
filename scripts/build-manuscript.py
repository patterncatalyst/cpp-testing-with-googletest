#!/usr/bin/env python3
"""Generate a Leanpub (Markua) manuscript from the Jekyll chapters.

The chapters in _docs/ are the single source. This script transforms them into
manuscript/: it strips the YAML front matter, turns each chapter's figure includes
into Markua image references (rendering the paired SVG to PNG under
manuscript/resources/), removes the Jekyll/Liquid constructs, and writes a Book.txt
that orders part dividers and chapters. Never hand-edit manuscript/ chapter files;
edit _docs/ and re-run.

    python scripts/build-manuscript.py

Requires cairosvg (for SVG -> PNG). Content chapters are order >= 1; the order-0
outline is a site navigation page and is not part of the book.
"""
from __future__ import annotations

import re
import shutil
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent
DOCS = ROOT / "_docs"
PARTS = ROOT / "_parts"
DIAGRAMS = ROOT / "assets" / "diagrams"
OUT = ROOT / "manuscript"
RESOURCES = OUT / "resources"


def parse_front_matter(text: str) -> tuple[dict, str]:
    """Split a Jekyll file into its YAML front matter (as a flat dict) and body."""
    m = re.match(r"^---\n(.*?)\n---\n(.*)$", text, re.S)
    if not m:
        return {}, text
    meta: dict[str, str] = {}
    for line in m.group(1).splitlines():
        if ":" in line:
            key, _, value = line.partition(":")
            meta[key.strip()] = value.strip().strip('"')
    return meta, m.group(2)


def collect(dir_path: Path) -> list[tuple[int, dict, str]]:
    """Return (order, meta, body) for each .md in dir_path, sorted by order."""
    items = []
    for path in sorted(dir_path.glob("*.md")):
        meta, body = parse_front_matter(path.read_text(encoding="utf-8"))
        order = int(meta.get("order", "0"))
        items.append((order, meta, body))
    items.sort(key=lambda t: t[0])
    return items


FIGURES: set[str] = set()


def convert_figure_include(match: re.Match) -> str:
    """Turn a {% include excalidraw.html file=.. caption=.. %} into a Markua image."""
    attrs = match.group(1)
    file_m = re.search(r'file="([^"]+)"', attrs)
    cap_m = re.search(r'caption="([^"]+)"', attrs)
    if not file_m:
        return ""
    name = file_m.group(1)
    caption = cap_m.group(1) if cap_m else ""
    FIGURES.add(name)
    return f"![{caption}](resources/{name}.png)"


def transform_body(body: str, title: str) -> str:
    """Convert one chapter body from Jekyll Markdown to Markua."""
    # Figure includes -> image references (records the figure for rendering).
    body = re.sub(r"\{%\s*include excalidraw\.html(.*?)%\}",
                  convert_figure_include, body)
    # Drop raw guards.
    body = body.replace("{% raw %}", "").replace("{% endraw %}", "")
    # A Markdown link whose URL is a Liquid expression -> keep only the link text.
    body = re.sub(r"\[([^\]]+)\]\(\{\{[^)]*\}\}\)", r"\1", body)
    # Any remaining Liquid -> gone.
    body = re.sub(r"\{\{.*?\}\}", "", body)
    body = re.sub(r"\{%.*?%\}", "", body)
    # The site renders the verification status as an inline HTML span; flatten it.
    body = body.replace(
        '<span class="status status--verified">verified</span>', "verified")
    # Collapse the runs of blank lines a removal can leave behind.
    body = re.sub(r"\n{3,}", "\n\n", body).strip()
    return f"# {title}\n\n{body}\n"


def slug(path_order: int, name: str) -> str:
    return f"{path_order:02d}-{name}"


def main() -> None:
    if OUT.exists():
        shutil.rmtree(OUT)
    RESOURCES.mkdir(parents=True)

    parts = {meta["part_name"]: (order, meta)
             for order, meta, _ in collect(PARTS)}
    chapters = [(o, m, b) for (o, m, b) in collect(DOCS) if o >= 1]

    book_lines: list[str] = []
    emitted_parts: set[str] = set()

    for order, meta, body in chapters:
        part_name = meta.get("part", "")
        if part_name and part_name not in emitted_parts:
            emitted_parts.add(part_name)
            part_order, part_meta = parts.get(part_name, (0, {}))
            divider = f"part-{part_order:02d}.md"
            title = part_meta.get("title", part_name)
            blurb = part_meta.get("blurb", "")
            (OUT / divider).write_text(
                f"# Part {part_order}: {title}\n\n{blurb}\n", encoding="utf-8")
            book_lines.append(divider)

        fname = slug(order, Path(meta.get("title", "chapter")).name
                     .lower().replace(" ", "-").replace("/", "-"))
        fname = re.sub(r"[^a-z0-9-]", "", fname) + ".md"
        (OUT / fname).write_text(
            transform_body(body, meta.get("title", "")), encoding="utf-8")
        book_lines.append(fname)

    (OUT / "Book.txt").write_text("\n".join(book_lines) + "\n", encoding="utf-8")

    # Render each referenced figure's SVG to a PNG under resources/.
    import cairosvg  # local import so the module is only needed at build time

    rendered = 0
    for name in sorted(FIGURES):
        svg = DIAGRAMS / f"{name}.svg"
        if svg.exists():
            cairosvg.svg2png(url=str(svg), write_to=str(RESOURCES / f"{name}.png"),
                             output_width=1600)
            rendered += 1
        else:
            print(f"  WARNING: figure not found: {svg}")

    print(f"Wrote {len(book_lines)} Book.txt entries "
          f"({len(emitted_parts)} parts, {len(chapters)} chapters) "
          f"and rendered {rendered} figures to manuscript/resources/.")


if __name__ == "__main__":
    main()
