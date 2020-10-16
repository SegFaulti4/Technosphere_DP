cmake_minimum_required(VERSION 3.17)
project(hw_2)

set(LOG_LIB_NAME log)
include_directories(include)
add_subdirectory(src)

add_executable(${PROJECT_NAME} main.cpp)
target_link_libraries(${PROJECT_NAME} ${LOG_LIB_NAME})
