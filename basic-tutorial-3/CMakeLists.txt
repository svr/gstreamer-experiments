
cmake_minimum_required(VERSION 4.3)
project(basic-tutorial-3 LANGUAGES CXX)

add_executable(${PROJECT_NAME} main.cpp)
target_link_libraries(${PROJECT_NAME} PRIVATE gst_common)
