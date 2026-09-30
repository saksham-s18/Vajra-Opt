#pragma once

#include <cstdint>
#include <limits>
#include <string_view>

namespace vajra {

/// Integer index and counter type used across all sparse representations and dimensions.
using Int = std::int64_t;

/// Floating-point real precision (FP64 / double standard).
using Real = double;

/// Mathematical infinity representation for bounds.
inline constexpr Real kInfinity = std::numeric_limits<Real>::infinity();

/// Optimization objective direction.
enum class ObjectiveSense { Minimize, Maximize };

/// Variable integrality classification.
enum class VariableType { Continuous, Integer, Binary };

/// Convert ObjectiveSense to string representation.
[[nodiscard]] constexpr std::string_view to_string(ObjectiveSense sense) noexcept {
    switch (sense) {
    case ObjectiveSense::Minimize:
        return "Minimize";
    case ObjectiveSense::Maximize:
        return "Maximize";
    }
    return "Unknown";
}

/// Convert VariableType to string representation.
[[nodiscard]] constexpr std::string_view to_string(VariableType type) noexcept {
    switch (type) {
    case VariableType::Continuous:
        return "Continuous";
    case VariableType::Integer:
        return "Integer";
    case VariableType::Binary:
        return "Binary";
    }
    return "Unknown";
}

} // namespace vajra
