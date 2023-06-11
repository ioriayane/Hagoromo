QT += testlib httpserver gui

CONFIG += qt console warn_on depend_includepath testcase
CONFIG -= app_bundle

TEMPLATE = app

SOURCES +=  tst_hagoromo_test.cpp

include(../common/common.pri)
include(../../lib/lib.pri)
include(../../app/qtquick/qtquick.pri)
include(../../openssl/openssl.pri)

RESOURCES += \
    hagoromo_test.qrc
