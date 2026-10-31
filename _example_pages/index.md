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

Chapters 1 and 2 build on the shared `examples/_template/` scaffold rather than a
chapter-specific example. More examples appear here as each chapter ships.
