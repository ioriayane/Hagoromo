QT += quick network quickcontrols2

TARGET = Hagoromo

SOURCES += \
    main.cpp \
    tools/translatorchanger.cpp

HEADERS += \
    tools/translatorchanger.h

QML_FILES = \
    qml/main.qml \
    qml/controls/ClickableFrame.qml \
    qml/controls/ComboBoxEx.qml \
    qml/controls/ProgressCircle.qml \
    qml/controls/IconButton.qml \
    qml/controls/IconLabelFrame.qml \
    qml/controls/ImageWithIndicator.qml \
    qml/controls/MessageBubble.qml \
    qml/controls/MoreButton.qml \
    qml/controls/RadioButtonEx.qml \
    qml/data/Account.qml \
    qml/data/AdjustedValues.qml \
    qml/data/ColumnSettings.qml \
    qml/dialogs/AccountDialog.qml \
    qml/dialogs/AddColumnDialog.qml \
    qml/dialogs/AddListDialog.qml \
    qml/dialogs/AddMutedWordDialog.qml \
    qml/dialogs/AddToListDialog.qml \
    qml/dialogs/AltEditDialog.qml \
    qml/dialogs/BlockedAccountsDialog.qml \
    qml/dialogs/BlockedListsDialog.qml \
    qml/dialogs/ColumnSettingDialog.qml \
    qml/dialogs/ContentFilterSettingDialog.qml \
    qml/dialogs/DiscoverFeedsDialog.qml \
    qml/dialogs/EditProfileDialog.qml \
    qml/dialogs/ImageClipDialog.qml \
    qml/dialogs/LanguageSelectionDialog.qml \
    qml/dialogs/LoginDialog.qml \
    qml/dialogs/LogViewDialog.qml \
    qml/dialogs/MessageDialog.qml \
    qml/dialogs/MutedAccountsDialog.qml \
    qml/dialogs/MutedListsDialog.qml \
    qml/dialogs/PostDialog.qml \
    qml/dialogs/ReportAccountDialog.qml \
    qml/dialogs/ReportMessageDialog.qml \
    qml/dialogs/ReportPostDialog.qml \
    qml/dialogs/SearchDialog.qml \
    qml/dialogs/SelectThreadGateDialog.qml \
    qml/dialogs/SettingDialog.qml \
    qml/parts/AccountLayout.qml \
    qml/parts/AccountList.qml \
    qml/parts/AggregatedAvatarImages.qml \
    qml/parts/ApplicationShortcut.qml \
    qml/parts/Author.qml \
    qml/parts/AvatarImage.qml \
    qml/parts/ChatErrorMessage.qml \
    qml/parts/CoverFrame.qml \
    qml/parts/CursorRect.qml \
    qml/parts/CursorRectHandle.qml \
    qml/parts/ImagePreview.qml \
    qml/parts/ExternalLinkCard.qml \
    qml/parts/FeedGeneratorLinkCard.qml \
    qml/parts/HandleHistoryPopup.qml \
    qml/parts/HashTagMenu.qml \
    qml/parts/ListLinkCard.qml \
    qml/parts/MentionSuggestionView.qml \
    qml/parts/NotificationDelegate.qml \
    qml/parts/PinnedIndicator.qml \
    qml/parts/PostControls.qml \
    qml/parts/PostDelegate.qml \
    qml/parts/PostInformation.qml \
    qml/parts/ProfileTabBar.qml \
    qml/parts/ProfileTabButton.qml \
    qml/parts/ReactionAuthor.qml \
    qml/parts/SelfLabelPopup.qml \
    qml/parts/SideBar.qml \
    qml/parts/TagLabel.qml \
    qml/parts/TagLabelLayout.qml \
    qml/parts/VersionInfomation.qml \
    qml/view/AnyProfileListView.qml \
    qml/view/ChatListView.qml \
    qml/view/ChatMessageListView.qml \
    qml/view/ColumnView.qml \
    qml/view/FeedGeneratorListView.qml \
    qml/view/ImageFullView.qml \
    qml/view/ListDetailView.qml \
    qml/view/ListsListView.qml \
    qml/view/LogTotalListView.qml \
    qml/view/NotificationListView.qml \
    qml/view/PostThreadView.qml \
    qml/view/ProfileListView.qml \
    qml/view/ProfileView.qml \
    qml/view/SuggestionProfileListView.qml \
    qml/view/TimelineView.qml

IMAGE_FILES += \
    qml/images/account.png \
    qml/images/account_icon.png \
    qml/images/account_off.png \
    qml/images/add.png \
    qml/images/add_image.png \
    qml/images/add_user.png \
    qml/images/arrow_back.png \
    qml/images/arrow_forward.png \
    qml/images/arrow_left_double.png \
    qml/images/arrow_left_single.png \
    qml/images/auto.png \
    qml/images/database.png \
    qml/images/block.png \
    qml/images/bookmark_add.png \
    qml/images/chat.png \
    qml/images/check.png \
    qml/images/close.png \
    qml/images/column.png \
    qml/images/copy.png \
    qml/images/delete.png \
    qml/images/edit.png \
    qml/images/expand_less.png \
    qml/images/expand_more.png \
    qml/images/feed.png \
    qml/images/hand.png \
    qml/images/home.png \
    qml/images/icon_mask.png \
    qml/images/label.png \
    qml/images/labeling.png \
    qml/images/language.png \
    qml/images/leave.png \
    qml/images/like.png \
    qml/images/list.png \
    qml/images/logo.png \
    qml/images/media.png \
    qml/images/more.png \
    qml/images/mute.png \
    qml/images/notification.png \
    qml/images/open_in_other.png \
    qml/images/pin.png \
    qml/images/quote.png \
    qml/images/refresh.png \
    qml/images/reply.png \
    qml/images/report.png \
    qml/images/repost.png \
    qml/images/search.png \
    qml/images/send.png \
    qml/images/settings.png \
    qml/images/tag.png \
    qml/images/terminal.png \
    qml/images/thread.png \
    qml/images/translate.png \
    qml/images/visibility_off.png \
    qml/images/visibility_on.png


resources.files = $$QML_FILES $$IMAGE_FILES
resources.prefix = /$${TARGET}
RESOURCES += resources

TRANSLATIONS += i18n/app_ja.ts

# icon
win32:RC_FILE = app.rc
mac:ICON = app.icns

CONFIG(release, debug|release):DEFINES += QT_NO_DEBUG_OUTPUT
CONFIG(HAGOROMO_RELEASE_BUILD):DEFINES += HAGOROMO_RELEASE_BUILD

# Additional import path used to resolve QML modules in Qt Creator's code model
QML_IMPORT_PATH =

# Additional import path used to resolve QML modules just for Qt Quick Designer
QML_DESIGNER_IMPORT_PATH =

include(../lib/lib.pri)
include(qtquick/qtquick.pri)
include(../openssl/openssl.pri)


win32:{
    CONFIG(debug,debug|release):install_dir = $$OUT_PWD/debug
    else: install_dir = $$OUT_PWD/release
}else{
    install_dir = $$OUT_PWD
}

translations.path = $$install_dir/translations
mac:translations.path = \
          $$install_dir/$${TARGET}.app/Contents/MacOS/translations
translations.files = $$PWD/i18n/*.qm
#qmファイルが存在しないとmakefileに追加されないので注意
INSTALLS += translations
