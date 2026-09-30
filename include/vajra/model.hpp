#pragma once

#include "vajra/sparse_matrix.hpp"
#include "vajra/types.hpp"

#include <optional>
#include <string>
#include <vector>

namespace vajra {

/// Optimization model representation supporting LP, MILP, and QP formulations.
///
/// Mathematical Formulation:
///   min/max   c^T x + 1/2 x^T Q x + c_0
///   s.t.      lhs <= A x <= rhs
///             lb <= x <= ub
///             x_j in Real, Integer, or {0, 1}
class Model {
  public:
    Model() = default;
    explicit Model(std::string name);

    // Model identification
    [[nodiscard]] const std::string& name() const noexcept { return name_; }
    void set_name(std::string name) { name_ = std::move(name); }

    // Objective specification
    [[nodiscard]] ObjectiveSense objective_sense() const noexcept { return sense_; }
    void set_objective_sense(ObjectiveSense sense) noexcept { sense_ = sense; }

    [[nodiscard]] Real objective_offset() const noexcept { return objective_offset_; }
    void set_objective_offset(Real offset) noexcept { objective_offset_ = offset; }

    [[nodiscard]] const std::vector<Real>& objective_coefficients() const noexcept {
        return objective_coefficients_;
    }
    void set_objective_coefficients(std::vector<Real> c);

    // Variable definitions
    [[nodiscard]] Int num_variables() const noexcept {
        return static_cast<Int>(variable_lower_bounds_.size());
    }

    [[nodiscard]] const std::vector<Real>& variable_lower_bounds() const noexcept {
        return variable_lower_bounds_;
    }
    [[nodiscard]] const std::vector<Real>& variable_upper_bounds() const noexcept {
        return variable_upper_bounds_;
    }
    [[nodiscard]] const std::vector<VariableType>& variable_types() const noexcept {
        return variable_types_;
    }
    [[nodiscard]] const std::vector<std::string>& variable_names() const noexcept {
        return variable_names_;
    }

    void set_variable_bounds(std::vector<Real> lower, std::vector<Real> upper);
    void set_variable_types(std::vector<VariableType> types);
    void set_variable_names(std::vector<std::string> names);

    /// Add a single variable and return its index.
    Int add_variable(Real lb = 0.0, Real ub = kInfinity, Real obj = 0.0,
                     VariableType type = VariableType::Continuous, std::string name = "");

    // Constraint definitions
    [[nodiscard]] Int num_constraints() const noexcept {
        return static_cast<Int>(constraint_lower_bounds_.size());
    }

    [[nodiscard]] const std::vector<Real>& constraint_lower_bounds() const noexcept {
        return constraint_lower_bounds_;
    }
    [[nodiscard]] const std::vector<Real>& constraint_upper_bounds() const noexcept {
        return constraint_upper_bounds_;
    }
    [[nodiscard]] const std::vector<std::string>& constraint_names() const noexcept {
        return constraint_names_;
    }

    void set_constraint_bounds(std::vector<Real> lower, std::vector<Real> upper);
    void set_constraint_names(std::vector<std::string> names);

    /// Add a single constraint row (lhs <= a^T x <= rhs) and return its index.
    Int add_constraint(Real lhs = -kInfinity, Real rhs = kInfinity, std::string name = "");

    // Linear constraint matrix A (m x n)
    [[nodiscard]] const SparseMatrix& constraint_matrix() const noexcept {
        return constraint_matrix_;
    }
    void set_constraint_matrix(SparseMatrix matrix);
    [[nodiscard]] Int num_nonzeros() const noexcept { return constraint_matrix_.num_nonzeros(); }

    // Quadratic objective matrix Q (n x n, optional)
    [[nodiscard]] bool has_quadratic_objective() const noexcept {
        return quadratic_matrix_.has_value();
    }
    [[nodiscard]] const std::optional<SparseMatrix>& quadratic_matrix() const noexcept {
        return quadratic_matrix_;
    }
    void set_quadratic_matrix(SparseMatrix Q);
    void clear_quadratic_matrix() noexcept { quadratic_matrix_.reset(); }

    /// Reset the entire model to an empty state.
    void clear() noexcept;

  private:
    std::string name_{"VajraModel"};
    ObjectiveSense sense_{ObjectiveSense::Minimize};
    Real objective_offset_{0.0};

    // Linear objective vector c
    std::vector<Real> objective_coefficients_;

    // Variable properties (size n)
    std::vector<Real> variable_lower_bounds_;
    std::vector<Real> variable_upper_bounds_;
    std::vector<VariableType> variable_types_;
    std::vector<std::string> variable_names_;

    // Constraint properties (size m)
    std::vector<Real> constraint_lower_bounds_;
    std::vector<Real> constraint_upper_bounds_;
    std::vector<std::string> constraint_names_;

    // Constraint matrix A (m x n)
    SparseMatrix constraint_matrix_{0, 0};

    // Optional quadratic objective matrix Q (n x n)
    std::optional<SparseMatrix> quadratic_matrix_{std::nullopt};
};

} // namespace vajra
