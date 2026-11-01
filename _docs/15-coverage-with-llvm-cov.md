---
title: "Measuring coverage with llvm-cov"
order: 15
part: "Tooling and workflow"
description: "See which lines and branches the suite actually runs: build instrumented with Clang's source-based coverage, report with llvm-cov, read line, region, and branch numbers, and know what coverage can and cannot tell you."
duration: "30 min"
---

## Learning objectives

By the end of this chapter you can:

- Build a test binary with Clang's source-based coverage and produce a report with llvm-cov.
- Run the four-step pipeline: instrument, run, merge the profile, report.
- Read line, region, and branch coverage, and tell the three apart.
- Use `llvm-cov show` to find the exact branch a suite never exercises.
- Explain why high coverage is necessary but not sufficient, and what it cannot measure.

## Diagram

{% include excalidraw.html file="15-coverage-pipeline" alt="The llvm-cov pipeline in four steps. Compile instrumented with -fprofile-instr-generate and -fcoverage-mapping, run the tests with LLVM_PROFILE_FILE set to produce a .profraw, merge it with llvm-profdata into a .profdata, then report line, region, and branch coverage with llvm-cov report or show." caption="Figure 15.1: Coverage is one build, one run, and one report: instrument, run, merge, report." %}

## Why this matters

A green suite tells you the tests that exist pass. It says nothing about the code those tests never touch. A function can be full of branches, half of them never run by any test, and the suite is still green, because a test can't fail on a line it doesn't execute. Coverage measurement fills that blind spot: it records which lines, which regions, and which branches the suite actually ran, so you can see the parts you're only assuming work.

Clang has source-based coverage built in, and it's precise: it instruments the code at compile time and reports not just line counts but region and branch counts, down to which side of an `if` was taken. This chapter runs that pipeline on a small fee calculation with four branches, shows the full suite reaching every one, then drops a test to watch a branch go uncovered and llvm-cov point straight at it. The chapter ends on the harder lesson, which is what coverage does not tell you: that a line running isn't the same as its result being checked, so coverage is a floor to clear, not a target to chase.

The code is in `examples/15-coverage/`. The run script there builds and runs it; its `README.md` covers the lanes, and `coverage.sh` runs the coverage pipeline.

## How the code works

### The code under test and its branches

`late_fee_cents` has four outcomes, each a branch worth covering:

```cpp
int late_fee_cents(int days_late, bool is_member) {
  if (days_late <= 0) {
    return 0;  // not late
  }
  if (is_member && days_late <= 3) {
    return 0;  // member grace period
  }
  const int fee = days_late * 100;
  if (fee > 2000) {
    return 2000;  // cap
  }
  return fee;
}
```

The suite has one test per outcome: not late, the member grace, the per-day fee, and the cap. With all four present, every branch runs. The point of the chapter is to see that, and then to see what a missing test looks like.

### The pipeline

Source-based coverage is four steps, drawn in Figure 15.1. First compile with the instrumentation flags, `-fprofile-instr-generate -fcoverage-mapping`, which the example gates behind an `ENABLE_COVERAGE` CMake option alongside the sanitizer options. Second, run the binary with `LLVM_PROFILE_FILE` set, which writes a raw profile. Third, merge the raw profile into an indexed one with `llvm-profdata`. Fourth, report with `llvm-cov`. The example's `coverage.sh` wraps all four:

```bash
[host]$ ./coverage.sh
```

```console
Filename                 Regions  Missed  Cover  Lines  Missed  Cover  Branches  Missed  Cover
src/shipping/late_fee.cpp    12       0  100.00%    13      0  100.00%        8       0  100.00%
TOTAL                        12       0  100.00%    13      0  100.00%        8       0  100.00%
```

The full suite covers everything: 100% of regions, lines, and branches. The three numbers measure different things, and the difference matters. Line coverage asks whether each line ran at all. Region coverage is finer: a region is a span of code with a single entry and exit, so a line with two statements or a short-circuiting `&&` has more regions than lines, and region coverage catches a partially-run line that line coverage would call covered. Branch coverage is finer still: it asks whether each side of each conditional was taken, so an `if` whose condition was only ever true counts as a covered line but a half-covered branch. Branch coverage is the one to watch, because it is the hardest to fool.

Branch coverage has a stricter cousin worth knowing. A compound condition like `is_member && days_late <= 3` counts as covered once each side has been both true and false, but that can happen without showing that each operand independently changes the outcome. Modified condition/decision coverage, which Clang exposes with `-fcoverage-mcdc`, tracks exactly that, and it is worth the extra instrumentation on code where a complex boolean guards something expensive to get wrong. For most code branch coverage is the right level, and MC/DC is more than the result repays.

