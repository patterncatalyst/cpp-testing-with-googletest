# 11-testing-threads: driving a thread-safe unit from a test

The example for Chapter 11. `shipping::ShipmentLedger` is a thread-safe accumulator:
many threads call `record` at once, and a mutex keeps `count` and `total_grams`
consistent. The tests drive it from several threads and assert the aggregate the
concurrency must preserve.

- A `std::latch` releases all worker threads together, so their work overlaps and a
  racy implementation has the best chance to misbehave.
- `std::jthread` auto-joins when the worker vector goes out of scope.
- `std::async` returns a value through a `std::future`; `get()` is the point where
  the test waits for the task before reading the ledger.

## Layout

```
11-testing-threads/
├── CMakeLists.txt        # builds the shipping lib + unit_tests; links Threads::Threads
├── demo.sh               # ./demo.sh [test|build|asan|tsan]
├── src/shipping/         # ledger.hpp / ledger.cpp: a mutex-protected accumulator
└── tests/                # ledger_test.cpp: latch, jthread, async/future
```

## Run it

```
[host]$ ./demo.sh test     # default lane: configure, build, ctest
[host]$ ./demo.sh asan     # AddressSanitizer + UndefinedBehaviorSanitizer
[host]$ ./demo.sh tsan     # ThreadSanitizer: the important lane for threaded code
```

## What it verifies

The ledger counts and sums correctly on one thread; eight threads recording a
thousand parcels each produce exactly the expected count and total; and two
`std::async` workers return parcel counts that match what the ledger accumulated.
The chapter removes the lock to show the count coming out short and varying from
run to run, and ThreadSanitizer reporting the data race.

## Verification

<div class="status status--verified">
  <strong>Verified</strong> on Fedora 44 (Clang 22.1.8, CMake 4.3.0, Conan 2.33.0,
  GoogleTest 1.18.0): <code>./demo.sh test</code>, <code>asan</code>, and
  <code>tsan</code> each report 3/3 passing from a clean build.
</div>
