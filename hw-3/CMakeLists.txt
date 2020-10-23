cmake_minimum_required(VERSION 3.17)
project(tcp)

set(PROJECT_SRC src/Connection.cpp src/Descriptor.cpp src/Server.cpp)
set(PROJECT_INCLUDE include)

add_library(${PROJECT_NAME} STATIC ${PROJECT_SRC})
target_include_directories(${PROJECT_NAME} PUBLIC ${PROJECT_INCLUDE})
