cmake_minimum_required(VERSION 3.17)

set(LIBRARY_OUTPUT_PATH ../bin)
add_library(${LOG_LIB_NAME} STATIC BaseLogger.cpp FileLogger.cpp
        FileDes.cpp StderrLogger.cpp StdoutLogger.cpp)
