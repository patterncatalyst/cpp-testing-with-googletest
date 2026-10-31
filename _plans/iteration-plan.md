---
title: "Iteration plan"
layout: plan
render_with_liquid: false
---

# Iteration plan

Internal roadmap (not published; `plans` is `output: false`). One `release/rNN`
branch per iteration, each shipped as a squash-merged PR plus an `rNN.x` tag.
Chapters are authored in order; every hands-on chapter ships one runnable example
under `examples/NN-name/` and at least one paired SVG + `.excalidraw` figure. Code
is `unverified` until actually built and run on Fedora 44.

| Iteration | Scope | Status |
|---|---|---|
| **r00** | Scaffold: site (lgtm-jekyll), outline, this plan, the `examples/_template/` C++ toolchain (lgtm-cpp: CMake presets + Conan + gtest/gmock + sanitizer lanes), `scripts/check-prereqs.sh`, hub card | in progress |
| **r01** | Part 0 + Part 1 — Ch0 Setup, Ch1 Why test / toolchain, Ch2 first test (CTest + `gtest_discover_tests`), Ch3 assertions vs expectations, Ch4 fixtures | planned |
| **r02** | Part 2 — Ch5 parameterized & typed, Ch6 death tests / exceptions / `std::expected`, Ch7 modern C++20/23 (concepts, ranges, coroutines) | planned |
| **r03** | Part 3 — Ch8 GoogleMock fundamentals, Ch9 advanced mocking & testability | planned |
| **r04** | Part 4 — Ch10 threaded code, Ch11 races & memory bugs (TSan/ASan/UBSan), Ch12 deterministic & stress testing / flaky-test hunting | planned |
| **r05** | Part 5 — Ch13 debugging with GDB, Ch14 coverage (llvm-cov), Ch15 CI & reproducible test runs (two-pass sanitizer matrix) | planned |
| **r06** | Polish — diagram audit, `manuscript/` + `build-manuscript.py` (Leanpub), idiomatic-review + professional-voice passes, flip verification footers to `verified` | planned |
| **r07** | Companion conference deck (60-minute cut), built last from finished chapters | planned |

## Decisions carried into every iteration

- **Local tools only, no containers, no VM** (user decision 2026-10-31). Chapter 0 is a local Fedora 44 toolchain install plus `check-prereqs.sh`, not a container.
- **GoogleTest via Conan**, pinned `gtest/1.18.0` (Abseil pinned transitively). `FetchContent` and Fedora `gtest-devel` are documented as alternatives in Ch1/Ch2, never the build path. (Resolves the PRD's open `<DECIDE>`.)
- **Clang primary, GCC cross-check**; CMake presets + CTest; `-Wall -Wextra -Wpedantic -Werror`.
- **ASan+UBSan in one preset, TSan in its own** — they cannot share a binary, so CI runs two passes.
- **Behavioral verification, not build success.** Every example README proves the effect, including showing a test fail when the code under test is wrong.
- **Dual license**: Apache-2.0 for code, CC BY 4.0 for prose.
