QT += quick network

TARGET = Hagoromo

SOURCES += \
        atprotocol/accessatprotocol.cpp \
        atprotocol/comatprotoservercreatesession.cpp \
        main.cpp \
        qtquick/createsession.cpp

HEADERS += \
    atprotocol/accessatprotocol.h \
    atprotocol/comatprotoservercreatesession.h \
    qtquick/createsession.h

QML_FILES = \
    qml/main.qml \
    qml/LoginDialog.qml

#INCLUDEPATH += \
#    atprotocol/

resources.files = $$QML_FILES
resources.prefix = /$${TARGET}
RESOURCES += resources

# Additional import path used to resolve QML modules in Qt Creator's code model
QML_IMPORT_PATH =

# Additional import path used to resolve QML modules just for Qt Quick Designer
QML_DESIGNER_IMPORT_PATH =


win32:{
    bin_dir=$$dirname(QMAKE_QMAKE)
    ssl_dir=$${bin_dir}/../../../Tools/OpenSSL/Win_x64
    ssl_dir=$$clean_path($$ssl_dir)

    CONFIG(debug,debug|release):install_dir = $$OUT_PWD/debug
    else: install_dir = $$OUT_PWD/release

    depend_files.path = $$install_dir
    depend_files.files = \
        $${ssl_dir}/bin/libcrypto-1_1-x64.dll \
        $${ssl_dir}/bin/libssl-1_1-x64.dll

    INSTALLS += depend_files
    QMAKE_POST_LINK += nmake -f $(MAKEFILE) install
}

