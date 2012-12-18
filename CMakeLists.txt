cmake_minimum_required (VERSION 2.6) 
project (silkc) 
set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/make/Modules)

find_package(LLVM 3.0 REQUIRED)
include_directories(include)
include_directories(${LLVM_INCLUDE_DIR})

#if (NOT CMAKE_BUILD_TYPE)
#  set (CMAKE_BUILD_TYPE "Release")
#endif()
set (CMAKE_BUILD_TYPE "Debug")

set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -D__STDC_LIMIT_MACROS=1 -D__STDC_CONSTANT_MACROS=1")
set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -pedantic -Wno-unused-parameter")
set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -stdlib=libc++")

add_subdirectory (lib) 
add_subdirectory (tools)
