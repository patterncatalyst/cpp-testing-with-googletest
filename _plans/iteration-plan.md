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
| **r00** | Scaffold: site (lgtm-jekyll), outline, this plan, the `examples/_template/` C++ toolchain (lgtm-cpp: CMake presets + Conan + gtest/gmock + sanitizer lanes), `scripts/check-prereqs.sh`, hub card | merged (PR #1 + hub card PR #45) |
| **r01** | Part 0 + Part 1 — Ch1 Setup, Ch2 Why test / toolchain, Ch3 first test (CTest + `gtest_discover_tests`), Ch4 assertions vs expectations, Ch5 fixtures | in progress |
| **r02** | Part 2 — Ch6 parameterized & typed, Ch7 death tests / exceptions / `std::expected`, Ch8 modern C++20/23 (concepts, ranges, coroutines) | planned |
| **r03** | Part 3 — Ch9 GoogleMock fundamentals, Ch10 advanced mocking & testability | planned |
| **r04** | Part 4 — Ch11 threaded code, Ch12 races & memory bugs (TSan/ASan/UBSan), Ch13 deterministic & stress testing / flaky-test hunting | planned |
| **r05** | Part 5 — Ch14 debugging with GDB, Ch15 coverage (llvm-cov), Ch16 CI & reproducible test runs (two-pass sanitizer matrix) | planned |
| **r06** | Polish — diagram audit, `manuscript/` + `build-manuscript.py` (Leanpub), idiomatic-review + professional-voice passes, flip verification footers to `verified` | planned |
| **r07** | Companion conference deck (60-minute cut), built last from finished chapters | planned |

## r01 progress

- `examples/_template/` verified on Fedora 44: `./demo.sh test`, `asan`, `tsan` all 4/4 from clean. (Clang 22.1.8, CMake 4.3.0, Ninja 1.13.2, Conan 2.33.0, GoogleTest 1.18.0.)
- Outline renumbered to the 1-based scheme (outline order 0 = Overview; content chapters = Chapter 1-16; Setup = Chapter 1).
- **Ch1 Setup** (`_docs/01-setup.md`) authored + Figure 1.1 (`assets/diagrams/01-build-and-test-loop`), footer `verified` (check-prereqs output + `_template` 4/4 captured here). No `examples/01-*` dir (setup points at `examples/_template`, mirroring the sibling prereqs chapter).
- Fixed a rule-14 leak: `_layouts/part_index.html` empty-state linked `/plans/iteration-plan/` ("roadmap"); now links the published chapters index.
- **Ch2 Why test / toolchain** (`_docs/02-why-test-and-the-toolchain.md`) authored + Figure 2.1 (`02-test-target-wiring`), footer `verified`. Walks the `_template` wiring (conanfile pin + generators, strict-flags INTERFACE libs, the `unit_tests` target, `gtest_discover_tests`); demonstrates a caught regression (break round-up -> 2/4 red -> revert) and a `--gtest_list_tests` cross-check, both run here and quoted path-relative (host paths scrubbed). No `examples/02-*` dir (dissects `examples/_template`).
- **Ch3 Your first test** (`_docs/03-your-first-test.md`) authored + Figure 3.1 (`03-billable-weight-branches`), footer `verified`. First fresh example `examples/03-first-test` (new `parcel` billable-weight module, `Dimensions` struct to dodge bugprone-easily-swappable-parameters, 3 TESTs), built + verified 3/3 on default/asan/tsan here. Covers TEST anatomy, choosing branch-covering cases, presets/CTest, `gtest_discover_tests`, running a subset (`-R`/`--gtest_filter`), see-it-fail (real output), `--gtest_list_tests` vs `ctest -N` cross-check. Examples index (`_example_pages/index.md`) updated + its rule-14 "next iteration" placeholder removed.
- **Ch4 Assertions and expectations** (`_docs/04-assertions-and-expectations.md`) authored + Figure 4.1 (`04-assert-vs-expect`), footer `verified`. New example `examples/04-assertions` (`split_into_shipments` -> vector; 5 TESTs: ASSERT-size-then-EXPECT-elements, whole-vector `EXPECT_EQ`, `EXPECT_TRUE`, `EXPECT_THROW`), verified 5/5 on all 3 lanes. Covers fatal vs non-fatal, the macro table, STREQ/FLOAT_EQ traps, custom `<<` messages, suites. ASSERT-abort shown with real output.
- **clang-tidy config change (applies to `_template` + all examples):** disabled `bugprone-easily-swappable-parameters` (fires on every 2-arg numeric fn in a teaching book) and `cppcoreguidelines-pro-bounds-avoid-unchecked-container-access` (flags the ASSERT-guarded `[]` this book teaches). Documented in `.clang-tidy` header. **Feed back to lgtm-cpp.**
- **Ch5 Fixtures** (`_docs/05-fixtures.md`) authored + Figure 5.1 (`05-fixture-lifecycle`), footer `verified`. New example `examples/05-fixtures` (`Manifest` class; `ManifestTest` fixture with `SetUp` + a helper; 5 TEST_F: start-state, add, fresh-fixture isolation proof, helper, throw-leaves-unchanged), verified 5/5 on all 3 lanes. Covers TEST_F, per-test lifecycle, RAII vs TearDown, SetUpTestSuite, fixture inheritance, when to use a fixture. Added `tests/.clang-tidy` (InheritParentConfig) disabling `cppcoreguidelines-non-private-member-variables-in-classes` for test code only (fixtures need protected members); still enforced in `src/`. **Feed back to lgtm-cpp.**
- **r01 COMPLETE (Ch1-5 + examples 03/04/05 all verified).** Next: open PR `release/r01` -> main (lgtm-github: squash, delete branch, tag `r01.x`), then r02 (Part 2: Ch6-8).

## Decisions carried into every iteration

- **Local tools only, no containers, no VM** (user decision 2026-10-31). Chapter 0 is a local Fedora 44 toolchain install plus `check-prereqs.sh`, not a container.
- **GoogleTest via Conan**, pinned `gtest/1.18.0` (Abseil pinned transitively). `FetchContent` and Fedora `gtest-devel` are documented as alternatives in Ch1/Ch2, never the build path. (Resolves the PRD's open `<DECIDE>`.)
- **Clang primary, GCC cross-check**; CMake presets + CTest; `-Wall -Wextra -Wpedantic -Werror`.
- **ASan+UBSan in one preset, TSan in its own** — they cannot share a binary, so CI runs two passes.
- **Behavioral verification, not build success.** Every example README proves the effect, including showing a test fail when the code under test is wrong.
- **Dual license**: Apache-2.0 for code, CC BY 4.0 for prose.
