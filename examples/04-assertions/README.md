# 04-assertions — ASSERT vs EXPECT

The example for Chapter 4. `shipping::split_into_shipments` divides a total weight
into shipments no heavier than a per-shipment maximum, returning a
`std::vector<int>`. A function that returns a collection is the natural place to
show the difference between a fatal assertion and a non-fatal one: assert the
size, then expect each element.

## Layout

```
04-assertions/
├── conanfile.py          # pins gtest/1.18.0
├── CMakeLists.txt        # builds the shipping lib + unit_tests target
├── demo.sh               # ./demo.sh [test|build|asan|tsan]
├── src/shipping/         # consolidation.hpp / consolidation.cpp
└── tests/                # consolidation_test.cpp — ASSERT_EQ, EXPECT_EQ, EXPECT_TRUE, EXPECT_THROW
```

## Run it

```
[host]$ ./demo.sh test     # default lane: configure, build, ctest
[host]$ ./demo.sh asan     # AddressSanitizer + UndefinedBehaviorSanitizer
[host]$ ./demo.sh tsan     # ThreadSanitizer
```

## What it verifies

`split_into_shipments(total, max)` fills each shipment to `max` except the last,
which carries the remainder; a zero total yields no shipments, and a non-positive
`max` throws. Each test `ASSERT_EQ`s the vector size before indexing into it, so a
wrong size stops that test instead of reading out of bounds.

## Verification

<div class="status status--verified">
  <strong>Verified</strong> on Fedora 44 (Clang 22.1.8, CMake 4.3.0, Conan 2.33.0,
  GoogleTest 1.18.0): <code>./demo.sh test</code>, <code>asan</code>, and
  <code>tsan</code> each report 5/5 passing from a clean build.
</div>
