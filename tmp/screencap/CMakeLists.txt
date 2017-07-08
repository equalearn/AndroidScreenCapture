cmake_minimum_required(VERSION 3.7)
project(screencap)

set(CMAKE_CXX_STANDARD 11)

set(SOURCE_FILES
        screencap.cpp)

include_directories("/run/media/lxy/000E004000077550/aosp_tuna/frameworks/native/include")
include_directories("/run/media/lxy/000E004000077550/aosp_tuna/system/core/libutils/include")
include_directories("/run/media/lxy/000E004000077550/aosp_tuna/external/skia/include/core")
add_executable(screencap ${SOURCE_FILES})