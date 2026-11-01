---
title: "Testing threaded code"
order: 11
part: "Testing concurrency"
description: "Write deterministic tests for concurrent code: launch threads with std::jthread, release them together with a latch to force overlap, synchronize with std::async and futures, and assert the aggregate that must hold no matter how the threads interleave."
duration: "35 min"
---

## Learning objectives

By the end of this chapter you can:

- Launch worker threads in a test with `std::jthread` and know when they join.
- Release threads together with a `std::latch` so their work overlaps and a race has the best chance to show.
- Use `std::async` and `std::future`, and recognize `get()` as the synchronization point.
- Assert an aggregate invariant that holds regardless of how the threads interleave.
- Explain why a passing concurrent test is evidence, not proof, and what fills the gap.

## Diagram

{% include excalidraw.html file="11-concurrent-test-anatomy" alt="A std::latch releases every worker at once. N worker threads each call record M times into a mutex-protected ShipmentLedger holding count and total. After the threads join, the test asserts count equals N times M and total equals N times M times the per-parcel weight." caption="Figure 11.1: A concurrent test releases N threads together, lets them contend, joins, and asserts the aggregate." %}

## Why this matters

Every test before this one ran on a single thread, where the order of operations is fixed and a result is the same on every run. Threaded code throws that away. Two threads incrementing the same counter can interleave in countless ways, and a test that happens to pass once may fail the next time with no change to the code. That nondeterminism is the whole difficulty: a test has to assert something true of every interleaving, not just the one that happened to run, and it has to be written so that a broken implementation is likely to reveal itself rather than slip through on a lucky schedule.

The way through is to separate what is deterministic from what is not. The schedule isn't deterministic, but the aggregate often is: if eight threads each record a thousand parcels, the final count must be exactly eight thousand no matter who ran when. A good concurrent test asserts that invariant, and it arranges the threads to collide as hard as possible so a missing lock turns into a wrong total. This chapter tests a thread-safe ledger that way, with the C++ standard threading tools and nothing exotic. It also sets up the two chapters that follow: the sanitizers that catch the race this test can only hint at, and the repetition that turns a flaky failure into a reliable one.

The code is in `examples/11-testing-threads/`. The run script there builds and runs it; its `README.md` covers how to drive each lane.

## How the code works

### The unit: a thread-safe ledger

`ShipmentLedger` accumulates a count and a total weight, and a mutex guards both:

```cpp
void ShipmentLedger::record(int grams) {
  const std::scoped_lock lock(mutex_);
  count_ += 1;
  total_grams_ += grams;
}

int ShipmentLedger::count() const {
  const std::scoped_lock lock(mutex_);
  return count_;
}
```

`record` takes the lock, updates both fields, and releases it when `lock` goes out of scope. `std::scoped_lock` is the RAII wrapper that acquires in its constructor and releases in its destructor, so the mutex is freed even if something between the braces throws. Both fields are updated under the one lock, which matters: a reader calling `count()` or `total_grams()` also takes the lock, so it never sees the count already bumped while the total is not. The `mutex_` is `mutable` because the read methods are `const` but still need to lock. This is the correct implementation the tests will confirm, and the one the chapter later breaks to watch the tests catch it.

A mutex isn't the only correct design. Because each field is a plain counter, the ledger could instead make `count_` and `total_grams_` into `std::atomic<int>` and drop the lock, trading the mutex for lock-free atomic additions. The test wouldn't change at all, which is the point: it asserts the observable aggregate, not the mechanism, so it validates either implementation and catches either one if it is wrong. The mutex is used here because it generalizes to invariants that span several fields: when two values must stay consistent with each other, a single atomic per field can't guarantee a reader sees a matching pair, and the lock can.

### Launching threads and forcing overlap

The core test runs many threads at once. A small helper launches them, releases them together, and joins them:

```cpp
void run_concurrently(int n_threads, const std::function<void(int)>& work) {
  std::latch start{1};
  std::vector<std::jthread> workers;
  workers.reserve(static_cast<std::size_t>(n_threads));
  for (int i = 0; i < n_threads; ++i) {
    workers.emplace_back([&work, &start, i] {
      start.wait();  // block until every worker is ready
      work(i);
    });
  }
  start.count_down();  // release all workers at once
}  // workers join here
```

Two standard tools carry this. A `std::jthread` is a thread that joins itself in its destructor, so when the `workers` vector is destroyed at the end of the function, every thread is joined without an explicit loop; it also won't leak a thread if the function throws. A `std::latch` is a one-shot countdown gate: each worker calls `start.wait()` and blocks, and the single `start.count_down()` drops the latch to zero and wakes them all at the same moment.

The latch is what makes the test adversarial. Without it, the first thread created might finish all its work before the last one starts, so the threads never actually overlap and a missing lock never gets exercised. Releasing them together maximizes the contention, which is exactly what you want when the goal is to provoke a race. A test that cannot make the threads collide cannot catch a bug that only appears when they do.

