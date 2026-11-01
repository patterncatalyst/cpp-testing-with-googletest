---
title: "Races and memory bugs"
order: 12
part: "Testing concurrency"
description: "Read what the sanitizers report. Run the suite under AddressSanitizer, UndefinedBehaviorSanitizer, and ThreadSanitizer, and see each one catch a bug written to trip it: a dangling view, a signed overflow, and a data race."
duration: "35 min"
---

## Learning objectives

By the end of this chapter you can:

- Say what each sanitizer finds: ASan for memory errors, UBSan for undefined behavior, TSan for data races.
- Run a suite under the two sanitizer lanes and know why ASan and TSan need separate binaries.
- Read a sanitizer report: the error kind, the faulting access, and the stacks that explain it.
- Recognize the bugs each one catches, shown here as a dangling view, a 32-bit overflow, and a shared accumulator.
- Treat a sanitizer lane as part of the test matrix, not a tool you reach for only after something breaks.

## Diagram

{% include excalidraw.html file="12-three-sanitizers" alt="AddressSanitizer and UndefinedBehaviorSanitizer share one binary. ASan finds memory errors like use-after-free, buffer overflow, and leaks, and catches a dangling span here. UBSan finds undefined behavior like overflow, bad shifts, and misalignment, and catches a 32-bit sum overflow. ThreadSanitizer is a separate binary that finds data races and catches a shared accumulator. ASan and UBSan combine in one build; TSan cannot join them." caption="Figure 12.1: Three sanitizers, what each finds, and the bug each catches in this example." %}

## Why this matters

The previous chapter ended on a gap: a concurrent test passes on the one schedule it happened to run, so a race can hide behind a green result. Sanitizers close that gap, and not only for races. They are compiler instrumentation that watches the program as it runs and reports the moment it does something undefined, whether or not that undefined thing produced a visibly wrong answer. A read past the end of a buffer, a use of freed memory, an integer overflow, a data race: each is a bug that can pass a test by luck, and each is caught the instant it happens under the right sanitizer.

That is why the sanitizer lanes have been in the build from the first chapter. A test asserts the result you thought to check; a sanitizer checks the things you did not think to, on the same run. This chapter takes the three functions of a small weight library, each written correctly so the whole suite passes clean on every lane, and then breaks each one to show the matching sanitizer catching it. The point isn't the bugs themselves; it's learning to read what the tools say, so when a real report appears you can find the cause quickly.

The code is in `examples/12-sanitizers/`. The run script there builds and runs it; its `README.md` covers how to drive each lane.

## How the code works

### The code under test

Three functions operate on a batch of parcel weights passed as a `std::span<const int>`, a non-owning view over a contiguous sequence. Each is written to be correct:

```cpp
long long total_weight(std::span<const int> weights) {
  long long sum = 0;  // 64-bit: a large batch cannot overflow this
  for (const int weight : weights) {
    sum += weight;
  }
  return sum;
}

int max_weight(std::span<const int> weights) {
  int largest = weights[0];
  for (std::size_t i = 1; i < weights.size(); ++i) {  // in bounds: [1, size)
    largest = std::max(weights[i], largest);
  }
  return largest;
}
```

`total_weight` accumulates into a `long long` so a batch whose 32-bit sum would overflow is still summed correctly. `max_weight` scans from index one to just below `size()`, staying in bounds. The third function, `parallel_total`, sums two halves on two threads, each into its own partial, and adds the results, so there's no shared state. The tests confirm all of this on every lane, and the suite passes clean under the default build, under ASan with UBSan, and under TSan. What follows breaks each function to show what the sanitizer says.

### UndefinedBehaviorSanitizer: a signed overflow

Signed integer overflow is undefined behavior in C++, not a wraparound you can rely on. Change `total_weight` to accumulate into a 32-bit `int` and feed it the large batch the test already covers, a hundred parcels of a billion grams, and UBSan reports the exact operation that overflowed:

```console
weights.cpp: runtime error: signed integer overflow:
2000000000 + 1000000000 cannot be represented in type 'int'
```

The report names the undefined operation, the two operands, and the type that could not hold the result. This is a bug a plain test might miss: on many machines the overflow wraps to a negative number and the program keeps running with a wrong total, which an assertion on the result would catch only if it checked that specific large batch. UBSan catches the overflow itself, regardless of what the wrapped value happened to be, which is why it belongs on a lane rather than in a single assertion.

