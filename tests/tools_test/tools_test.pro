QT += testlib httpserver gui

CONFIG += qt console warn_on depend_includepath testcase
CONFIG -= app_bundle

TEMPLATE = app
DEFINES += HAGOROMO_UNIT_TEST

SOURCES +=  tst_tools_test.cpp

include(../common/common.pri)
include(../../lib/lib.pri)
include(../../openssl/openssl.pri)
