#include "vajra/solution.hpp"

#include <gtest/gtest.h>

namespace vajra {
namespace {

TEST(SolutionTest, DefaultState) {
    const Solution sol;
    EXPECT_EQ(sol.status, SolveStatus::Unknown);
    EXPECT_DOUBLE_EQ(sol.objective_value, 0.0);
    EXPECT_TRUE(sol.primal_variables.empty());
    EXPECT_TRUE(sol.dual_variables.empty());
    EXPECT_TRUE(sol.reduced_costs.empty());
    EXPECT_EQ(sol.iteration_count, 0);
    EXPECT_DOUBLE_EQ(sol.solve_time_seconds, 0.0);
    EXPECT_FALSE(sol.is_optimal());
    EXPECT_FALSE(sol.has_primal_solution());
    EXPECT_FALSE(sol.has_dual_solution());
}

TEST(SolutionTest, OptimalSolutionQueries) {
    Solution sol;
    sol.status = SolveStatus::Optimal;
    sol.objective_value = -42.5;
    sol.primal_variables = {1.0, 2.0, 3.0};
    sol.dual_variables = {0.5, 1.5};
    sol.reduced_costs = {0.0, 0.0, 0.0};
    sol.iteration_count = 12;
    sol.solve_time_seconds = 0.005;

    EXPECT_TRUE(sol.is_optimal());
    EXPECT_TRUE(sol.has_primal_solution());
    EXPECT_TRUE(sol.has_dual_solution());
    EXPECT_DOUBLE_EQ(sol.objective_value, -42.5);
    EXPECT_EQ(sol.iteration_count, 12);
}

TEST(SolutionTest, SolveStatusToString) {
    EXPECT_EQ(to_string(SolveStatus::Optimal), "Optimal");
    EXPECT_EQ(to_string(SolveStatus::Infeasible), "Infeasible");
    EXPECT_EQ(to_string(SolveStatus::Unbounded), "Unbounded");
    EXPECT_EQ(to_string(SolveStatus::InfeasibleOrUnbounded), "InfeasibleOrUnbounded");
    EXPECT_EQ(to_string(SolveStatus::IterationLimit), "IterationLimit");
    EXPECT_EQ(to_string(SolveStatus::TimeLimit), "TimeLimit");
    EXPECT_EQ(to_string(SolveStatus::NumericalError), "NumericalError");
    EXPECT_EQ(to_string(SolveStatus::InvalidModel), "InvalidModel");
    EXPECT_EQ(to_string(SolveStatus::NotImplemented), "NotImplemented");
    EXPECT_EQ(to_string(SolveStatus::UserCancelled), "UserCancelled");
    EXPECT_EQ(to_string(SolveStatus::Unknown), "Unknown");
}

} // namespace
} // namespace vajra
