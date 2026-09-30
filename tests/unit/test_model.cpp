#include "vajra/model.hpp"

#include <gtest/gtest.h>

namespace vajra {
namespace {

TEST(ModelTest, DefaultConstruction) {
    Model model;
    EXPECT_EQ(model.num_variables(), 0);
    EXPECT_EQ(model.num_constraints(), 0);
    EXPECT_EQ(model.num_nonzeros(), 0);
    EXPECT_EQ(model.objective_sense(), ObjectiveSense::Minimize);
    EXPECT_DOUBLE_EQ(model.objective_offset(), 0.0);
    EXPECT_FALSE(model.has_quadratic_objective());
}

TEST(ModelTest, NamedConstruction) {
    Model model("ProductionPlanning");
    EXPECT_EQ(model.name(), "ProductionPlanning");
    model.set_name("PortfolioOptimization");
    EXPECT_EQ(model.name(), "PortfolioOptimization");
}

TEST(ModelTest, AddVariables) {
    Model model;
    const Int x0 = model.add_variable(0.0, 10.0, 2.5, VariableType::Continuous, "x0");
    const Int x1 = model.add_variable(0.0, 1.0, -1.0, VariableType::Binary, "x1");
    const Int x2 = model.add_variable(-5.0, 5.0, 0.0, VariableType::Integer, "x2");

    EXPECT_EQ(x0, 0);
    EXPECT_EQ(x1, 1);
    EXPECT_EQ(x2, 2);
    EXPECT_EQ(model.num_variables(), 3);

    EXPECT_DOUBLE_EQ(model.variable_lower_bounds()[0], 0.0);
    EXPECT_DOUBLE_EQ(model.variable_upper_bounds()[0], 10.0);
    EXPECT_DOUBLE_EQ(model.objective_coefficients()[0], 2.5);
    EXPECT_EQ(model.variable_types()[0], VariableType::Continuous);
    EXPECT_EQ(model.variable_names()[0], "x0");

    EXPECT_EQ(model.variable_types()[1], VariableType::Binary);
    EXPECT_EQ(model.variable_types()[2], VariableType::Integer);
}

TEST(ModelTest, AddConstraints) {
    Model model;
    const Int c0 = model.add_constraint(-kInfinity, 100.0, "resource_limit");
    const Int c1 = model.add_constraint(50.0, 50.0, "demand_equality");
    const Int c2 = model.add_constraint(10.0, 20.0, "range_constraint");

    EXPECT_EQ(c0, 0);
    EXPECT_EQ(c1, 1);
    EXPECT_EQ(c2, 2);
    EXPECT_EQ(model.num_constraints(), 3);

    EXPECT_DOUBLE_EQ(model.constraint_lower_bounds()[0], -kInfinity);
    EXPECT_DOUBLE_EQ(model.constraint_upper_bounds()[0], 100.0);
    EXPECT_DOUBLE_EQ(model.constraint_lower_bounds()[1], 50.0);
    EXPECT_DOUBLE_EQ(model.constraint_upper_bounds()[1], 50.0);
    EXPECT_EQ(model.constraint_names()[0], "resource_limit");
}

TEST(ModelTest, ConstraintMatrixAssignment) {
    Model model;
    model.add_variable(0.0, 10.0, 1.0);
    model.add_variable(0.0, 10.0, 2.0);
    model.add_constraint(0.0, 5.0);

    const std::vector<Triplet> triplets = {
        {0, 0, 1.5},
        {0, 1, 2.5}
    };
    SparseMatrix A = SparseMatrix::from_triplets(1, 2, triplets);
    model.set_constraint_matrix(std::move(A));

    EXPECT_EQ(model.num_nonzeros(), 2);
    EXPECT_EQ(model.constraint_matrix().num_rows(), 1);
    EXPECT_EQ(model.constraint_matrix().num_cols(), 2);
}

TEST(ModelTest, QuadraticObjectiveMatrix) {
    Model model;
    model.add_variable(0.0, 1.0);
    model.add_variable(0.0, 1.0);

    EXPECT_FALSE(model.has_quadratic_objective());

    const std::vector<Triplet> q_triplets = {
        {0, 0, 2.0},
        {1, 1, 4.0}
    };
    SparseMatrix Q = SparseMatrix::from_triplets(2, 2, q_triplets);
    model.set_quadratic_matrix(std::move(Q));

    EXPECT_TRUE(model.has_quadratic_objective());
    ASSERT_TRUE(model.quadratic_matrix().has_value());
    EXPECT_EQ(model.quadratic_matrix()->num_nonzeros(), 2);

    model.clear_quadratic_matrix();
    EXPECT_FALSE(model.has_quadratic_objective());
}

TEST(ModelTest, ClearResetsState) {
    Model model("ToClear");
    model.add_variable(0.0, 1.0, 5.0);
    model.add_constraint(0.0, 10.0);
    model.set_objective_sense(ObjectiveSense::Maximize);
    model.set_objective_offset(42.0);

    model.clear();

    EXPECT_EQ(model.num_variables(), 0);
    EXPECT_EQ(model.num_constraints(), 0);
    EXPECT_EQ(model.num_nonzeros(), 0);
    EXPECT_EQ(model.objective_sense(), ObjectiveSense::Minimize);
    EXPECT_DOUBLE_EQ(model.objective_offset(), 0.0);
    EXPECT_FALSE(model.has_quadratic_objective());
}

} // namespace
} // namespace vajra