### Watching a branch go uncovered

Coverage shows its worth when a branch is missed. Running the same pipeline over a subset of the suite, with the cap test filtered out, drops the numbers:

```bash
[host]$ ./coverage.sh --gtest_filter=-LateFee.ChargesUpToACap
```

```console
Filename                 Regions  Missed   Cover  Lines  Missed   Cover  Branches  Missed   Cover
src/shipping/late_fee.cpp    12       1  91.67%    13      2  84.62%        8       1  87.50%
```

One region, two lines, and one branch are now missed. `llvm-cov show` with `-show-branches=count` annotates the source with the counts and points at exactly which branch:

```console
   12|      2|  const int fee = days_late * 100;
   13|      2|  if (fee > 2000) {
  ------------------
  |  Branch (13:7): [True: 0, False: 2]
  ------------------
   14|      0|    return 2000;  // cap
   15|      0|  }
   16|      2|  return fee;
```

The leftmost column is the execution count. Line 13 ran twice, but the branch annotation shows `[True: 0, False: 2]`: the condition was false both times and true never, so the capped path, line 14, has a count of 0 and is marked uncovered. That is the missing cap test made visible. The report said a branch was uncovered; `show` said which one, which is the difference between knowing you have a gap and knowing where it is. Adding the test back takes the True count above zero and the line to full coverage.

### More of the workflow

A few options turn this from a one-off command into a usable habit. `llvm-cov show -format=html -output-dir=cov-html` writes an annotated HTML tree, which is easier to scan than terminal output when a file has many regions; the red lines jump out. The report can be scoped: passing only `src/shipping/late_fee.cpp` as the example does keeps the test file and GoogleTest's own headers out of the numbers, so the percentage describes the code under test rather than the harness. For code that should not count, a `// LCOV_EXCL_LINE` style marker or an ignore regex excludes a line llvm-cov would otherwise hold against you, such as an unreachable `default` on an exhaustive switch.

Two practices make coverage part of CI rather than a thing you remember to run. `llvm-cov report` can fail a build when coverage drops below a floor, which turns a silent erosion into a red check; the next chapter wires exactly that kind of gate. And diff coverage, measuring only the lines a change touched, is often more useful than the whole-project number: a pull request that adds an untested branch shows as a gap on its own lines even when the project total barely moves. Tooling like this reports per-change coverage from the same `.profdata` the pipeline already produces.

A larger project has several test binaries, and the pipeline scales to them without changing shape. Each binary writes its own raw profile, and `llvm-profdata merge` takes all of them at once to produce a single `.profdata` that `llvm-cov` reports against. Giving `LLVM_PROFILE_FILE` a pattern with `%p`, which expands to the process id, keeps parallel runs from overwriting each other's output, so a whole suite split across binaries still yields one combined number.

GCC offers its own coverage path for the cross-check lane. Compiling with `--coverage` emits `.gcda` and `.gcno` files that `gcov`, and the friendlier `gcovr` and `lcov` front ends, turn into a report. It's less precise about regions and branches than Clang's source-based coverage, so this book uses the LLVM pipeline as the primary one, but the GCC path is worth knowing when the cross-check build is the one in hand.

> **AI sidebar.** An assistant asked to "get coverage up" will write tests that execute the missing lines without asserting anything meaningful about them, a test that calls the function on the uncovered input and checks nothing, or checks something trivially true. That turns the report green while testing nothing, which is worse than the gap, because the gap was at least visible. When you use a model to close a coverage hole, judge each new test by what it asserts, not by the branch it lights up: ask what wrong behavior the test would catch, and if the answer is none, the test is coverage theater. The useful request is narrower: point the model at the specific uncovered branch from `llvm-cov show` and ask what input reaches it and what the correct result on that input is, then write the assertion around that result yourself.

## Build, run, observe

The example's suite passes on every lane, and the coverage build is separate from them:

```bash
[host]$ cd examples/15-coverage
[host]$ ./demo.sh test
```

```console
100% tests passed, 0 tests failed out of 4
```

Coverage instrumentation is its own build, into `build-cov/`, because the instrumented binary is slower and isn't what you ship or what you run the sanitizers on. Keeping it a separate lane, the way the sanitizers are separate lanes, means the normal test build stays fast and the coverage numbers come from a build dedicated to producing them. The coverage build turns clang-tidy off, since analysis is not the point of that pass, and the result is the report above.

