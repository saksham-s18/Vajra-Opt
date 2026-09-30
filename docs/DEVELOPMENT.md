# Vajra-Opt Development Guide

This document describes the environment setup, build workflows, testing practices, and engineering standards for developing **Vajra-Opt**.

---

## 1. Prerequisites and Toolchain

Vajra-Opt requires a modern C++20 toolchain:

| Tool | Minimum Version | Recommended Version | Purpose |
|:---|:---:|:---:|:---|
| **C++ Compiler** | GCC 12 / Clang 15 / MSVC 19.34+ | GCC 14+ / Clang 18+ / MSVC 19.40+ | C++20 core language support |
| **CMake** | 3.25 | 3.28+ | Project configuration and build generation |
| **Git** | 2.30+ | Latest | Version control and FetchContent downloads |
| **clang-format** | 15.0+ | 18.0+ | Code formatting automation |
| **NVIDIA CUDA** *(Optional)* | 12.0+ | 12.4+ / 13.x | Optional GPU acceleration |

---

## 2. Build Configurations

### 2.1 CPU Debug Build (Default for Local Development)

```bash
cmake -B build-debug \
    -DCMAKE_BUILD_TYPE=Debug \
    -DBUILD_TESTING=ON \
    -DVAJRA_ENABLE_WARNINGS_AS_ERRORS=ON

cmake --build build-debug --config Debug -j
```

### 2.2 CPU Release Build (Optimized)

```bash
cmake -B build-release \
    -DCMAKE_BUILD_TYPE=Release \
    -DBUILD_TESTING=ON

cmake --build build-release --config Release -j
```

### 2.3 Optional CUDA Build

CUDA support is strictly optional. If the NVIDIA CUDA Toolkit is installed:

```bash
cmake -B build-cuda \
    -DCMAKE_BUILD_TYPE=Release \
    -DVAJRA_ENABLE_CUDA=ON \
    -DBUILD_TESTING=ON

cmake --build build-cuda --config Release -j
```

### 2.4 Sanitizer Builds (AddressSanitizer + UndefinedBehaviorSanitizer)

On Linux/macOS with GCC or Clang:

```bash
cmake -B build-sanitizers \
    -DCMAKE_BUILD_TYPE=Debug \
    -DVAJRA_ENABLE_SANITIZERS=ON \
    -DBUILD_TESTING=ON

cmake --build build-sanitizers -j
ctest --test-dir build-sanitizers --output-on-failure
```

On Windows with MSVC:

```bash
cmake -B build-asan \
    -DCMAKE_BUILD_TYPE=Debug \
    -DVAJRA_ENABLE_SANITIZERS=ON \
    -DBUILD_TESTING=ON

cmake --build build-asan --config Debug
ctest --test-dir build-asan -C Debug --output-on-failure
```

---

## 3. Running Tests

### 3.1 Using CTest

```bash
ctest --test-dir build-debug -C Debug --output-on-failure
```

### 3.2 Running the Test Executable Directly

```bash
# Debug executable
./build-debug/tests/vajra_unit_tests

# Filter specific tests
./build-debug/tests/vajra_unit_tests --gtest_filter="ValidationTest.*"
```

---

## 4. Code Formatting

All source and header files must conform to the project `.clang-format` configuration.

### Check Formatting

```bash
clang-format --dry-run --Werror \
    include/vajra/*.hpp \
    src/core/*.cpp \
    tests/unit/*.cpp
```

### Apply Formatting

```bash
clang-format -i \
    include/vajra/*.hpp \
    src/core/*.cpp \
    tests/unit/*.cpp
```

---

## 5. Engineering Standards

1. **Clean-Room Development**: Do not copy or adapt source code from any existing optimization solver. Implement solely from mathematical specifications and published literature.
2. **Zero-Warning Policy**: All code must compile cleanly with `-Wall -Wextra -Wpedantic -Werror` (or `/W4 /WX` on MSVC).
3. **No Compromise on Tolerances**: Tolerances must reflect genuine mathematical precision (default feasibility: $10^{-8}$, integrality: $10^{-6}$). Never weaken tolerances to force a failing test to pass.
4. **Modular Separation**: Algorithmic code in solver engines must not directly depend on hardware specifics; all hardware acceleration goes through explicit abstraction layers.
5. **Differential Verification**: Any future GPU kernel must be verified against an exact CPU reference implementation.
