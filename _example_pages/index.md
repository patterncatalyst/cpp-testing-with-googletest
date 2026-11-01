---
title: "Examples"
layout: plan
permalink: /examples/
description: "Every runnable example in this repo, mapped to the chapter it supports."
---

Every hands-on chapter has a matching runnable example under
[`examples/`](https://github.com/{{ site.github_username }}/{{ site.github_repo }}/tree/main/examples)
in the repo. Each is a copy of `examples/_template/` (the CMake + Conan + GoogleTest
toolchain) with the chapter's test code filled in, a `demo.sh` that builds and runs
it, and a README that verifies the behavior.

## Shipped examples

| Chapter | Example | What it verifies |
|---|---|---|
| Chapter 3: Your first test | [`examples/03-first-test`](https://github.com/{{ site.github_username }}/{{ site.github_repo }}/tree/main/examples/03-first-test) | Billable weight is the greater of a parcel's volumetric and actual weight. |
| Chapter 4: Assertions and expectations | [`examples/04-assertions`](https://github.com/{{ site.github_username }}/{{ site.github_repo }}/tree/main/examples/04-assertions) | Splitting a weight into shipments, verified with `ASSERT`/`EXPECT`, container comparison, and exception checks. |
| Chapter 5: Fixtures | [`examples/05-fixtures`](https://github.com/{{ site.github_username }}/{{ site.github_repo }}/tree/main/examples/05-fixtures) | A `Manifest` tested through a `TEST_F` fixture with shared `SetUp`, per-test isolation, and a fixture helper. |
| Chapter 6: Parameterized and typed tests | [`examples/06-parameterized-and-typed`](https://github.com/{{ site.github_username }}/{{ site.github_repo }}/tree/main/examples/06-parameterized-and-typed) | `delivery_band` over a `TEST_P` boundary table and `clamp_non_negative<T>` over a `TYPED_TEST` type list. |
| Chapter 7: Death tests, exceptions, and `std::expected` | [`examples/07-failure-paths`](https://github.com/{{ site.github_username }}/{{ site.github_repo }}/tree/main/examples/07-failure-paths) | A parser returning `std::expected`, a throwing variant checked by its message, and an invariant verified with `EXPECT_DEATH`. |
| Chapter 8: Testing modern C++20 and C++23 | [`examples/08-modern-cpp`](https://github.com/{{ site.github_username }}/{{ site.github_repo }}/tree/main/examples/08-modern-cpp) | A `Weighable` concept checked with `static_assert`, a lazy ranges pipeline folded with `fold_left`, and `std::generator` coroutines including an infinite one tested with `take`. |
| Chapter 9: GoogleMock fundamentals | [`examples/09-mocking-fundamentals`](https://github.com/{{ site.github_username }}/{{ site.github_repo }}/tree/main/examples/09-mocking-fundamentals) | A `Dispatcher` tested against a `MockCarrierGateway`: `MOCK_METHOD`, `EXPECT_CALL` with matchers and cardinalities, `Times(0)`, and `ON_CALL` background stubs. |
| Chapter 10: Advanced mocking and testability | [`examples/10-advanced-mocking`](https://github.com/{{ site.github_username }}/{{ site.github_repo }}/tree/main/examples/10-advanced-mocking) | A `BookingService` with two injected collaborators: `InSequence` ordering, `NiceMock`/`StrictMock`, `SaveArg` capture, and a `FakeAuditLog`. |
| Chapter 11: Testing threaded code | [`examples/11-testing-threads`](https://github.com/{{ site.github_username }}/{{ site.github_repo }}/tree/main/examples/11-testing-threads) | A thread-safe `ShipmentLedger` driven from many threads with a `std::latch` and `std::jthread`, plus `std::async` workers, asserting the aggregate. |
| Chapter 12: Races and memory bugs | [`examples/12-sanitizers`](https://github.com/{{ site.github_username }}/{{ site.github_repo }}/tree/main/examples/12-sanitizers) | Three correct weight functions, each broken in the chapter to show UBSan (overflow), ASan (a dangling view), and TSan (a shared accumulator) catching it. |
| Chapter 13: Deterministic and stress testing | [`examples/13-deterministic-and-stress`](https://github.com/{{ site.github_username }}/{{ site.github_repo }}/tree/main/examples/13-deterministic-and-stress) | A `QuoteCache` with injected time and a `choose_backup_carrier` with an injected seed, then stressed with `--gtest_repeat` and `--gtest_shuffle`. |
| Chapter 14: Debugging a failing test with GDB | [`examples/14-debugging`](https://github.com/{{ site.github_username }}/{{ site.github_repo }}/tree/main/examples/14-debugging) | A tiered-rate function with a planted boundary bug, located under GDB with a conditional breakpoint, `print`, `next`, and `backtrace`. |
| Chapter 15: Measuring coverage with llvm-cov | [`examples/15-coverage`](https://github.com/{{ site.github_username }}/{{ site.github_repo }}/tree/main/examples/15-coverage) | A four-branch late-fee function and a `coverage.sh` that instruments, runs, and reports line, region, and branch coverage; dropping a test shows the uncovered branch. |

Chapters 1 and 2 build on the shared `examples/_template/` scaffold rather than a
chapter-specific example. More examples appear here as each chapter ships.
