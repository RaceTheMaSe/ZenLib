cmake_minimum_required(VERSION 2.4...2.9)
project(ZenLib)

include(ExternalProject)
include(cmake/clang-format.cmake)

set(PHYSFS_BUILD_SHARED OFF)
cmake_policy(SET CMP0077 NEW)

set(CMAKE_CXX_STANDARD 17)

# 3rd-party dependencies
set(PHYSFS_BUILD_TEST OFF CACHE STRING "" FORCE)
add_subdirectory(lib/physfs)

include_directories(lib/physfs/src)
include_directories(.)

# Internal libraries
add_subdirectory(utils)
add_subdirectory(vdfs)
add_subdirectory(zenload)
add_subdirectory(daedalus)
