cmake_minimum_required(VERSION 3.25)

project(VajraOpt
    VERSION 0.1.0
    DESCRIPTION "Indigenous GPU-Accelerated Optimization Solver"
    HOMEPAGE_URL "https://github.com/saksham-s18/Vajra-Opt"
    LANGUAGES CXX
)

# -----------------------------------------------------------------------------
# C++ Language Standard
# -----------------------------------------------------------------------------
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Export compile commands for language servers and tooling
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# -----------------------------------------------------------------------------
# Build Options
# -----------------------------------------------------------------------------
option(VAJRA_ENABLE_CUDA "Enable NVIDIA CUDA GPU acceleration" OFF)
option(VAJRA_ENABLE_SANITIZERS "Enable Address and Undefined Behavior sanitizers" OFF)
option(VAJRA_ENABLE_WARNINGS_AS_ERRORS "Treat compiler warnings as errors" ON)
option(BUILD_TESTING "Build unit and numerical tests" ON)

# -----------------------------------------------------------------------------
# Optional CUDA Support
# -----------------------------------------------------------------------------
if(VAJRA_ENABLE_CUDA)
    include(CheckLanguage)
    check_language(CUDA)
    if(CMAKE_CUDA_COMPILER)
        enable_language(CUDA)
        set(CMAKE_CUDA_STANDARD 20)
        set(CMAKE_CUDA_STANDARD_REQUIRED ON)
        find_package(CUDAToolkit REQUIRED)
        message(STATUS "CUDA acceleration enabled (Toolkit: ${CUDAToolkit_VERSION})")
    else()
        message(WARNING "CUDA requested but no CUDA compiler found. Building CPU-only.")
        set(VAJRA_ENABLE_CUDA OFF)
    endif()
else()
    message(STATUS "Building CPU-only (CUDA disabled).")
endif()

# -----------------------------------------------------------------------------
# Sanitizer Configuration
# -----------------------------------------------------------------------------
if(VAJRA_ENABLE_SANITIZERS)
    if(MSVC)
        add_compile_options(/fsanitize=address)
    else()
        add_compile_options(-fsanitize=address,undefined -fno-omit-frame-pointer)
        add_link_options(-fsanitize=address,undefined)
    endif()
endif()

# -----------------------------------------------------------------------------
# Core Library Target: vajra_core
# -----------------------------------------------------------------------------
add_library(vajra_core
    src/core/model.cpp
    src/core/sparse_matrix.cpp
    src/core/validation.cpp
    src/core/solver.cpp
)

add_library(vajra::core ALIAS vajra_core)

target_include_directories(vajra_core
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
)

# Compiler Warnings and Strictness
if(MSVC)
    target_compile_options(vajra_core PRIVATE /W4 /permissive-)
    if(VAJRA_ENABLE_WARNINGS_AS_ERRORS)
        target_compile_options(vajra_core PRIVATE /WX)
    endif()
else()
    target_compile_options(vajra_core PRIVATE -Wall -Wextra -Wpedantic)
    if(VAJRA_ENABLE_WARNINGS_AS_ERRORS)
        target_compile_options(vajra_core PRIVATE -Werror)
    endif()
endif()


# CUDA defines if enabled
if(VAJRA_ENABLE_CUDA)
    target_compile_definitions(vajra_core PUBLIC VAJRA_HAS_CUDA=1)
    target_link_libraries(vajra_core PUBLIC CUDA::cudart)
endif()

# -----------------------------------------------------------------------------
# Testing Configuration
# -----------------------------------------------------------------------------
if(BUILD_TESTING)
    enable_testing()
    add_subdirectory(tests)
endif()
