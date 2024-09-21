QT += testlib httpserver gui quick

CONFIG += qt console warn_on depend_includepath testcase
CONFIG -= app_bundle

TEMPLATE = app
DEFINES += HAGOROMO_UNIT_TEST

SOURCES +=  tst_http_test.cpp

include(../common/common.pri)
include(../deps.pri)
include(../../openssl/openssl.pri)
include(../../zlib/zlib.pri)

RESOURCES += \
    http_test.qrc
