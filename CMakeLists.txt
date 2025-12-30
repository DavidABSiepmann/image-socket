cmake_minimum_required(VERSION 3.5)
project(image_socket_mgr LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Use portable settings and enable stricter warnings for all targets
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release CACHE STRING "Build type" FORCE)
endif()
add_compile_options(-Wall -Wextra -Wpedantic)

# Helpful default: enable automoc/autouic/autorcc for Qt targets in subdirs
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

# Enable testing at the top level
enable_testing()

# Add library and subprojects
add_subdirectory(src)
add_subdirectory(apps/client)
add_subdirectory(apps/server)
add_subdirectory(tests)

message(STATUS "Top-level project: ${PROJECT_NAME}")
message(STATUS "To build with Ninja: mkdir -p build && cd build && cmake -G Ninja .. && ninja")
