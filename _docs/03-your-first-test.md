---
title: "Your first test"
order: 3
part: "Foundations"
description: "Write a GoogleTest suite from an empty file: the anatomy of a TEST, choosing cases that cover the branches, and how CMake presets, CTest, and gtest_discover_tests run them."
duration: "25 min"
---

## Learning objectives

By the end of this chapter you can:

- Write a `TEST` from an empty file and explain each part: the suite name, the case name, and the assertion.
- Put the expected and actual values in the order GoogleTest's output assumes.
- Choose test cases that cover a function's branches rather than repeating one path.
- Configure and run the suite with `cmake --preset`, `cmake --build --preset`, and `ctest --preset`.
- Confirm that `gtest_discover_tests` registered exactly the cases the source declares.

## Diagram

{% include excalidraw.html file="03-billable-weight-branches" alt="The billable-weight rule: volumetric grams is computed from the dimensions as length times width times height divided by five, billable weight is the greater of volumetric and actual weight, and the three test cases each cover one outcome: a bulky parcel bills volumetric, a heavy parcel bills actual, and a tie bills either." caption="Figure 3.1: The billable-weight rule and the branch each test case covers." %}

## Why this matters

Chapter 2 walked a suite that already existed. This chapter writes one from an empty file, because the mechanics only stick once you've typed them yourself. Reading a passing test tells you it passes; writing one teaches you where the assertions go, how the build finds them, and how a failure would read if the code were wrong. The example is `examples/03-first-test`: the same `_template` scaffold with one new library file and one new test file, so everything around the code is build wiring you already met, and the only new material is the test and the function it checks.

The code is in `examples/03-first-test/`. The run script there builds and runs it; its `README.md` covers how to drive each lane.

## The function under test

A carrier doesn't bill a parcel on its scale weight alone. A large, light box takes the same space on a truck as a dense one, so carriers charge the greater of the actual weight and a volumetric weight derived from the parcel's size. The example captures that rule in one function:

```cpp
namespace shipping {

struct Dimensions {
  int length_cm;
  int width_cm;
  int height_cm;
};

[[nodiscard]] int billable_weight_grams(Dimensions box, int actual_grams);

}  // namespace shipping
```

The dimensions travel together in a `Dimensions` struct rather than as three loose `int` parameters. That's not decoration. Three adjacent `int` arguments at a call site are trivially easy to transpose, and the compiler can't catch `billable_weight_grams(30, 40, 25, ...)` written when you meant `40, 30, 25`, because every argument has the same type. A named struct makes the grouping explicit and the call site readable, and it's the kind of small interface decision a test will later let you change without fear. The implementation is the rule itself:

```cpp
int billable_weight_grams(Dimensions box, int actual_grams) {
  const int volumetric_grams =
      (box.length_cm * box.width_cm * box.height_cm) / 5;
  return std::max(volumetric_grams, actual_grams);
}
```

Volume in cubic centimetres divided by five gives the volumetric weight in grams, the common 5000-cubic-centimetres-per-kilogram convention. Weights and dimensions are `int` grams and centimetres rather than floating-point values, because integer arithmetic gives an exact, repeatable result a test can assert with `EXPECT_EQ`; a float would invite rounding differences that turn an equality check into a guessing game. `std::max` returns the larger of the volumetric and scale weights. Two branches, and a tie, which is exactly what the tests have to cover.

## How the code works

### The anatomy of a test

A GoogleTest case is a macro, a name, and a body:

```cpp
TEST(BillableWeight, BillsVolumetricForALightBulkyParcel) {
  // 40 x 30 x 25 cm = 30000 cm3 -> 6000 g volumetric, above the 2000 g scale.
  EXPECT_EQ(shipping::billable_weight_grams(
                shipping::Dimensions{40, 30, 25}, 2000),
            6000);
}
```

`TEST(BillableWeight, BillsVolumetricForALightBulkyParcel)` declares one case. The first argument is the suite name, a label that groups related cases; the second is the case name, which should read as the behavior it pins down, so a failure report names a fact rather than a number. The body runs real code and makes one assertion. `EXPECT_EQ(a, b)` checks that `a` equals `b` and, if they differ, prints both values and keeps going. The argument order matters for the message, not the result: GoogleTest labels the first argument as the value it computed and the second as the value you expected, so putting the call first and the literal `6000` second makes the failure read the right way round. The next chapter covers the wider assertion vocabulary; `EXPECT_EQ` is enough to write a first suite.

`EXPECT_EQ` asks two things of its arguments. They have to be comparable with `==`, which `int` values are, and they have to be printable so the failure message can show them. GoogleTest prints the built-in types out of the box; for a type of your own, the message falls back to a hex dump of the bytes unless you give the type an `operator<<`. The values here are plain `int` grams, so both requirements hold with no extra work, and the failure message reads in the units the function works in. This is also why the tests assert on the returned `int` rather than on a `Dimensions` value: comparing the result keeps each assertion on a type GoogleTest already prints clearly.

