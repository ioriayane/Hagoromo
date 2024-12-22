
# build arch
mac:QMAKE_APPLE_DEVICE_ARCHS=x86_64

INCLUDEPATH += $$PWD

HEADERS += \
    $$PWD/unittest_common.h \
    $$PWD/webserver.h

SOURCES += \
    $$PWD/unittest_common.cpp \
    $$PWD/webserver.cpp
