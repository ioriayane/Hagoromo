QT += quick network quickcontrols2

TARGET = Hagoromo

SOURCES += \
    main.cpp \
    qtquick/accountlistmodel.cpp \
    qtquick/anyfeedlistmodel.cpp \
    qtquick/atpabstractlistmodel.cpp \
    qtquick/authorfeedlistmodel.cpp \
    qtquick/columnlistmodel.cpp \
    qtquick/createsession.cpp \
    qtquick/encryption.cpp \
    qtquick/followerslistmodel.cpp \
    qtquick/followslistmodel.cpp \
    qtquick/notificationlistmodel.cpp \
    qtquick/postthreadlistmodel.cpp \
    qtquick/recordoperator.cpp \
    qtquick/timelinelistmodel.cpp \
    qtquick/translator.cpp \
    qtquick/userprofile.cpp

HEADERS += \
    qtquick/accountlistmodel.h \
    qtquick/anyfeedlistmodel.h \
    qtquick/atpabstractlistmodel.h \
    qtquick/authorfeedlistmodel.h \
    qtquick/columnlistmodel.h \
    qtquick/common.h \
    qtquick/createsession.h \
    qtquick/encryption.h \
    qtquick/encryption_seed.h \
    qtquick/followerslistmodel.h \
    qtquick/followslistmodel.h \
    qtquick/notificationlistmodel.h \
    qtquick/postthreadlistmodel.h \
    qtquick/recordoperator.h \
    qtquick/thumbnailprovider.h \
    qtquick/timelinelistmodel.h \
    qtquick/translator.h \
    qtquick/userprofile.h

QML_FILES = \
    qml/main.qml \
    qml/controls/ClickableFrame.qml \
    qml/controls/ProgressCircle.qml \
    qml/controls/IconButton.qml \
    qml/controls/ImageWithIndicator.qml \
    qml/dialogs/AccountDialog.qml \
    qml/dialogs/AddColumnDialog.qml \
    qml/dialogs/ColumnSettingDialog.qml \
    qml/dialogs/LoginDialog.qml \
    qml/dialogs/PostDialog.qml \
    qml/dialogs/SettingDialog.qml \
    qml/parts/Author.qml \
    qml/parts/AvatarImage.qml \
    qml/parts/ImagePreview.qml \
    qml/parts/NotificationDelegate.qml \
    qml/parts/PostControls.qml \
    qml/parts/PostDelegate.qml \
    qml/parts/ReactionAuthor.qml \
    qml/view/ColumnView.qml \
    qml/view/ImageFullView.qml \
    qml/view/NotificationListView.qml \
    qml/view/PostThreadView.qml \
    qml/view/ProfileListView.qml \
    qml/view/ProfileView.qml \
    qml/view/TimelineView.qml

IMAGE_FILES += \
    qml/images/account.png \
    qml/images/account_icon.png \
    qml/images/add_image.png \
    qml/images/add_user.png \
    qml/images/arrow_back.png \
    qml/images/arrow_forward.png \
    qml/images/arrow_left_double.png \
    qml/images/arrow_left_single.png \
    qml/images/auto.png \
    qml/images/check.png \
    qml/images/close.png \
    qml/images/column.png \
    qml/images/delete.png \
    qml/images/edit.png \
    qml/images/expand_less.png \
    qml/images/expand_more.png \
    qml/images/icon_mask.png \
    qml/images/like.png \
    qml/images/logo.png \
    qml/images/more.png \
    qml/images/quote.png \
    qml/images/reply.png \
    qml/images/repost.png \
    qml/images/settings.png



resources.files = $$QML_FILES $$IMAGE_FILES
resources.prefix = /$${TARGET}
RESOURCES += resources

CONFIG(release, debug|release):DEFINES += QT_NO_DEBUG_OUTPUT

# Additional import path used to resolve QML modules in Qt Creator's code model
QML_IMPORT_PATH =

# Additional import path used to resolve QML modules just for Qt Quick Designer
QML_DESIGNER_IMPORT_PATH =

include(../lib/lib.pri)

bin_dir=$$dirname(QMAKE_QMAKE)
open_ssl_dir=$${bin_dir}/../../../Tools/OpenSSL
open_ssl_dir=$$clean_path($$open_ssl_dir)

win32:{
    open_ssl_dir=$${open_ssl_dir}/Win_x64

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
unix: {
    open_ssl_dir=$${open_ssl_dir}/src
    LIBS += -L$${open_ssl_dir} -lssl -lcrypto
}
