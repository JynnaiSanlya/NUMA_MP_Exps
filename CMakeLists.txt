cmake_minimum_required(VERSION 3.10.0)
project(NUMA_MP_Exps LANGUAGES C CXX)

set(CMAKE_CXX_COMPILER "clang++")
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_C_COMPILER "clang")

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -lstdc++")

find_package(OpenMP REQUIRED)
if(OpenMP_CXX_FOUND)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${OpenMP_C_FLAGS} -lnuma")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS} -lnuma")
    link_libraries(OpenMP::OpenMP_CXX)
endif()

add_subdirectory(src)

