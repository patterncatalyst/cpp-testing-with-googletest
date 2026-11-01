---
title: "Why test C++, and the toolchain"
order: 2
part: "Foundations"
description: "What a test actually buys you, and how the build files turn a library and a test file into a running GoogleTest suite: the conanfile, the CMake test target, gtest_discover_tests, and CTest."
duration: "25 min"
---

## Learning objectives

By the end of this chapter you can:

- Explain what a unit test verifies and why a green build is not the same thing as correct code.
- Read `examples/_template/conanfile.py` and say what the `gtest/1.18.0` pin and the two generators produce.
- Read the CMake test target and explain how the library, the test file, and GoogleTest are linked into one `unit_tests` executable.
- Explain what `gtest_discover_tests` registers, and how CTest reports pass or fail per case.
- Watch a real regression get caught: break the code under test, see the suite go red, and restore it.

## Diagram

{% include excalidraw.html file="02-test-target-wiring" alt="The build files compose a running suite: conanfile.py provides GoogleTest (gtest_main and gmock), CMake links it with the shipping library and the test source into one unit_tests executable, gtest_discover_tests registers one CTest case per TEST, and ctest runs them and reports per case." caption="Figure 2.1: How the build files turn a library and a test file into a running suite." %}

## Why this matters

Chapter 1 proved the toolchain runs. This chapter answers the question a working engineer actually has before writing a test: what does it buy me, and what are the pieces that carry it from a source file to a pass-or-fail result. The example is `examples/_template`, the minimal project every later chapter copies. Its library prices a parcel for shipping, and the price is exactly the kind of logic that fails quietly. A rounding rule that drops a partial kilogram doesn't crash. It undercharges every shipment, forever, and nothing in the build complains. A test is what turns that silent class of bug into a loud one.

The code is in `examples/_template/`. Its `README.md` covers the demo lanes; this chapter walks the build files that make the suite run and the library those files test.

## What a test buys you

A unit test is an executable statement of what the code should do, checked by a machine every time you build. Prose in a comment ages and lies; a test either passes or fails against the code as it is right now. That's the first thing it buys: an executable specification, written in the same language as the code, that can't drift away from the truth without the suite noticing.

The second thing it buys is a floor under change. The pricing function in this example, `shipping::quote_cents`, charges a flat base per zone plus a per-kilogram rate, with any partial kilogram rounded up to a whole one:

```cpp
int quote_cents(int weight_grams, Zone zone) {
  if (weight_grams <= 0) {
    throw std::invalid_argument("weight_grams must be positive");
  }
  const int kilograms = (weight_grams + 999) / 1000;  // round up to whole kg
  return base_cents(zone) + (kilograms * per_kilogram_cents(zone));
}
```

Three behaviors here are easy to get wrong and impossible to see by reading the output of one happy-path run: the round-up, the rejection of a non-positive weight, and the ordering that makes a farther zone cost more. The suite pins all three down, one `TEST` each:

```cpp
TEST(RateQuote, RoundsAPartialKilogramUp) {
  // 1 g still bills as one whole kilogram: 500 + 120 = 620.
  EXPECT_EQ(shipping::quote_cents(1, shipping::Zone::Domestic), 620);
}

TEST(RateQuote, RejectsNonPositiveWeight) {
  EXPECT_THROW((void)shipping::quote_cents(0, shipping::Zone::Domestic),
               std::invalid_argument);
}
```

```cpp
TEST(RateQuote, ChargesMoreForFartherZones) {
  EXPECT_GT(shipping::quote_cents(1000, shipping::Zone::International),
            shipping::quote_cents(1000, shipping::Zone::Domestic));
}
```

The third test doesn't pin an exact number. It states a relationship: the same parcel costs more to an international zone than a domestic one. `EXPECT_GT` checks that the first value is greater than the second, and a relative assertion like this survives a change to the zone rate table that an exact-value assertion would not. You can raise every rate and this test still holds, because what it protects is the ordering, not the arithmetic.

`EXPECT_EQ` checks a value, `EXPECT_THROW` checks that a call throws the exception type you name, and `EXPECT_GT` checks an ordering. The next chapter takes the assertion vocabulary further. What matters now is that each test states one fact about the function, and the machine rechecks every fact on every build. That's the floor: you can change the rounding math, the zone table, or the internal helpers, and the day you change the behavior by accident, a named test fails and tells you which fact you broke.

A green suite still isn't proof of correctness, and that's the discipline this book holds throughout. A test that asserts nothing, or asserts something trivially true, passes and teaches you nothing. Correct means the test checks the behavior that matters and would fail if that behavior broke. The last section of this chapter proves these tests have that property by breaking the code and watching them catch it.

## How the code works

Three files turn source into a running suite: the Conan recipe that supplies GoogleTest, the CMake file that links everything into a test executable, and the test file itself. Figure 2.1 shows how they connect.

### The Conan recipe

`examples/_template/conanfile.py` declares one dependency and generates the files CMake needs to find it:

