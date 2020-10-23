cmake_minimum_required(VERSION 3.17)
project(log)

set(PROJECT_SRC src/BaseLogger.cpp src/FileLogger.cpp
        src/Descriptor.cpp src/StderrLogger.cpp src/StdoutLogger.cpp)
set(PROJECT_INCLUDE include)

add_library(${PROJECT_NAME} STATIC ${PROJECT_SRC})
target_include_directories(${PROJECT_NAME} PUBLIC ${PROJECT_INCLUDE})
#target_sources(${PROJECT_NAME} PUBLIC main.cpp)
