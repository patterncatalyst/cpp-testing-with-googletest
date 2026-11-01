---
title: "Setup"
order: 1
part: "Prerequisites"
description: "Install and prove the exact Fedora 44 toolchain every example in this book builds and tests with: Clang, CMake, Ninja, Conan, GDB, and GoogleTest via Conan."
duration: "20 min"
---

## Learning objectives

By the end of this chapter you can:

- Install the exact local toolchain every example builds with on Fedora 44: Clang and the Clang tools, CMake, Ninja, Conan 2, and GDB.
- Run `conan profile detect` once and explain what it records.
- Verify the toolchain with `scripts/check-prereqs.sh` and read its output against a dated known-good version matrix.
- Build and test any example through the `default`, `asan-ubsan`, and `tsan` lanes, and explain why the sanitizers need separate build directories.
- Tear the build output and the Conan cache back down to a clean machine.

## Diagram

{% include excalidraw.html file="01-build-and-test-loop" alt="The local build-and-test loop: edit source, conan install resolves GoogleTest, cmake configures with Ninja, cmake builds with Clang under -Werror, ctest runs the GoogleTest cases, you observe the result, and a failing test sends you back to the source. The same loop runs in three lanes with separate build directories: default, asan-ubsan, and tsan." caption="Figure 1.1: The build-and-test loop every example runs, and the three lanes that share it." %}

## Why this matters

Every chapter in this book hands you real, compiling, tested C++. That promise is only as good as the toolchain behind it, so this chapter makes the toolchain real on your machine before a single test shows up. There's no container here to paper over a missing compiler or a stale CMake. You install Clang, CMake, Ninja, Conan, and GDB directly on Fedora 44, point Conan at a profile that describes your machine once, and from there every example builds the same four-step way: `conan install`, `cmake --preset`, `cmake --build`, `ctest`. Learn that loop once and it carries the whole book.

The code is in `examples/_template/`. It is the minimal GoogleTest project every later example is a copy of, and running it is how you prove your setup works. Its `README.md` covers what it contains and how to drive it.

## Why local tools, no containers

A container is a fine way to pin a toolchain, and it's also a way to hide the thing this book wants you to watch: what Clang, CMake, Conan, and GoogleTest actually do on a real machine, and how they hand off to each other. A reader who only ever typed `docker compose up` never sees `CMakePresets.json` pick a generator, never sees Conan resolve a dependency graph, and never builds the instinct for reading a failed configure step. Testing lives at that level. So the reference platform for this book is Fedora 44 with the toolchain installed straight onto the host, and nothing in any chapter asks you to run a container or to become root beyond the one `dnf install` below.

## The toolchain, tool by tool

Six tools do the work, and each has one job.

**Clang** is the primary compiler. Version 22 is the first release where the C++23 library surface this book leans on is solid enough to teach from without caveats, and it is the compiler every example was written and verified against. The Clang tools ride along with it: `clang-tidy` runs static analysis on every compile, `clang-format` enforces one house style, and `clangd` is the language server both editors use.

**GCC** is a cross-check, not the default. GCC 16 compiles everything here correctly, and building an example under it once in a while is a cheap way to catch a construct that Clang accepts but the standard does not require. The shipped presets point at Clang because that's the single compiler the examples are verified against release to release.

**CMake** describes each build: which files compile into which target, which standard applies, and which runner picks up the result. This book uses CMake presets, so you never type a long configure line by hand; you name a preset and CMake reads the rest from `CMakePresets.json`.

**Ninja** is the generator CMake hands its build graph to. It rebuilds faster than Make on the one-line-change-then-rerun cycle you live in while working through a chapter, and every preset names it explicitly.

**Conan** is the one piece an experienced C++ developer may not have met, because C++ went most of its history without a widely adopted package manager. Conan 2 resolves this book's test dependency, GoogleTest, downloads or builds the exact pinned version, and writes a CMake toolchain file that `cmake --preset` consumes automatically. The pin lives in each example's `conanfile.py` as `gtest/1.18.0`, with Abseil pinned transitively, so every reader resolves the identical library.

**GDB** is the debugger. You won't need it until the chapter on debugging failing tests, but it installs in the same step as everything else, so it's listed here and set aside until then.

GoogleTest itself isn't a tool you install. It arrives through Conan the first time you build, which is exactly the point of using a package manager: the framework is a pinned dependency of the project, not a system package you manage by hand. `FetchContent` and Fedora's own `gtest-devel` package are both workable alternatives, covered later as alternatives, never as the build path this book takes.

## Installing on Fedora 44

Everything except Conan comes straight from Fedora's repositories in one command:

```bash
[host]$ sudo dnf install -y clang clang-tools-extra lld cmake ninja-build gdb gcc-c++ git python3-pip pipx
```

`clang-tools-extra` is what provides `clang-tidy`, `clang-format`, and `clangd`; `lld` is Clang's companion linker; `gcc-c++` is the cross-check compiler described above. Install Conan with `pipx`, which gives it an isolated environment and tracks upstream Conan's own release cadence rather than waiting on a distribution rebuild:

