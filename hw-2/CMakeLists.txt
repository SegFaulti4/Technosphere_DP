cmake_minimum_required(VERSION 3.17)
project(log)

set(CMAKE_CXX_STANDARD 17)
set(PROJECT_SRC src/BaseLogger.cpp src/FileLogger.cpp
        src/Descriptor.cpp src/StderrLogger.cpp src/StdoutLogger.cpp)
set(PROJECT_INCLUDE include)

add_library(${PROJECT_NAME} STATIC ${PROJECT_SRC})
target_include_directories(${PROJECT_NAME} PUBLIC ${PROJECT_INCLUDE})

add_executable(test_1 test/test_1.cpp)
target_include_directories(test_1 PUBLIC ${PROJECT_INCLUDE})
target_link_libraries(test_1 PUBLIC ${PROJECT_NAME})

add_executable(test_2 test/test_2.cpp)
target_include_directories(test_2 PUBLIC ${PROJECT_INCLUDE})
target_link_libraries(test_2 PUBLIC ${PROJECT_NAME})

add_executable(test_3 test/test_3.cpp)
target_include_directories(test_3 PUBLIC ${PROJECT_INCLUDE})
target_link_libraries(test_3 PUBLIC ${PROJECT_NAME})
