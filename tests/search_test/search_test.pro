QT += testlib httpserver gui quick

CONFIG += qt console warn_on depend_includepath testcase
CONFIG -= app_bundle

TEMPLATE = app
DEFINES += HAGOROMO_UNIT_TEST

SOURCES +=  tst_search_test.cpp

include(../common/common.pri)
include(../deps.pri)
include(../../app/qtquick/qtquick.pri)
include(../../openssl/openssl.pri)

RESOURCES += \
    search_test.qrc
