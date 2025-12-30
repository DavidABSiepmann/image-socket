cmake_minimum_required(VERSION 3.16)

# Define executable for component state tests
add_executable(qml_component_test_all_components
    test_qml_component_state.cpp
)

# Link Qt libraries
target_link_libraries(qml_component_test_all_components
    Qt5::Core
    Qt5::Gui
    Qt5::Qml
    Qt5::Quick
    Qt5::Test
)

# Add include paths for Qt
target_include_directories(qml_component_test_all_components PRIVATE
    ${Qt5Core_INCLUDE_DIRS}
    ${Qt5Gui_INCLUDE_DIRS}
    ${Qt5Qml_INCLUDE_DIRS}
    ${Qt5Quick_INCLUDE_DIRS}
    ${Qt5Test_INCLUDE_DIRS}
)

# Enable automoc for this target (processes Q_OBJECT macros)
set_target_properties(qml_component_test_all_components PROPERTIES
    AUTOMOC ON
)

# Register individual test cases with CTest
# Each test method becomes a separate CTest test
add_test(NAME qml_component_theme_default_properties
         COMMAND qml_component_test_all_components test_theme_default_properties)

add_test(NAME qml_component_infooverlay_visibility_toggle
         COMMAND qml_component_test_all_components test_infooverlay_visibility_toggle)

add_test(NAME qml_component_styledbutton_enabled_state
         COMMAND qml_component_test_all_components test_styledbutton_enabled_state)

add_test(NAME qml_component_statusbar_text_binding
         COMMAND qml_component_test_all_components test_statusbar_text_binding)

add_test(NAME qml_component_initialization_properties
         COMMAND qml_component_test_all_components test_component_initialization_properties)

add_test(NAME qml_component_mockbackend_signal_emission
         COMMAND qml_component_test_all_components test_mockbackend_signal_emission)

add_test(NAME qml_component_state_sequence
         COMMAND qml_component_test_all_components test_component_state_sequence)

add_test(NAME qml_component_theme_mode_defaults
         COMMAND qml_component_test_all_components test_theme_mode_defaults)
