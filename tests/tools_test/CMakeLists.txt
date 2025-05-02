cmake_minimum_required(VERSION 3.16)

project(ToolsTest LANGUAGES CXX)

enable_testing()

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt6 REQUIRED COMPONENTS
    Core
    Gui
    HttpServer
    Test
    # libのlogaccess.hをd pointerにしたら消せる
    Sql
    # libのopengraphprotocol.hをd pointerにしたら消せる
    Xml
)

add_executable(ToolsTest
    tst_tools_test.cpp
    tools_test.qrc
)
add_test(NAME tools_test COMMAND ToolsTest)

target_link_libraries(ToolsTest PRIVATE
    Qt6::Core
    Qt6::Gui
    Qt6::HttpServer
    Qt6::Test
    TestsCommon
    HagoromoLib
    # libのd pointerにしたら消せる
    Qt6::Sql
    Qt6::Xml
)

target_compile_definitions(ToolsTest PRIVATE
    HAGOROMO_UNIT_TEST
)
