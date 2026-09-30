#include "vajra/options.hpp"

#include <gtest/gtest.h>

namespace vajra {
namespace {

TEST(OptionsTest, DefaultTolerancesAndLimits) {
    const Options options;

    EXPECT_DOUBLE_EQ(options.primal_feasibility_tolerance, 1e-8);
    EXPECT_DOUBLE_EQ(options.dual_feasibility_tolerance, 1e-8);
    EXPECT_DOUBLE_EQ(options.optimality_tolerance, 1e-8);
    EXPECT_DOUBLE_EQ(options.integrality_tolerance, 1e-6);

    EXPECT_DOUBLE_EQ(options.time_limit_seconds, 3600.0);
    EXPECT_EQ(options.max_iterations, 100000);

    EXPECT_FALSE(options.use_gpu);
    EXPECT_EQ(options.device_id, 0);
    EXPECT_EQ(options.num_threads, 1);

    EXPECT_EQ(options.log_level, LogLevel::Warning);
    EXPECT_FALSE(options.print_to_console);
}

TEST(OptionsTest, CustomOptionsMutation) {
    Options options;
    options.primal_feasibility_tolerance = 1e-6;
    options.time_limit_seconds = 60.0;
    options.max_iterations = 5000;
    options.use_gpu = true;
    options.device_id = 1;
    options.num_threads = 8;
    options.log_level = LogLevel::Debug;
    options.print_to_console = true;

    EXPECT_DOUBLE_EQ(options.primal_feasibility_tolerance, 1e-6);
    EXPECT_DOUBLE_EQ(options.time_limit_seconds, 60.0);
    EXPECT_EQ(options.max_iterations, 5000);
    EXPECT_TRUE(options.use_gpu);
    EXPECT_EQ(options.device_id, 1);
    EXPECT_EQ(options.num_threads, 8);
    EXPECT_EQ(options.log_level, LogLevel::Debug);
    EXPECT_TRUE(options.print_to_console);
}

TEST(OptionsTest, LogLevelStringConversion) {
    EXPECT_EQ(to_string(LogLevel::Off), "Off");
    EXPECT_EQ(to_string(LogLevel::Error), "Error");
    EXPECT_EQ(to_string(LogLevel::Warning), "Warning");
    EXPECT_EQ(to_string(LogLevel::Info), "Info");
    EXPECT_EQ(to_string(LogLevel::Debug), "Debug");
}

} // namespace
} // namespace vajra
