cmake_minimum_required(VERSION 3.5)

find_package(Qt5 COMPONENTS Core WebSockets Test Gui Qml Quick REQUIRED)
find_package(Protobuf REQUIRED)
find_package(OpenCV REQUIRED)

# -------------------------------------------------------------------
# WEBSOCKET TESTS (qt_websocket_*)
# -------------------------------------------------------------------
# WebSocket server and client integration tests

set(QT_WEBSOCKET_TESTS
    websocket/test_websocket_server_accept.cpp
    websocket/test_websocket_messages.cpp
    websocket/test_websocket_multiple_clients.cpp
    websocket/test_websocket_frame_integrity.cpp
    websocket/test_websocket_robustness.cpp
)

set(QT_WEBSOCKET_FIXTURES
    fixtures/test_websocket_environment.cpp
)

foreach(test_file ${QT_WEBSOCKET_TESTS})
    get_filename_component(test_name ${test_file} NAME_WE)
    set(target_name "qt_websocket_${test_name}")

    add_executable(${target_name} ${test_file} ${QT_WEBSOCKET_FIXTURES})

    target_link_libraries(${target_name}
        Qt5::Core
        Qt5::Test
        Qt5::WebSockets
        Qt5::Network
        Qt5::Gui
        imagesocket
        protobuf::libprotobuf-lite
        opencv_core
        opencv_imgcodecs
    )

    target_include_directories(${target_name} PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${CMAKE_BINARY_DIR}/src/imagesocket/generated
    )

    set_target_properties(${target_name} PROPERTIES
        AUTOMOC ON
        AUTORCC ON
        CXX_STANDARD 17
        CXX_STANDARD_REQUIRED ON
    )

    add_test(NAME ${target_name} COMMAND ${target_name})
endforeach()

# -------------------------------------------------------------------
# STATE TESTS (qt_state_*)
# -------------------------------------------------------------------
# State machine and transition tests

set(QT_STATE_TESTS
    state/test_server_bridge_initial_state.cpp
    state/test_server_start_stop_transitions.cpp
    state/test_connection_state_transitions.cpp
)

foreach(test_file ${QT_STATE_TESTS})
    get_filename_component(test_name ${test_file} NAME_WE)
    set(target_name "qt_state_${test_name}")

    add_executable(${target_name} ${test_file})

    target_link_libraries(${target_name}
        Qt5::Core
        Qt5::Test
        Qt5::WebSockets
        Qt5::Network
        Qt5::Gui
        imagesocket
    )

    target_include_directories(${target_name} PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
    )

    set_target_properties(${target_name} PROPERTIES
        AUTOMOC ON
        AUTORCC ON
        CXX_STANDARD 17
        CXX_STANDARD_REQUIRED ON
    )

    add_test(NAME ${target_name} COMMAND ${target_name})
endforeach()

# -------------------------------------------------------------------
# SIGNALS TESTS (qt_signals_*)
# -------------------------------------------------------------------
# Signal emission and parameter validation tests

set(QT_SIGNALS_TESTS
    signals/test_status_message_signal.cpp
    signals/test_fps_signals.cpp
    signals/test_client_signals.cpp
)

foreach(test_file ${QT_SIGNALS_TESTS})
    get_filename_component(test_name ${test_file} NAME_WE)
    set(target_name "qt_signals_${test_name}")

    add_executable(${target_name} ${test_file})

    target_link_libraries(${target_name}
        Qt5::Core
        Qt5::Test
        Qt5::WebSockets
        Qt5::Network
        Qt5::Gui
        imagesocket
    )

    target_include_directories(${target_name} PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
    )

    set_target_properties(${target_name} PROPERTIES
        AUTOMOC ON
        AUTORCC ON
        CXX_STANDARD 17
        CXX_STANDARD_REQUIRED ON
    )

    add_test(NAME ${target_name} COMMAND ${target_name})
endforeach()

# -------------------------------------------------------------------
# MODELS TESTS (qt_models_*)
# -------------------------------------------------------------------
# ClientModel row insertion/removal and role data tests

set(QT_MODELS_TESTS
    models/test_client_model_rows.cpp
    models/test_model_role_data.cpp
)

foreach(test_file ${QT_MODELS_TESTS})
    get_filename_component(test_name ${test_file} NAME_WE)
    set(target_name "qt_models_${test_name}")

    add_executable(${target_name} ${test_file})

    target_link_libraries(${target_name}
        Qt5::Core
        Qt5::Test
        Qt5::WebSockets
        Qt5::Network
        Qt5::Gui
        imagesocket
    )

    target_include_directories(${target_name} PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
    )

    set_target_properties(${target_name} PROPERTIES
        AUTOMOC ON
        AUTORCC ON
        CXX_STANDARD 17
        CXX_STANDARD_REQUIRED ON
    )

    add_test(NAME ${target_name} COMMAND ${target_name})
endforeach()

# -------------------------------------------------------------------
# SMOKE TESTS (qt_smoke_*)
# -------------------------------------------------------------------
# Basic instantiation and framework dependency tests

set(QT_SMOKE_TESTS
    smoke/test_component_instantiation.cpp
    smoke/test_dependencies.cpp
)

foreach(test_file ${QT_SMOKE_TESTS})
    get_filename_component(test_name ${test_file} NAME_WE)
    set(target_name "qt_smoke_${test_name}")

    add_executable(${target_name} ${test_file})

    target_link_libraries(${target_name}
        Qt5::Core
        Qt5::Test
        Qt5::WebSockets
        Qt5::Gui
        imagesocket
    )

    target_include_directories(${target_name} PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
    )

    set_target_properties(${target_name} PROPERTIES
        AUTOMOC ON
        AUTORCC ON
        CXX_STANDARD 17
        CXX_STANDARD_REQUIRED ON
    )

    add_test(NAME ${target_name} COMMAND ${target_name})
endforeach()

# Create a dummy target to ensure the category is recognized
add_library(unit_qt_cpp INTERFACE)
message(STATUS "Qt C++ tests infrastructure ready")
message(STATUS "  Qt State tests: ${QT_STATE_TESTS}")
message(STATUS "  Qt Signals tests: ${QT_SIGNALS_TESTS}")
message(STATUS "  Qt Models tests: ${QT_MODELS_TESTS}")
message(STATUS "  Qt WebSocket tests: ${QT_WEBSOCKET_TESTS}")
message(STATUS "  Qt Smoke tests: ${QT_SMOKE_TESTS}")