The `reserve` before the loop isn't incidental. Reserving the vector up front means emplacing a thread never reallocates the buffer, so the already-running `jthread` objects are not moved while their threads are live, which keeps the setup straightforward to reason about. For work that repeats in rounds rather than running once, `std::barrier` is the reusable cousin of the one-shot latch: it releases the threads at the start of each round and resets, where a latch fires a single time.

### Asserting the aggregate

With the helper in place, the main test records from eight threads and checks the totals:

```cpp
TEST(ShipmentLedger, ConcurrentRecordsAreAllCountedAndSummed) {
  ShipmentLedger ledger;
  constexpr int thread_count = 8;
  constexpr int records_per_thread = 1000;
  constexpr int grams = 250;

  run_concurrently(thread_count, [&ledger](int /*thread_index*/) {
    for (int j = 0; j < records_per_thread; ++j) {
      ledger.record(grams);
    }
  });

  EXPECT_EQ(ledger.count(), thread_count * records_per_thread);
  EXPECT_EQ(ledger.total_grams(), thread_count * records_per_thread * grams);
}
```

The interleaving of those eight thousand `record` calls is different on every run, but the assertion is not. Eight thousand calls must leave a count of eight thousand and a total of two million grams, because addition is associative and each call contributes exactly once. That is the invariant that holds across all schedules. A missing lock breaks it by losing updates: two threads read the same `count_`, both add one, and both write back the same value, so one increment vanishes and the final count comes in low. The assertion on the exact total is what turns that lost update into a failure.

### Returning a value: std::async and futures

Not all concurrency is fire-and-forget. When a worker computes something the caller needs, `std::async` runs it and hands back a `std::future` for the result:

```cpp
TEST(ShipmentLedger, AsyncWorkersReturnCountsThatMatchTheLedger) {
  ShipmentLedger ledger;
  const auto worker = [&ledger](int parcels, int grams) {
    for (int i = 0; i < parcels; ++i) {
      ledger.record(grams);
    }
    return parcels;
  };

  std::future<int> first = std::async(std::launch::async, worker, 300, 100);
  std::future<int> second = std::async(std::launch::async, worker, 700, 100);
  const int recorded = first.get() + second.get();  // get() joins both tasks

  EXPECT_EQ(recorded, 1000);
  EXPECT_EQ(ledger.count(), 1000);
  EXPECT_EQ(ledger.total_grams(), 100000);
}
```

`std::async(std::launch::async, ...)` runs the task on a new thread. The explicit `std::launch::async` policy matters: without it the implementation may defer the task and run it lazily on the calling thread when `get()` is called, which wouldn't be concurrent at all. Each call returns a `std::future<int>`, and `get()` blocks until the task finishes and then returns its value. That blocking is the synchronization: once both `get()` calls have returned, both workers have finished, so reading the ledger afterward is safe and sees all their records. Calling `get()` before reading the shared state is the ordering guarantee the test relies on; reading the ledger while a future was still pending would be a race of its own.

A future also carries an exception. If the task throws, the exception is stored in the shared state and rethrown when `get()` is called, so a worker that fails surfaces its error on the calling thread instead of terminating the process. A test can assert that with `EXPECT_THROW` wrapped around the `get()`, which is the threaded echo of the exception tests from the earlier chapter: the mechanism is the same, only the throw now crosses a thread boundary.

> **AI sidebar.** Asked to test threaded code, an assistant often writes a test that starts threads but never forces them to overlap, so the threads run one after another and a missing lock sails through. Check that the test has a real contention point, a latch or a barrier that releases the workers together, and not just a loop that creates and immediately joins each thread. The other frequent miss is `sleep`-based synchronization: a `std::this_thread::sleep_for` to "let the threads finish" is both slow and unreliable, and the fix is a latch, a future's `get()`, or a join, never a timed guess. Finally, a generated concurrent test tends to assert nothing stronger than "it did not crash." Push for an aggregate invariant, an exact count or sum that a lost update would violate, and ask whether the suite is also run under ThreadSanitizer, because a green run on one schedule is not evidence of thread safety on its own.

## Build, run, observe

Run the default lane:

```bash
[host]$ cd examples/11-testing-threads
[host]$ ./demo.sh test
```

```console
100% tests passed, 0 tests failed out of 3
```

Three tests: the single-threaded sanity check, the eight-thread aggregate, and the async workers. The important lane for this chapter is ThreadSanitizer, which instruments memory accesses to detect data races:

```bash
[host]$ ./demo.sh tsan
```

```console
100% tests passed, 0 tests failed out of 3
```

