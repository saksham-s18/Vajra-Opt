include(FetchContent)

set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
set(BUILD_GMOCK OFF CACHE BOOL "" FORCE)
set(INSTALL_GTEST OFF CACHE BOOL "" FORCE)

FetchContent_Declare(
    googletest
    GIT_REPOSITORY https://github.com/google/googletest.git
    GIT_TAG        v1.16.0
)

FetchContent_MakeAvailable(googletest)

# -----------------------------------------------------------------------------
# Unit Tests Executable
# -----------------------------------------------------------------------------
add_executable(vajra_unit_tests
    unit/test_model.cpp
    unit/test_validation.cpp
    unit/test_options.cpp
    unit/test_solution.cpp
    unit/test_solver.cpp
    unit/test_sparse_matrix.cpp
)

target_link_libraries(vajra_unit_tests
    PRIVATE
        vajra_core
        GTest::gtest_main
)

if(MSVC)
    target_compile_options(vajra_unit_tests PRIVATE /W4 /permissive-)
    if(VAJRA_ENABLE_WARNINGS_AS_ERRORS)
        target_compile_options(vajra_unit_tests PRIVATE /WX)
    endif()
else()
    target_compile_options(vajra_unit_tests PRIVATE -Wall -Wextra -Wpedantic)
    if(VAJRA_ENABLE_WARNINGS_AS_ERRORS)
        target_compile_options(vajra_unit_tests PRIVATE -Werror)
    endif()
endif()

include(GoogleTest)
gtest_discover_tests(vajra_unit_tests)