```bash
[host]$ pipx install conan
[host]$ pipx ensurepath   # first time only, so ~/.local/bin is on PATH
```

Open a fresh shell so the updated `PATH` takes effect, then confirm what landed:

```bash
[host]$ conan --version
Conan version 2.33.0
```

Any Conan 2 release works. This book floors at 2.32 and verifies against 2.33.0; nothing later in the 2.x line changes the commands you run.

## The one-time step: `conan profile detect`

Before Conan can resolve anything, it needs a profile describing the machine it runs on: operating system, compiler, compiler version, architecture, and default build type. Generate one the first time you work in the repository:

```bash
[host]$ conan profile detect
```

Conan inspects your installed compiler, which is Clang on a default Fedora 44 setup, and writes the result to `~/.conan2/profiles/default`. Every `conan install` this book asks you to run reads that file implicitly, so you never pass compiler flags to Conan by hand. If you later reinstall your compiler or move to a new Fedora release, rerun the command with `--force` rather than hand-editing the generated file. The profile is per-user, not per-repository, so you generate it once and every project on the machine shares it; a teammate who clones this book runs the same one-time step on their own machine, and the pinned dependency in `conanfile.py` keeps both of you on the identical GoogleTest regardless.

## Verifying your setup

`scripts/check-prereqs.sh` checks each tool, prints the version it found, and exits non-zero on anything missing or too old. It touches nothing and builds nothing, so it is safe as both a first local step and a CI gate:

```console
[host]$ ./scripts/check-prereqs.sh
Checking C++ toolchain prerequisites...
  clang: clang version 22.1.8 (Fedora 22.1.8-4.fc44)
  clang++: clang version 22.1.8 (Fedora 22.1.8-4.fc44)
  clang-tidy: LLVM (http://llvm.org/):
  clang-format: clang-format version 22.1.8 (Fedora 22.1.8-4.fc44)
  clangd: clangd version 22.1.8 (Fedora 22.1.8-4.fc44)
  cmake: cmake version 4.3.0
  ninja: 1.13.2
  conan: Conan version 2.33.0
  git: git version 2.56.0

All prerequisites present.
```

Your own versions will be whatever `dnf` and `pipx` installed, which is expected. The script names the specific missing or too-old tool when something falls short of the floor, so a clear message is the first signal of a problem rather than a mysterious CMake or Conan failure three steps later. The known-good toolchain this book is verified against, as of October 2026 on Fedora 44:

| Tool | Version | Install |
|---|---|---|
| Clang / `clang-tidy` / `clang-format` / `clangd` | 22.1.8 | `dnf install clang clang-tools-extra` |
| CMake | 4.3.0 | `dnf install cmake` |
| Ninja | 1.13.2 | `dnf install ninja-build` |
| Conan | 2.33.0 | `pipx install conan` |
| GDB | 18.1 | `dnf install gdb` |
| GCC / `g++` (cross-check) | 16.2.1 | `dnf install gcc-c++` |
| GoogleTest | 1.18.0 | via Conan (`conanfile.py` pin) |
| git | 2.56.0 | `dnf install git` |

## The build loop every example follows

Every example builds and tests the same way, so learning it on `examples/_template` teaches it for the whole book. Each example carries a `demo.sh` that wraps the four real steps behind one lane name:

```bash
[host]$ cd examples/_template
[host]$ ./demo.sh test     # default lane: configure, build, run the suite
[host]$ ./demo.sh asan     # AddressSanitizer + UndefinedBehaviorSanitizer
[host]$ ./demo.sh tsan     # ThreadSanitizer, in its own binary
```

Underneath, `./demo.sh test` runs `conan install . -of build -s build_type=Debug -s compiler.cppstd=23 --build=missing`, which resolves GoogleTest the first time and writes a CMake toolchain file to `build/conan_toolchain.cmake`. `cmake --preset default` then configures against that toolchain file using Ninja, `cmake --build --preset default` compiles the library and its tests under Clang with `-Wall -Wextra -Wpedantic -Werror`, and `ctest --preset default --output-on-failure` runs the suite. CTest finds every case through `gtest_discover_tests`, so it reports pass or fail per test, not just per binary.

Each preset in `CMakePresets.json` is a named bundle of exactly that configure step: the Ninja generator, the build directory, and the path to the Conan toolchain file at `build/conan_toolchain.cmake`. Naming a preset is how you avoid retyping those settings, and it's why one command works for every example without edits. The configure, build, and test presets share a name, so `cmake --preset default`, `cmake --build --preset default`, and `ctest --preset default` all act on the same configured tree. When you want to see the real commands a lane runs rather than the `demo.sh` wrapper, open that file; it's a short, readable shell script, and every chapter's example ships the same one.

