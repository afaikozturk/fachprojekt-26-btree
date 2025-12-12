cmake_minimum_required(VERSION 3.15)
project(b_tree_fachprojekt)

include(ExternalProject)

# Build configuration
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
set(CMAKE_CXX_FLAGS_DEBUG "-O0 -g3")
set(CMAKE_CXX_FLAGS_RELEASE "-O3 -g -DNDEBUG")
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE RELEASE)
endif()
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY  ${CMAKE_BINARY_DIR}/lib)

message("Building in '${CMAKE_BUILD_TYPE}' mode.")

# External Projects
ExternalProject_Add(
    argparse-external
    GIT_REPOSITORY "https://github.com/p-ranav/argparse"
    GIT_TAG "v2.9"
    PREFIX "../lib/argparse"
    INSTALL_COMMAND cmake -E echo ""
)

ExternalProject_Add(
    json-external
    GIT_REPOSITORY "https://github.com/nlohmann/json"
    GIT_TAG "v3.11.2"
    PREFIX "../lib/json"
    INSTALL_COMMAND cmake -E echo ""
    BUILD_COMMAND cmake -E echo ""
    CONFIGURE_COMMAND cmake -E echo ""
)

ExternalProject_Add(
    perf-cpp-external
    GIT_REPOSITORY "https://github.com/jmuehlig/perf-cpp"
    GIT_TAG "v0.9.0"
    PREFIX "../lib/perf-cpp"
    INSTALL_COMMAND cmake -E echo ""
)

# Includes
include_directories(src lib/argparse/src/argparse-external/include lib/json/src/json-external/include lib/perf-cpp/src/perf-cpp-external/include)
link_directories(lib/perf-cpp/src/perf-cpp-external-build)

add_executable(b_tree
        src/main.cpp
        src/benchmark/workload_set.cpp
)
add_dependencies(b_tree argparse-external json-external perf-cpp-external)
target_link_libraries(b_tree perf-cpp)

# Generate test data
add_custom_target(ycsb ${CMAKE_SOURCE_DIR}/script/generate_ycsb.py a randint)