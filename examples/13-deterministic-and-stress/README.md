# 13-deterministic-and-stress: deterministic tests and stress runs

The example for Chapter 13. Two units are built so a test controls the source of
nondeterminism instead of fighting it:

- `QuoteCache` decides freshness against a caller-supplied time, so a test picks the
  moment rather than sleeping for the TTL to pass.
- `choose_backup_carrier` takes the random engine as a parameter, so a test seeds it
  and gets a reproducible choice.

The suite is then run under `--gtest_repeat` and `--gtest_shuffle` to show it stays
stable across many runs and any order, which is how you flush out a hidden
dependence on run order or timing.

## Layout

```
13-deterministic-and-stress/
├── CMakeLists.txt        # builds the shipping lib + unit_tests target
├── demo.sh               # ./demo.sh [test|build|asan|tsan]
├── src/shipping/         # planner.hpp / planner.cpp: QuoteCache + choose_backup_carrier
└── tests/                # planner_test.cpp: injected time, injected seed
```

## Run it

```
[host]$ ./demo.sh test                              # default lane
[host]$ ./build/unit_tests --gtest_repeat=500 --gtest_shuffle   # stress run
[host]$ ./demo.sh asan                              # ASan + UBSan
[host]$ ./demo.sh tsan                              # ThreadSanitizer
```

## What it verifies

`QuoteCache` returns the quote inside its TTL and `nullopt` at or past it, with time
supplied as a parameter. `choose_backup_carrier` is reproducible for a fixed seed,
picks the expected candidate for seed 12345, and always returns one of the
candidates. The chapter reseeds the exact-pick test from `std::random_device` to
show it fail on most runs, which is the flakiness a fixed seed removes.

## Verification

<div class="status status--verified">
  <strong>Verified</strong> on Fedora 44 (Clang 22.1.8, CMake 4.3.0, Conan 2.33.0,
  GoogleTest 1.18.0): <code>./demo.sh test</code>, <code>asan</code>, and
  <code>tsan</code> each report 6/6 passing from a clean build, and
  <code>--gtest_repeat=500 --gtest_shuffle</code> stays green.
</div>
