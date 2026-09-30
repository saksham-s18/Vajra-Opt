#pragma once

#include <string_view>

namespace vajra {

/// Solver termination status enum.
enum class SolveStatus {
    /// Optimal solution found within tolerances.
    Optimal,
    /// Problem proven mathematically infeasible.
    Infeasible,
    /// Problem proven unbounded (objective improves indefinitely).
    Unbounded,
    /// Solver determined problem is either infeasible or unbounded.
    InfeasibleOrUnbounded,
    /// Iteration limit reached before convergence.
    IterationLimit,
    /// Wall-clock time limit reached before convergence.
    TimeLimit,
    /// Numerical instability or singularity encountered.
    NumericalError,
    /// Model failed validation checks (invalid dimensions, bounds, or indices).
    InvalidModel,
    /// Requested solver algorithm or subsystem is not yet implemented.
    NotImplemented,
    /// Solver was interrupted or cancelled by the user.
    UserCancelled,
    /// Unknown or uninitialized status.
    Unknown
};

/// Convert SolveStatus to human-readable string view.
[[nodiscard]] constexpr std::string_view to_string(SolveStatus status) noexcept {
    switch (status) {
    case SolveStatus::Optimal:
        return "Optimal";
    case SolveStatus::Infeasible:
        return "Infeasible";
    case SolveStatus::Unbounded:
        return "Unbounded";
    case SolveStatus::InfeasibleOrUnbounded:
        return "InfeasibleOrUnbounded";
    case SolveStatus::IterationLimit:
        return "IterationLimit";
    case SolveStatus::TimeLimit:
        return "TimeLimit";
    case SolveStatus::NumericalError:
        return "NumericalError";
    case SolveStatus::InvalidModel:
        return "InvalidModel";
    case SolveStatus::NotImplemented:
        return "NotImplemented";
    case SolveStatus::UserCancelled:
        return "UserCancelled";
    case SolveStatus::Unknown:
        return "Unknown";
    }
    return "Unknown";
}

} // namespace vajra
