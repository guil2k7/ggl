cmake_minimum_required(VERSION 3.22)
project("GCL" VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

option(GCL_BUILD_EXAMPLES "Build examples" ON)
option(GCL_BUILD_STATIC "Build GCL as a static library" ON)

add_subdirectory(src)

if(GCL_BUILD_EXAMPLES)
    add_subdirectory(examples)
endif()
