cmake_minimum_required(VERSION 3.17)
project(http)

set(CMAKE_CXX_FLAGS -pthread)
set(CMAKE_CXX_STANDARD 17)

set(TCP_LIB_NAME tcp)
set(TCP_LIB_INCLUDE lib/tcp/include)
set(TCP_LIB_SRC lib/tcp/src/Connection.cpp lib/tcp/src/Descriptor.cpp lib/tcp/src/Server.cpp)

set(NET_LIB_NAME net)
set(NET_LIB_INCLUDE lib/net/include)
set(NET_LIB_SRC lib/net/src/BufferedConnection.cpp lib/net/src/EPoll.cpp lib/net/src/Service.cpp)

set(LOGGER_LIB_NAME logger)
set(LOGGER_LIB_INCLUDE lib/logger/include)
set(LOGGER_LIB_SRC lib/logger/src/BaseLogger.cpp lib/logger/src/Global.cpp lib/logger/src/Logger.cpp)

set(COROUTINE_LIB_NAME coroutine)
set(COROUTINE_LIB_INCLUDE lib/coroutine/include)
set(COROUTINE_LIB_SRC lib/coroutine/src/Coroutine.cpp)

set(PROJECT_SRC src/HttpService.cpp src/HttpConnection.cpp)
set(PROJECT_INCLUDE include)

add_library(${TCP_LIB_NAME} STATIC ${TCP_LIB_SRC})
target_include_directories(${TCP_LIB_NAME} PUBLIC ${TCP_LIB_INCLUDE})

add_library(${NET_LIB_NAME} STATIC ${NET_LIB_SRC})
target_include_directories(${NET_LIB_NAME} PUBLIC ${NET_LIB_INCLUDE})
target_link_libraries(${NET_LIB_NAME} PUBLIC ${TCP_LIB_NAME})

add_library(${LOGGER_LIB_NAME} STATIC ${LOGGER_LIB_SRC})
target_include_directories(${LOGGER_LIB_NAME} PUBLIC ${LOGGER_LIB_INCLUDE})

add_library(${COROUTINE_LIB_NAME} STATIC ${COROUTINE_LIB_SRC})
target_include_directories(${COROUTINE_LIB_NAME} PUBLIC ${COROUTINE_LIB_INCLUDE})

add_library(${PROJECT_NAME} STATIC ${PROJECT_SRC})
target_include_directories(${PROJECT_NAME} PUBLIC ${PROJECT_INCLUDE})
target_link_libraries(${PROJECT_NAME} PUBLIC ${NET_LIB_NAME} ${LOGGER_LIB_NAME} ${COROUTINE_LIB_NAME})

add_executable(test main.cpp)
target_link_libraries(test PUBLIC ${PROJECT_NAME})
