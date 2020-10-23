cmake_minimum_required(VERSION 3.17)
project(hw_1)

set(PROCESS_LIB src/Descriptor.cpp src/Process.cpp src/Pipe.cpp)
set(PROCESS_INCLUDE include)
#set(CMAKE_LIBRARY_OUTPUT_DIRECTORY bin)

add_executable(${PROJECT_NAME} main.cpp)
target_sources(${PROJECT_NAME} PUBLIC ${PROCESS_LIB})
target_include_directories(${PROJECT_NAME} PUBLIC ${PROCESS_INCLUDE})
#target_sources(${PROJECT_NAME} PUBLIC main.cpp)
