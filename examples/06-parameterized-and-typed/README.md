# 06-parameterized-and-typed: one body, many inputs and many types

The example for Chapter 6. It shows the two GoogleTest tools for running one test
body many times: `TEST_P` over a list of inputs, and `TYPED_TEST` over a list of
types.

- `shipping::delivery_band(int)` classifies a parcel as Letter, Parcel, or Freight
  by weight. A `TEST_P` checks it against a table of boundary rows.
- `shipping::clamp_non_negative<T>(T)` raises a negative measurement to zero for
  any arithmetic type. A `TYPED_TEST` checks it for `int`, `long`, and `double`.

## Layout

```
06-parameterized-and-typed/
├── CMakeLists.txt        # builds the shipping lib + unit_tests target
├── demo.sh               # ./demo.sh [test|build|asan|tsan]
├── src/shipping/         # delivery_band.hpp/.cpp (TEST_P subject)
│                         # clamp.hpp (header-only template, TYPED_TEST subject)
└── tests/                # measures_test.cpp: TEST_P + TYPED_TEST
```

## Run it

```
[host]$ ./demo.sh test     # default lane: configure, build, ctest
[host]$ ./demo.sh asan     # AddressSanitizer + UndefinedBehaviorSanitizer
[host]$ ./demo.sh tsan     # ThreadSanitizer
```

List the generated cases, which show one row or type each:

```
[host]$ ./build/unit_tests --gtest_list_tests
```

## What it verifies

The `TEST_P` table covers the band boundaries (500 and 30000 grams and the first
gram past each). The `TYPED_TEST` checks that `clamp_non_negative` leaves a
non-negative value alone and raises a negative one to zero, for each of the three
types. Eleven cases in all: five parameterized plus three types times two typed
bodies.

## Verification

<div class="status status--verified">
  <strong>Verified</strong> on Fedora 44 (Clang 22.1.8, CMake 4.3.0, Conan 2.33.0,
  GoogleTest 1.18.0): <code>./demo.sh test</code>, <code>asan</code>, and
  <code>tsan</code> each report 11/11 passing from a clean build.
</div>
