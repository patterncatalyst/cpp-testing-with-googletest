---
title: "Outline"
order: 0
part: "Prerequisites"
description: "What this book covers and how it is organized: from an empty directory to a mockable, sanitized, CI-run C++ test suite."
---

This book takes a working C++ engineer from an empty directory to a test suite
that mocks its dependencies, runs clean under sanitizers, and executes in CI. It
assumes you can build a C++ project; it does not assume you have ever owned its
tests. Everything runs locally on Fedora 44 with Clang, CMake, Conan, and
GoogleTest. There are no containers and no VM.

Every hands-on chapter ships one runnable example under `examples/NN-name/` with
a `demo.sh` that builds, runs, and shows the result, and a README that verifies
the behavior rather than the exit code, including showing a test failing when the
code under test is wrong.

## The arc

| Part | Theme | Chapters |
|---|---|---|
| **0 — Prerequisites** | a working toolchain before any test | 1 |
| **1 — Foundations** | first test, CTest, assertions, fixtures | 2–5 |
| **2 — Beyond the basics** | parameterized, typed, death tests, modern C++ | 6–8 |
| **3 — Mocking** | GoogleMock and designing for testability | 9–10 |
| **4 — Testing concurrency** | threads, sanitizers, flaky-test hunting | 11–13 |
| **5 — Tooling and workflow** | GDB, coverage, CI | 14–16 |

## Chapters

**Part 0 — Prerequisites**

- **1. Setup** — Fedora 44, Clang, CMake, Conan, Ninja, GDB; `check-prereqs.sh`; a dated known-good version matrix; teardown.

**Part 1 — Foundations**

- **2. Why test C++, and the toolchain** — what a test buys you, and the CMake + Conan + Clang toolchain this book builds on.
- **3. Your first test** — CMake presets, CTest, and `gtest_discover_tests`.
- **4. Assertions and expectations** — `ASSERT_*` versus `EXPECT_*`, and organizing suites.
- **5. Fixtures** — `TEST_F`, setup and teardown, and shared state done safely.

**Part 2 — Beyond the basics**

- **6. Parameterized and typed tests** — one test body over many inputs and many types.
- **7. Death tests, exceptions, and `std::expected`** — asserting on failure paths.
- **8. Testing modern C++20/23** — concepts, ranges, and coroutines under test.

**Part 3 — Mocking**

- **9. GoogleMock fundamentals** — `MOCK_METHOD`, `EXPECT_CALL`, and the seams that make a class mockable.
- **10. Advanced mocking** — matchers, actions, sequences, and designing for testability.

**Part 4 — Testing concurrency**

- **11. Testing threaded code** — `std::jthread`, latches, barriers, and futures.
- **12. Finding races and memory bugs** — a real data race caught by ThreadSanitizer, plus ASan and UBSan.
- **13. Deterministic and stress testing** — hunting flaky tests with `--gtest_repeat` and `--gtest_shuffle`.

**Part 5 — Tooling and workflow**

- **14. Debugging failing tests with GDB** — breakpoints on failure and `--gtest_break_on_failure`.
- **15. Code coverage with llvm-cov** — measuring what the suite actually exercises.
- **16. CI and reproducible test runs** — a GitHub Actions matrix that runs the suite twice: ASan/UBSan, then TSan.
