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

Examples are listed here as each chapter ships. The toolchain template is in place;
chapter examples land starting in the next iteration.
