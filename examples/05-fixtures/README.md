# 05-fixtures: TEST_F, SetUp, and shared state

The example for Chapter 5. `shipping::Manifest` accumulates the parcels loaded for
one truck and reports their count and combined weight. Several tests want to start
from the same loaded manifest, which is what a fixture provides: `SetUp` prepares
the state once per test, and every `TEST_F` runs against its own fresh instance.

## Layout

```
05-fixtures/
├── conanfile.py          # pins gtest/1.18.0
├── CMakeLists.txt        # builds the shipping lib + unit_tests target
├── demo.sh               # ./demo.sh [test|build|asan|tsan]
├── src/shipping/         # manifest.hpp / manifest.cpp
└── tests/                # manifest_test.cpp (TEST_F + a fixture); tests/.clang-tidy
```

## Run it

```
[host]$ ./demo.sh test     # default lane: configure, build, ctest
[host]$ ./demo.sh asan     # AddressSanitizer + UndefinedBehaviorSanitizer
[host]$ ./demo.sh tsan     # ThreadSanitizer
```

## What it verifies

The `ManifestTest` fixture adds two parcels in `SetUp`. The cases check the
starting state, that adding a parcel updates the count and total, that each test
starts from a fresh fixture rather than inheriting the previous one, that a
fixture helper keeps repetitive setup out of a case, and that a non-positive
weight throws and leaves the manifest unchanged.

## Verification

<div class="status status--verified">
  <strong>Verified</strong> on Fedora 44 (Clang 22.1.8, CMake 4.3.0, Conan 2.33.0,
  GoogleTest 1.18.0): <code>./demo.sh test</code>, <code>asan</code>, and
  <code>tsan</code> each report 5/5 passing from a clean build.
</div>
