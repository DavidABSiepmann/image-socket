cmake_minimum_required(VERSION 3.5)

# Find and integrate GoogleTest
include(FetchContent)
FetchContent_Declare(
    googletest
    URL https://github.com/google/googletest/archive/refs/tags/v1.14.0.zip
)

set(CMAKE_AUTOMOC OFF)
set(CMAKE_AUTOUIC OFF)
set(CMAKE_AUTORCC OFF)

# For Windows: Prevent overriding the parent project's compiler/linker settings
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

# Include required packages
find_package(Protobuf REQUIRED)

# Set up include path for fixtures directory
set(FIXTURES_DIR ${CMAKE_CURRENT_SOURCE_DIR}/fixtures)

# -------------------------------------------------------------------
# PROTOCOL TESTS (unit_protocol_*)
# -------------------------------------------------------------------
# Protocol tests verify protobuf serialization, validation, and parsing

# Protocol test: Serialization roundtrip
add_executable(unit_protocol_serialization protocol/test_protobuf_serialization.cpp)
target_include_directories(unit_protocol_serialization PRIVATE 
    ${CMAKE_BINARY_DIR}/src/imagesocket/generated
    ${FIXTURES_DIR}
)
add_dependencies(unit_protocol_serialization proto_imagesocket_generated)
target_link_libraries(unit_protocol_serialization PRIVATE imagesocket GTest::gtest GTest::gtest_main)
add_test(NAME unit_protocol_serialization COMMAND unit_protocol_serialization)

# Protocol test: Message validation
add_executable(unit_protocol_validation protocol/test_protobuf_validation.cpp)
target_include_directories(unit_protocol_validation PRIVATE 
    ${CMAKE_BINARY_DIR}/src/imagesocket/generated
    ${FIXTURES_DIR}
)
add_dependencies(unit_protocol_validation proto_imagesocket_generated)
target_link_libraries(unit_protocol_validation PRIVATE imagesocket GTest::gtest GTest::gtest_main)
add_test(NAME unit_protocol_validation COMMAND unit_protocol_validation)

# Protocol test: Payload size limits
add_executable(unit_protocol_size_limits protocol/test_payload_size_limits.cpp)
target_include_directories(unit_protocol_size_limits PRIVATE 
    ${CMAKE_BINARY_DIR}/src/imagesocket/generated
    ${FIXTURES_DIR}
)
add_dependencies(unit_protocol_size_limits proto_imagesocket_generated)
target_link_libraries(unit_protocol_size_limits PRIVATE imagesocket GTest::gtest GTest::gtest_main)
add_test(NAME unit_protocol_size_limits COMMAND unit_protocol_size_limits)

# Protocol test: Message type discrimination
add_executable(unit_protocol_type_discrimination protocol/test_message_type_discrimination.cpp)
target_include_directories(unit_protocol_type_discrimination PRIVATE 
    ${CMAKE_BINARY_DIR}/src/imagesocket/generated
    ${FIXTURES_DIR}
)
add_dependencies(unit_protocol_type_discrimination proto_imagesocket_generated)
target_link_libraries(unit_protocol_type_discrimination PRIVATE imagesocket GTest::gtest GTest::gtest_main)
add_test(NAME unit_protocol_type_discrimination COMMAND unit_protocol_type_discrimination)

# -------------------------------------------------------------------
# PARSING TESTS (unit_parsing_*)
# -------------------------------------------------------------------
# Parsing tests verify configuration parsing, byte order, framing, and state transitions

# Parsing test: Configuration parsing
add_executable(unit_parsing_configuration parsing/test_configuration_parsing.cpp)
target_include_directories(unit_parsing_configuration PRIVATE ${FIXTURES_DIR})
target_link_libraries(unit_parsing_configuration PRIVATE GTest::gtest GTest::gtest_main)
add_test(NAME unit_parsing_configuration COMMAND unit_parsing_configuration)

# Parsing test: Byte order conversion
add_executable(unit_parsing_byte_order parsing/test_byte_order.cpp)
target_include_directories(unit_parsing_byte_order PRIVATE ${FIXTURES_DIR})
target_link_libraries(unit_parsing_byte_order PRIVATE GTest::gtest GTest::gtest_main)
add_test(NAME unit_parsing_byte_order COMMAND unit_parsing_byte_order)

# Parsing test: Frame parsing (length-prefixed messages)
add_executable(unit_parsing_frames parsing/test_frame_parsing.cpp)
target_include_directories(unit_parsing_frames PRIVATE ${FIXTURES_DIR})
target_link_libraries(unit_parsing_frames PRIVATE GTest::gtest GTest::gtest_main)
add_test(NAME unit_parsing_frames COMMAND unit_parsing_frames)

