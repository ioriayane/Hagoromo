QT += testlib gui

CONFIG += qt console warn_on depend_includepath testcase
CONFIG -= app_bundle

TEMPLATE = app
DEFINES += HAGOROMO_UNIT_TEST

SOURCES +=  tst_realtime_test.cpp

include(../../lib/lib.pri)
include(../../openssl/openssl.pri)

RESOURCES += \
    realtime_test.qrc
