cmake_minimum_required(VERSION 3.20)
project(CppPlace LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# ── Dependencies via FetchContent ──────────────────────────────────────────────
include(FetchContent)

# Boost — try a system install first; fall back to FetchContent so the project
# builds out-of-the-box on machines without a system Boost. The fallback uses
# the official Boost CMake superproject (1.86 has full modular CMake support).
find_package(Boost 1.74 QUIET COMPONENTS thread)

if (NOT Boost_FOUND)
    message(STATUS "Boost not found system-wide — fetching boost-1.86 (this takes a while on first configure)")
    set(BOOST_INCLUDE_LIBRARIES thread system asio beast uuid)
    set(BOOST_ENABLE_CMAKE ON)
    FetchContent_Declare(
        Boost
        URL https://github.com/boostorg/boost/releases/download/boost-1.86.0/boost-1.86.0-cmake.tar.xz
        DOWNLOAD_EXTRACT_TIMESTAMP TRUE
    )
    FetchContent_MakeAvailable(Boost)

    # The fetched build exposes targets like Boost::thread / Boost::headers
    # under the same names find_package would, so the rest of the file works
    # unchanged.
endif()

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
file(GLOB_RECURSE CORE_SOURCES "src/common/*.cpp" "src/models/*.cpp"
                               "src/server/*.cpp" "src/services/*.cpp"
                               "src/utils/*.cpp")
list(FILTER CORE_SOURCES EXCLUDE REGEX ".*/main\\.cpp$")   # main goes into its own target

add_library(cppplace_core STATIC ${CORE_SOURCES})
target_include_directories(cppplace_core PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/src)
target_link_libraries(cppplace_core PUBLIC
    Boost::thread Boost::asio Boost::beast Boost::uuid Boost::headers
    nlohmann_json::nlohmann_json)

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

# ── Qt client (optional) ───────────────────────────────────────────────────────
# The Qt client is built only when Qt6 (Widgets + Network, optionally Test)
# is found on the system. Configure with -DCMAKE_PREFIX_PATH=<qt-install>.
find_package(Qt6 6.2 COMPONENTS Widgets Network QUIET)

if (Qt6_FOUND)
    message(STATUS "Qt6 found — building cppplace_client")

    set(CMAKE_AUTOMOC ON)
    set(CMAKE_AUTORCC ON)

    file(GLOB CLIENT_SOURCES "src/client/*.cpp")
    list(FILTER CLIENT_SOURCES EXCLUDE REGEX ".*/main\\.cpp$")

    add_library(cppplace_client_lib STATIC ${CLIENT_SOURCES})
    target_include_directories(cppplace_client_lib PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/src)
    target_link_libraries(cppplace_client_lib
        PUBLIC Qt6::Widgets Qt6::Network nlohmann_json::nlohmann_json)

    add_executable(cppplace_client
        src/client/main.cpp
        src/client/resources/resources.qrc)
    target_link_libraries(cppplace_client PRIVATE cppplace_client_lib)

    # ── Qt client tests ───────────────────────────────────────────────────────
    find_package(Qt6 6.2 COMPONENTS Test QUIET)
    if (Qt6Test_FOUND)
        file(GLOB CLIENT_TEST_SOURCES "tests/client/*.cpp")
        add_executable(cppplace_client_tests ${CLIENT_TEST_SOURCES})
        target_link_libraries(cppplace_client_tests
            PRIVATE cppplace_client_lib cppplace_core Qt6::Test)
        add_test(NAME cppplace_client_tests COMMAND cppplace_client_tests)
    endif()
else()
    message(STATUS "Qt6 not found — skipping cppplace_client (set CMAKE_PREFIX_PATH to enable)")
endif()
