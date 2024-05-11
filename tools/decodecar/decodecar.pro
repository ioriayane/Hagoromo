QT = core

CONFIG += c++17 cmdline

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
        main.cpp \
        $$PWD/../../lib/tools/cardecoder.cpp \
        $$PWD/../../lib/tools/base32.cpp \
        $$PWD/../../lib/tools/leb128.cpp
HEADERS += \
        $$PWD/../../lib/tools/cardecoder.h \
        $$PWD/../../lib/tools/base32.h \
        $$PWD/../../lib/tools/leb128.h

INCLUDEPATH += \
        $$PWD/../../lib/tools

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target
