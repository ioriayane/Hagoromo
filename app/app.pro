QT += quick network quickcontrols2

TARGET = Hagoromo

SOURCES += \
    main.cpp

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
    qml/dialogs/SearchDialog.qml \
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
    qml/images/home.png \
    qml/images/icon_mask.png \
    qml/images/like.png \
    qml/images/logo.png \
    qml/images/more.png \
    qml/images/notification.png \
    qml/images/quote.png \
    qml/images/reply.png \
    qml/images/repost.png \
    qml/images/search.png \
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
include(qtquick/qtquick.pri)
include(../openssl/openssl.pri)

