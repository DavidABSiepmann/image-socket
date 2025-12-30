cmake_minimum_required(VERSION 3.5)
project(example_client LANGUAGES CXX)

set(CLIENT_SRC_DIR ${CMAKE_SOURCE_DIR}/apps/client)

find_package(OpenCV REQUIRED)
find_package(Boost REQUIRED COMPONENTS system)

add_executable(send_image_client ${CLIENT_SRC_DIR}/example_client.cpp)

set_target_properties(send_image_client PROPERTIES
    AUTOMOC OFF
    AUTOUIC OFF
    AUTORCC OFF)

# The imagesocket library and include path
target_include_directories(send_image_client PRIVATE ${CMAKE_SOURCE_DIR}/src)

target_link_libraries(send_image_client PRIVATE imagesocket ${OpenCV_LIBS} ${Boost_LIBRARIES})

set_target_properties(send_image_client PROPERTIES OUTPUT_NAME "send_image_client")
