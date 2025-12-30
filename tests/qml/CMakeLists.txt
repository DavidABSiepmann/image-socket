cmake_minimum_required(VERSION 3.5)

find_package(Qt5 COMPONENTS Core Qml Quick Test REQUIRED)

# -------------------------------------------------------------------
# QML SMOKE TESTS (qml_smoke_*)
# -------------------------------------------------------------------
# Verifies that all QML components load without errors
add_subdirectory(smoke)

# -------------------------------------------------------------------
# QML COMPONENT TESTS (qml_component_*)
# -------------------------------------------------------------------
# Tests component state, visibility, and property binding
add_subdirectory(components)

# -------------------------------------------------------------------
# QML INTEGRATION TESTS (qml_integration_*)
# -------------------------------------------------------------------
# Tests multi-component interactions and state propagation
add_subdirectory(integration)

# Summary
add_library(unit_qml INTERFACE)
message(STATUS "QML tests infrastructure ready")
message(STATUS "  - QML smoke tests added")
message(STATUS "  - QML component state tests added")
message(STATUS "  - QML integration tests added")