```python
class ExampleConan(ConanFile):
    settings = "os", "compiler", "build_type", "arch"

    def requirements(self):
        self.requires("gtest/1.18.0")

    def generate(self):
        CMakeToolchain(self, generator="Ninja").generate()
        CMakeDeps(self).generate()
```

`requirements` pins GoogleTest to exactly `1.18.0`, never a range, so every reader resolves the identical library down to the patch release; GoogleTest 1.18 pulls Abseil and RE2, and Conan pins those transitively, so there is nothing left to manage by hand. `generate` writes two things. `CMakeToolchain` with the Ninja generator produces `build/conan_toolchain.cmake`, the toolchain file the CMake preset reads. `CMakeDeps` produces the package config files that make `find_package(GTest)` resolve to the Conan-built library rather than hunting your system for one. The recipe omits `cmake_layout`, so that with `conan install . -of build`, both generated outputs land flat in `build/`, which is exactly where `CMakePresets.json` points its `toolchainFile`; a reader expecting the usual `cmake_layout` should know the flat layout is what these presets require.

Conan isn't the only way to get GoogleTest. Two alternatives come up often. `FetchContent` downloads and builds GoogleTest as part of your own CMake configure, which keeps everything in one tool but rebuilds the framework from source on a clean build and pins the version in CMake rather than in a recipe. Fedora's `gtest-devel` package installs a system copy, which is fast but ties the example to whatever version the distribution ships. This book uses Conan because the pin lives in one recipe, the build is cached across examples, and every reader resolves the identical version regardless of their distribution; the alternatives are fine for a project that has already standardized on one of them.

### The standard and the strict flags

Before any target, the CMake file pins the language and the checks every target inherits:

```cmake
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_library(project_warnings INTERFACE)
target_compile_options(project_warnings INTERFACE
  -Wall -Wextra -Wpedantic -Werror -Wshadow -Wconversion)
```

Requiring C++23 and turning extensions off keeps the code portable: it won't quietly compile against a GNU-only extension that another compiler rejects. `project_warnings` is an INTERFACE library, a target that builds nothing itself and exists only to carry settings. Any target that links it inherits `-Wall -Wextra -Wpedantic -Werror` and the rest, so the warning policy lives in one place and both the library and the tests pick it up by linking one name. `-Werror` is the line that matters most for a test suite: a warning becomes a build failure, so a narrowing conversion or a shadowed variable stops the build before it can reach a test and be ignored.

A second INTERFACE library, `project_sanitizers`, carries the ASan or TSan flags for the active lane, and a guard near the top refuses the one combination that cannot work:

```cmake
if(ENABLE_ASAN_UBSAN AND ENABLE_TSAN)
  message(FATAL_ERROR "ASan/UBSan and TSan cannot be combined. Use one sanitizer lane.")
endif()
```

That guard is why Chapter 1's three lanes are three separate build directories: the build refuses to configure a binary that asks for both sanitizers at once, so the mistake surfaces at configure time with a clear message rather than at link time with a cryptic one.

### The CMake test target

`examples/_template/CMakeLists.txt` builds two targets on top of the standard and the strict flags just shown. The library under test comes first:

```cmake
add_library(shipping src/shipping/rate_quote.cpp)
target_include_directories(shipping PUBLIC src)
target_link_libraries(shipping PRIVATE project_warnings project_sanitizers)
```

`shipping` is the real code, built the same strict way the tests are: `project_warnings` carries `-Wall -Wextra -Wpedantic -Werror`, and `project_sanitizers` carries whatever sanitizer flags the active lane turned on. Building the library and the tests with the identical flags is what makes a passing test meaningful; a library compiled with looser checks than its tests isn't the library the tests vouch for.

The test executable is the piece that pulls GoogleTest in:

```cmake
find_package(GTest REQUIRED)

enable_testing()
add_executable(unit_tests tests/rate_quote_test.cpp)
target_link_libraries(unit_tests
  PRIVATE shipping GTest::gtest_main GTest::gmock project_warnings project_sanitizers)

include(GoogleTest)
gtest_discover_tests(unit_tests
  XML_OUTPUT_DIR ${CMAKE_BINARY_DIR}/test-results)
```

`find_package(GTest REQUIRED)` resolves the config files Conan generated, and fails the configure immediately if they are missing rather than failing later with a confusing link error. `add_executable(unit_tests ...)` compiles the test file into one binary. The link line is where the suite comes together. `shipping` is the code under test. `GTest::gtest_main` is GoogleTest plus a ready-made `main()` that runs every registered test, so the example writes no `main` of its own. `GTest::gmock` links GoogleMock, available for the mocking chapters even though this example doesn't use it yet. `enable_testing()` turns on CTest for the project.

