cmake_minimum_required(VERSION 3.19)

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
    # libのopengraphprotocol.hをd pointerにしたら消せる
    Xml
)

get_filename_component(folder_name ${CMAKE_CURRENT_SOURCE_DIR} NAME)
add_executable(${PROJECT_NAME}
    tst_${folder_name}.cpp
    ${folder_name}.qrc
)

target_link_libraries(${PROJECT_NAME} PRIVATE
    Qt6::Core
    Qt6::Gui
    Qt6::HttpServer
    Qt6::Test
    TestsCommon
    HagoromoLib
    # libのd pointerにしたら消せる
    Qt6::Xml
)

include(../tests.cmake)