Even a one-line test has a shape worth keeping, because larger tests keep it too: arrange the inputs, act by calling the code, assert on the result. Here the arrange and act fold into the single `EXPECT_EQ` call, where `Dimensions{40, 30, 25}` and `2000` are the arrangement and `billable_weight_grams(...)` is the act. As a function under test grows, splitting those steps onto their own lines keeps a test readable, and the fixtures chapter builds on exactly that structure.

The case name is documentation the runner prints back to you, so it earns the same care as a function name. `BillsVolumetricForALightBulkyParcel` says what holds and under which condition; a name like `Test1` or `Works` would force a reader to open the body to learn what broke. One behavior per case follows from the same idea: a case that asserts three unrelated facts reports only the first failure and hides the rest, and its name can't describe all three. Keep a case narrow enough that its name is the whole truth.

### Choosing the cases

A function with two branches and a tie needs a case for each, not three cases for the same path. The suite picks inputs that land on different outcomes:

```cpp
TEST(BillableWeight, BillsActualForASmallHeavyParcel) {
  // 10 x 10 x 10 cm = 1000 cm3 -> 200 g volumetric, below the 5000 g scale.
  EXPECT_EQ(shipping::billable_weight_grams(
                shipping::Dimensions{10, 10, 10}, 5000),
            5000);
}

TEST(BillableWeight, BillsEitherWhenTheyAreEqual) {
  // 20 x 10 x 10 cm = 2000 cm3 -> 400 g volumetric, equal to the scale weight.
  EXPECT_EQ(shipping::billable_weight_grams(
                shipping::Dimensions{20, 10, 10}, 400),
            400);
}
```

The first case forces volumetric weight to win, the second forces actual weight to win, and the third sits them exactly equal, the boundary where a wrong comparison operator hides. Figure 3.1 shows the same split: each arrow is a case. Three cases that all made volumetric win would pass against a function that ignored actual weight entirely, and a suite that passes against broken code teaches nothing. Covering the branches is what gives the green result its meaning.

The test file puts all three in an anonymous namespace and includes `<gtest/gtest.h>`. It writes no `main`, because the `GTest::gtest_main` target linked in `CMakeLists.txt` supplies one that runs every registered case.

Two things are out of scope here by design, and the comments say so rather than letting a reader assume the function is more defensive than it is. The function trusts its inputs to be positive; it doesn't validate them, so a zero or negative value produces a meaningless number instead of an error. It also multiplies three `int` values before dividing, which overflows for parcels far larger than any real package. Both are reasonable simplifications for a first example, and the chapter on death tests and exceptions is where input validation and failure paths get their own treatment.

Each case also stands on its own. GoogleTest constructs a fresh test object per case and runs the cases independently, so none depends on another having run first, and the suite behaves the same whether CTest runs the three in order, in reverse, or shuffled. That independence is what lets you run a single case in isolation while chasing a failure, which is the next thing to reach for.

### Configuring and running with presets

The build steps are the ones from Chapter 1, named here as the three commands `demo.sh` runs for you:

```bash
[host]$ cmake --preset default
[host]$ cmake --build --preset default
[host]$ ctest --preset default --output-on-failure
```

`cmake --preset default` reads `CMakePresets.json`, which names the Ninja generator, the `build/` directory, and the Conan toolchain file, and configures the project there. `cmake --build --preset default` compiles `parcel.cpp` and `parcel_test.cpp` into the `unit_tests` binary. During that build, `gtest_discover_tests` runs the fresh binary, asks which cases it holds, and registers each as its own CTest test. `ctest --preset default` then runs them and reports one line per case. Because the configure, build, and test presets share the name `default`, the three commands act on the same configured tree with nothing to wire up by hand. `CMakePresets.json` also defines a `release` preset for an optimized build and the two sanitizer presets from Chapter 1; `demo.sh` additionally offers a `build` lane that configures and compiles without running the suite. The `default` lane is the one you live in while writing tests.

### Running one case at a time

While chasing a failure you rarely want the whole suite. Both tools run a subset. CTest filters by a regular expression over the case name:

```console
[host]$ ctest --preset default -R BillsActualForASmallHeavyParcel
1/1 Test #2: BillableWeight.BillsActualForASmallHeavyParcel ...   Passed    0.00 sec
100% tests passed, 0 tests failed out of 1
```

The binary filters with `--gtest_filter`, which takes a `suite.case` glob:

```console
[host]$ ./build/unit_tests --gtest_filter='BillableWeight.BillsVolumetric*'
[ RUN      ] BillableWeight.BillsVolumetricForALightBulkyParcel
[       OK ] BillableWeight.BillsVolumetricForALightBulkyParcel (0 ms)
[  PASSED  ] 1 test.
```

Filtering also works at the suite level: `ctest -R BillableWeight` or `--gtest_filter='BillableWeight.*'` runs every case under the suite, which is how you re-run one component's tests without the rest of a larger project. Running one case keeps the loop tight: change the code, rerun the single failing case in well under a second, and run the full suite again only once it passes. CTest's `-R` is the right tool from a script or a CI job; `--gtest_filter` is the one to use when you're already running the binary under a debugger.

