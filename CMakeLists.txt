cmake_minimum_required(VERSION 3.5)

project(CANopenMotor)
add_definitions(-std=c++11)
include_directories(include)
ADD_EXECUTABLE(main src/main.cpp)
target_link_libraries(main usbcan pthread)