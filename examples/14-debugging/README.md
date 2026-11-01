# 14-debugging: debugging a failing test under GDB

The example for Chapter 14. `shipping::tiered_cost_cents` charges by weight tier,
and `batch_cost_cents` sums the tiers across a batch. Both are small enough to step
through, which is the point: the chapter plants a boundary bug in the tier logic and
walks through finding it in GDB, using a conditional breakpoint, `print`, `next`,
and `backtrace`.

## Layout

```
14-debugging/
├── CMakeLists.txt        # builds the shipping lib + unit_tests target (Debug: -g)
├── demo.sh               # ./demo.sh [test|build|asan|tsan]
├── src/shipping/         # rates.hpp / rates.cpp: tiered_cost_cents, batch_cost_cents
└── tests/                # rates_test.cpp: one case per tier boundary
```

## Run it

```
[host]$ ./demo.sh test     # default lane: configure, build, ctest
[host]$ ./demo.sh asan     # ASan + UBSan
[host]$ ./demo.sh tsan     # ThreadSanitizer
```

## Debug it

```
[host]$ gdb -q --args ./build/unit_tests --gtest_filter=TieredCost.ChargesByWeightTier
(gdb) break shipping::tiered_cost_cents if weight_grams == 2000
(gdb) run
(gdb) print weight_grams
(gdb) next
(gdb) backtrace
```

## What it verifies

`tiered_cost_cents` charges each tier correctly, tested at every boundary (the top
of one tier and the bottom of the next), and treats a non-positive weight as the
lightest tier. `batch_cost_cents` sums the tiered cost across a batch and returns
zero for an empty one. The chapter breaks the middle tier boundary to show the
failing test and the GDB session that locates the bug.

## Verification

<div class="status status--verified">
  <strong>Verified</strong> on Fedora 44 (Clang 22.1.8, CMake 4.3.0, Conan 2.33.0,
  GoogleTest 1.18.0, GDB 18.1): <code>./demo.sh test</code>, <code>asan</code>, and
  <code>tsan</code> each report 4/4 passing from a clean build.
</div>
