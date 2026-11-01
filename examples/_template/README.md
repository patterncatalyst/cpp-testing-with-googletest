# _template: the example scaffold

Every runnable example in this book is a copy of this directory with the code
under test and its tests filled in. It is a minimal, working GoogleTest project:
a small library (`src/shipping/rate_quote.cpp`) and its tests
(`tests/rate_quote_test.cpp`), built with CMake presets over a Conan-provided
GoogleTest, with warnings-as-errors, clang-tidy on every compile, and the two
sanitizer lanes wired in.

## Layout

```
_template/
├── conanfile.py          # pins gtest/1.18.0 (Abseil pinned transitively)
├── CMakeLists.txt        # C++23, -Werror, clang-tidy, sanitizer options, test target
├── CMakePresets.json     # default · asan-ubsan · tsan · release (each its own build dir)
├── .clang-format         # house style
├── .clang-tidy           # WarningsAsErrors: '*'; a lint finding fails the build
├── demo.sh               # build and run: ./demo.sh [test|build|asan|tsan]
├── src/shipping/         # the library under test
└── tests/                # the GoogleTest cases
```

## Run it

```
[host]$ ./demo.sh test     # default lane: configure, build, ctest
[host]$ ./demo.sh asan     # AddressSanitizer + UndefinedBehaviorSanitizer
[host]$ ./demo.sh tsan     # ThreadSanitizer (its own binary; never shares with ASan)
```

Each lane runs `conan install` (building GoogleTest the first time), configures the
matching preset, builds, and runs CTest. ASan/UBSan and TSan build into separate
directories (`build-asan`, `build-tsan`) because they cannot share a binary.

## The demo contract (identical across every example)

- `./demo.sh` with no argument runs the default lane (`test`).
- `build` configures and builds without running tests.
- `asan` / `tsan` build and run the suite under that sanitizer.
- Exit status is the test result: 0 only when every test passed.

## Verification

Behavioral, not build-only: the suite asserts the rate-quote math and that a
non-positive weight is rejected, and a chapter's example also shows a test failing
when the code under test is wrong.

<div class="status status--verified">
  <strong>Verified</strong> on Fedora 44 (Clang 22.1.8, CMake 4.3.0, Conan 2.33.0,
  GoogleTest 1.18.0): <code>./demo.sh test</code>, <code>asan</code>, and
  <code>tsan</code> each report 4/4 passing from a clean build.
</div>
