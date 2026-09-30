#include "vajra/model.hpp"

#include <utility>

namespace vajra {

Model::Model(std::string name) : name_(std::move(name)) {}

void Model::set_objective_coefficients(std::vector<Real> c) {
    objective_coefficients_ = std::move(c);
}

void Model::set_variable_bounds(std::vector<Real> lower, std::vector<Real> upper) {
    variable_lower_bounds_ = std::move(lower);
    variable_upper_bounds_ = std::move(upper);
}

void Model::set_variable_types(std::vector<VariableType> types) {
    variable_types_ = std::move(types);
}

void Model::set_variable_names(std::vector<std::string> names) {
    variable_names_ = std::move(names);
}

Int Model::add_variable(Real lb, Real ub, Real obj, VariableType type, std::string name) {
    const Int var_idx = num_variables();
    variable_lower_bounds_.push_back(lb);
    variable_upper_bounds_.push_back(ub);
    objective_coefficients_.push_back(obj);
    variable_types_.push_back(type);
    variable_names_.push_back(std::move(name));
    return var_idx;
}

void Model::set_constraint_bounds(std::vector<Real> lower, std::vector<Real> upper) {
    constraint_lower_bounds_ = std::move(lower);
    constraint_upper_bounds_ = std::move(upper);
}

void Model::set_constraint_names(std::vector<std::string> names) {
    constraint_names_ = std::move(names);
}

Int Model::add_constraint(Real lhs, Real rhs, std::string name) {
    const Int constr_idx = num_constraints();
    constraint_lower_bounds_.push_back(lhs);
    constraint_upper_bounds_.push_back(rhs);
    constraint_names_.push_back(std::move(name));
    return constr_idx;
}

void Model::set_constraint_matrix(SparseMatrix matrix) {
    constraint_matrix_ = std::move(matrix);
}

void Model::set_quadratic_matrix(SparseMatrix Q) {
    quadratic_matrix_ = std::move(Q);
}

void Model::clear() noexcept {
    name_ = "VajraModel";
    sense_ = ObjectiveSense::Minimize;
    objective_offset_ = 0.0;
    objective_coefficients_.clear();
    variable_lower_bounds_.clear();
    variable_upper_bounds_.clear();
    variable_types_.clear();
    variable_names_.clear();
    constraint_lower_bounds_.clear();
    constraint_upper_bounds_.clear();
    constraint_names_.clear();
    constraint_matrix_.clear();
    quadratic_matrix_.reset();
}

} // namespace vajra
