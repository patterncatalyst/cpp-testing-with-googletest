# Template CMake build every example copies.
# C++23, warnings-as-errors, Conan-provided GoogleTest, sanitizer lanes,
# clang-tidy on every compile: a small library under test plus its test target.
#
# Build via presets (see CMakePresets.json), which consume the Conan toolchain:
#   conan install . -of build -s build_type=Debug --build=missing
#   cmake --preset default && cmake --build --preset default
#   ctest --preset default --output-on-failure
cmake_minimum_required(VERSION 3.25)
project(gtest_example LANGUAGES CXX)

# ── Language standard: C++23, required, no compiler extensions ────────────────
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# ── Options ───────────────────────────────────────────────────────────────────
option(ENABLE_CLANG_TIDY "Run clang-tidy on every compile" ON)
option(ENABLE_ASAN_UBSAN "Build with AddressSanitizer + UndefinedBehaviorSanitizer" OFF)
option(ENABLE_TSAN       "Build with ThreadSanitizer (never combine with ASAN_UBSAN)" OFF)

if(ENABLE_ASAN_UBSAN AND ENABLE_TSAN)
  message(FATAL_ERROR "ASan/UBSan and TSan cannot be combined. Use one sanitizer lane.")
endif()

# ── Warnings-as-errors for every target ──────────────────────────────────────
add_library(project_warnings INTERFACE)
target_compile_options(project_warnings INTERFACE
  -Wall -Wextra -Wpedantic -Werror -Wshadow -Wconversion)

# ── Sanitizer flags, applied per lane ────────────────────────────────────────
add_library(project_sanitizers INTERFACE)
if(ENABLE_ASAN_UBSAN)
  target_compile_options(project_sanitizers INTERFACE
    -fsanitize=address,undefined -fno-omit-frame-pointer -g)
  target_link_options(project_sanitizers INTERFACE -fsanitize=address,undefined)
endif()
if(ENABLE_TSAN)
  target_compile_options(project_sanitizers INTERFACE
    -fsanitize=thread -fno-omit-frame-pointer -g)
  target_link_options(project_sanitizers INTERFACE -fsanitize=thread)
endif()

# ── clang-tidy on every compile (gated so a fast build can skip it) ───────────
if(ENABLE_CLANG_TIDY)
  find_program(CLANG_TIDY_EXE NAMES clang-tidy)
  if(CLANG_TIDY_EXE)
    set(CMAKE_CXX_CLANG_TIDY "${CLANG_TIDY_EXE}")
  else()
    message(WARNING "clang-tidy not found; analysis disabled for this build")
  endif()
endif()

# ── Dependencies, resolved from Conan's CMakeDeps ─────────────────────────────
find_package(GTest REQUIRED)
# This example starts threads (std::jthread, std::async), so it links the system
# threads library. On Linux this is pthreads.
set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)

# ── Library under test ────────────────────────────────────────────────────────
add_library(shipping src/shipping/ledger.cpp)
target_include_directories(shipping PUBLIC src)
target_link_libraries(shipping
  PRIVATE project_warnings project_sanitizers PUBLIC Threads::Threads)

# ── Tests ─────────────────────────────────────────────────────────────────────
enable_testing()
add_executable(unit_tests tests/ledger_test.cpp)
target_link_libraries(unit_tests
  PRIVATE shipping GTest::gtest_main GTest::gmock project_warnings project_sanitizers
          Threads::Threads)

include(GoogleTest)
gtest_discover_tests(unit_tests
  XML_OUTPUT_DIR ${CMAKE_BINARY_DIR}/test-results)
