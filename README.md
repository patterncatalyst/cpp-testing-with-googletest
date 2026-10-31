# Testing C++ with GoogleTest

A chaptered, runnable guide to testing modern C++ (C++20/23) with GoogleTest and
GoogleMock on Fedora 44 — from your first test to concurrency, sanitizers, and CI.
Built as a Jekyll site in the Pattern Catalyst house style, with one runnable
example per hands-on chapter.

**Live site:** https://patterncatalyst.github.io/cpp-testing-with-googletest/

## What it covers

From an empty directory to a test suite that mocks its dependencies, runs clean
under ASan/UBSan/TSan, and executes in CI. Six parts, sixteen chapters
(0–15): prerequisites, foundations (CMake presets, CTest, assertions, fixtures),
beyond the basics (parameterized/typed/death tests, modern C++), mocking with
GoogleMock, testing concurrency, and the tooling and workflow around it (GDB,
llvm-cov, CI).

## Toolchain

Fedora 44, local tools only — no containers, no VM. Clang (primary) with GCC as a
cross-check, CMake + Ninja with presets, Conan 2 for dependencies, GoogleTest +
GoogleMock pinned via Conan, and the ASan/UBSan and TSan sanitizer lanes.

```
examples/_template/     # CMake + Conan + gtest toolchain each example copies
scripts/check-prereqs.sh
```

Run `scripts/check-prereqs.sh` first; it exits non-zero on anything missing.

## Build the site locally

```
bundle install
bundle exec jekyll serve --baseurl ""
```

## Status

Scaffolding (r00). Chapters land iteration by iteration; each example ships
`unverified` until it has been built and run, and a per-chapter footer records the
behavioral evidence.

## License

Code is licensed under **Apache-2.0** (see [LICENSE](LICENSE)). The prose, diagrams,
and other written content are licensed under **CC BY 4.0** (see
[LICENSE-docs.md](LICENSE-docs.md)).
