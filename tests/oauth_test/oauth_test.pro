QT += testlib httpserver gui

CONFIG += qt console warn_on depend_includepath testcase
CONFIG -= app_bundle

TEMPLATE = app
DEFINES += HAGOROMO_UNIT_TEST

SOURCES +=  tst_oauth_test.cpp

include(../common/common.pri)
include(../deps.pri)
include(../../openssl/openssl.pri)

RESOURCES += \
    oauth_test.qrc
