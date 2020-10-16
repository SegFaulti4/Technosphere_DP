cmake_minimum_required(VERSION 3.17)

set(LIBRARY_OUTPUT_PATH ../bin)
add_library(${TMP_LIB_NAME} STATIC FileDes.cpp Pipe.cpp Process.cpp)
