# Vajra-Opt: Indigenous GPU-Accelerated Optimization Solver

[![CI](https://github.com/saksham-s18/Vajra-Opt/actions/workflows/ci.yml/badge.svg)](https://github.com/saksham-s18/Vajra-Opt/actions/workflows/ci.yml)
[![C++20](https://img.shields.io/badge/standard-C%2B%2B20-blue.svg)](https://en.cppreference.com/w/cpp/20)
[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](LICENSE)

**Vajra-Opt** is a high-performance, from-scratch mathematical optimization solver engineered for linear and quadratic mathematical programming, featuring modular architecture, sparse numerical computation, multicore CPU parallelization, and NVIDIA CUDA GPU acceleration.

> **Problem Statement**: SIH 2026 — SIH26119  
> *“Indigenous GPU-Accelerated Optimization Solver”*

---

## Project Objectives

The long-term objective of Vajra-Opt is to provide a production-grade optimization engine capable of solving:

1. **Linear Programming (LP)**: $\min/\max\ c^T x \quad \text{s.t.} \quad l \le A x \le u, \quad lb \le x \le ub$
2. **Mixed Integer Linear Programming (MILP)**: Linear programming with integrality constraints $x_j \in \mathbb{Z}$.
3. **Convex Quadratic Programming (QP)**: Convex quadratic objective $c^T x + \frac{1}{2} x^T Q x$ with linear constraints.
4. **Mixed Integer Quadratic Programming (MIQP)**: Quadratic programming with integrality constraints.

### Key Architectural Pillars

- **Correctness First**: Strict numerical verification against KKT optimality conditions before performance optimization.
- **Clean-Room Implementation**: Built from first mathematical principles and published peer-reviewed literature without copying code from existing solvers (see [docs/PROVENANCE.md](docs/PROVENANCE.md)).
- **Hardware Abstraction Boundary**: Clean separation between mathematical optimization logic and backend linear algebra engines (CPU sparse linear algebra vs. GPU CUDA kernels).
- **Optional CUDA**: 100% of core algorithms will build and execute cleanly on CPU-only workstations without CUDA installed.

---

## Current Status: Phase 0 (Project Foundation)

Vajra-Opt is currently in **Phase 0 — Project Foundation**.

| Component | Status | Details |
|:---|:---:|:---|
| **Build System & Toolchain** | **Operational** | CMake 3.25+, C++20, optional CUDA, multi-config (Debug/Release) |
| **Public Core Interfaces** | **Operational** | `Model`, `Options`, `Solution`, `SolveStatus`, `SparseMatrix` (CSC/COO) |
| **Model Validation Layer** | **Operational** | Dimension consistency, range sanity, NaN/Inf detection, sparsity checks |
| **Central Solver Seam** | **Operational** | `vajra::solve(model, options)` entry point returning structured status |
| **Unit Test Suite** | **Operational** | GoogleTest integration with 100% pass rate on foundation types |
| **Continuous Integration** | **Operational** | GitHub Actions workflow for multi-platform build, tests, and formatting |
| *CPU Simplex Engine (Primal/Dual)* | *Planned (Phase 1)* | Under active development |
| *GPU Linear Algebra & CUDA Kernels* | *Planned (Phase 2)* | Under active development |
| *Presolve & Scaling Subsystem* | *Planned (Phase 3)* | Under active development |
| *First-Order Methods (PDHG)* | *Planned (Phase 4)* | Under active development |
| *Branch-and-Bound / Cuts (MIP)* | *Planned (Phase 5)* | Under active development |

*Note: Algorithmic solver routines (simplex, interior point, PDHG, branch-and-bound) are not yet implemented in Phase 0. Calling `vajra::solve()` on a valid model currently returns `SolveStatus::NotImplemented`.*

---

## Building from Source

### Prerequisites

- **C++ Compiler**: Supporting C++20 (GCC 12+, Clang 15+, or MSVC 19.34+ / VS 2022+)
- **CMake**: Version 3.25 or higher
- **Git**: For source versioning and FetchContent dependencies
- **NVIDIA CUDA Toolkit** *(Optional)*: CUDA 12.0+ for GPU acceleration (CPU build works completely without CUDA)

### Quick Start (CPU Build)

```bash
# Clone the repository
git clone https://github.com/saksham-s18/Vajra-Opt.git
cd Vajra-Opt

# Configure with CMake (Debug configuration)
cmake -B build -DCMAKE_BUILD_TYPE=Debug -DBUILD_TESTING=ON

# Build the project
cmake --build build --config Debug

# Run the test suite
ctest --test-dir build -C Debug --output-on-failure
```

### Release Build

```bash
cmake -B build-release -DCMAKE_BUILD_TYPE=Release -DBUILD_TESTING=ON
cmake --build build-release --config Release
ctest --test-dir build-release -C Release --output-on-failure
```

### Optional CUDA Build

To enable NVIDIA GPU acceleration support (when CUDA is installed):

```bash
cmake -B build-cuda -DVAJRA_ENABLE_CUDA=ON -DCMAKE_BUILD_TYPE=Release
cmake --build build-cuda --config Release
```

---

## Code Formatting

Vajra-Opt enforces strict code formatting using `clang-format`:

```bash
# Check formatting
clang-format --dry-run --Werror include/vajra/*.hpp src/core/*.cpp tests/unit/*.cpp

# Apply formatting in-place
clang-format -i include/vajra/*.hpp src/core/*.cpp tests/unit/*.cpp
```

---

## Repository Structure

```
Vajra-Opt/
├── .clang-format         # LLVM C++20 code formatting configuration
├── .gitignore            # Git exclusion rules for artifacts
├── .github/
│   └── workflows/
│       └── ci.yml        # GitHub Actions CI workflow
├── CMakeLists.txt        # Root CMake configuration
├── README.md             # Project overview and instructions
├── LICENSE               # MIT License
├── docs/
│   ├── ARCHITECTURE.md   # System architecture and roadmap
│   ├── DEVELOPMENT.md    # Developer guide and contribution workflows
│   └── PROVENANCE.md     # Clean-room implementation and citations policy
├── include/
│   └── vajra/            # Public C++20 API headers
│       ├── model.hpp
│       ├── options.hpp
│       ├── solution.hpp
│       ├── solver.hpp
│       ├── sparse_matrix.hpp
│       ├── status.hpp
│       ├── types.hpp
│       ├── validation.hpp
│       └── vajra.hpp
├── src/
│   └── core/             # Core foundation implementation
│       ├── model.cpp
│       ├── solver.cpp
│       ├── sparse_matrix.cpp
│       └── validation.cpp
└── tests/
    ├── CMakeLists.txt    # Test suite build definition
    └── unit/             # GoogleTest unit test cases
```

---

## Documentation

- [System Architecture](docs/ARCHITECTURE.md)
- [Development and Contributing Guide](docs/DEVELOPMENT.md)
- [Provenance and Academic Citations](docs/PROVENANCE.md)

---

## License

Vajra-Opt is licensed under the [MIT License](LICENSE).