TSan passing matters more than the default lane here. The default run confirms the totals come out right on the schedule that happened to run; TSan confirms that no two threads touched shared state without synchronization, which is the property that makes the right totals hold on every schedule. The mutex is what earns both green results, and the next chapter is about what TSan reports when it is missing.

## See it fail

Removing the lock from `record`, so the increments run unguarded, breaks the aggregate. The count comes in short, and because the loss depends on timing, it is different every run:

```console
[host]$ ./build/unit_tests --gtest_filter='*Concurrent*'   # lock removed
Expected equality of these values:
  ledger.count()
    Which is: 3119
  thread_count * records_per_thread
    Which is: 8000
```

A second run reports `4182`, a third `3045`: the failure is real but its value is nondeterministic, which is the signature of a race. That flakiness is also the danger, because a count that is usually wrong might occasionally come out right and let the bug pass. ThreadSanitizer removes the luck. On the same broken code it reports the race directly, pointing at the unguarded write:

```console
WARNING: ThreadSanitizer: data race
  Write of size 4 at 0x... by thread T2:
    #0 shipping::ShipmentLedger::record(int) src/shipping/ledger.cpp:7
```

TSan flags the race whether or not the aggregate happened to come out wrong on that run, which is why it belongs in the test matrix for any threaded code. The next chapter is devoted to reading reports like this one.

## Cross-check

Listing the cases confirms the three tests in the one suite:

```console
[host]$ ./build/unit_tests --gtest_list_tests
ShipmentLedger.
  RecordsCountAndTotalSequentially
  ConcurrentRecordsAreAllCountedAndSummed
  AsyncWorkersReturnCountsThatMatchTheLedger
```

The names trace the progression: correctness on one thread, the aggregate under contention, and values returned through futures.

## What a passing run proves

A green concurrent run is weaker evidence than a green single-threaded one, and the reason sits in how threads run. A single run executes one interleaving out of an enormous number, and the scheduler picks which. If a race exists but the sampled schedule happened to avoid it, the test passes while the bug stays, which is the same flakiness the broken code showed, seen from the other side.

{% include excalidraw.html file="11-one-run-not-proof" alt="One test run exercises one interleaving of very many. That run can come out green because the schedule happened to work, while a race still hides in a schedule not sampled. ThreadSanitizer flags the race on any run, and repetition samples more schedules. A passing run narrows the doubt; the sanitizer and repetition close it." caption="Figure 11.2: One passing run samples a single schedule, so a race can hide; the sanitizer and repetition close the gap." %}

Two tools close the rest, and Figure 11.2 places them. ThreadSanitizer, the next chapter's subject, checks the memory model on whatever schedule it observes and flags an unsynchronized access even when the aggregate came out right, so one TSan run is worth many ordinary ones. Repetition, the chapter after that, runs the test enough times, with enough scheduling variation, to make a rare interleaving likely to surface. A concurrent test like this one is the first of the three, not the whole story: it states the invariant and forces contention, and the sanitizer and the repeated runs turn "passed once" into something closer to "holds in general."

## What you learned

- A concurrent test asserts an aggregate invariant that holds across all interleavings, like an exact count or sum, not just that the code ran.
- `std::jthread` joins in its destructor, so a vector of them joins when it goes out of scope; a `std::latch` releases the workers together to force overlap.
- `std::async(std::launch::async, ...)` returns a `std::future`, and `get()` is the synchronization point that must come before reading shared state.
- A missing lock loses updates and makes the total come out low and nondeterministic; the exact-total assertion catches it when the schedule cooperates.
- A passing concurrent run is evidence on one schedule, not proof; ThreadSanitizer and repetition, the next two chapters, close the gap.

The next chapter reads the sanitizers in depth: the data races TSan catches and the memory bugs ASan and UBSan find, each shown on code written to trip them.

## For deeper coverage

- [cppreference on `std::jthread`](https://en.cppreference.com/w/cpp/thread/jthread): the auto-joining thread and cooperative cancellation.
- [cppreference on `std::latch`](https://en.cppreference.com/w/cpp/thread/latch): the single-use countdown gate.
- [cppreference on `std::async`](https://en.cppreference.com/w/cpp/thread/async): launch policies and the returned future.
- *C++ Concurrency in Action* by Anthony Williams ([Manning](https://www.manning.com/books/c-plus-plus-concurrency-in-action-second-edition)): the standard thread library and testing concurrent code.
- The full reading list lives on the [bibliography page]({{ '/bibliography/' | relative_url }}).

---

*Verification status: <span class="status status--verified">verified</span>. On Fedora 44 (Clang 22.1.8, CMake 4.3.0, Conan 2.33.0, GoogleTest 1.18.0), `examples/11-testing-threads` builds and reports 3/3 on `./demo.sh test`, `asan`, and `tsan`; the lost-update failure and the ThreadSanitizer race report are quoted from real runs of the lock-removed code with host paths removed. Re-run all lanes before a release.*
