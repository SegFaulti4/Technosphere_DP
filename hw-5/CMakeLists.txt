cmake_minimum_required(VERSION 3.17)
project(net)

set(CMAKE_CXX_STANDARD 17)
set(SERVER_LIB_NAME tcp)
set(SERVER_LIB_INCLUDE lib)
set(SERVER_LIB_SRC lib/Connection.cpp lib/Descriptor.cpp lib/Server.cpp)
set(PROJECT_SRC src/BufferedConnection.cpp src/EPoll.cpp src/Service.cpp)
set(PROJECT_INCLUDE include)

add_library(${SERVER_LIB_NAME} STATIC ${SERVER_LIB_SRC})
target_include_directories(${SERVER_LIB_NAME} PUBLIC ${SERVER_LIB_INCLUDE})

add_library(${PROJECT_NAME} STATIC ${PROJECT_SRC})
target_link_libraries(${PROJECT_NAME} PUBLIC ${SERVER_LIB_NAME})
target_include_directories(${PROJECT_NAME} PUBLIC ${PROJECT_INCLUDE} ${SERVER_LIB_INCLUDE})

add_executable(test test/test.cpp)
target_link_libraries(test PUBLIC ${PROJECT_NAME})
