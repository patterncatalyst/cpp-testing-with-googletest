---
title: "Testing modern C++20 and C++23"
order: 8
part: "Beyond the basics"
description: "Test the features that define modern C++: a concept checked at compile time, a lazy ranges pipeline materialized to a value, and a generator coroutine whose yields you collect, including an infinite one made safe by laziness."
duration: "30 min"
---

## Learning objectives

By the end of this chapter you can:

- Test a concept at compile time with `static_assert` and at run time as a `constexpr` bool, and read the compiler error a failed constraint produces.
- Explain why a ranges pipeline computes nothing until it is consumed, and test one by materializing its result.
- Collect what a `std::generator` coroutine yields into a container and assert on it.
- Test an infinite generator safely by taking a fixed prefix with `std::views::take`.
- Decide what belongs in a test: your logic over the standard library, not the standard library itself.

## Diagram

{% include excalidraw.html file="08-modern-features-testing" alt="Three rows map a modern C++ feature to when it runs and how you test it. A concept that constrains a template is checked when the code compiles and is tested with static_assert and a constexpr bool. A lazy ranges pipeline of filter then transform runs only when fold_left pulls it, and is tested by materializing the result and comparing it. A generator coroutine yields values on demand and may never terminate, and is tested by collecting the yielded values or taking a prefix." caption="Figure 8.1: Three modern features, each with its own moment to run and its own way to test it." %}

## Why this matters

Concepts, ranges, and coroutines are the features that make C++20 and C++23 feel like a different language from C++11. Each one changes where computation happens: a concept moves a correctness check to compile time, a ranges pipeline defers its work until something consumes it, and a coroutine suspends in the middle of a function and resumes later. Those shifts are what make the features worth using, and they're also what makes testing them a little different from testing an ordinary function. A concept is satisfied or not before the program runs, so the most natural test is one the compiler performs for you. A pipeline holds a recipe rather than a result, so a test has to force it to produce one. A generator can describe an unbounded sequence, so a test has to decide how much of it to pull. This chapter tests one small example of each, and the technique in every case follows from understanding when the feature actually runs.

The code is in `examples/08-modern-cpp/`. The run script there builds and runs it; its `README.md` covers how to drive each lane.

## How the code works

### A concept is checked at compile time: static_assert

A concept names the requirements a type must meet to be used in a given place. Here `Weighable` says that a type exposes an integer weight in grams, and `heavier_weight` is constrained to accept only such types:

```cpp
struct Parcel {
  int weight_grams = 0;
  bool fragile = false;
};

template <typename T>
concept Weighable = requires(const T& item) {
  { item.weight_grams } -> std::convertible_to<int>;
};

template <Weighable T>
constexpr int heavier_weight(const T& first, const T& second) {
  return first.weight_grams >= second.weight_grams ? first.weight_grams
                                                   : second.weight_grams;
}
```

The `requires` expression is the heart of it. It lists an expression, `item.weight_grams`, and a constraint on that expression's type, `std::convertible_to<int>`. A type satisfies `Weighable` when the body type-checks: `Parcel` does, because it has an `int` member named `weight_grams`. The constraint is structural, not nominal, so any type with the right shape qualifies without declaring that it means to.

Because a concept is evaluated by the compiler, the first and cheapest test of one is a `static_assert`. It states an expectation the build must satisfy, and a failed `static_assert` stops compilation with the message you wrote:

```cpp
static_assert(shipping::Weighable<shipping::Parcel>,
              "Parcel exposes weight_grams, so it must be Weighable");
static_assert(!shipping::Weighable<int>,
              "a plain int has no weight_grams, so it must not be Weighable");
```

Both directions matter. The positive assertion pins that the type you expect to work does, and the negative one pins that a type you expect to be rejected is. A concept that accidentally accepts everything would pass the first check and fail the second, so writing both turns the concept's boundary into something the build enforces. These assertions live at namespace scope in the test file, not inside a `TEST`; if the file compiles, they have already held, and there is nothing for CTest to run.

The concept is structural, which is easy to show with a second type that was never mentioned when `Weighable` was written:

```cpp
struct Crate {
  int weight_grams;
  double volume_litres;
};
static_assert(shipping::Weighable<Crate>, "Crate has weight_grams too");
```

