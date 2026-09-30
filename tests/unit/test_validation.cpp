#include "vajra/validation.hpp"

#include <cmath>
#include <gtest/gtest.h>

namespace vajra {
namespace {

TEST(ValidationTest, EmptyModelIsValid) {
    Model model;
    const ValidationResult res = validate_model(model);
    EXPECT_TRUE(res.is_valid());
    EXPECT_TRUE(res.errors().empty());
}

TEST(ValidationTest, ValidStandardLP) {
    Model model;
    model.add_variable(0.0, 10.0, 3.0, VariableType::Continuous, "x0");
    model.add_variable(0.0, 20.0, 5.0, VariableType::Continuous, "x1");
    model.add_constraint(0.0, 100.0, "c0");

    const std::vector<Triplet> triplets = {
        {0, 0, 1.0},
        {0, 1, 2.0}
    };
    model.set_constraint_matrix(SparseMatrix::from_triplets(1, 2, triplets));

    const ValidationResult res = validate_model(model);
    EXPECT_TRUE(res.is_valid());
    EXPECT_TRUE(res.errors().empty());
}

TEST(ValidationTest, VariableDimensionMismatch) {
    Model model;
    model.add_variable(0.0, 10.0, 1.0);
    model.set_objective_coefficients({1.0, 2.0}); // size 2 != size 1

    const ValidationResult res = validate_model(model);
    EXPECT_FALSE(res.is_valid());
    ASSERT_FALSE(res.errors().empty());
    EXPECT_NE(res.to_string().find("Objective coefficients dimension mismatch"), std::string::npos);
}

TEST(ValidationTest, InvertedVariableBounds) {
    Model model;
    model.add_variable(15.0, 10.0, 1.0); // lb (15) > ub (10)

    const ValidationResult res = validate_model(model);
    EXPECT_FALSE(res.is_valid());
    EXPECT_NE(res.to_string().find("exceeding upper bound"), std::string::npos);
}

TEST(ValidationTest, InvertedConstraintBounds) {
    Model model;
    model.add_variable(0.0, 1.0, 1.0);
    model.add_constraint(100.0, 50.0); // lhs (100) > rhs (50)

    const ValidationResult res = validate_model(model);
    EXPECT_FALSE(res.is_valid());
    EXPECT_NE(res.to_string().find("exceeding upper bound"), std::string::npos);
}

TEST(ValidationTest, InfiniteBoundaryErrors) {
    Model model1;
    model1.add_variable(kInfinity, kInfinity);
    EXPECT_FALSE(validate_model(model1).is_valid());

    Model model2;
    model2.add_variable(-kInfinity, -kInfinity);
    EXPECT_FALSE(validate_model(model2).is_valid());

    Model model3;
    model3.add_constraint(kInfinity, kInfinity);
    EXPECT_FALSE(validate_model(model3).is_valid());

    Model model4;
    model4.add_constraint(-kInfinity, -kInfinity);
    EXPECT_FALSE(validate_model(model4).is_valid());
}

TEST(ValidationTest, NaNDetection) {
    const double qnan = std::numeric_limits<double>::quiet_NaN();

    // NaN in variable lower bound
    Model m1;
    m1.add_variable(qnan, 10.0, 1.0);
    EXPECT_FALSE(validate_model(m1).is_valid());

    // NaN in variable upper bound
    Model m2;
    m2.add_variable(0.0, qnan, 1.0);
    EXPECT_FALSE(validate_model(m2).is_valid());

    // NaN in objective
    Model m3;
    m3.add_variable(0.0, 1.0, qnan);
    EXPECT_FALSE(validate_model(m3).is_valid());

    // NaN in constraint bounds
    Model m4;
    m4.add_constraint(qnan, 10.0);
    EXPECT_FALSE(validate_model(m4).is_valid());

    Model m5;
    m5.add_constraint(0.0, qnan);
    EXPECT_FALSE(validate_model(m5).is_valid());
}

TEST(ValidationTest, BinaryVariableBounds) {
    Model m_valid;
    m_valid.add_variable(0.0, 1.0, 1.0, VariableType::Binary);
    EXPECT_TRUE(validate_model(m_valid).is_valid());

    Model m_invalid;
    m_invalid.add_variable(-1.0, 1.0, 1.0, VariableType::Binary);
    EXPECT_FALSE(validate_model(m_invalid).is_valid());

    Model m_invalid2;
    m_invalid2.add_variable(0.0, 2.0, 1.0, VariableType::Binary);
    EXPECT_FALSE(validate_model(m_invalid2).is_valid());
}

TEST(ValidationTest, SparseMatrixDimensionMismatch) {
    Model model;
    model.add_variable(0.0, 1.0, 1.0);
    model.add_constraint(0.0, 1.0);

    // Matrix is 2x2 but model is 1x1
    SparseMatrix A(2, 2);
    model.set_constraint_matrix(std::move(A));

    const ValidationResult res = validate_model(model);
    EXPECT_FALSE(res.is_valid());
    EXPECT_NE(res.to_string().find("does not match"), std::string::npos);
}

TEST(ValidationTest, SparseMatrixInvalidRowIndex) {
    Model model;
    model.add_variable(0.0, 1.0, 1.0);
    model.add_constraint(0.0, 1.0); // 1 constraint -> valid row is 0

    // Construct raw CSC with out-of-range row index
    std::vector<Int> col_ptr = {0, 1};
    std::vector<Int> row_indices = {5}; // Row 5 is invalid for m=1
    std::vector<Real> values = {1.0};
    SparseMatrix A(1, 1, std::move(col_ptr), std::move(row_indices), std::move(values));
    model.set_constraint_matrix(std::move(A));

    const ValidationResult res = validate_model(model);
    EXPECT_FALSE(res.is_valid());
    EXPECT_NE(res.to_string().find("invalid row index"), std::string::npos);
}

TEST(ValidationTest, SparseMatrixNaNEntry) {
    Model model;
    model.add_variable(0.0, 1.0, 1.0);
    model.add_constraint(0.0, 1.0);

    const double qnan = std::numeric_limits<double>::quiet_NaN();
    std::vector<Int> col_ptr = {0, 1};
    std::vector<Int> row_indices = {0};
    std::vector<Real> values = {qnan};
    SparseMatrix A(1, 1, std::move(col_ptr), std::move(row_indices), std::move(values));
    model.set_constraint_matrix(std::move(A));

    const ValidationResult res = validate_model(model);
    EXPECT_FALSE(res.is_valid());
    EXPECT_NE(res.to_string().find("is NaN"), std::string::npos);
}

TEST(ValidationTest, QuadraticMatrixDimensionMismatch) {
    Model model;
    model.add_variable(0.0, 1.0, 1.0); // n = 1

    SparseMatrix Q(2, 2); // Q dimension 2x2 != 1x1
    model.set_quadratic_matrix(std::move(Q));

    const ValidationResult res = validate_model(model);
    EXPECT_FALSE(res.is_valid());
    EXPECT_NE(res.to_string().find("Quadratic objective matrix dimensions"), std::string::npos);
}

} // namespace
} // namespace vajra
