cmake_minimum_required(VERSION 3.28)
project(infra)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# options
option(INFRA_BUILD_TESTS "" OFF)

add_library(infra INTERFACE)
target_include_directories(infra INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/include)

# tests
if(INFRA_BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()