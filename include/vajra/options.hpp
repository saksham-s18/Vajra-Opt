#pragma once

#include "vajra/types.hpp"

#include <string_view>

namespace vajra {

/// Logging level for solver diagnostics and execution tracing.
enum class LogLevel { Off, Error, Warning, Info, Debug };

[[nodiscard]] constexpr std::string_view to_string(LogLevel level) noexcept {
    switch (level) {
    case LogLevel::Off:
        return "Off";
    case LogLevel::Error:
        return "Error";
    case LogLevel::Warning:
        return "Warning";
    case LogLevel::Info:
        return "Info";
    case LogLevel::Debug:
        return "Debug";
    }
    return "Unknown";
}

/// Solver configuration options and algorithmic parameters.
struct Options {
    // Numerical tolerances
    Real primal_feasibility_tolerance{1e-8};
    Real dual_feasibility_tolerance{1e-8};
    Real optimality_tolerance{1e-8};
    Real integrality_tolerance{1e-6};

    // Termination limits
    Real time_limit_seconds{3600.0};
    Int max_iterations{100000};

    // Hardware and parallel execution
    bool use_gpu{false};
    int device_id{0};
    int num_threads{1};

    // Diagnostics and output
    LogLevel log_level{LogLevel::Warning};
    bool print_to_console{false};
};

} // namespace vajra
