cmake_minimum_required(VERSION 3.20)
project(CppPlace LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# ── Dependencies via FetchContent ──────────────────────────────────────────────
include(FetchContent)

# Boost (thread + headers; Beast & Asio are header-only inside Boost::headers;
#        Boost.System is header-only since 1.69 — no separate component needed)
find_package(Boost 1.74 REQUIRED COMPONENTS thread)

# GoogleTest
FetchContent_Declare(
    googletest
    GIT_REPOSITORY https://github.com/google/googletest.git
    GIT_TAG        v1.14.0
)
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

# nlohmann/json
FetchContent_Declare(
    nlohmann_json
    GIT_REPOSITORY https://github.com/nlohmann/json.git
    GIT_TAG        v3.11.3
)
FetchContent_MakeAvailable(nlohmann_json)

# ── Core library ───────────────────────────────────────────────────────────────
file(GLOB_RECURSE CORE_SOURCES "src/*.cpp")
list(FILTER CORE_SOURCES EXCLUDE REGEX ".*/main\\.cpp$")   # main goes into its own target

add_library(cppplace_core STATIC ${CORE_SOURCES})
target_include_directories(cppplace_core PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/src)
target_link_libraries(cppplace_core PUBLIC Boost::thread Boost::headers nlohmann_json::nlohmann_json)

# ── Server executable ───────────────────────────────────────────────────────────
add_executable(cppplace_server src/server/main.cpp)
target_link_libraries(cppplace_server PRIVATE cppplace_core)

# ── Tests ──────────────────────────────────────────────────────────────────────
enable_testing()

file(GLOB TEST_SOURCES "tests/*.cpp")

add_executable(cppplace_tests ${TEST_SOURCES})
target_link_libraries(cppplace_tests PRIVATE cppplace_core GTest::gtest_main GTest::gmock)

include(GoogleTest)
gtest_discover_tests(cppplace_tests)
