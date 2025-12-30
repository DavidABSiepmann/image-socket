# QML Integration Tests
# Tests multi-component interactions and state propagation

cmake_minimum_required(VERSION 3.16)

set(TEST_NAME qml_integration_test_all_interactions)
set(TEST_SOURCES test_qml_integration.cpp)

add_executable(${TEST_NAME} ${TEST_SOURCES})

target_link_libraries(${TEST_NAME}
    Qt5::Core
    Qt5::Gui
    Qt5::Qml
    Qt5::Quick
    Qt5::Test
)

set_target_properties(${TEST_NAME} PROPERTIES
    AUTOMOC ON
    AUTORCC ON
)

# Register individual integration tests
add_test(NAME qml_integration_running_state_propagates
    COMMAND ${TEST_NAME} test_running_state_propagates_to_components)

add_test(NAME qml_integration_client_selection_updates_display
    COMMAND ${TEST_NAME} test_client_selection_updates_display)

add_test(NAME qml_integration_error_handling
    COMMAND ${TEST_NAME} test_error_state_triggers_diagnostics_and_toast)

add_test(NAME qml_integration_theme_propagation
    COMMAND ${TEST_NAME} test_theme_propagates_to_all_components)

add_test(NAME qml_integration_multi_state_coordination
    COMMAND ${TEST_NAME} test_multiple_state_changes_coordinate)

add_test(NAME qml_integration_signal_chains
    COMMAND ${TEST_NAME} test_component_signal_chains)
