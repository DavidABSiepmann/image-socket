cmake_minimum_required(VERSION 3.5)
project(server_app LANGUAGES CXX)

# Ensure automoc is enabled so Qt's moc runs for headers with Q_OBJECT
set(CMAKE_AUTOMOC ON)

# Source directory for server app
set(SERVER_SRC_DIR ${CMAKE_SOURCE_DIR}/apps/server)

set(SERVER_SOURCES
    ${SERVER_SRC_DIR}/main.cpp
    ${SERVER_SRC_DIR}/src/diagnosticsmodel.cpp
    ${SERVER_SRC_DIR}/src/diagnosticsmanager.cpp
    ${SERVER_SRC_DIR}/include/diagnosticsmodel.h
    ${SERVER_SRC_DIR}/include/diagnosticsmanager.h
    ${SERVER_SRC_DIR}/qml.qrc
)

set(SERVER_INCLUDE_DIRS
    ${SERVER_SRC_DIR}/include
)

# Expose imagesocket headers (library built in src/imagesocket)
include_directories(${CMAKE_SOURCE_DIR}/src)

find_package(Qt5 COMPONENTS Quick Network Qml REQUIRED)
find_package(OpenCV REQUIRED)

add_executable(server_receiver ${SERVER_SOURCES})

target_include_directories(server_receiver PRIVATE ${SERVER_SRC_DIR} ${SERVER_INCLUDE_DIRS})

target_link_libraries(server_receiver PRIVATE Qt5::Quick Qt5::Network Qt5::Qml imagesocket ${OpenCV_LIBS})

# Keep the on-disk binary name consistent with existing builds
set_target_properties(server_receiver PROPERTIES OUTPUT_NAME "server-receiver")

# Ensure rpath behaviour is friendly for local testing
if(UNIX)
    set_target_properties(server_receiver PROPERTIES INSTALL_RPATH "$ORIGIN/../lib")
endif()

message(STATUS "Configured server target (source dir: ${SERVER_SRC_DIR})")