The last two lines are the ones worth slowing down on. `include(GoogleTest)` loads the CMake module that ships with GoogleTest, which defines the `gtest_discover_tests` command. That command runs the freshly built `unit_tests` binary at build time, asks it which tests it contains, and registers each one as its own CTest case. The older way, a single `add_test(NAME unit_tests COMMAND unit_tests)`, registers the whole binary as one opaque case: CTest then reports one pass or one fail for the entire file. `gtest_discover_tests` instead gives CTest one case per `TEST`, so a failure names the exact behavior that broke, and you can run or re-run a single case by name. The `XML_OUTPUT_DIR` argument writes a JUnit-style report per run, which the CI chapter later feeds to GitHub.

### The test file

`examples/_template/tests/rate_quote_test.cpp` includes `<gtest/gtest.h>`, includes the library header, and puts its tests in an anonymous namespace. That namespace matters more than it looks. GoogleTest links every test file into one binary, so two files that each defined a file-scope helper named `make_parcel` would collide at link time; wrapping each file's helpers in an unnamed namespace gives them internal linkage and sidesteps the clash. Each `TEST(RateQuote, Name)` macro expands to a registered test function; the first argument is the suite name, the second is the case name, and the two together are what CTest reports. The macro hides a fair amount: it declares a class derived from GoogleTest's test fixture, puts your body in that class's `TestBody` method, and registers an instance with the global test registry before `main` runs. That registration is how the `main` from `GTest::gtest_main` finds and runs a test the file never explicitly called. You write the behavior; the macro handles the bookkeeping.

## Build, run, observe

Run the default lane from the example directory:

```bash
[host]$ cd examples/_template
[host]$ ./demo.sh test
```

A clean run ends with CTest's summary:

```console
100% tests passed, 0 tests failed out of 4

Total Test time (real) =   0.01 sec
```

Four cases, four passes. Now prove the suite earns that result. Change the round-up in `src/shipping/rate_quote.cpp` to a truncation, the single most plausible way to get this function wrong:

```cpp
const int kilograms = weight_grams / 1000;  // drops the partial kilogram
```

Rerun `./demo.sh test`, and two cases turn red because a 1-gram parcel now bills as zero kilograms:

```console
tests/rate_quote_test.cpp:16: Failure
Expected equality of these values:
  shipping::quote_cents(1, shipping::Zone::Domestic)
    Which is: 500
  620
[  FAILED  ] RateQuote.RoundsAPartialKilogramUp

50% tests passed, 2 tests failed out of 4
The following tests FAILED:
	  1 - RateQuote.ChargesBasePlusPerKilogram (Failed)
	  2 - RateQuote.RoundsAPartialKilogramUp (Failed)
```

The failure names the case, the file and line, the expression it evaluated, and the two values that failed to match: it found 500 where it wanted 620. That's the whole point of the chapter in one screen. The bug was a one-character change that no happy-path run would reveal, and the suite turned it into a precise, named failure. Restore the line with `git checkout -- src/shipping/rate_quote.cpp` and the suite goes green again.

## Cross-check

CTest tells you four cases ran. To confirm those are the four tests actually in the source, ask the binary directly, without going through CTest at all:

```console
[host]$ ./build/unit_tests --gtest_list_tests
RateQuote.
  ChargesBasePlusPerKilogram
  RoundsAPartialKilogramUp
  ChargesMoreForFartherZones
  RejectsNonPositiveWeight
```

The four names under the `RateQuote` suite match the four `TEST` macros in `rate_quote_test.cpp` one for one, and they match the four cases CTest ran. Two independent tools agreeing, the GoogleTest binary listing its own tests and CTest running them, is how you confirm `gtest_discover_tests` registered exactly what the source declares, with nothing dropped and nothing duplicated.

## What you learned

- A test is an executable specification that the build rechecks every time; a green suite means the facts you wrote down still hold, not that the code is correct in some larger sense.
- The three build files compose a suite: `conanfile.py` pins and provides GoogleTest, `CMakeLists.txt` links it with the library under test into `unit_tests`, and `gtest_discover_tests` registers one CTest case per `TEST`.
- `GTest::gtest_main` supplies `main()`, so the example writes none; `gtest_discover_tests` beats a single `add_test` because a failure names the exact case.

The next chapter writes a test from an empty file, so you build this wiring yourself rather than reading a copy of it.

## For deeper coverage

- The [GoogleTest primer](https://google.github.io/googletest/primer.html): `TEST`, test suites, and the assertion basics this chapter leans on.
- The [CMake `GoogleTest` module documentation](https://cmake.org/cmake/help/latest/module/GoogleTest.html): `gtest_discover_tests`, its options, and how it differs from `add_test`.
- *Professional CMake: A Practical Guide* by Craig Scott ([Crascit](https://crascit.com/professional-cmake/)): targets, linking, and the CTest integration this example relies on.
- The full reading list lives on the [bibliography page]({{ '/bibliography/' | relative_url }}).

---

*Verification status: <span class="status status--verified">verified</span>. On Fedora 44 (Clang 22.1.8, Conan 2.33.0, GoogleTest 1.18.0), `examples/_template` builds and reports 4/4 on `./demo.sh test`; the regression shown turns two cases red as quoted; `./build/unit_tests --gtest_list_tests` lists the four cases shown. Re-run all lanes before a release.*
