# CLAUDE.md — Testing C++ with GoogleTest

## What this is

A chapter-based Jekyll tutorial (and Leanpub book from the same source) teaching
C++ testing with GoogleTest and GoogleMock on Fedora 44, built in the Pattern
Catalyst house style. Chapters live in `_docs/`, runnable examples in `examples/`,
diagrams in `assets/diagrams/`. It goes in the hub's **C++ Programming** group.

## Ground rules

- **Fedora 44 only.** No macOS or Windows instructions.
- **C++20 and C++23.** There is no C++21 — never write it.
- **Clang is primary; GCC appears only as a cross-check.**
- **Local tools only. No containers, no VM** (user decision 2026-10-31). The whole
  toolchain is local. If a chapter ever seems to need a container or root, stop and
  raise it rather than writing one in.
- Every code sample is real and runnable. No stubs, no ellipses.
- **Build success is a checkpoint, never the finish line.** A suite that compiles and
  goes green while asserting nothing meaningful is still broken. Each example's README
  verifies the behavior, not the exit code — including showing the test FAIL when the
  code under test is wrong.
- Code is `unverified` until it has actually been compiled and run here. Never
  self-promote code to `verified` from authoring alone.
- Prefer "real" or "practical" over "honest" in prose. Follow `lgtm-professional-voice`:
  no em dashes, no ASCII `--` dash substitute, no hype vocabulary, no chapter-code
  shorthand (`Ch12`) or work-item codes in reader-facing prose or figures.

## Skills to use (read the skill before the task — do not improvise the house style)

| Task | Skill |
|---|---|
| C++ toolchain, build, test infra, sanitizers | `lgtm-cpp` |
| Scaffold or change the site | `lgtm-jekyll` |
| Write or revise a chapter | `lgtm-tutorial` |
| Make a figure | `lgtm-diagram-generator` |
| Build the conference deck (last) | `lgtm-presentation` |
| Repo, commits, releases | `lgtm-github` |
| Idiomatic C++ review (pre-release) | `lgtm-idiomatic-review` |
| Prose voice pass (pre-release) | `lgtm-professional-voice` |
| Plan → build → validate orchestration | `lgtm-relay` |

Not used here: `lgtm-podman-stack` / `lgtm-docker-stack` (no containers) and
`lgtm-systems-programming` (no VM).

## Repo layout

    _config.yml            site identity, collections, default layouts
    _parts/*.md            one per Part; part_name is the join key
    _docs/NN-*.md          chapters, ordered by `order` (00-outline is order 0)
    examples/NN-name/      one runnable example per hands-on chapter
    examples/_template/    CMake + Conan + gtest toolchain each example copies
    assets/diagrams/       paired name.svg + name.excalidraw
    assets/css/site.css    the theme; all design tokens live here
    scripts/               check-prereqs.sh, generate_diagram.py, build helpers
    _plans/                iteration-plan.md (output: false — internal)

## House style (pinned — apply identically everywhere)

- **Build: CMake presets + CTest**, `gtest_discover_tests` for registration; Ninja
  generator; `-Wall -Wextra -Wpedantic -Werror`.
- **GoogleTest via Conan**, pinned `gtest/1.18.0` (Abseil pinned transitively).
  `FetchContent` and Fedora `gtest-devel` are documented as *alternatives* in the
  early chapters, never the build path.
- **Sanitizers**: one `asan-ubsan` preset and a separate `tsan` preset. ASan and TSan
  cannot share a binary — two passes, always.
- Command prefixes show where a command runs: `[host]$` (there is no container prefix).
- Example dirs are `examples/NN-topic/` with a `demo.sh` and a `README.md`, each a
  copy of `examples/_template/` with the test code filled in.
- Single-line, shell-safe commands.

## Chapter workflow

1. Front matter: `title`, `order`, `part`, `description`, `duration`. `part:` must
   exactly equal a `_parts` file's `part_name` (case + spacing). Quote any value with a colon.
2. Chapter spine: learning objectives → diagram → why this matters → concept → how the
   code works → **AI sidebar** → build/run/observe → cross-check → what you learned →
   for deeper coverage → verification status.
3. Depth: **2,000-word PROSE floor, 2,500 soft target** (excluding code, captions, and
   command output; the AI sidebar, learning-objective bullets, and summary bullets count
   as prose). Lean toward 2,500 as a chapter earns it, and the more diagrams, the closer
   to 2,500. Value, never padding. `scan.sh --stats` undercounts because it omits bullets
   and blockquotes; a true prose count (code and diagrams removed) is the measure.
4. **AI sidebar (every chapter):** one `> **AI sidebar.**` blockquote placed just before
   "Build, run, observe", on how an LLM pairing partner tends to write tests for the
   chapter's topic and what to check in what it returns. The design-patterns-books
   convention; keep it to one per chapter, no section heading.
5. Reference example files by full path: `examples/NN-name/file`.
6. One runnable example + at least one paired SVG + `.excalidraw` figure per hands-on chapter;
   a chapter covering several distinct mechanisms earns a second figure.

## Diagrams

Generate paired SVG + `.excalidraw` with `scripts/generate_diagram.py`; embed with the
`excalidraw.html` include. Dark labels. No chapter/part numbers or internal codes baked
into a figure. Wrap literal Liquid braces in `{% raw %} … {% endraw %}` in prose.

## Validate before shipping a chapter

- Front matter parses as YAML and has a `part` that matches a `_parts` `part_name`.
- No unescaped `{{ }}` outside relative_url / include / site.
- Examples: `cmake --preset …` + `ctest --preset …` pass clean on the default preset,
  then separately under `asan-ubsan` and `tsan`.
- Run `scan.sh --tier ban` (professional-voice) and the idiomatic-review scan on touched files.

## Chapter 0 is prerequisites

Gets the reader to a working Fedora 44 toolchain before anything else: exact, dated
versions; one copy-pasteable install block; `scripts/check-prereqs.sh` (exits non-zero
on anything missing); a dated known-good version matrix; teardown. No container setup.

## Book output (Leanpub) — same source, two products

Chapters live once in `_docs/`. `scripts/build-manuscript.py` generates `manuscript/`
in Markua (never hand-edit `manuscript/` chapter files). Include code from files
(`<<[Caption](code/NN-name/main.cpp)`), never paste it, so the book and `examples/`
cannot drift. Outputs are PDF and EPUB. Added once ~3 chapters exist.

## Commits and releases (lgtm-github)

Conventional Commits, `type(scope): summary`, imperative, ≤ 72 chars. Types: docs,
site, demo, ci, chore, fix, feat, refactor, style. Scopes: `§N` for `_docs/NN-*`,
`demo-NN` for `examples/NN-*`, `rNN.x` for a release iteration. One `release/rNN`
branch per iteration → squash-merged PR → `rNN.x` tag. Never commit to main directly;
auto-merge is disabled, so poll checks then `gh pr merge --squash --delete-branch`.

## License

Apache-2.0 for code (`LICENSE`), CC BY 4.0 for prose and figures (`LICENSE-docs.md`).

## Do not

- Claim a test result that was not actually run.
- Reintroduce `white-space: nowrap` on table-cell code in `site.css`.
- Reference "the roadmap" or "the requirements" in reader-facing prose.
- Add a chapter without a `_plans/iteration-plan.md` row.
- Write a container or VM path — this project is local tools only.
