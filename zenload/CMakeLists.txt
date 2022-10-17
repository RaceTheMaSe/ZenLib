cmake_minimum_required(VERSION 2.9)
project(ZenLoad)

file(GLOB SRC
    *.cpp
    *.h
)

add_library(zenload STATIC ${SRC})
target_link_libraries(zenload utils vdfs)
target_include_directories(zenload PUBLIC ..)
