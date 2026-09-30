#include "vajra/solver.hpp"

#include <gtest/gtest.h>

namespace vajra {
namespace {

TEST(SolverTest, SolveValidModelReturnsNotImplemented) {
    Model model("ValidLP");
    model.add_variable(0.0, 10.0, 1.0, VariableType::Continuous, "x0");
    model.add_variable(0.0, 5.0, 2.0, VariableType::Continuous, "x1");
    model.add_constraint(0.0, 10.0, "c0");

    const std::vector<Triplet> triplets = {
        {0, 0, 1.0},
        {0, 1, 1.0}
    };
    model.set_constraint_matrix(SparseMatrix::from_triplets(1, 2, triplets));

    const Solution sol = solve(model);

    EXPECT_EQ(sol.status, SolveStatus::NotImplemented);
    EXPECT_GE(sol.solve_time_seconds, 0.0);
    EXPECT_NE(sol.status_message.find("Phase 0"), std::string::npos);
    EXPECT_FALSE(sol.is_optimal());
}

TEST(SolverTest, SolveInvalidModelReturnsInvalidModelStatus) {
    Model model("InvalidBounds");
    // Invalid variable bounds: lb (10.0) > ub (0.0)
    model.add_variable(10.0, 0.0, 1.0);

    const Solution sol = solve(model);

    EXPECT_EQ(sol.status, SolveStatus::InvalidModel);
    EXPECT_GE(sol.solve_time_seconds, 0.0);
    EXPECT_NE(sol.status_message.find("exceeding upper bound"), std::string::npos);
}

TEST(SolverTest, SolveWithOptions) {
    Model model;
    model.add_variable(0.0, 1.0, 1.0);

    Options options;
    options.time_limit_seconds = 10.0;
    options.num_threads = 4;
    options.use_gpu = false;

    const Solution sol = solve(model, options);
    EXPECT_EQ(sol.status, SolveStatus::NotImplemented);
}

} // namespace
} // namespace vajra
