QT += quick network quickcontrols2

TARGET = Hagoromo

SOURCES += \
        atprotocol/accessatprotocol.cpp \
        atprotocol/appbskyfeedgettimeline.cpp \
        atprotocol/comatprotorepocreaterecord.cpp \
        atprotocol/comatprotoservercreatesession.cpp \
        atprotocol/lexicons_func.cpp \
        atprotocol/lexicons_func_unknown.cpp \
        main.cpp \
        qtquick/accountlistmodel.cpp \
        qtquick/createrecord.cpp \
        qtquick/createsession.cpp \
        qtquick/timelinelistmodel.cpp

HEADERS += \
    atprotocol/accessatprotocol.h \
    atprotocol/appbskyfeedgettimeline.h \
    atprotocol/comatprotorepocreaterecord.h \
    atprotocol/comatprotoservercreatesession.h \
    atprotocol/lexicons.h \
    atprotocol/lexicons_func.h \
    atprotocol/lexicons_func_unknown.h \
    qtquick/accountlistmodel.h \
    qtquick/createrecord.h \
    qtquick/createsession.h \
    qtquick/timelinelistmodel.h

QML_FILES = \
    qml/main.qml \
    qml/LoginDialog.qml \
    qml/TimelineView.qml \
    qml/IconButton.qml \
    qml/PostDialog.qml \
    qml/parts/ImagePreview.qml \
    qml/parts/Author.qml \
    qml/parts/ReactionAuthor.qml

IMAGE_FILES += \
    qml/images/add_user.png \
    qml/images/edit.png \
    qml/images/expand_less.png \
    qml/images/expand_more.png \
    qml/images/like.png \
    qml/images/more.png \
    qml/images/reply.png \
    qml/images/repost.png \
    qml/images/settings.png


#INCLUDEPATH += \
#    atprotocol/

resources.files = $$QML_FILES $$IMAGE_FILES
resources.prefix = /$${TARGET}
RESOURCES += resources

# Additional import path used to resolve QML modules in Qt Creator's code model
QML_IMPORT_PATH =

# Additional import path used to resolve QML modules just for Qt Quick Designer
QML_DESIGNER_IMPORT_PATH =


win32:{
    bin_dir=$$dirname(QMAKE_QMAKE)
    open_ssl_dir=$${bin_dir}/../../../Tools/OpenSSL/Win_x64
    open_ssl_dir=$$clean_path($$open_ssl_dir)

    LIBS += $${open_ssl_dir}/lib/libssl.lib \
            $${open_ssl_dir}/lib/libcrypto.lib
    INCLUDEPATH += $${open_ssl_dir}/include

    CONFIG(debug,debug|release):install_dir = $$OUT_PWD/debug
    else: install_dir = $$OUT_PWD/release

    depend_files.path = $$install_dir
    depend_files.files = \
        $${open_ssl_dir}/bin/libcrypto-1_1-x64.dll \
        $${open_ssl_dir}/bin/libssl-1_1-x64.dll

    INSTALLS += depend_files
    QMAKE_POST_LINK += nmake -f $(MAKEFILE) install
}
unix: LIBS += -lssl -lcrypto
