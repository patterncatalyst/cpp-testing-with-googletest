# Companion deck — build notes

A Red Hat-branded 16:9 deck, built with pptxgenjs (lgtm-presentation house style),
mined from the finished 16 chapters as a 60-minute conference cut.

| Deck source | Output |
|-------------|--------|
| `deck.js` | `cpp-testing-with-googletest.pptx` (25 slides) |

## Rebuild
```sh
cd presentation
npm install pptxgenjs          # first time only (node_modules is gitignored)
# regenerate figure PNGs from the site diagrams:
python3 - <<'PY'
import cairosvg, glob, os
for f in glob.glob("../assets/diagrams/*.svg"):
    name = os.path.splitext(os.path.basename(f))[0]
    cairosvg.svg2png(url=f, write_to=f"png/{name}.png", output_width=1800)
PY
export NODE_PATH=$(npm root -g) && node deck.js
```

Figures in `png/` are rendered from `../assets/diagrams/*.svg`. Speaker notes match
slides 1:1. Conventions: Red Hat fonts, the logo on every content slide,
shipping-domain examples, and ideas conveyed directly with no chapter references on
the slides. The deck follows the book's six parts: foundations, beyond the basics,
mocking, concurrency, and tooling, led by the thesis that a test is only worth the
behavior it pins.

## QA
```sh
soffice --headless --convert-to pdf --outdir qa cpp-testing-with-googletest.pptx
pdftoppm -jpeg -r 90 -f 1 -l 1 qa/cpp-testing-with-googletest.pdf qa/pg
```