The overflow is undefined only because the accumulator is signed. Unsigned overflow is defined to wrap in C++, so the same loop over an `unsigned` would not be a UBSan finding; it would wrap silently, which is worse, because no tool flags defined-but-wrong behavior. The fix is not to switch to unsigned to quiet the tool, but to use a type wide enough for the real range, which is what the correct `long long` does.

UBSan can also run in a trapping mode, where a check that fires executes an illegal instruction and aborts instead of printing a diagnosis. That mode suits a hardened production build, where you want the program to stop rather than carry on in an undefined state. For testing, the reporting mode used here is the one you want, because its operand-level message tells you what overflowed and by how much.

### AddressSanitizer: a dangling view

A `std::span` doesn't own its data; it points at memory owned elsewhere. That makes it fast and makes a dangling span easy to write: return one that outlives the container it views, and every read touches freed memory. A caller that builds a batch in a local vector and returns a span over it has exactly this bug:

```cpp
std::span<const int> make_batch() {
  std::vector<int> local{100, 250, 300};
  return local;  // the vector's heap buffer is freed when local is destroyed
}
// total_weight(make_batch()) then reads freed memory
```

Run that under AddressSanitizer and it reports a heap-use-after-free, with three stacks that together tell the whole story:

```console
ERROR: AddressSanitizer: heap-use-after-free ... READ of size 4 ...
    #0 shipping::total_weight(...) weights.cpp:10
freed by thread T0 here:
    ... std::vector<int>::~vector() ... make_batch()
previously allocated by thread T0 here:
    ... std::vector<int>::vector(...) ... make_batch()
SUMMARY: AddressSanitizer: heap-use-after-free weights.cpp:10 in total_weight
```

{% include excalidraw.html file="12-report-anatomy" alt="A sanitizer report in four parts. First the error kind and faulting access: heap-use-after-free, READ of size 4. Then where the bad access happened: frame zero in total_weight in weights.cpp. Then where the memory was freed: the vector destructor in make_batch. Then where it was allocated: the vector constructor. Read the three stacks together." caption="Figure 12.2: An ASan report reads top to bottom: what went wrong, where, then the free and the allocation that frame it." %}

Figure 12.2 lays out how to read it. The first block is the error kind, heap-use-after-free, and the faulting access, a read of four bytes, with the stack showing it happened inside `total_weight`. The second block, freed by, shows where that memory was released: the vector's destructor running at the end of `make_batch`. The third, previously allocated by, shows where it came from: the vector's constructor. Reading the three together turns a crash into a cause: the memory `total_weight` read was the vector's buffer, freed when the local vector died, allocated when the batch was built. The fix is in the caller, not in `total_weight`, and the report points straight at it. ASan also finds buffer overflows and leaks, and reads them the same way. A leak report lists only the allocation stack, because there's no faulting access to point at, and it prints at program exit rather than at a point of use. A use-after-return, reading a local after its function has returned, reads like the use-after-free above, with a stack-after-return region in place of a freed heap one.

### ThreadSanitizer: a data race

The previous chapter saw TSan flag a missing lock. The same tool catches the same class of bug here in different clothes. Rewrite `parallel_total` so both threads add into one shared accumulator with no synchronization, and TSan reports the race:

```console
WARNING: ThreadSanitizer: data race
  Write of size 8 ... by thread T1:
    #0 shipping::parallel_total(...)::operator() weights.cpp:29
  Previous write of size 8 ... by main thread:
    #0 shipping::parallel_total(...)::operator() weights.cpp:29
SUMMARY: ThreadSanitizer: data race weights.cpp:29 in parallel_total
```

The two stacks are the two conflicting accesses: a write by thread T1 and a previous write by the main thread, both at the same line, the unsynchronized `total += weight`. TSan doesn't need the race to produce a wrong answer; it detects that two threads touched the same memory with no happens-before relation between them, which is the definition of the race. That is the property the correct version preserves by giving each thread its own partial, and the one the aggregate test alone couldn't guarantee.

### Why two binaries

The sanitizers aren't all interchangeable in one build. ASan and UBSan share instrumentation and run together in a single binary, which is why the `asan-ubsan` lane carries both. TSan uses a different, incompatible shadow-memory scheme and must be its own binary, which is why there is a separate `tsan` lane and the two never combine. The project's CMake presets encode this: `asan-ubsan` and `tsan` build into different directories, and the demo script runs them as two passes.