## Build, run, observe

Run the default lane:

```bash
[host]$ cd examples/03-first-test
[host]$ ./demo.sh test
```

Three cases, three passes:

```console
1/3 Test #1: BillableWeight.BillsVolumetricForALightBulkyParcel ...   Passed    0.00 sec
2/3 Test #2: BillableWeight.BillsActualForASmallHeavyParcel .......   Passed    0.00 sec
3/3 Test #3: BillableWeight.BillsEitherWhenTheyAreEqual ...........   Passed    0.00 sec

100% tests passed, 0 tests failed out of 3
```

Each case CTest ran carries the `suite.case` name from the source, which is what makes a failure point straight at the behavior that broke. The `--output-on-failure` flag keeps a passing run quiet and prints the GoogleTest message only for a case that fails, which is what you want from a suite you run constantly. After the first build, Ninja recompiles only what changed, so the edit-run loop on a single file stays near-instant. The command's exit status is zero only when every case passed, which is the signal a CI job keys on. The suite also passes under both sanitizer lanes, `./demo.sh asan` and `./demo.sh tsan`, which exercise the same three cases with AddressSanitizer and ThreadSanitizer instrumentation.

### See it fail

A test you've never watched fail is a test you don't yet trust. Change the divisor in `src/shipping/parcel.cpp` from `/ 5` to `/ 6`, the kind of off-by-one a tired afternoon produces, and rerun `./demo.sh test`:

```console
tests/parcel_test.cpp:11: Failure
Expected equality of these values:
  shipping::billable_weight_grams(shipping::Dimensions{40, 30, 25}, 2000)
    Which is: 5000
  6000
[  FAILED  ] BillableWeight.BillsVolumetricForALightBulkyParcel
```

GoogleTest names the file and line, prints the call it evaluated, reports that it computed 5000, and shows the 6000 you expected. The bulky-parcel case catches the wrong divisor because that case was chosen to make volumetric weight the deciding value; the small-heavy case keeps passing, because its result comes from the scale weight the bug doesn't touch. That's the payoff of covering the branches: the failure is specific, and the cases that still pass tell you where the bug is not. With the bug in place, `ctest --preset default -R BillsVolumetric` reruns just the broken case, so you confirm a fix without waiting on the rest. Restore the `/ 5` and the suite is green again.

## Cross-check

Two independent tools should agree on which cases exist. Ask the binary directly:

```console
[host]$ ./build/unit_tests --gtest_list_tests
BillableWeight.
  BillsVolumetricForALightBulkyParcel
  BillsActualForASmallHeavyParcel
  BillsEitherWhenTheyAreEqual
```

Then ask CTest to list what it would run, without running anything:

```console
[host]$ ctest --preset default -N
  Test #1: BillableWeight.BillsVolumetricForALightBulkyParcel
  Test #2: BillableWeight.BillsActualForASmallHeavyParcel
  Test #3: BillableWeight.BillsEitherWhenTheyAreEqual
Total Tests: 3
```

The binary lists three cases, CTest lists the same three, and both match the three `TEST` macros in `parcel_test.cpp`. When these three agree, you know `gtest_discover_tests` registered exactly what the source declares, with nothing dropped and nothing stale from an earlier build. This check matters most when you add or rename a case: because discovery runs at build time, a case you add shows up in `ctest -N` only after the next `cmake --build`, so a `ctest` run that seems to miss a brand-new test usually just needs a rebuild first.

## What you learned

- A `TEST` is a suite name, a case name that reads as the behavior, and a body with an assertion; name the case for the fact it checks.
- `EXPECT_EQ(actual, expected)` prints both values on a mismatch, so argument order controls whether the message reads correctly.
- Cover a function's branches with one case each, including the boundary; three cases on one path prove almost nothing.
- `cmake --preset` configures, `cmake --build --preset` compiles, and `ctest --preset` runs, with `gtest_discover_tests` turning each `TEST` into its own CTest case.

The next chapter separates `ASSERT_*` from `EXPECT_*` and fills in the rest of the assertion vocabulary.

## For deeper coverage

- The [GoogleTest primer](https://google.github.io/googletest/primer.html): `TEST`, suites, and `EXPECT_EQ` from first principles.
- The [CMake presets documentation](https://cmake.org/cmake/help/latest/manual/cmake-presets.7.html): what a configure, build, and test preset each hold.
- *Professional CMake: A Practical Guide* by Craig Scott ([Crascit](https://crascit.com/professional-cmake/)): presets, CTest, and target wiring in depth.
- The full reading list lives on the [bibliography page]({{ '/bibliography/' | relative_url }}).

---

*Verification status: <span class="status status--verified">verified</span>. On Fedora 44 (Clang 22.1.8, CMake 4.3.0, Conan 2.33.0, GoogleTest 1.18.0), `examples/03-first-test` builds and reports 3/3 on `./demo.sh test`, `asan`, and `tsan`; `--gtest_list_tests` and `ctest -N` list the three cases shown. Re-run all lanes before a release.*
