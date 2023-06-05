cmake_minimum_required(VERSION 3.5)
project(send_image_client)

set(CMAKE_CXX_STANDARD 11)

set(Boost_USE_STATIC_LIBS ON)
find_package(Boost REQUIRED COMPONENTS system)
include_directories(${Boost_INCLUDE_DIRS})

find_package( OpenCV REQUIRED )
include_directories( ${OpenCV_INCLUDE_DIRS} )


include_directories( ${PROJECT_SOURCE_DIR} )

add_executable( send_image_client test.cpp imageSocketClient.cpp )
target_link_libraries( send_image_client ${OpenCV_LIBS} ${Boost_LIBRARIES} )