cmake_minimum_required(VERSION 3.5)

project(CANopenMotor)
add_definitions(-std=c++17)
include_directories(
    include
    /usr/include/c++/7
    )

ADD_EXECUTABLE(canopen src/main.cpp)
target_link_libraries(canopen usbcan pthread)

ADD_EXECUTABLE(canopen_test test/test.cpp)
target_link_libraries(canopen_test usbcan pthread)