`Crate` satisfies `Weighable` because it has the member the concept asks for, and `heavier_weight(small_crate, large_crate)` compiles and runs for the same reason. A test that exercises the constrained function over both `Parcel` and `Crate` records that the constraint is about shape, not about a named base class or an interface.

A concept is also a `constexpr bool`, so the same facts read naturally as ordinary expectations when you want them to appear in the test report rather than only in the build:

```cpp
TEST(WeighableConcept, ParcelSatisfiesItAndIntDoesNot) {
  EXPECT_TRUE(shipping::Weighable<shipping::Parcel>);
  EXPECT_FALSE(shipping::Weighable<int>);
}
```

This runs at test time and shows up as a named case alongside the others. It asserts the same thing the `static_assert` does, so you would usually pick one; the `static_assert` catches the problem earlier, at the cost of a less friendly report, while the `TEST` version is visible in CTest output. For a concept that guards a public API, having the check appear as a named test can be worth the slightly later feedback.

The payoff of constraining `heavier_weight` shows up when a caller gets it wrong. Passing a type that is not `Weighable`, a plain `int`, is rejected at the call site with a message that names the unmet constraint:

```console
error: no matching function for call to 'heavier_weight'
  return shipping::heavier_weight(5, 10);
         ^~~~~~~~~~~~~~~~~~~~~~~~
note: candidate template ignored: constraints not satisfied [with T = int]
constexpr int heavier_weight(const T& first, const T& second) {
              ^
note: because 'int' does not satisfy 'Weighable'
template <Weighable T>
          ^
note: because 'item.weight_grams' would be invalid: member reference base
type 'const int' is not a structure or union
  { item.weight_grams } -> std::convertible_to<int>;
        ^
```

The error points at the constraint and then at the exact requirement that failed, rather than at some line deep inside the function body where `first.weight_grams` would have been a hard error. That readable rejection is the reason to write the concept in the first place, and the negative `static_assert` is how you keep it working: it's a compile-time test that the boundary is where you think it is.

### A ranges pipeline is lazy: materialize to test it

The manifest computes the total weight of the parcels at or above a threshold. It expresses that as a pipeline of views rather than a loop:

```cpp
int heavy_parcel_weight(const std::vector<Parcel>& parcels, int threshold_grams) {
  auto heavy = parcels | std::views::filter([threshold_grams](const Parcel& parcel) {
                 return parcel.weight_grams >= threshold_grams;
               }) |
               std::views::transform(&Parcel::weight_grams);

  return std::ranges::fold_left(heavy, 0, std::plus<>{});
}
```

`std::views::filter` keeps the parcels that pass the predicate, and `std::views::transform` projects each survivor to its `weight_grams` using a pointer-to-member, which `std::invoke` turns into the member access. The key fact for testing is in the word views: `heavy` is not a container and holds no results. It's a lazy description of a computation, and neither the filter nor the transform touches a single parcel until something iterates the pipeline. That something is `std::ranges::fold_left`, a C++23 algorithm that walks the view once, combining each element into an accumulator that starts at the seed `0`.

Laziness is why a test has to force the pipeline to produce a value. Here the fold does that inside the function, so the function returns a plain `int` and the test looks ordinary:

```cpp
TEST(HeavyParcelWeight, SumsOnlyParcelsAtOrAboveTheThreshold) {
  const std::vector<shipping::Parcel> manifest{
      {.weight_grams = 200, .fragile = false},
      {.weight_grams = 1200, .fragile = false},
      {.weight_grams = 3000, .fragile = true},
  };
  EXPECT_EQ(shipping::heavy_parcel_weight(manifest, 1000), 4200);
}
```

The cases that pin the behavior are the ones at the edges of the filter. A threshold is a boundary, and a boundary test is the one that catches an off-by-one between `>` and `>=`:

```cpp
TEST(HeavyParcelWeight, ThresholdIsInclusive) {
  const std::vector<shipping::Parcel> manifest{
      {.weight_grams = 1000, .fragile = false},
      {.weight_grams = 999, .fragile = false},
  };
  EXPECT_EQ(shipping::heavy_parcel_weight(manifest, 1000), 1000);
}
```

A parcel exactly at the threshold must count, and one a gram below must not. If the predicate used `>` instead of `>=`, this test fails with `Which is: 0` against an expected `1000`, because the only qualifying parcel dropped out. The other edge is an empty result: when nothing passes the filter, the fold returns its seed unchanged, so `heavy_parcel_weight` returns `0` rather than misbehaving on an empty range. That case is worth its own test, because the seed is the only thing standing between an empty pipeline and a wrong answer.

