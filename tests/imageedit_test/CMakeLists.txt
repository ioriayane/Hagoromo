cmake_minimum_required(VERSION 3.19)

project(ImageEditTest LANGUAGES CXX)

enable_testing()

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt6 REQUIRED COMPONENTS
    Core
    Gui
    Test
)

get_filename_component(folder_name ${CMAKE_CURRENT_SOURCE_DIR} NAME)
add_executable(${PROJECT_NAME}
    tst_${folder_name}.cpp
)

target_link_libraries(${PROJECT_NAME} PRIVATE
    Qt6::Core
    Qt6::Gui
    Qt6::Test
    HagoromoLib
    HagoromoGuiLib
)

include(../tests.cmake)
