# 09-mocking-fundamentals — a mock stands in for a collaborator

The example for Chapter 9. `shipping::Dispatcher` orchestrates a booking through a
`shipping::CarrierGateway`: it checks the region is served, gets a quote, and books
only if the quote fits the budget. The tests replace the real gateway with a
GoogleMock double so `Dispatcher` can be tested with no network.

- `MockCarrierGateway` declares one `MOCK_METHOD` per virtual on the interface.
- `EXPECT_CALL` sets expectations: argument matchers (`StrEq`, `Ge`, `_`), a
  cardinality (`Times(0)`, the default `WillOnce`), and a return action (`Return`).
- `ON_CALL` + `WillByDefault` stubs background interactions without asserting they
  happen, so one `EXPECT_CALL` can focus the test on the interaction that matters.

## Layout

```
09-mocking-fundamentals/
├── CMakeLists.txt        # builds the shipping lib + unit_tests target (links GTest::gmock)
├── demo.sh               # ./demo.sh [test|build|asan|tsan]
├── src/shipping/         # carrier.hpp (interface), dispatcher.hpp/.cpp (unit under test)
└── tests/                # dispatcher_test.cpp — MOCK_METHOD, EXPECT_CALL, matchers, ON_CALL
```

## Run it

```
[host]$ ./demo.sh test     # default lane: configure, build, ctest
[host]$ ./demo.sh asan     # AddressSanitizer + UndefinedBehaviorSanitizer
[host]$ ./demo.sh tsan     # ThreadSanitizer
```

## What it verifies

`Dispatcher` books and returns the reference when the region is served and the
quote is within budget; it books nothing when the region is unserved (never even
quoting) and when the quote exceeds the budget. The matcher test confirms the
weight passes through to the quote. The `ON_CALL` test shows default stubs for the
background calls with a single `EXPECT_CALL` on the booking. The chapter also
breaks the budget check to show the `Times(0)` expectation on `book` catching it.

## Verification

<div class="status status--verified">
  <strong>Verified</strong> on Fedora 44 (Clang 22.1.8, CMake 4.3.0, Conan 2.33.0,
  GoogleTest 1.18.0): <code>./demo.sh test</code>, <code>asan</code>, and
  <code>tsan</code> each report 5/5 passing from a clean build.
</div>
