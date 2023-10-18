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

if(CMAKE_CXX_COMPILER_ID MATCHES "MSVC")
  target_compile_options(utils PUBLIC /W4)
  target_compile_options(vdfs PUBLIC /W4)
  target_compile_options(zenload PUBLIC /W4)
  target_compile_options(daedalus PUBLIC /W4)
else()
  target_compile_options(utils PUBLIC -Wall -Wextra -Wpedantic -Werror)
  target_compile_options(vdfs PUBLIC -Wall -Wextra -Wpedantic -Werror)
  target_compile_options(zenload PUBLIC -Wall -Wextra -Wpedantic -Werror)
  target_compile_options(daedalus PUBLIC -Wall -Wextra -Wpedantic -Werror)
endif()
