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
| **r01** | Part 0 + Part 1 — Ch1 Setup, Ch2 Why test / toolchain, Ch3 first test (CTest + `gtest_discover_tests`), Ch4 assertions vs expectations, Ch5 fixtures | merged (PR #2, tag r01.0) |
| **r02** | Part 2 — Ch6 parameterized & typed, Ch7 death tests / exceptions / `std::expected`, Ch8 modern C++20/23 (concepts, ranges, coroutines) | in progress |
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

## r02 progress (branch `release/r02`)

- **Ch6 Parameterized and typed tests** (`_docs/06-parameterized-and-typed-tests.md`) authored + Figure 6.1 (`06-parameterized-and-typed`), footer `verified`. New example `examples/06-parameterized-and-typed`: `TEST_P` over a `delivery_band` boundary table (5 rows, struct param + name generator -> `/w1`../w30001`) and `TYPED_TEST` of `clamp_non_negative<T>` over `Types<int,long,double>`; 11/11 on all 3 lanes. Covers `TestWithParam`/`GetParam`, `INSTANTIATE_TEST_SUITE_P` + generators (Values/ValuesIn/Range/Bool/Combine), typed fixtures + `this->`, type-parameterized (`TYPED_TEST_P`) as advanced, when-not-to.
- **Ch7 Death tests, exceptions, and `std::expected`** (`_docs/07-death-tests-exceptions-and-expected.md`) authored + TWO figures (`07-failure-styles`, `07-death-test-fork`), footer `verified`. New example `examples/07-failure-paths`: `parse_weight` -> `std::expected`, `parse_weight_or_throw` (message checked via try/catch + `what()`), `remaining_capacity` with `assert` tested by `EXPECT_DEATH`; 8/8 all lanes incl. the death test (abort-based, single-threaded, so no sanitizer gotcha). ~2230 prose words.
- **clang-tidy: disabled the whole `cppcoreguidelines-pro-bounds-*` profile** (opt-in; fights `operator[]` and `from_chars` pointer pairs) + `bugprone-suspicious-stringview-data-usage` (false-positive on two-pointer from_chars). Replaced the two individual pro-bounds disables. Re-synced `.clang-tidy` across examples 03-07. **Feed back to lgtm-cpp.**
- **AI sidebar convention adopted (user request 2026-11-01):** one `> **AI sidebar.**` blockquote per chapter before "Build, run, observe" (the design-patterns-books convention), framed around how an LLM tends to write TESTS for the topic and what to check. Ch7 has one; retrofitting Ch1-6.
- **WORD TARGET (user restated):** 2k prose floor, 2.5k soft upper, closer to 2.5k with more diagrams; value not padding. Code-heavy chapters earn the words with real content (design rationale, gotchas, a 2nd diagram).
- **Ch8 Testing modern C++20 and C++23** (`_docs/08-testing-modern-cpp.md`) authored + TWO figures (`08-modern-features-testing`, `08-lazy-generator-pull`), footer `verified`. New example `examples/08-modern-cpp`: a `Weighable` concept + constrained `heavier_weight` (tested with positive/negative `static_assert`, a `constexpr` bool `TEST`, and a structural `Crate`); `heavy_parcel_weight` as a lazy `views::filter | views::transform` pipeline folded with `std::ranges::fold_left` (inclusive-threshold + empty-fold edges); `batch_weights` + infinite `running_total` `std::generator` coroutines (collected via loop / `std::ranges::to`, infinite one bounded with `std::views::take`). 10/10 on all 3 lanes; `static_assert`s are compile-time so they never reach CTest. See-it-fail captured both ways: the concept constraint-not-satisfied compile error (quoted) and a `>`-vs-`>=` boundary break (reverted). ~2150 true-prose words. Examples index updated.
- **r02 COMPLETE & MERGED** (PR #3, squash, main@910afed, tag `r02.0`, release published, Pages deployed, branch deleted).

## r03 progress (branch `release/r03`)

- **Ch9 GoogleMock fundamentals** (`_docs/09-mocking-fundamentals.md`) authored + TWO figures (`09-mock-seam`, `09-expect-call-anatomy`), footer `verified`. New example `examples/09-mocking-fundamentals`: `CarrierGateway` abstract interface (the seam), `Dispatcher` unit under test (serves -> quote -> book with budget guard), `MockCarrierGateway` (3x `MOCK_METHOD`). 5 TESTs: happy path, unserved short-circuit (`Times(0)` on quote+book), over-budget (`Times(0)` on book), matcher pass-through (`Ge(0)`/`_`), `ON_CALL`+`WillByDefault` background stubs. 5/5 all 3 lanes. Covers `MOCK_METHOD` qualifier specs + `override`, `EXPECT_CALL` anatomy (mock/method+matchers/cardinality/action), matchers (`StrEq`/`Ge`/`_`), cardinalities (`Times(0)` emphasis), `ON_CALL` vs `EXPECT_CALL`, naggy-mock uninteresting-call warning (sets up Ch10 NiceMock). See-it-fail: flipped budget comparison -> `book` `Times(0)` fails ("over-saturated"), reverted. ~2120 true-prose words. Examples index updated.
- **clang-tidy: added `-bugprone-unchecked-optional-access` to the TEST-scoped `tests/.clang-tidy`** (alongside the fixtures `-non-private-member-variables` disable). Reason: `ASSERT_TRUE(opt.has_value())` then `*opt` is the guarded-access idiom the book teaches, but clang-tidy can't model gtest ASSERT's return-on-failure. Still enforced in `src/` (dispatcher never derefs an optional). **Feed back to lgtm-cpp.**
- **Ch10 Advanced mocking and testability** (`_docs/10-advanced-mocking-and-testability.md`) authored + TWO figures (`10-mock-strictness`, `10-mock-vs-fake`), footer `verified`. New example `examples/10-advanced-mocking`: `BookingService` depends on `CarrierGateway` (reused from 09) + a new `AuditLog` interface, both constructor-injected by reference; books then records in order. 5 TESTs: `InSequence` ordering (book before record), `NiceMock` (silences background audit), `StrictMock` (pins exact carrier calls, replaces a row of `Times(0)`), `SaveArg<0>` capture of the audited message, and a `FakeAuditLog` (working in-memory impl, inspected after). 5/5 all 3 lanes. Covers DI seams (ctor/method/template), Nice/Strict strictness spectrum, `InSequence`/`Sequence`/`After`, the action library (`Return`/`SaveArg`/`Invoke`/`DoAll`/`WillRepeatedly`/`Throw`), faking vs mocking. See-it-fail: removed the unserved early return -> StrictMock fails on unexpected `quote_cents`, reverted. ~2080 true-prose words. Examples index updated.
- **clang-tidy:** `const`-qualified local returned by value trips `performance-no-automatic-move` under `-Werror`; drop the `const` on a to-be-moved return. (Not a config change; a coding note.)
- **r03 COMPLETE (Ch9-10 + examples 09/10 all verified).** Next: open PR `release/r03` -> main (lgtm-github: squash, delete branch, tag `r03.0`, notes-only release).

## Decisions carried into every iteration

- **Local tools only, no containers, no VM** (user decision 2026-10-31). Chapter 0 is a local Fedora 44 toolchain install plus `check-prereqs.sh`, not a container.
- **GoogleTest via Conan**, pinned `gtest/1.18.0` (Abseil pinned transitively). `FetchContent` and Fedora `gtest-devel` are documented as alternatives in Ch1/Ch2, never the build path. (Resolves the PRD's open `<DECIDE>`.)
- **Clang primary, GCC cross-check**; CMake presets + CTest; `-Wall -Wextra -Wpedantic -Werror`.
- **ASan+UBSan in one preset, TSan in its own** — they cannot share a binary, so CI runs two passes.
- **Behavioral verification, not build success.** Every example README proves the effect, including showing a test fail when the code under test is wrong.
- **Dual license**: Apache-2.0 for code, CC BY 4.0 for prose.
