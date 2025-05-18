cmake_minimum_required(VERSION 3.19)

project(TestsCommon LANGUAGES CXX)

set(CMAKE_AUTOMOC ON)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt6 REQUIRED COMPONENTS
    Core
    HttpServer
)

add_library(TestsCommon STATIC
    unittest_common.cpp
    unittest_common.h
    webserver.cpp
    webserver.h
)

target_include_directories(TestsCommon INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(TestsCommon PRIVATE
    Qt6::Core
    Qt6::HttpServer
)
