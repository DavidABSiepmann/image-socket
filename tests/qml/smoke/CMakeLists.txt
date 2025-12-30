# tests/qml/smoke/CMakeLists.txt
# 
# Smoke tests for QML components
# Verifies that QML files compile without syntax or import errors
# 
# Note: Components depending on C++ backend modules (ImageSocket, ImageServerBridge)
# or scene graph are skipped - they'll be tested in integration tests

find_package(Qt5 COMPONENTS Core Qml Test REQUIRED)

set(CMAKE_AUTOMOC ON)

# Define PROJECT_SOURCE_DIR for test file path resolution
add_executable(qml_smoke_test_all_components test_qml_smoke.cpp)

set_target_properties(qml_smoke_test_all_components PROPERTIES AUTOMOC ON)

target_compile_definitions(qml_smoke_test_all_components PRIVATE
    PROJECT_SOURCE_DIR="${CMAKE_SOURCE_DIR}")

target_link_libraries(qml_smoke_test_all_components PRIVATE
    Qt5::Core
    Qt5::Qml
    Qt5::Test)

# Register tests

add_test(
    NAME qml_smoke_theme_component_loads
    COMMAND qml_smoke_test_all_components test_theme_component_loads)

add_test(
    NAME qml_smoke_theme_properties_accessible
    COMMAND qml_smoke_test_all_components test_theme_properties_accessible)

add_test(
    NAME qml_smoke_info_overlay_component_compiles
    COMMAND qml_smoke_test_all_components test_info_overlay_component_compiles)

add_test(
    NAME qml_smoke_toast_notification_component_compiles
    COMMAND qml_smoke_test_all_components test_toast_notification_component_compiles)

add_test(
    NAME qml_smoke_styled_button_component_compiles
    COMMAND qml_smoke_test_all_components test_styled_button_component_compiles)

add_test(
    NAME qml_smoke_clients_panel_component_compiles
    COMMAND qml_smoke_test_all_components test_clients_panel_component_compiles)

add_test(
    NAME qml_smoke_diagnostics_panel_component_compiles
    COMMAND qml_smoke_test_all_components test_diagnostics_panel_component_compiles)

add_test(
    NAME qml_smoke_all_simple_qml_components_compile
    COMMAND qml_smoke_test_all_components test_all_simple_qml_components_compile)
