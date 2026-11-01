# 12-sanitizers: reading what ASan, UBSan, and TSan report

The example for Chapter 12. Three small, correct functions over a batch of parcel
weights, each written to stay inside the rules a sanitizer enforces:

- `total_weight` sums into a 64-bit accumulator, so a large batch cannot overflow.
- `max_weight` scans a non-empty batch within bounds.
- `parallel_total` sums two halves on two threads, each with its own partial, so
  there is no shared state to race on.

The chapter breaks each one in turn to show the matching sanitizer catching it:
UBSan on a 32-bit overflow, ASan on a dangling view (heap-use-after-free), and TSan
on a shared unsynchronized accumulator.

## Layout

```
12-sanitizers/
├── CMakeLists.txt        # builds the shipping lib + unit_tests; links Threads::Threads
├── demo.sh               # ./demo.sh [test|build|asan|tsan]
├── src/shipping/         # weights.hpp / weights.cpp: total, max, and parallel sum
└── tests/                # weights_test.cpp: the correct behavior on all lanes
```

## Run it

```
[host]$ ./demo.sh test     # default lane: configure, build, ctest
[host]$ ./demo.sh asan     # AddressSanitizer + UndefinedBehaviorSanitizer
[host]$ ./demo.sh tsan     # ThreadSanitizer
```

ASan/UBSan share one binary (`build-asan`); TSan has its own (`build-tsan`). The
two cannot be combined, so the suite is run in two passes.

## What it verifies

`total_weight` sums a batch and handles one whose 32-bit sum would overflow;
`max_weight` finds the largest, including a single-element batch; `parallel_total`
matches the sequential total. All five tests pass clean on the default, ASan/UBSan,
and TSan lanes, because the code is written to be correct on all three.

## Verification

<div class="status status--verified">
  <strong>Verified</strong> on Fedora 44 (Clang 22.1.8, CMake 4.3.0, Conan 2.33.0,
  GoogleTest 1.18.0): <code>./demo.sh test</code>, <code>asan</code>, and
  <code>tsan</code> each report 5/5 passing from a clean build.
</div>
