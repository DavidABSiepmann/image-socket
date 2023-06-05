# Image socket server and client

This is a simple example of a socket server and client that sends images from a client to a server.

## Dependencies

- OpenCV 3.4.1
- Boost 1.65.1
- C++ 11
- CMake 3.5
- Linux

## Build

Clone the repository, replace the video path in `client-sender/test.cpp:22` with the path to your video file. Then build the client:

```bash
mkdir build && cd build
cmake ../client-sender
make -j16
./send_image_client
```

Then build the server, can use qtcreator to open the project and build it. Or use the following commands:

```bash
# mkdir -p build && cd build # If you haven't created the build directory yet, but if you already created it, you don't need to create it again.
qmake ../server-receiver/server-receiver.pro
make -j16
./server-receiver
```

## How it works

The client connects to the server and sends the images from the video file. The server receives the images and displays them in a window. To start a transmission need click in the button start on the server window. To stop the transmission need click in the button stop on the server window. If the client disconnects, the server will wait for a new connection. If the server disconnects, the client will try to reconnect to the server.

If after click in the button start on the server window, the server doesn't receive any image check if the client is connected to the server. If the client is connected to the server, check if the client is sending images. If the client is sending images, check if the server is receiving images. If the server is receiving images, check if the server is displaying the images.

## How to use in your project

For the client copy the files `imageSocketClient.h` and `imageSocketClient.cpp` to your project and add the following lines to your `CMakeLists.txt`:

```cmake
find_package(OpenCV REQUIRED)
find_package(Boost REQUIRED COMPONENTS system thread)

include_directories(${OpenCV_INCLUDE_DIRS})
include_directories(${Boost_INCLUDE_DIRS})

add_executable(your_executable_name your_source_files.cpp imageSocketClient.cpp)
target_link_libraries(your_executable_name ${OpenCV_LIBS} ${Boost_LIBRARIES})
```

For the server copy the files `imageSocketServer.h` and `imageSocketServer.cpp` to your project and add the following lines to your .pro file:

```pro
QT += core gui widgets network

INCLUDEPATH += /usr/local/include/opencv4
LIBS += -L/usr/local/lib -lopencv_core -lopencv_highgui -lopencv_imgproc -lopencv_videoio

SOURCES += \
    your_source_files.cpp \
    imageSocketServer.cpp
```
