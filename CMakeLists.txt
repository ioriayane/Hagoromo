cmake_minimum_required(VERSION 3.16)
project(HagoromoProject LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(DEFINED ENV{QTDIR})
    message(STATUS "QTDIR=$ENV{QTDIR}")
    set(CMAKE_PREFIX_PATH "$ENV{QTDIR}")
endif()

add_subdirectory(lib)
# add_subdirectory(AppTest)
# add_subdirectory(StaticLibTest)
# add_subdirectory(SampleTest)
