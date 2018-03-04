cmake_minimum_required(VERSION 2.8)
project(P2)

set(CMAKE_CXX_STANDARD 17)

find_package (Threads)

set(SOURCE_FILES
        Complex.cc
        Complex.h
        InputImage.cc
        InputImage.h
        fft2d.cc
        )
add_executable(P2 ${SOURCE_FILES})

target_link_libraries (P2 ${CMAKE_THREAD_LIBS_INIT})

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -pthread")