QT += testlib httpserver gui quick

CONFIG += qt console warn_on depend_includepath testcase
CONFIG -= app_bundle

TEMPLATE = app
DEFINES += HAGOROMO_UNIT_TEST

SOURCES +=  tst_log_test.cpp

include(../common/common.pri)
include(../../lib/lib.pri)
include(../../openssl/openssl.pri)
include(../../app/qtquick/qtquick.pri)

RESOURCES += \
    log_test.qrc
