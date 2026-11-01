#!/usr/bin/env bash
# coverage.sh - build with LLVM source-based coverage, run the suite, and report.
# Usage: ./coverage.sh [extra unit_tests args, e.g. --gtest_filter=...]
#
# Builds into build-cov/ with -fprofile-instr-generate -fcoverage-mapping, runs the
# tests to produce a .profraw, merges it to .profdata, and prints a coverage report.
# Pass a --gtest_filter to see coverage for a subset of the suite.
set -euo pipefail
cd "$(dirname "$0")"

conan install . -of build -s build_type=Debug -s compiler.cppstd=23 --build=missing
cmake -S . -B build-cov -G Ninja \
  -DCMAKE_TOOLCHAIN_FILE="$(pwd)/build/conan_toolchain.cmake" \
  -DCMAKE_C_COMPILER=clang -DCMAKE_CXX_COMPILER=clang++ \
  -DCMAKE_BUILD_TYPE=Debug -DENABLE_CLANG_TIDY=OFF -DENABLE_COVERAGE=ON
cmake --build build-cov

LLVM_PROFILE_FILE=build-cov/unit_tests.profraw ./build-cov/unit_tests "$@"
llvm-profdata merge -sparse build-cov/unit_tests.profraw -o build-cov/unit_tests.profdata

echo
echo "== Coverage report =="
llvm-cov report ./build-cov/unit_tests \
  -instr-profile=build-cov/unit_tests.profdata \
  src/shipping/late_fee.cpp
echo
echo "To see annotated source with region and branch counts:"
echo "  llvm-cov show ./build-cov/unit_tests -instr-profile=build-cov/unit_tests.profdata \\"
echo "    -show-branches=count src/shipping/late_fee.cpp"
