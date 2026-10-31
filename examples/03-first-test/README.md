# 03-first-test — your first GoogleTest suite

The example for Chapter 3. A single small function, `shipping::billable_weight_grams`,
and the first test file written against it from scratch. It is the `_template`
scaffold with one library file and one test file filled in, so the whole project
is the build wiring plus the code that matters.

## Layout

```
03-first-test/
├── conanfile.py          # pins gtest/1.18.0
├── CMakeLists.txt        # builds the shipping lib + unit_tests target
├── CMakePresets.json     # default · asan-ubsan · tsan · release
├── demo.sh               # ./demo.sh [test|build|asan|tsan]
├── src/shipping/         # parcel.hpp / parcel.cpp — the code under test
└── tests/                # parcel_test.cpp — the three TEST cases
```

## Run it

```
[host]$ ./demo.sh test     # default lane: configure, build, ctest
[host]$ ./demo.sh asan     # AddressSanitizer + UndefinedBehaviorSanitizer
[host]$ ./demo.sh tsan     # ThreadSanitizer (its own binary)
```

List what CTest will run, straight from the binary:

```
[host]$ ./build/unit_tests --gtest_list_tests
```

## What it verifies

`billable_weight_grams` returns the greater of a parcel's actual (scale) weight
and its volumetric weight, where volumetric grams is the box volume in cubic
centimetres divided by 5. The three cases cover the two branches and the tie:
a light bulky box bills volumetric, a small heavy box bills actual, and an equal
case returns that shared value.

## Verification

<div class="status status--verified">
  <strong>Verified</strong> on Fedora 44 (Clang 22.1.8, CMake 4.3.0, Conan 2.33.0,
  GoogleTest 1.18.0): <code>./demo.sh test</code>, <code>asan</code>, and
  <code>tsan</code> each report 3/3 passing from a clean build.
</div>
