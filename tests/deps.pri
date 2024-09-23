QT += xml sql websockets

win32:CONFIG(release, debug|release): LIBS += -L$$OUT_PWD/../../lib/release/ -llib
else:win32:CONFIG(debug, debug|release): LIBS += -L$$OUT_PWD/../../lib/debug/ -llib
else:unix: LIBS += -L$$OUT_PWD/../../lib/ -llib

INCLUDEPATH += $$PWD/../lib
DEPENDPATH += $$PWD/../lib

win32-g++:CONFIG(release, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../../lib/release/liblib.a
else:win32-g++:CONFIG(debug, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../../lib/debug/liblib.a
else:win32:!win32-g++:CONFIG(release, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../../lib/release/lib.lib
else:win32:!win32-g++:CONFIG(debug, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../../lib/debug/lib.lib
else:unix: PRE_TARGETDEPS += $$OUT_PWD/../../lib/liblib.a

# openssl.priなどで追加した依存ファイルのコピーに必要
win32:QMAKE_POST_LINK += nmake -f $(MAKEFILE) install
