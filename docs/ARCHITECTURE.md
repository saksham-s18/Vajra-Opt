# Vajra-Opt Architecture Specification

## 1. System Overview

Vajra-Opt is architected as a modular, layered mathematical optimization solver designed for linear programming (LP), mixed-integer linear programming (MILP), and quadratic programming (QP/MIQP).

The system balances rigorous numerical robustness on CPU with high-throughput acceleration on modern NVIDIA GPUs.

```
                    +---------------------------+
                    |    Client Application     |
                    +---------------------------+
                                  |
                                  v
                    +---------------------------+
                    |     Public API (Model)    |
                    +---------------------------+
                                  |
                                  v
                    +---------------------------+
                    |  Model Validation Layer   |
                    +---------------------------+
                                  |
                                  v
                    +---------------------------+
                    |  Problem Classification   |  [PLANNED]
                    +---------------------------+
                                  |
              +-------------------+-------------------+
              |                   |                   |
              v                   v                   v
        +-----------+       +-----------+       +-----------+
        | LP Engine |       | QP Engine |       |   MILP    |  [PLANNED]
        +-----------+       +-----------+       +-----------+
              |                   |                   |
              +-------------------+-------------------+
                                  |
                                  v
                    +---------------------------+
                    |  Hardware Abstraction    |  [PLANNED]
                    |         Boundary          |
                    +---------------------------+
                                  |
                   +--------------+--------------+
                   |                             |
                   v                             v
       +-----------------------+     +-----------------------+
       |   CPU Linear Algebra  |     |  GPU Linear Algebra   |  [PLANNED]
       |  Sparse LU / Updates  |     |   CUDA FP64 Kernels   |
       +-----------------------+     +-----------------------+
                   |                             |
                   +--------------+--------------+
                                  |
                                  v
                    +---------------------------+
                    |    Postsolve & Unscaling  |  [PLANNED]
                    +---------------------------+
                                  |
                                  v
                    +---------------------------+
                    | Independent Verification  |  [PLANNED]
                    |        (KKT Check)        |
                    +---------------------------+
                                  |
                                  v
                    +---------------------------+
                    |     Solution Return       |
                    +---------------------------+
```

---

## 2. Component Breakdown

### 2.1 Public API (`include/vajra/`) — [IMPLEMENTED: Phase 0]
- **`Model`**: Problem representation supporting linear and quadratic objectives, row bounds ($lhs \le Ax \le rhs$), variable bounds ($lb \le x \le ub$), variable types (Continuous, Integer, Binary), and sparse matrices in CSC/COO format.
- **`SparseMatrix`**: Sparse storage in Compressed Sparse Column (CSC) and coordinate triplet (COO) forms with duplicate accumulation.
- **`Options`**: Algorithmic tolerances, iteration limits, wall-clock time limits, threading controls, and GPU enablement flags.
- **`Solution`**: Comprehensive output container storing `SolveStatus`, primal values $x$, dual multipliers $y$, reduced costs $rc$, iteration counts, and solve times.
- **`SolveStatus`**: Strongly typed termination outcomes (`Optimal`, `Infeasible`, `Unbounded`, `IterationLimit`, `TimeLimit`, `NumericalError`, `InvalidModel`, `NotImplemented`, etc.).
- **`solve()`**: Top-level solver entry point.

### 2.2 Model Validation Layer (`src/core/validation.cpp`) — [IMPLEMENTED: Phase 0]
Guarantees mathematical sanity before any solver algorithm commences:
- Dimensional consistency across vectors and matrices.
- Bounds consistency ($lb \le ub$, $lhs \le rhs$).
- Boundary sanity (disallows $lb = +\infty$ or $ub = -\infty$).
- Finite value verification (rejects `NaN` and unintended infinities).
- Sparse matrix validity (monotone column pointers, valid sorted row indices, non-empty bounds).
- Binary variable bound conformance ($x \in [0, 1]$).

### 2.3 Presolve and Scaling Subsystem — [PLANNED: Phase 3]
- **Presolve**: Empty row/col removal, singleton row/col processing, bound tightening, implied free variables, dual presolve.
- **Scaling**: Ruiz equilibration, geometric mean scaling, Curtis-Reid equilibrium to minimize matrix condition numbers.

### 2.4 Solver Engines — [PLANNED: Phases 1, 2, 4, 5]
1. **CPU Simplex Engine (Phase 1)**:
   - Revised Primal and Dual Simplex.
   - Sparse LU factorization with threshold partial pivoting.
   - Fast basis updates (Forrest-Tomlin or Bartels-Golub).
   - Harris two-pass ratio test for numerical stability.
   - Steepest-edge and devex pricing strategies.
2. **GPU First-Order Solver Engine (Phase 2 & 4)**:
   - Primal-Dual Hybrid Gradient (PDHG / Chambolle-Pock) for massive-scale LP.
   - Adaptive step sizes and Halpern acceleration.
   - Custom CUDA FP64 SpMV kernels and residual reduction kernels.
3. **Quadratic Programming Engine**:
   - Active-set method for small/medium QPs.
   - ADMM / Operator splitting for large-scale convex QPs.
4. **Mixed-Integer Engine (MIP) (Phase 5)**:
   - Branch-and-Bound framework.
   - Gomory mixed-integer cuts and knapsack covers.
   - Strong branching and pseudocost branching heuristics.

### 2.5 Hardware Abstraction Boundary — [PLANNED: Phase 2]
- The solver algorithms interface with linear algebra through abstract algebraic primitives (`SpMV`, `SpMV_T`, `Solve_LU`, `InnerProduct`, `Norms`).
- The CPU backend implements these using cache-optimized routines and multicore OpenMP parallelization.
- The GPU backend implements these using asynchronous CUDA streams, pinned memory transfers, and FP64 arithmetic kernels.

### 2.6 Independent Solution Verification (KKT Checker) — [PLANNED: Phase 1+]
Every generated candidate solution will be checked against Karush-Kuhn-Tucker (KKT) optimality conditions:
1. **Primal Feasibility**: $\| \max(0, lb - x) \|_\infty \le \epsilon_{\text{primal}}$, $\| \max(0, x - ub) \|_\infty \le \epsilon_{\text{primal}}$, $\| \max(0, lhs - Ax, Ax - rhs) \|_\infty \le \epsilon_{\text{primal}}$.
2. **Dual Feasibility**: Verification of dual multiplier signs and reduced cost sign consistency.
3. **Complementary Slackness**: Verification that active constraints and non-basic variables satisfy complementarity.

---

## 3. Implementation Phasing Roadmap

| Phase | Milestone | Scope | Status |
|:---:|:---|:---|:---:|
| **0** | **Project Foundation** | Architecture, C++20 CMake build, core types, validation, GTest suite, CI | **COMPLETED** |
| **1** | **CPU Linear Algebra & Simplex** | Sparse LU factorization, basis updates, Revised Dual Simplex for LP | *Next* |
| **2** | **GPU Linear Algebra & Acceleration** | CUDA FP64 kernels, GPU memory management, differential testing | *Scheduled* |
| **3** | **Presolve & Matrix Scaling** | LP presolver, Ruiz scaling, postsolve recovery | *Scheduled* |
| **4** | **First-Order Methods (PDHG)** | GPU-accelerated PDHG for large-scale LPs | *Scheduled* |
| **5** | **Mixed Integer & QP Extensions** | Branch-and-Bound, cutting planes, convex QP solver | *Scheduled* |
