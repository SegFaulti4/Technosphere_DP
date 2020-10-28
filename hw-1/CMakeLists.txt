cmake_minimum_required(VERSION 3.17)
project(hw_1)

set(PROCESS_LIB src/Descriptor.cpp src/Process.cpp src/Pipe.cpp)
set(PROCESS_INCLUDE include)

add_executable(${PROJECT_NAME} main.cpp)
target_sources(${PROJECT_NAME} PUBLIC ${PROCESS_LIB})
target_include_directories(${PROJECT_NAME} PUBLIC ${PROCESS_INCLUDE})

add_executable(bypass test/bypass.cpp)

add_executable(test test/test.cpp)
target_sources(test PUBLIC ${PROCESS_LIB})
target_include_directories(test PUBLIC ${PROCESS_INCLUDE})
