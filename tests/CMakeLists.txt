cmake_minimum_required(VERSION 3.5)

# Enable testing at the tests/ level
enable_testing()

# Include subdirectories for different test categories
add_subdirectory(unit)
add_subdirectory(qt)
add_subdirectory(qml)

message(STATUS "Tests infrastructure configured: unit, qt, qml")
