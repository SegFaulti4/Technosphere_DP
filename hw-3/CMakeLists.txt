cmake_minimum_required(VERSION 3.17)
project(tcp)

set(CMAKE_CXX_STANDARD 17)
set(PROJECT_SRC src/Connection.cpp src/Descriptor.cpp src/Server.cpp)
set(PROJECT_INCLUDE include)

add_library(${PROJECT_NAME} STATIC ${PROJECT_SRC})
target_include_directories(${PROJECT_NAME} PUBLIC ${PROJECT_INCLUDE})

add_executable(client_test test/client_test.cpp)
target_link_libraries(client_test PUBLIC ${PROJECT_NAME})
target_include_directories(client_test PUBLIC ${PROJECT_INCLUDE})

add_executable(server_test test/server_test.cpp)
target_link_libraries(server_test PUBLIC ${PROJECT_NAME})
target_include_directories(server_test PUBLIC ${PROJECT_INCLUDE})
