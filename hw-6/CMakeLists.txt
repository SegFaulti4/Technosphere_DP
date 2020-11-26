cmake_minimum_required(VERSION 3.17)
project(http)

set(CMAKE_CXX_FLAGS -pthread)
set(CMAKE_CXX_STANDARD 17)

set(TCP_LIB_NAME tcp)
set(TCP_LIB_INCLUDE lib/tcp)
set(TCP_LIB_SRC lib/tcp/Connection.cpp lib/tcp/Descriptor.cpp lib/tcp/Server.cpp)

set(NET_LIB_NAME net)
set(NET_LIB_INCLUDE lib/net)
set(NET_LIB_SRC lib/net/BufferedConnection.cpp lib/net/EPoll.cpp lib/net/Service.cpp)

set(PROJECT_SRC src/HttpService.cpp src/HttpConnection.cpp)
set(PROJECT_INCLUDE include)

add_library(${TCP_LIB_NAME} STATIC ${TCP_LIB_SRC})
target_include_directories(${TCP_LIB_NAME} PUBLIC ${TCP_LIB_INCLUDE})

add_library(${NET_LIB_NAME} STATIC ${NET_LIB_SRC})
target_include_directories(${NET_LIB_NAME} PUBLIC ${NET_LIB_INCLUDE})
target_link_libraries(${NET_LIB_NAME} PUBLIC ${TCP_LIB_NAME})

add_library(${PROJECT_NAME} STATIC ${PROJECT_SRC})
target_include_directories(${PROJECT_NAME} PUBLIC ${PROJECT_INCLUDE})
target_link_libraries(${PROJECT_NAME} PUBLIC ${NET_LIB_NAME})
