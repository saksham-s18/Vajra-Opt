#pragma once

#include "vajra/status.hpp"
#include "vajra/types.hpp"

#include <string>
#include <vector>

namespace vajra {

/// Optimization result returned by the solver.
struct Solution {
    /// Termination status of the solve.
    SolveStatus status{SolveStatus::Unknown};

    /// Objective function value at the termination point.
    Real objective_value{0.0};

    /// Primal variable assignments (x vector of size n).
    std::vector<Real> primal_variables;

    /// Dual multipliers for constraints (y vector of size m).
    std::vector<Real> dual_variables;

    /// Reduced costs for variables (z / rc vector of size n).
    std::vector<Real> reduced_costs;

    /// Total simplex/interior-point/B&B iterations executed.
    Int iteration_count{0};

    /// Wall-clock solve time in seconds.
    Real solve_time_seconds{0.0};

    /// Informational message describing termination cause or validation errors.
    std::string status_message;

    /// Check if the solve terminated with an optimal status.
    [[nodiscard]] bool is_optimal() const noexcept { return status == SolveStatus::Optimal; }

    /// Check if a primal variable solution vector is available.
    [[nodiscard]] bool has_primal_solution() const noexcept { return !primal_variables.empty(); }

    /// Check if dual multipliers are available.
    [[nodiscard]] bool has_dual_solution() const noexcept { return !dual_variables.empty(); }
};

} // namespace vajra
