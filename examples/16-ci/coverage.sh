#!/usr/bin/env bash
# coverage.sh - instrumented build + llvm-cov report, gated at a minimum percentage.
# Set MIN_COVERAGE to change the gate (default 90). Exits non-zero below the gate.
set -euo pipefail
cd "$(dirname "$0")"

MIN_COVERAGE="${MIN_COVERAGE:-90}"

conan install . -of build -s build_type=Debug -s compiler.cppstd=23 --build=missing
cmake -S . -B build-cov -G Ninja \
  -DCMAKE_TOOLCHAIN_FILE="$(pwd)/build/conan_toolchain.cmake" \
  -DCMAKE_C_COMPILER=clang -DCMAKE_CXX_COMPILER=clang++ \
  -DCMAKE_BUILD_TYPE=Debug -DENABLE_CLANG_TIDY=OFF -DENABLE_COVERAGE=ON
cmake --build build-cov

LLVM_PROFILE_FILE=build-cov/unit_tests.profraw ./build-cov/unit_tests
llvm-profdata merge -sparse build-cov/unit_tests.profraw -o build-cov/unit_tests.profdata

region=$(llvm-cov report ./build-cov/unit_tests \
  -instr-profile=build-cov/unit_tests.profdata src/shipping/zones.cpp \
  | awk '/^TOTAL/ {gsub("%","",$4); print $4}')
echo "region coverage: ${region}% (gate ${MIN_COVERAGE}%)"
awk -v got="$region" -v min="$MIN_COVERAGE" 'BEGIN { exit (got+0 >= min+0) ? 0 : 1 }' \
  || { echo "FAIL: coverage ${region}% is below the ${MIN_COVERAGE}% gate" >&2; exit 1; }
