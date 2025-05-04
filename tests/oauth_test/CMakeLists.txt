cmake_minimum_required(VERSION 3.16)

project(OauthTest LANGUAGES CXX C)

enable_testing()

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(WIN32)
set(OPENSSL_ROOT_DIR "${CMAKE_PREFIX_PATH}/../../Tools/OpenSSLv3/Win_x64")
else()
set(OPENSSL_ROOT_DIR "${CMAKE_SOURCE_DIR}/openssl")
endif()

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
find_package(OpenSSL REQUIRED)

add_executable(${PROJECT_NAME}
    tst_oauth_test.cpp
    oauth_test.qrc
    # for openSSL
    $<$<BOOL:${WIN32}>:${CMAKE_SOURCE_DIR}/3rdparty/openssl/ms/applink.c>
)
add_test(NAME ${PROJECT_NAME} COMMAND ${PROJECT_NAME})

target_link_libraries(${PROJECT_NAME} PRIVATE
    Qt6::Core
    Qt6::Gui
    Qt6::HttpServer
    Qt6::Test
    TestsCommon
    HagoromoLib
    OpenSSL::SSL
    OpenSSL::Crypto
    # libのd pointerにしたら消せる
    Qt6::Sql
    Qt6::Xml
)

target_include_directories(${PROJECT_NAME} PRIVATE
    ${OPENSSL_INCLUDE_DIR}
)

include(../tests.cmake)