# Parsing test: State transitions
add_executable(unit_parsing_state_transitions parsing/test_state_transitions.cpp)
target_include_directories(unit_parsing_state_transitions PRIVATE 
    ${CMAKE_BINARY_DIR}/src/imagesocket/generated
    ${FIXTURES_DIR}
)
add_dependencies(unit_parsing_state_transitions proto_imagesocket_generated)
target_link_libraries(unit_parsing_state_transitions PRIVATE imagesocket GTest::gtest GTest::gtest_main)
add_test(NAME unit_parsing_state_transitions COMMAND unit_parsing_state_transitions)

# -------------------------------------------------------------------
# CLIENT LOGIC TESTS (unit_client_*)
# -------------------------------------------------------------------
# Client logic tests verify reconnection backoff, state machine, configuration accumulation, and error handling

# Client test: Reconnection backoff calculation
add_executable(unit_client_backoff client/test_reconnect_backoff.cpp)
target_include_directories(unit_client_backoff PRIVATE ${FIXTURES_DIR})
target_link_libraries(unit_client_backoff PRIVATE GTest::gtest GTest::gtest_main)
add_test(NAME unit_client_backoff COMMAND unit_client_backoff)

# Client test: State machine transitions
add_executable(unit_client_state_machine client/test_client_state_machine.cpp)
target_include_directories(unit_client_state_machine PRIVATE ${FIXTURES_DIR})
target_link_libraries(unit_client_state_machine PRIVATE GTest::gtest GTest::gtest_main)
add_test(NAME unit_client_state_machine COMMAND unit_client_state_machine)

# Client test: Configuration accumulation
add_executable(unit_client_accumulation client/test_configuration_accumulation.cpp)
target_include_directories(unit_client_accumulation PRIVATE ${FIXTURES_DIR})
target_link_libraries(unit_client_accumulation PRIVATE GTest::gtest GTest::gtest_main)
add_test(NAME unit_client_accumulation COMMAND unit_client_accumulation)

# Client test: Error callback propagation
add_executable(unit_client_error_callback client/test_error_callback.cpp)
target_include_directories(unit_client_error_callback PRIVATE ${FIXTURES_DIR})
target_link_libraries(unit_client_error_callback PRIVATE GTest::gtest GTest::gtest_main gmock)
add_test(NAME unit_client_error_callback COMMAND unit_client_error_callback)

# -------------------------------------------------------------------
# SMOKE TESTS (test_smoke_*)
# -------------------------------------------------------------------
# Smoke tests verify basic library linkage, enum availability, and object construction

# Smoke test: Linkage verification
add_executable(smoke_linkage smoke/test_smoke_linkage.cpp)
target_include_directories(smoke_linkage PRIVATE ${CMAKE_BINARY_DIR}/src/imagesocket/generated)
add_dependencies(smoke_linkage proto_imagesocket_generated)
target_link_libraries(smoke_linkage PRIVATE imagesocket GTest::gtest GTest::gtest_main)
add_test(NAME smoke_linkage COMMAND smoke_linkage)

# Smoke test: Protobuf enum verification
add_executable(smoke_enums smoke/test_smoke_enums.cpp)
target_include_directories(smoke_enums PRIVATE ${CMAKE_BINARY_DIR}/src/imagesocket/generated)
add_dependencies(smoke_enums proto_imagesocket_generated)
target_link_libraries(smoke_enums PRIVATE imagesocket GTest::gtest GTest::gtest_main)
add_test(NAME smoke_enums COMMAND smoke_enums)

# Smoke test: Object construction verification
add_executable(smoke_construction smoke/test_smoke_construction.cpp)
target_include_directories(smoke_construction PRIVATE ${CMAKE_BINARY_DIR}/src/imagesocket/generated)
add_dependencies(smoke_construction proto_imagesocket_generated)
target_link_libraries(smoke_construction PRIVATE imagesocket GTest::gtest GTest::gtest_main)
add_test(NAME smoke_construction COMMAND smoke_construction)

# Smoke test: Fixtures availability and usage
add_executable(smoke_fixtures smoke/test_smoke_fixtures.cpp)
target_include_directories(smoke_fixtures PRIVATE 
    ${CMAKE_BINARY_DIR}/src/imagesocket/generated
    ${FIXTURES_DIR}
)
add_dependencies(smoke_fixtures proto_imagesocket_generated)
target_link_libraries(smoke_fixtures PRIVATE imagesocket GTest::gtest GTest::gtest_main gmock)
add_test(NAME smoke_fixtures COMMAND smoke_fixtures)

# Create a dummy target to ensure the category is recognized
add_library(unit_cpp_std INTERFACE)
message(STATUS "Unit (C++ std/Boost) tests infrastructure ready")
message(STATUS "  Smoke tests: smoke_linkage, smoke_enums, smoke_construction, smoke_fixtures")
