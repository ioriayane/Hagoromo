cmake_minimum_required(VERSION 3.16)

project(HagoromoTest2 LANGUAGES CXX)

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

add_executable(HagoromoTest2
    tst_hagoromo_test2.cpp
    hagoromo_test2.qrc
)
add_test(NAME hagoromo_test2 COMMAND HagoromoTest2)

target_link_libraries(HagoromoTest2 PRIVATE
    Qt6::Core
    Qt6::Gui
    Qt6::HttpServer
    Qt6::Test
    TestsCommon
    HagoromoLib
    HagoromoGuiLib
    # libのd pointerにしたら消せる
    Qt6::Sql
    Qt6::Xml
)

target_compile_definitions(HagoromoTest2 PRIVATE
    HAGOROMO_UNIT_TEST
)
