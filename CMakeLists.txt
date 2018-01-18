cmake_minimum_required(VERSION 3.5)
project(SaliencyDetection)

set(CMAKE_CXX_STANDARD 11)
find_package( OpenCV REQUIRED )

set(SOURCE_FILES main.cpp SaliencyDetect.cpp SaliencyDetect.h)
add_executable(SaliencyDetection ${SOURCE_FILES})
target_link_libraries( SaliencyDetection ${OpenCV_LIBS} )