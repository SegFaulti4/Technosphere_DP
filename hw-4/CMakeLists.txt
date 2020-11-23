cmake_minimum_required(VERSION 3.17)
project(shmem)

set(CMAKE_CXX_FLAGS -pthread)
set(CMAKE_CXX_STANDARD 17)
set(PROJECT_INCLUDE include)
set(PROJECT_SRC src/SharedAllocator.cpp src/Semaphore.cpp src/SharedMem.cpp)

add_library(${PROJECT_NAME} STATIC ${PROJECT_SRC})
target_include_directories(${PROJECT_NAME} PUBLIC ${PROJECT_INCLUDE})

add_executable(test main.cpp)
target_link_libraries(test PUBLIC ${PROJECT_NAME})
