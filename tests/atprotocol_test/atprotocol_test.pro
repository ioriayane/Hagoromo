QT += testlib httpserver gui

CONFIG += qt console warn_on depend_includepath testcase
CONFIG -= app_bundle

TEMPLATE = app

SOURCES +=  tst_atprotocol_test.cpp

include(../common/common.pri)
include(../../lib/lib.pri)

RESOURCES += \
    atprotocol_test.qrc
