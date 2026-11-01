# 16-ci — the test matrix, run the same way locally and in CI

The example for Chapter 16. A small `delivery_zone` lookup stands in for the code
under test; the subject is the pipeline around it. `ci.sh` runs the whole matrix,
default plus both sanitizer lanes plus a coverage gate, and `workflows/tests.yml` is
the GitHub Actions translation of the same four stages.

## Layout

```
16-ci/
├── CMakeLists.txt        # the usual lanes plus an ENABLE_COVERAGE option
├── demo.sh               # ./demo.sh [test|build|asan|tsan] — one lane
├── ci.sh                 # the full matrix: default, asan, tsan, coverage gate
├── coverage.sh           # instrumented build + llvm-cov, gated at MIN_COVERAGE
├── workflows/tests.yml   # the GitHub Actions workflow (copy to .github/workflows/)
├── src/shipping/         # zones.hpp / zones.cpp
└── tests/                # zones_test.cpp
```

## Run it

```
[host]$ ./ci.sh                    # everything, exactly what CI runs
[host]$ MIN_COVERAGE=95 ./ci.sh    # raise the coverage gate
[host]$ ./demo.sh asan             # a single lane, as during development
```

`ci.sh` runs the default lane, the ASan/UBSan lane, and the TSan lane in turn, then
builds an instrumented binary and fails if region coverage falls below the gate
(90% by default). The GitHub Actions workflow runs the three lanes as a matrix and
coverage as its own job, so the same checks run on every push.

## What it verifies

`delivery_zone` returns zone 1 for domestic, 2 for EU and UK, and 3 otherwise. The
three tests cover every branch, so all three lanes pass and coverage is 100%, above
the gate. Running `ci.sh` locally produces the same result a CI run would.

## Verification

<div class="status status--verified">
  <strong>Verified</strong> on Fedora 44 (Clang 22.1.8, CMake 4.3.0, Conan 2.33.0,
  GoogleTest 1.18.0, LLVM 22.1.8): <code>./ci.sh</code> runs all four stages green
  (3/3 on each lane, 100% region coverage above the 90% gate). The workflow in
  <code>workflows/tests.yml</code> passes <code>actionlint</code>.
</div>
