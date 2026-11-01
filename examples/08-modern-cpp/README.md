# 08-modern-cpp: concepts, ranges, and coroutines under test

The example for Chapter 8. It shows how to test three C++20/23 features:

- A `Weighable` concept constrains `shipping::heavier_weight`. The concept is
  checked at compile time with `static_assert` and read as a `constexpr` bool in
  an ordinary `TEST`.
- `shipping::heavy_parcel_weight` is a lazy ranges pipeline (filter then project)
  folded with `std::ranges::fold_left`, tested by materializing the result.
- `shipping::batch_weights` and `shipping::running_total` are `std::generator`
  coroutines, tested by collecting what they yield; the infinite one proves
  laziness with `std::views::take`.

## Layout

```
08-modern-cpp/
├── CMakeLists.txt        # builds the shipping lib + unit_tests target
├── demo.sh               # ./demo.sh [test|build|asan|tsan]
├── src/shipping/         # parcel.hpp (concept), manifest.* (ranges), batching.* (coroutines)
└── tests/                # modern_test.cpp: static_assert, ranges, generator tests
```

## Run it

```
[host]$ ./demo.sh test     # default lane: configure, build, ctest
[host]$ ./demo.sh asan     # AddressSanitizer + UndefinedBehaviorSanitizer
[host]$ ./demo.sh tsan     # ThreadSanitizer
```

## What it verifies

`Weighable` accepts a type with an integer `weight_grams` and rejects one without;
`heavier_weight` returns the larger weight for any weighable type. `heavy_parcel_weight`
sums only parcels at or above an inclusive threshold and returns zero when none
qualify. `batch_weights` splits a total into full batches plus a remainder and
yields nothing for a zero total; `running_total` is infinite, and the test takes
only the first three values, which terminates only because the generator is lazy.

## Verification

<div class="status status--verified">
  <strong>Verified</strong> on Fedora 44 (Clang 22.1.8, CMake 4.3.0, Conan 2.33.0,
  GoogleTest 1.18.0): <code>./demo.sh test</code>, <code>asan</code>, and
  <code>tsan</code> each report 10/10 passing from a clean build.
</div>
