cmake_minimum_required(VERSION 3.5)
project(imagesocket LANGUAGES CXX)

set(SOCKET_SRC_DIR ${CMAKE_CURRENT_SOURCE_DIR})

set(IMAGESOCKET_SOURCES
    ${CMAKE_SOURCE_DIR}/src/network/websocketserver.cpp
    ${CMAKE_SOURCE_DIR}/src/network/clientsession.cpp
    ${CMAKE_SOURCE_DIR}/src/network/clientmodel.cpp
    ${CMAKE_SOURCE_DIR}/src/network/imageserverbridge.cpp
    ${CMAKE_SOURCE_DIR}/src/network/qmlimageprovider.cpp
    ${CMAKE_SOURCE_DIR}/src/network/websocketimageclient.cpp
)

add_library(imagesocket STATIC ${IMAGESOCKET_SOURCES})

# Enable strict warnings for this target and propagate to consumers
target_compile_options(imagesocket PUBLIC -Wall -Wextra -Wpedantic)

# Expose headers
target_include_directories(imagesocket PUBLIC ${SOCKET_SRC_DIR} ${CMAKE_SOURCE_DIR}/src/network)

# Protobuf generation for control.proto
find_package(Protobuf REQUIRED)

set(PROTO_DIR ${CMAKE_SOURCE_DIR}/apps/proto)
# Place generated files under ${CMAKE_BINARY_DIR}/src/imagesocket/generated to match tests
set(PROTO_GEN_DIR ${CMAKE_CURRENT_BINARY_DIR}/imagesocket/generated)

file(MAKE_DIRECTORY ${PROTO_GEN_DIR})

add_custom_command(OUTPUT ${PROTO_GEN_DIR}/control.pb.cc ${PROTO_GEN_DIR}/control.pb.h
    COMMAND ${Protobuf_PROTOC_EXECUTABLE} --cpp_out=lite:${PROTO_GEN_DIR} -I ${PROTO_DIR} ${PROTO_DIR}/control.proto
    DEPENDS ${PROTO_DIR}/control.proto
    COMMENT "Generating protobuf-lite sources for control.proto"
)

set_property(SOURCE 
    ${PROTO_GEN_DIR}/control.pb.cc 
    ${PROTO_GEN_DIR}/control.pb.h
    PROPERTY SKIP_AUTOGEN ON)

add_custom_target(proto_imagesocket_generated DEPENDS ${PROTO_GEN_DIR}/control.pb.cc ${PROTO_GEN_DIR}/control.pb.h)
add_dependencies(imagesocket proto_imagesocket_generated)

# Include generated headers and compile generated sources into imagesocket
target_include_directories(imagesocket PUBLIC ${PROTO_GEN_DIR})
target_sources(imagesocket PRIVATE ${PROTO_GEN_DIR}/control.pb.cc)

if(TARGET protobuf::libprotobuf-lite)
    target_link_libraries(imagesocket PUBLIC protobuf::libprotobuf-lite)
else()
    target_link_libraries(imagesocket PUBLIC protobuf::libprotobuf)
endif()

# Propagate OpenCV, Boost and Qt to consumers
find_package(OpenCV REQUIRED)
find_package(Boost REQUIRED COMPONENTS system)
find_package(Qt5 COMPONENTS Core Network Qml Quick WebSockets REQUIRED)

target_link_libraries(imagesocket PUBLIC ${OpenCV_LIBS} ${Boost_LIBRARIES} Qt5::Core Qt5::Network Qt5::Qml Qt5::Quick Qt5::WebSockets Qt5::Gui)

message(STATUS "Configured imagesocket static library with sources from ${SOCKET_SRC_DIR}")
