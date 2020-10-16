cmake_minimum_required(VERSION 3.17)
project(hw_1)

set(TMP_LIB_NAME TmpLib)
include_directories(include)
add_subdirectory(src)

add_executable(${PROJECT_NAME} main.cpp)
target_link_libraries(${PROJECT_NAME} ${TMP_LIB_NAME})
