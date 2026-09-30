# Vajra-Opt Provenance, Clean-Room Policy, and Citations

## 1. Clean-Room Implementation Policy

**Vajra-Opt** is an original, from-scratch mathematical optimization solver developed for the Smart India Hackathon (SIH 2026, Problem Statement SIH26119: *“Indigenous GPU-Accelerated Optimization Solver”*).

### 1.1 Strict Non-Copying Rule

No source code, header files, internal representations, or proprietary structures have been copied, adapted, decompiled, or translated from any existing optimization solver, including but not limited to:
- SANKHYA
- HiGHS
- SoPlex / SCIP
- COIN-OR Clp / Cbc
- GLPK
- lp_solve
- OSQP
- PDLP / cuPDLP
- Commercial solvers (Gurobi, CPLEX, FICO Xpress, Mosek)

Any prior study of existing open-source solvers was strictly for conceptual understanding of:
- High-level optimization algorithms and standard mathematical formulations.
- Architectural component separation (presolve, basis updates, pricing, postsolve).
- Common numerical failure modes and degenerate pivoting phenomena.
- Benchmark methodologies and standard testing corpora.

### 1.2 Development Grounding

All algorithmic and mathematical components in Vajra-Opt are derived and implemented strictly from:
1. Canonical mathematical formulations of optimization problems.
2. Peer-reviewed textbooks in numerical linear algebra and mathematical programming.
3. Published academic papers and conference proceedings with full attribution.

---

## 2. Mathematical References and Literature Grounding

The mathematical and algorithmic foundation of Vajra-Opt is grounded in the following foundational literature:

### 2.1 Linear Programming & The Simplex Method
- **Dantzig, G. B.** (1963). *Linear Programming and Extensions*. Princeton University Press.
- **Bixby, R. E.** (2002). "Solving real-world linear programs: A decade and more of progress." *Operations Research*, 50(1), 3-15.
- **Forrest, J. J., & Tomlin, J. A.** (1972). "Updated triangular factors of the basis to maintain sparsity in the revised simplex method." *Mathematical Programming*, 2(1), 263-278.
- **Bartels, R. H., & Golub, G. H.** (1969). "The simplex method of linear programming using LU decomposition." *Communications of the ACM*, 12(5), 266-268.
- **Harris, P. M.** (1973). "Pivot selection methods of the Devex LP code." *Mathematical Programming*, 5(1), 1-28.
- **Koberstein, A.** (2008). "The dual simplex method, techniques for a fast and stable implementation." *PhD Thesis*, University of Paderborn.
- **Huangfu, Q., & Hall, J. A. J.** (2018). "Parallelizing the dual revised simplex method." *Mathematical Programming Computation*, 10(1), 119-142.

### 2.2 First-Order Methods & GPU Acceleration
- **Chambolle, A., & Pock, T.** (2011). "A first-order primal-dual algorithm for convex problems with applications to imaging." *Journal of Mathematical Imaging and Vision*, 40(1), 120-145.
- **Applegate, D., Díaz, M., Hinder, O., Lu, H., Lubin, M., O'Donoghue, B., & Schaller, W.** (2021). "Practical Large-Scale Linear Programming using Primal-Dual Hybrid Gradient." *Advances in Neural Information Processing Systems (NeurIPS)*.
- **Lu, H., Yang, P., & Hinder, O.** (2023). "cuPDLP.jl: A GPU Implementation of Primal-Dual Hybrid Gradient for Linear Programming." *arXiv preprint*.

### 2.3 Sparse Linear Algebra & Presolve
- **Davis, T. A.** (2006). *Direct Methods for Sparse Linear Systems*. SIAM Fundamentals of Algorithms.
- **Andersen, E. D., & Andersen, K. D.** (1995). "Presolving in linear programming." *Mathematical Programming*, 71(2), 221-245.
- **Ruiz, D.** (2001). "A scaling algorithm to equilibrate both rows and columns norms in matrices." *Rapport de recherche du laboratoire RAL*, Rutherford Appleton Laboratory.

### 2.4 Mixed-Integer Linear & Quadratic Programming
- **Nemhauser, G. L., & Wolsey, L. A.** (1988). *Integer and Combinatorial Optimization*. John Wiley & Sons.
- **Achterberg, T.** (2007). *Constraint Integer Programming*. PhD Thesis, TU Berlin.
- **Nocedal, J., & Wright, S. J.** (2006). *Numerical Optimization*. Springer Series in Operations Research and Financial Engineering.

---

## 3. Benchmark Corpora Attribution

When benchmark testing suites are integrated in future phases (e.g. Netlib LP, Mittelmann benchmarks, MIPLIB 2017), test instances will retain their respective original author citations, copyrights, and usage terms without modification.

---

## 4. Policy for Future Contributors

All contributors to Vajra-Opt must agree to adhere strictly to this clean-room implementation policy. Pull requests containing verbatim or near-verbatim code from external optimization solvers will be rejected immediately.