When a function returns a view instead of a folded value, the test does the materializing. C++23 gives you `std::ranges::to`, which runs a range into a container in one call, so a view-returning function is tested with `auto result = make_view(input) | std::ranges::to<std::vector>();` followed by an `EXPECT_EQ` against the expected vector. The pattern is the same either way: a lazy pipeline carries no result until you pull it, so a test pulls it and compares what comes out.

### A coroutine yields values: collect them

A `std::generator` is a coroutine that produces a sequence one value at a time. `batch_weights` splits a total into batches of at most `batch_grams` and yields the weight of each:

```cpp
std::generator<int> batch_weights(int total_grams, int batch_grams) {
  int remaining = total_grams;
  while (remaining > 0) {
    const int this_batch = std::min(remaining, batch_grams);
    co_yield this_batch;  // hand one value back, then suspend here
    remaining -= this_batch;
  }
}
```

`co_yield` is what makes this a coroutine. Each time the consumer asks for a value, the function runs up to the `co_yield`, hands that value out, and suspends with all its local state intact. The next request resumes it on the line after the `co_yield`, so `remaining` keeps its value across suspensions. The function reads like a loop that produces a series, and `std::generator` turns it into an input range you can iterate with a range-based `for`.

Testing a finite generator means collecting what it yields and comparing. A range-based `for` drives it to exhaustion:

```cpp
TEST(BatchWeights, SplitsIntoFullBatchesPlusARemainder) {
  std::vector<int> batches;
  for (const int weight : shipping::batch_weights(2500, 1000)) {
    batches.push_back(weight);
  }
  EXPECT_EQ(batches, (std::vector<int>{1000, 1000, 500}));
}
```

The same `std::ranges::to` from the ranges section collects a generator in one line, since a generator is just an input range: `auto batches = shipping::batch_weights(2000, 1000) | std::ranges::to<std::vector>();`. Either form gives a `std::vector<int>` to compare with `EXPECT_EQ`, and comparing the whole vector at once, rather than element by element, reports a readable diff of the full sequence when it's wrong. The edge case here is a zero total, which yields nothing: the `while` never enters, the generator is empty, and the collected vector is empty too.

The interesting case is a generator that never ends. `running_total` yields a sum that grows forever:

```cpp
std::generator<int> running_total(int step) {
  int total = 0;
  while (true) {  // no termination: the caller decides when to stop pulling
    total += step;
    co_yield total;
  }
}
```

A range-based `for` over this would never return, so the test pulls a fixed prefix with `std::views::take`, which stops asking after a set number of values:

```cpp
TEST(RunningTotal, IsLazySoAnInfiniteSequenceIsSafeToTake) {
  std::vector<int> first_three;
  for (const int value : shipping::running_total(500) | std::views::take(3)) {
    first_three.push_back(value);
  }
  EXPECT_EQ(first_three, (std::vector<int>{500, 1000, 1500}));
}
```

{% include excalidraw.html file="08-lazy-generator-pull" alt="An infinite running_total generator feeds views::take(3), which feeds the test's collected vector. The generator yields a value on demand; take pulls three and stops; the test compares the collected {500, 1000, 1500}. A note records that the loop body resumes three times and never a fourth, and that laziness is what makes the test terminate." caption="Figure 8.2: views::take pulls a fixed prefix, so the infinite generator's loop body runs only three times." %}

The test terminates for the same reason the generator is useful: nothing computes a value until the consumer pulls it. `std::views::take(3)` requests three values and then stops iterating, so the coroutine resumes three times and its `while (true)` body runs three times, never a fourth. Figure 8.2 traces the single pull-and-yield exchange that repeats three times. The infinite loop is not a bug a test should avoid; it's a lazy description of a sequence, and the test chooses how much of it to realize. This is the same laziness as the ranges pipeline, made vivid: a pipeline you never consume does no work, and a generator you stop pulling stops running.

