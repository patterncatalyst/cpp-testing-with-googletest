# 15-coverage: measuring coverage with llvm-cov

The example for Chapter 15. `shipping::late_fee_cents` has four branches (not late, a
member grace, a per-day fee, and a cap), so there is something for coverage to
measure. `coverage.sh` builds an instrumented binary, runs the suite, and prints a
line, region, and branch report with llvm-cov.

## Layout

```
15-coverage/
├── CMakeLists.txt        # adds an ENABLE_COVERAGE option alongside the sanitizer lanes
├── demo.sh               # ./demo.sh [test|build|asan|tsan]
├── coverage.sh           # build instrumented, run, and report with llvm-cov
├── src/shipping/         # late_fee.hpp / late_fee.cpp: a four-branch fee
└── tests/                # late_fee_test.cpp: one case per branch
```

## Run it

```
[host]$ ./demo.sh test                                   # default lane
[host]$ ./coverage.sh                                     # full suite: 100%
[host]$ ./coverage.sh --gtest_filter=-LateFee.ChargesUpToACap   # drops the cap branch
```

`coverage.sh` builds into `build-cov/` with `-fprofile-instr-generate
-fcoverage-mapping` (clang only), runs the tests to produce a `.profraw`, merges it
with `llvm-profdata`, and reports with `llvm-cov`. Passing a `--gtest_filter` shows
coverage for a subset, which is how the chapter demonstrates an uncovered branch.

## What it verifies

`late_fee_cents` returns zero when not late and for a member within the grace,
charges 100 cents per day past the grace, and caps the fee at 2000. The four tests
cover all four branches, so the full suite reports 100% region, line, and branch
coverage; dropping the cap test drops branch coverage to 87.5%, which the chapter
uses to show llvm-cov pointing at the untested branch.

## Verification

<div class="status status--verified">
  <strong>Verified</strong> on Fedora 44 (Clang 22.1.8, CMake 4.3.0, Conan 2.33.0,
  GoogleTest 1.18.0, LLVM 22.1.8): <code>./demo.sh test</code>, <code>asan</code>, and
  <code>tsan</code> each report 4/4 passing, and <code>./coverage.sh</code> reports
  100% coverage for the full suite.
</div>
