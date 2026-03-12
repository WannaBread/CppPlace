cmake_minimum_required(VERSION 3.20)
project(CppPlace LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# ── Dependencies via FetchContent ──────────────────────────────────────────────
include(FetchContent)

# Boost (thread + headers)
find_package(Boost 1.74 REQUIRED COMPONENTS thread)

# GoogleTest
FetchContent_Declare(
    googletest
    GIT_REPOSITORY https://github.com/google/googletest.git
    GIT_TAG        v1.14.0
)
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

# ── Core library ───────────────────────────────────────────────────────────────
file(GLOB_RECURSE CORE_SOURCES "src/*.cpp")

add_library(cppplace_core STATIC ${CORE_SOURCES})
target_include_directories(cppplace_core PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/src)
target_link_libraries(cppplace_core PUBLIC Boost::thread Boost::headers)

# ── Tests ──────────────────────────────────────────────────────────────────────
enable_testing()

file(GLOB TEST_SOURCES "tests/*.cpp")

add_executable(cppplace_tests ${TEST_SOURCES})
target_link_libraries(cppplace_tests PRIVATE cppplace_core GTest::gtest_main)

include(GoogleTest)
gtest_discover_tests(cppplace_tests)