Running both costs time, and so does each lane on its own. ASan roughly doubles memory use and runs a program around twice as slow; TSan is heavier still, often five to fifteen times slower. That cost is fine for a test suite, which is small and bounded, and it is why the sanitizers go on the test binaries and not the shipped build: a release build turns them off, and the test matrix turns them on. The lanes also pass a few options through environment variables. The demo script sets `halt_on_error=1` so the first finding stops the run rather than letting a damaged program stumble onward, and `abort_on_error=1` under ASan so a finding fails the test process; `UBSAN_OPTIONS=print_stacktrace=1` adds a stack to each UBSan report, which otherwise prints only its one-line diagnosis. The defaults are quiet, and a report without a stack is much harder to act on.

### What the sanitizers do not catch

A sanitizer only checks the code a run actually executes. It's dynamic instrumentation, not static analysis, so a use-after-free on a branch no test takes is one it never sees. That makes sanitizer coverage a function of test coverage: the more inputs and schedules the suite drives, the more the tools can watch. It is the previous chapter's lesson from the other side. A race TSan did not report may only mean the racing schedule never ran, and a clean ASan lane over a thin suite proves little. Sanitizers multiply the value of good tests; they don't replace them, and the next chapter is about driving enough runs to give them something to see.

> **AI sidebar.** An assistant asked to "fix the sanitizer error" tends to silence it rather than fix it: adding a suppression, widening a buffer by one, or wrapping the line in a cast that hides the overflow without preventing it. Read the report yourself first and find the cause the three stacks point at, then judge whether a proposed fix addresses that cause or just quiets the tool. Ask specifically whether a suggested change removes the undefined behavior or merely stops ASan from noticing, because the two look similar in a diff and behave very differently in production. The opposite failure is a model that treats a sanitizer finding as noise and proposes disabling the lane; a sanitizer report is a real bug on the schedule or input that triggered it, and the fix is in the code, not the build flags.

## Build, run, observe

The example's correct code passes both sanitizer lanes:

```bash
[host]$ cd examples/12-sanitizers
[host]$ ./demo.sh asan
[host]$ ./demo.sh tsan
```

```console
100% tests passed, 0 tests failed out of 5
```

That both lanes are green is the result to want: the functions are correct, so there is nothing to report. The reports above come from breaking each function in turn, which is how you learn to read them before a real one appears. A clean run under ASan and UBSan says the suite triggered no memory error or undefined behavior on the inputs it exercised, and a clean TSan run says no two threads raced on the schedule it observed. Neither is a proof over all inputs, but each is far stronger than a default run, because it checks the undefined things the assertions never mention.

## Cross-check

Listing the cases confirms the five tests, the correct behavior the sanitizers agree with:

```console
[host]$ ./build/unit_tests --gtest_list_tests
TotalWeight.
  SumsABatch
  HandlesALargeBatchWithoutOverflow
MaxWeight.
  FindsTheLargestInTheBatch
  HandlesASingleElement
ParallelTotal.
  MatchesTheSequentialTotal
```

The large-batch test and the parallel-total test are the two that matter most here: the first is the input that makes a 32-bit overflow visible, and the second is the computation TSan watches for a race.

## What you learned

- Sanitizers are runtime instrumentation that reports undefined behavior as it happens, catching bugs a result-only assertion can miss.
- UBSan reports the exact undefined operation, like a signed overflow with its operands and type; it catches the overflow even when the wrapped value looks plausible.
- ASan reports a memory error with three stacks, the faulting access plus where the memory was freed and allocated, which together name the cause.
- TSan reports a data race as two conflicting accesses with no synchronization between them, whether or not the race corrupted the result.
- ASan with UBSan is one binary and TSan is another; run both lanes as part of the test matrix, not only after a failure.

The next chapter turns to the tests that fail only sometimes: making concurrent tests deterministic, repeating them to flush out rare interleavings, and hunting the flaky failures that remain.

## For deeper coverage

- The [AddressSanitizer documentation](https://clang.llvm.org/docs/AddressSanitizer.html): the memory errors it finds and how to read its reports.
- The [UndefinedBehaviorSanitizer documentation](https://clang.llvm.org/docs/UndefinedBehaviorSanitizer.html): the checks it runs and how to enable them.
- The [ThreadSanitizer documentation](https://clang.llvm.org/docs/ThreadSanitizer.html): the data-race detector and its report format.
- The full reading list lives on the [bibliography page]({{ '/bibliography/' | relative_url }}).

---

*Verification status: <span class="status status--verified">verified</span>. On Fedora 44 (Clang 22.1.8, CMake 4.3.0, Conan 2.33.0, GoogleTest 1.18.0), `examples/12-sanitizers` builds and reports 5/5 on `./demo.sh test`, `asan`, and `tsan`; the UBSan, ASan, and TSan reports are quoted from real runs of the broken variants with host paths removed. Re-run all lanes before a release.*
