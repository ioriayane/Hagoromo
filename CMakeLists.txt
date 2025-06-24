cmake_minimum_required(VERSION 3.16)
project(HagoromoProject LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)


add_subdirectory(lib)
add_subdirectory(app)
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    add_subdirectory(tests)
    add_subdirectory(tools)
endif()
