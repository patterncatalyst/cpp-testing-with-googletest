# 07-failure-paths: expected, exceptions, and death tests

The example for Chapter 7. It shows the three ways a function reports failure and
how each is tested:

- `shipping::parse_weight` returns `std::expected<int, ParseError>`: a recoverable
  rejection is a value, tested with `has_value()`, `*result`, and `error()`.
- `shipping::parse_weight_or_throw` throws `std::invalid_argument` with a message,
  tested by catching it and inspecting `what()`.
- `shipping::remaining_capacity` guards an invariant with `assert`, tested with
  `EXPECT_DEATH` that the process aborts when the invariant is broken.

## Layout

```
07-failure-paths/
├── CMakeLists.txt        # builds the shipping lib + unit_tests target
├── demo.sh               # ./demo.sh [test|build|asan|tsan]
├── src/shipping/         # intake.hpp / intake.cpp
└── tests/                # intake_test.cpp: expected, EXPECT_THROW, EXPECT_DEATH
```

## Run it

```
[host]$ ./demo.sh test     # default lane: configure, build, ctest
[host]$ ./demo.sh asan     # AddressSanitizer + UndefinedBehaviorSanitizer
[host]$ ./demo.sh tsan     # ThreadSanitizer
```

## What it verifies

`parse_weight` returns the grams for valid text and the right `ParseError` for
empty, non-numeric, and non-positive text. `parse_weight_or_throw` returns the
grams for valid text and throws with a specific message otherwise. `remaining_capacity`
returns the difference when valid and aborts when `used` exceeds `total`. The
death-test suite is named with a `DeathTest` suffix so GoogleTest runs it first.

## Verification

<div class="status status--verified">
  <strong>Verified</strong> on Fedora 44 (Clang 22.1.8, CMake 4.3.0, Conan 2.33.0,
  GoogleTest 1.18.0): <code>./demo.sh test</code>, <code>asan</code>, and
  <code>tsan</code> each report 8/8 passing from a clean build, death test included.
</div>
