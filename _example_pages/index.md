---
title: "Examples"
layout: plan
permalink: /examples/
description: "Every runnable example in this repo, mapped to the chapter it supports."
---

Every hands-on chapter has a matching runnable example under
[`examples/`](https://github.com/{{ site.github_username }}/{{ site.github_repo }}/tree/main/examples)
in the repo. Each is a copy of `examples/_template/` (the CMake + Conan + GoogleTest
toolchain) with the chapter's test code filled in, a `demo.sh` that builds and runs
it, and a README that verifies the behavior.

## Shipped examples

| Chapter | Example | What it verifies |
|---|---|---|
| Chapter 3: Your first test | [`examples/03-first-test`](https://github.com/{{ site.github_username }}/{{ site.github_repo }}/tree/main/examples/03-first-test) | Billable weight is the greater of a parcel's volumetric and actual weight. |
| Chapter 4: Assertions and expectations | [`examples/04-assertions`](https://github.com/{{ site.github_username }}/{{ site.github_repo }}/tree/main/examples/04-assertions) | Splitting a weight into shipments, verified with `ASSERT`/`EXPECT`, container comparison, and exception checks. |
| Chapter 5: Fixtures | [`examples/05-fixtures`](https://github.com/{{ site.github_username }}/{{ site.github_repo }}/tree/main/examples/05-fixtures) | A `Manifest` tested through a `TEST_F` fixture with shared `SetUp`, per-test isolation, and a fixture helper. |
| Chapter 6: Parameterized and typed tests | [`examples/06-parameterized-and-typed`](https://github.com/{{ site.github_username }}/{{ site.github_repo }}/tree/main/examples/06-parameterized-and-typed) | `delivery_band` over a `TEST_P` boundary table and `clamp_non_negative<T>` over a `TYPED_TEST` type list. |

Chapters 1 and 2 build on the shared `examples/_template/` scaffold rather than a
chapter-specific example. More examples appear here as each chapter ships.