The three lanes are the same loop with different instrumentation, and each builds into its own directory. The `default` lane uses `build/` and gives you the fastest edit-build-test cycle. The `asan-ubsan` lane uses `build-asan/` and links AddressSanitizer and UndefinedBehaviorSanitizer together. The `tsan` lane uses `build-tsan/` and links ThreadSanitizer. ASan and TSan cannot coexist in one binary, which is why they are two presets and two directories rather than a single instrumented build, and why continuous integration later runs the suite in two passes. Keeping each lane in its own directory also means switching between them never forces a full reconfigure.

A green run is a checkpoint, not the finish line. A suite that compiles and passes while asserting nothing meaningful is still broken, so every example's `README.md` verifies the behavior it claims, including showing a test fail when the code under test is wrong. That discipline starts in the next chapter and holds through the last.

## The first build, and what Conan caches

The first time you run a lane, Conan builds GoogleTest from source, so that run takes a minute or two longer than every run after it. Conan caches the compiled library under `~/.conan2/`, keyed by the exact version and your profile, so the second build and every build after it reuse the cache and the `conan install` step returns almost immediately. The version isn't left to chance. Each example's `conanfile.py` pins it exactly:

```python
def requirements(self):
    self.requires("gtest/1.18.0")
```

Pinning a single version rather than a range is what makes a clone reproducible: the test you watch pass here is built against the same GoogleTest the next reader resolves, down to the patch release. GoogleTest 1.18 depends on Abseil, and Conan pins that transitively, so the dependency is resolved for you and never becomes something you manage by hand.

Analysis runs on the way through, not as a separate step. The build turns `clang-tidy` on for every compile and treats its findings as errors, the same way it treats compiler warnings, so a lint problem fails the build before it can reach a test. The `.clang-tidy` and `.clang-format` files at the repository root hold those rules once for every example, which is why a file you format or a check you configure behaves identically across the whole book. You don't run either tool by hand in the normal loop; configuring a lane is enough to apply both.

## Using an editor

Both editors drive the same presets the command line does, with nothing project-specific to configure. In **CLion**, open the repository folder; it detects `CMakePresets.json` and offers `default`, `asan-ubsan`, and `tsan` in the configuration selector, with full breakpoint debugging against the Conan-generated toolchain. In **VS Code** with the CMake Tools and clangd extensions, run **CMake: Select Configure Preset** from the command palette and pick the same lane; the Test Explorer then lists every case CTest discovered. Because the build directory and toolchain file are identical whether you build from an editor or a terminal, you can switch between them mid-chapter without reconfiguring.

## Teardown

Local tools leave nothing running between sessions, so there's no service to stop. To reclaim disk, remove the build directories, and clear the Conan cache only when you want to force a full re-download:

```bash
[host]$ rm -rf build build-asan build-tsan
[host]$ conan remove "*" --confirm   # optional: clears the cached GoogleTest
```

> **AI sidebar.** Ask an assistant to "set up a C++ test project" and it tends to reach for whatever it saw most often: GoogleTest pulled in with CMake's `FetchContent`, a bare `CMakeLists.txt` with no presets, and, as often as not, a `Dockerfile` you never asked for. Check three things before accepting it. The versions it pins should be real and current rather than a plausible-looking guess; it should not add a second test framework alongside GoogleTest; and it should match this book's shape, Conan-pinned `gtest`, CMake presets, and no container. A `Dockerfile` or an unpinned dependency is the usual sign it reached for a generic template instead of your setup.

## What you learned

- The six-tool local toolchain this book builds on, what each tool is for, and the exact versions it is verified against on Fedora 44.
- The one-time `conan profile detect` step and why every `conan install` depends on it.
- The four-step build loop (`conan install`, `cmake --preset`, `cmake --build`, `ctest`) and the three lanes that share it, each in its own build directory because ASan and TSan cannot share a binary.

With the toolchain proven, the next chapter asks what a test actually buys you and walks the same toolchain from the testing side.

## For deeper coverage

- The [GoogleTest documentation](https://google.github.io/googletest/): the authoritative reference for the framework every example uses, from first assertion to GoogleMock.
- The [Conan 2 documentation](https://docs.conan.io/2/): profiles, the `conanfile.py`, and how the generated CMake toolchain file is consumed by presets.
- *Professional CMake: A Practical Guide* by Craig Scott ([Crascit](https://crascit.com/professional-cmake/)): presets, targets, and the CTest integration this book relies on.
- The full reading list behind this book lives on the [bibliography page]({{ '/bibliography/' | relative_url }}).

---

*Verification status: <span class="status status--verified">verified</span>. On Fedora 44 (Clang 22.1.8, CMake 4.3.0, Ninja 1.13.2, Conan 2.33.0, GDB 18.1, GCC 16.2.1, GoogleTest 1.18.0), `scripts/check-prereqs.sh` reports every tool present, and `examples/_template` builds and tests 4/4 under `./demo.sh test`, `asan`, and `tsan`. Re-confirm the version matrix against upstream before each release; the install block is the standard Fedora 44 path.*
