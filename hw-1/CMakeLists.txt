cmake_minimum_required(VERSION 3.17)
project(hw_1)

set(PROCESS_LIB_SRC src/Descriptor.cpp src/Process.cpp src/Pipe.cpp)
set(PROCESS_LIB_NAME Process)
set(PROCESS_INCLUDE include)

add_library(${PROCESS_LIB_NAME} STATIC ${PROCESS_LIB_SRC})
target_include_directories(${PROCESS_LIB_NAME} PUBLIC ${PROCESS_INCLUDE})

add_executable(${PROJECT_NAME} main.cpp)
target_include_directories(${PROJECT_NAME} PUBLIC ${PROCESS_INCLUDE})
target_link_libraries(${PROJECT_NAME} PUBLIC ${PROCESS_LIB_NAME})

add_executable(bypass test/bypass.cpp)

add_executable(test test/test.cpp)
target_include_directories(test PUBLIC ${PROCESS_INCLUDE})
target_link_libraries(test PUBLIC ${PROCESS_LIB_NAME})
