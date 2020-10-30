cmake_minimum_required(VERSION 3.17)
project(shmem)

set(CMAKE_CXX_STANDARD 17)
set(PROJECT_INCLUDE include)
set(PROJECT_SRC src/SharedAllocator.cpp src/Semaphore.cpp)

add_executable(${PROJECT_NAME} main.cpp)
target_compile_options(${PROJECT_NAME} PUBLIC -lpthread -pthread -lm -lrt)
target_include_directories(${PROJECT_NAME} PUBLIC ${PROJECT_INCLUDE})
target_sources(${PROJECT_NAME} PUBLIC ${PROJECT_SRC})
