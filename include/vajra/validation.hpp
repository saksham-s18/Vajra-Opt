#pragma once

#include "vajra/model.hpp"

#include <string>
#include <vector>

namespace vajra {

/// Result of model validation checks.
class ValidationResult {
  public:
    ValidationResult() = default;

    [[nodiscard]] bool is_valid() const noexcept { return errors_.empty(); }
    [[nodiscard]] const std::vector<std::string>& errors() const noexcept { return errors_; }
    [[nodiscard]] const std::vector<std::string>& warnings() const noexcept { return warnings_; }

    void add_error(std::string message) { errors_.push_back(std::move(message)); }
    void add_warning(std::string message) { warnings_.push_back(std::move(message)); }

    /// Formats errors and warnings into a readable summary string.
    [[nodiscard]] std::string to_string() const;

  private:
    std::vector<std::string> errors_;
    std::vector<std::string> warnings_;
};

/// Validate the mathematical and structural integrity of an optimization model.
/// Checks dimensions, bound validity (lb <= ub, lhs <= rhs), index bounds, and non-NaN values.
[[nodiscard]] ValidationResult validate_model(const Model& model);

} // namespace vajra
