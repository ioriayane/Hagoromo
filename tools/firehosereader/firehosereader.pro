QT = core network websockets

CONFIG += c++17 cmdline

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
        $$PWD/../../lib/extension/com/atproto/sync/comatprotosyncsubscribereposex.cpp \
        $$PWD/../../lib/atprotocol/lexicons_func_unknown.cpp \
        $$PWD/../../lib/atprotocol/lexicons_func.cpp \
        $$PWD/../../lib/atprotocol/accessatprotocol.cpp \
        $$PWD/../../lib/atprotocol/app/bsky/actor/appbskyactorgetprofiles.cpp \
        $$PWD/../../lib/http/httpaccess.cpp \
        $$PWD/../../lib/http/httpaccessmanager.cpp \
        $$PWD/../../lib/http/httpreply.cpp \
        main.cpp \
        $$PWD/../../lib/tools/cardecoder.cpp \
        $$PWD/../../lib/tools/base32.cpp \
        $$PWD/../../lib/tools/leb128.cpp
HEADERS += \
        $$PWD/../../lib/tools/cardecoder.h \
        $$PWD/../../lib/tools/base32.h \
        $$PWD/../../lib/tools/leb128.h \
        $$PWD/../../lib/extension/com/atproto/sync/comatprotosyncsubscribereposex.h \
        $$PWD/../../lib/atprotocol/lexicons.h \
        $$PWD/../../lib/atprotocol/lexicons_func_unknown.h \
        $$PWD/../../lib/atprotocol/lexicons_func.h \
        $$PWD/../../lib/atprotocol/accessatprotocol.h \
        $$PWD/../../lib/atprotocol/app/bsky/actor/appbskyactorgetprofiles.h \
        $$PWD/../../lib/http/httpaccess.h \
        $$PWD/../../lib/http/httpaccessmanager.h \
        $$PWD/../../lib/http/httpreply.h

INCLUDEPATH += \
        $$PWD/../../lib/tools \
        $$PWD/../../lib \
        $$PWD/../../lib/extension/com/atproto/sync \
        $$PWD/../../3rdparty/cpp-httplib

include(../../openssl/openssl.pri)

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target