> **AI sidebar.** Asked to test modern C++, an assistant tends to test the standard library instead of your code: a case that checks `std::views::filter` filters, or that a `std::generator` yields in order, which pins library behavior you did not write. Point it at your logic instead, the predicate and the threshold boundary, the batch arithmetic and its remainder, the seed an empty fold returns. For concepts it often writes only the positive `static_assert` and omits the negative one, so a concept that accepts too much passes; ask for the rejection case explicitly. The sharpest tell is an infinite generator: a model that has not reasoned about laziness will either iterate it to exhaustion, hanging the test, or avoid the case entirely. A correct suite takes a bounded prefix with `std::views::take` and says why that terminates.

## Build, run, observe

Run the default lane:

```bash
[host]$ cd examples/08-modern-cpp
[host]$ ./demo.sh test
```

```console
100% tests passed, 0 tests failed out of 10
```

Ten cases: three on the concept and its constrained function, three on the ranges pipeline, and four on the generators. The two `static_assert` lines and the `Crate` assertion are not among the ten, because they were checked while the file compiled; if any had failed, the build would have stopped before CTest ran. That split is the point of the chapter in miniature: the compile-time checks never reach the runner, and the runtime checks do.

The whole suite passes under `./demo.sh asan` and `./demo.sh tsan` as well. Nothing here starts a thread, so the sanitizer lanes report the same ten passing cases; they're still worth running, because the coroutine machinery allocates a frame, and ASan would flag a leak or a use-after-free in that frame if the generator mishandled its lifetime.

## Cross-check

Listing the cases confirms the four suites and the ten runnable tests, with the compile-time assertions absent as expected:

```console
[host]$ ./build/unit_tests --gtest_list_tests
WeighableConcept.
  ParcelSatisfiesItAndIntDoesNot
HeavierWeight.
  ReturnsTheLargerOfTwoWeights
  WorksForAnyWeighableType
HeavyParcelWeight.
  SumsOnlyParcelsAtOrAboveTheThreshold
  ThresholdIsInclusive
  ReturnsZeroWhenNothingQualifies
BatchWeights.
  SplitsIntoFullBatchesPlusARemainder
  DividesEvenlyWithNoRemainder
  YieldsNothingForAZeroTotal
RunningTotal.
  IsLazySoAnInfiniteSequenceIsSafeToTake
```

The listing shows ten tests and no sign of the three `static_assert` checks, which matches the design: a concept is tested by the compiler, so its checks never appear in the runner's inventory.

## What to put under test

Modern features make it tempting to test the wrong thing. `std::views::filter`, `std::ranges::fold_left`, and `std::generator` are part of the standard library, tested far more thoroughly than any suite here will, so a test that checks they work is effort spent pinning someone else's code. The value is in the logic you wrote on top of them: the predicate's threshold and whether it is inclusive, the batch arithmetic and its remainder, the seed an empty fold returns, the concept's boundary between accepted and rejected types. Each of those is a decision you made, and each is where a bug would live. Test the decision, lean on the library, and let the compiler carry the checks it can.

## What you learned

- A concept is checked at compile time; test it with a positive and a negative `static_assert`, and read the compiler's constraint-not-satisfied message as the failure report.
- A ranges pipeline is lazy and holds no result; test it by materializing the output, with `std::ranges::fold_left` or `std::ranges::to`, and pin the filter's boundaries.
- A `std::generator` yields values on demand; collect a finite one into a vector and compare the whole sequence at once.
- An infinite generator is safe to test with `std::views::take`, which pulls a fixed prefix so the coroutine's loop runs only as many times as you ask.

This chapter closes Part 2. The next part turns to GoogleMock: replacing a real collaborator with a test double so you can test a unit in isolation.

## For deeper coverage

- [cppreference on concepts](https://en.cppreference.com/w/cpp/language/constraints): the `requires` expression, constraint satisfaction, and `static_assert`.
- [cppreference on the ranges library](https://en.cppreference.com/w/cpp/ranges): views, lazy evaluation, and the range adaptors.
- [cppreference on `std::generator`](https://en.cppreference.com/w/cpp/coroutine/generator): the C++23 coroutine generator and its range interface.
- The full reading list lives on the [bibliography page]({{ '/bibliography/' | relative_url }}).

---

*Verification status: <span class="status status--verified">verified</span>. On Fedora 44 (Clang 22.1.8, CMake 4.3.0, Conan 2.33.0, GoogleTest 1.18.0), `examples/08-modern-cpp` builds and reports 10/10 on `./demo.sh test`, `asan`, and `tsan`; the constraint-not-satisfied error and the `--gtest_list_tests` listing are quoted from real runs. Re-run all lanes before a release.*
