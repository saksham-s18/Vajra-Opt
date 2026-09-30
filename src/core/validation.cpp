#include "vajra/validation.hpp"

#include <cmath>
#include <sstream>

namespace vajra {

std::string ValidationResult::to_string() const {
    if (is_valid()) {
        if (warnings_.empty()) {
            return "Model validation passed successfully.";
        }
        std::ostringstream oss;
        oss << "Model validation passed with " << warnings_.size() << " warning(s):\n";
        for (const auto& w : warnings_) {
            oss << "  [WARNING] " << w << "\n";
        }
        return oss.str();
    }

    std::ostringstream oss;
    oss << "Model validation failed with " << errors_.size() << " error(s):\n";
    for (const auto& err : errors_) {
        oss << "  [ERROR] " << err << "\n";
    }
    for (const auto& w : warnings_) {
        oss << "  [WARNING] " << w << "\n";
    }
    return oss.str();
}

ValidationResult validate_model(const Model& model) {
    ValidationResult result;

    const Int num_vars = model.num_variables();
    const Int num_constrs = model.num_constraints();

    // 1. Variable dimension checks
    const auto& lb = model.variable_lower_bounds();
    const auto& ub = model.variable_upper_bounds();
    const auto& obj = model.objective_coefficients();
    const auto& vtypes = model.variable_types();
    const auto& vnames = model.variable_names();

    if (static_cast<Int>(ub.size()) != num_vars) {
        result.add_error("Variable upper bounds dimension mismatch: expected " +
                         std::to_string(num_vars) + ", got " + std::to_string(ub.size()));
    }
    if (static_cast<Int>(obj.size()) != num_vars) {
        result.add_error("Objective coefficients dimension mismatch: expected " +
                         std::to_string(num_vars) + ", got " + std::to_string(obj.size()));
    }
    if (!vtypes.empty() && static_cast<Int>(vtypes.size()) != num_vars) {
        result.add_error("Variable types dimension mismatch: expected " + std::to_string(num_vars) +
                         ", got " + std::to_string(vtypes.size()));
    }
    if (!vnames.empty() && static_cast<Int>(vnames.size()) != num_vars) {
        result.add_error("Variable names dimension mismatch: expected " + std::to_string(num_vars) +
                         ", got " + std::to_string(vnames.size()));
    }

    // 2. Constraint dimension checks
    const auto& lhs = model.constraint_lower_bounds();
    const auto& rhs = model.constraint_upper_bounds();
    const auto& cnames = model.constraint_names();

    if (static_cast<Int>(rhs.size()) != num_constrs) {
        result.add_error("Constraint upper bounds dimension mismatch: expected " +
                         std::to_string(num_constrs) + ", got " + std::to_string(rhs.size()));
    }
    if (!cnames.empty() && static_cast<Int>(cnames.size()) != num_constrs) {
        result.add_error("Constraint names dimension mismatch: expected " +
                         std::to_string(num_constrs) + ", got " + std::to_string(cnames.size()));
    }

    // 3. Objective offset check
    if (std::isnan(model.objective_offset())) {
        result.add_error("Objective offset is NaN.");
    }

    // 4. Variable bounds and numerical checks
    for (Int j = 0; j < num_vars; ++j) {
        const Real l = lb[static_cast<std::size_t>(j)];
        const Real u = ub[static_cast<std::size_t>(j)];

        if (std::isnan(l)) {
            result.add_error("Variable " + std::to_string(j) + " lower bound is NaN.");
        }
        if (std::isnan(u)) {
            result.add_error("Variable " + std::to_string(j) + " upper bound is NaN.");
        }
        if (!std::isnan(l) && !std::isnan(u) && l > u) {
            result.add_error("Variable " + std::to_string(j) + " has lower bound " +
                             std::to_string(l) + " exceeding upper bound " + std::to_string(u));
        }
        if (l == kInfinity) {
            result.add_error("Variable " + std::to_string(j) + " has lower bound +inf.");
        }
        if (u == -kInfinity) {
            result.add_error("Variable " + std::to_string(j) + " has upper bound -inf.");
        }

        if (j < static_cast<Int>(obj.size())) {
            const Real c = obj[static_cast<std::size_t>(j)];
            if (std::isnan(c)) {
                result.add_error("Variable " + std::to_string(j) +
                                 " objective coefficient is NaN.");
            }
            if (std::isinf(c)) {
                result.add_error("Variable " + std::to_string(j) +
                                 " objective coefficient is infinite.");
            }
        }

        if (j < static_cast<Int>(vtypes.size()) &&
            vtypes[static_cast<std::size_t>(j)] == VariableType::Binary) {
            if (l < 0.0 || u > 1.0) {
                result.add_error("Binary variable " + std::to_string(j) + " has bounds [" +
                                 std::to_string(l) + ", " + std::to_string(u) +
                                 "] outside [0, 1].");
            }
        }
    }

    // 5. Constraint bounds checks
    for (Int i = 0; i < num_constrs; ++i) {
        const Real l = lhs[static_cast<std::size_t>(i)];
        const Real u = rhs[static_cast<std::size_t>(i)];

        if (std::isnan(l)) {
            result.add_error("Constraint " + std::to_string(i) + " lower bound (lhs) is NaN.");
        }
        if (std::isnan(u)) {
            result.add_error("Constraint " + std::to_string(i) + " upper bound (rhs) is NaN.");
        }
        if (!std::isnan(l) && !std::isnan(u) && l > u) {
            result.add_error("Constraint " + std::to_string(i) + " has lower bound " +
                             std::to_string(l) + " exceeding upper bound " + std::to_string(u));
        }
        if (l == kInfinity) {
            result.add_error("Constraint " + std::to_string(i) + " has lower bound +inf.");
        }
        if (u == -kInfinity) {
            result.add_error("Constraint " + std::to_string(i) + " has upper bound -inf.");
        }
    }

    // 6. Constraint matrix checks
    const auto& A = model.constraint_matrix();
    if (!A.empty()) {
        if (A.num_rows() != num_constrs) {
            result.add_error("Constraint matrix row dimension (" + std::to_string(A.num_rows()) +
                             ") does not match constraint count (" + std::to_string(num_constrs) +
                             ")");
        }
        if (A.num_cols() != num_vars) {
            result.add_error("Constraint matrix column dimension (" + std::to_string(A.num_cols()) +
                             ") does not match variable count (" + std::to_string(num_vars) + ")");
        }

        const auto& col_ptr = A.col_ptr();
        const auto& row_indices = A.row_indices();
        const auto& vals = A.values();

        if (static_cast<Int>(col_ptr.size()) != A.num_cols() + 1) {
            result.add_error("Constraint matrix col_ptr size (" + std::to_string(col_ptr.size()) +
                             ") must be num_cols + 1 (" + std::to_string(A.num_cols() + 1) + ")");
        } else {
            if (col_ptr[0] != 0) {
                result.add_error("Constraint matrix col_ptr[0] must be 0, got " +
                                 std::to_string(col_ptr[0]));
            }
            if (col_ptr.back() != static_cast<Int>(vals.size())) {
                result.add_error(
                    "Constraint matrix col_ptr.back() (" + std::to_string(col_ptr.back()) +
                    ") does not match values count (" + std::to_string(vals.size()) + ")");
            }
        }

        if (row_indices.size() != vals.size()) {
            result.add_error("Constraint matrix row_indices count (" +
                             std::to_string(row_indices.size()) +
                             ") does not match values count (" + std::to_string(vals.size()) + ")");
        }

        if (static_cast<Int>(col_ptr.size()) == A.num_cols() + 1) {
            for (Int c = 0; c < A.num_cols(); ++c) {
                const Int start = col_ptr[static_cast<std::size_t>(c)];
                const Int end = col_ptr[static_cast<std::size_t>(c + 1)];
                if (start > end) {
                    result.add_error("Constraint matrix non-monotonic col_ptr at column " +
                                     std::to_string(c));
                    break;
                }
                Int prev_row = -1;
                for (Int idx = start; idx < end; ++idx) {
                    const Int r = row_indices[static_cast<std::size_t>(idx)];
                    const Real v = vals[static_cast<std::size_t>(idx)];

                    if (r < 0 || r >= num_constrs) {
                        result.add_error("Constraint matrix entry at col " + std::to_string(c) +
                                         " has invalid row index " + std::to_string(r));
                    }
                    if (r <= prev_row) {
                        result.add_error("Constraint matrix column " + std::to_string(c) +
                                         " has unsorted or duplicate row index " +
                                         std::to_string(r));
                    }
                    prev_row = r;

                    if (std::isnan(v)) {
                        result.add_error("Constraint matrix entry at (" + std::to_string(r) + ", " +
                                         std::to_string(c) + ") is NaN.");
                    }
                    if (std::isinf(v)) {
                        result.add_error("Constraint matrix entry at (" + std::to_string(r) + ", " +
                                         std::to_string(c) + ") is infinite.");
                    }
                }
            }
        }
    } else if (num_constrs > 0 && num_vars > 0) {
        // Model declared constraints and variables but constraint matrix is empty 0x0
        result.add_warning("Model has " + std::to_string(num_constrs) + " constraint(s) and " +
                           std::to_string(num_vars) +
                           " variable(s) with an empty constraint matrix.");
    }

    // 7. Quadratic matrix checks (if present)
    if (model.has_quadratic_objective()) {
        const auto& Q = *model.quadratic_matrix();
        if (Q.num_rows() != num_vars || Q.num_cols() != num_vars) {
            result.add_error("Quadratic objective matrix dimensions (" +
                             std::to_string(Q.num_rows()) + " x " + std::to_string(Q.num_cols()) +
                             ") must be " + std::to_string(num_vars) + " x " +
                             std::to_string(num_vars));
        }
        for (const auto& v : Q.values()) {
            if (std::isnan(v)) {
                result.add_error("Quadratic objective matrix contains NaN value.");
                break;
            }
            if (std::isinf(v)) {
                result.add_error("Quadratic objective matrix contains infinite value.");
                break;
            }
        }
    }

    return result;
}

} // namespace vajra
