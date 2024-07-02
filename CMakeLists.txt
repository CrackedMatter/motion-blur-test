cmake_minimum_required(VERSION 3.5)

project(motionblurtest)

set(CMAKE_CXX_STANDARD 17)

add_library(motionblurtest SHARED src/main.cpp)
target_link_libraries(motionblurtest PUBLIC EGL GLESv2)

install(TARGETS motionblurtest LIBRARY DESTINATION mods)