## Cross-check

Listing the cases shows the four tests, one per branch of the fee:

```console
[host]$ ./build/unit_tests --gtest_list_tests
LateFee.
  IsZeroWhenNotLate
  IsZeroForAMemberWithinTheGrace
  ChargesPerDayPastTheGrace
  ChargesUpToACap
```

`ChargesUpToACap` is the case the chapter filters out to open the branch gap; the others cover the early returns and the per-day fee. The four names read as the four branches, which is the correspondence coverage then confirms.

## What coverage does not tell you

A coverage number is easy to over-trust, so it's worth being exact about its limits, which Figure 15.2 lays out. Coverage tells you a line or branch ran. It does not tell you the test checked the result: a test that calls `late_fee_cents(50, false)` and asserts nothing covers the cap branch just as fully as one that asserts the answer is 2000, and only the second would catch the cap being wrong, and the gap at least stays visible in the report. Coverage also says nothing about the inputs you did not try. A function with 100% branch coverage can still fail on an edge the tests never passed it, because covering a branch means taking it once, not taking it with every value that matters. The boundary bug from the previous chapter is the clean example: a suite that tested `tiered_cost_cents` only at 100 and 9000 grams would cover every branch and still miss the wrong answer at exactly 2000, because coverage counts the branch as taken without caring which input took it.

{% include excalidraw.html file="15-coverage-limits" alt="Two columns. Coverage tells you which lines and branches ran and where a whole branch went untested. Coverage cannot tell you whether the test asserted the result or whether an untested input breaks it. A note reads: 100% coverage with weak assertions still passes a broken program." caption="Figure 15.2: Coverage measures what ran, not what was checked." %}

So treat coverage as a floor, not a target. Low coverage is a reliable warning: a branch nothing runs is a branch nothing tests, and that's worth fixing. High coverage is weaker reassurance: it means the tests reach the code, not that they check it. The useful habit is to read an uncovered branch as a prompt, find the input that reaches it, decide the correct result, and write a real assertion, which is the opposite of chasing a percentage. Coverage points at the gaps; closing them well is still the work of writing a good test.

There's no universal right number, either. A team that mandates ninety percent can reach it with shallow tests, while a parser handling untrusted input might warrant every branch and a plain data-holding struct almost none. The target worth aiming at is coverage of the logic that can be wrong: the branches, the boundaries, and the error paths, decided file by file rather than by one percentage applied everywhere. A threshold still has a place when it only forbids regressions, never letting coverage drop below where it already is, which catches a change that adds code without tests while leaving the judgment of how much is enough to the people writing them.

## What you learned

- Clang source-based coverage is a four-step pipeline: compile with `-fprofile-instr-generate -fcoverage-mapping`, run to emit a `.profraw`, merge with `llvm-profdata`, and report with `llvm-cov`.
- Line coverage asks whether a line ran, region coverage catches partially-run lines, and branch coverage asks whether each side of a conditional was taken; branch coverage is the hardest to fool.
- `llvm-cov show -show-branches=count` annotates the source and names the exact uncovered branch, turning a missed-branch number into a location.
- Run coverage as its own build lane, separate from the normal and sanitizer builds, because the instrumented binary is slower and single-purpose.
- Coverage is a floor, not a target: it shows what ran, not whether the result was asserted or whether an untested input would break.

The next chapter ties the tooling together into a reproducible pipeline: running the suite, both sanitizer lanes, and coverage in CI, so every change is checked the same way on every push.

## For deeper coverage

- The [Clang source-based coverage documentation](https://clang.llvm.org/docs/SourceBasedCodeCoverage.html): the instrumentation flags and the `llvm-profdata` and `llvm-cov` workflow.
- The [llvm-cov command guide](https://llvm.org/docs/CommandGuide/llvm-cov.html): `report`, `show`, and the region and branch options.
- The full reading list lives on the [bibliography page]({{ '/bibliography/' | relative_url }}).

---

*Verification status: <span class="status status--verified">verified</span>. On Fedora 44 (Clang 22.1.8, CMake 4.3.0, Conan 2.33.0, GoogleTest 1.18.0, LLVM 22.1.8), `examples/15-coverage` builds and reports 4/4 on `./demo.sh test`, `asan`, and `tsan`; `./coverage.sh` reports 100% region, line, and branch coverage for the full suite, and the subset report and annotated `show` output are quoted from real runs with host paths removed. Re-run all lanes before a release.*
