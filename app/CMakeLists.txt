cmake_minimum_required(VERSION 3.16)

project(Hagoromo VERSION 0.1 LANGUAGES CXX)

set(CMAKE_AUTORCC ON)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(WIN32)
    set(RESOUCE_FILES app.rc)
endif()
set(RESOUCE_FILES
    app.qrc
    ../lib/lib.qrc
)

find_package(Qt6 REQUIRED COMPONENTS
    Core
    Core5Compat
    Gui
    LinguistTools
    Network
    Quick
    QuickControls2
    # libのlogaccess.hをd pointerにしたら消せる
    Sql
    # libのopengraphprotocol.hをd pointerにしたら消せる
    Xml
)

qt_standard_project_setup(REQUIRES 6.8)
qt_standard_project_setup(
    I18N_TRANSLATED_LANGUAGES ja
)

qt_add_executable(Hagoromo
    main.cpp
    qtquick/account/accountlistmodel.cpp
    qtquick/account/accountlistmodel.h
    qtquick/account/createsession.cpp
    qtquick/account/createsession.h
    qtquick/atpabstractlistmodel.cpp
    qtquick/atpabstractlistmodel.h
    qtquick/blog/blogentrylistmodel.cpp
    qtquick/blog/blogentrylistmodel.h
    qtquick/chat/atpchatabstractlistmodel.cpp
    qtquick/chat/atpchatabstractlistmodel.h
    qtquick/chat/chatlistmodel.cpp
    qtquick/chat/chatlistmodel.h
    qtquick/chat/chatmessagelistmodel.cpp
    qtquick/chat/chatmessagelistmodel.h
    qtquick/chat/chatnotificationlistmodel.cpp
    qtquick/chat/chatnotificationlistmodel.h
    qtquick/column/columnlistmodel.cpp
    qtquick/column/columnlistmodel.h
    qtquick/column/feedtypelistmodel.cpp
    qtquick/column/feedtypelistmodel.h
    qtquick/controls/calendartablemodel.cpp
    qtquick/controls/calendartablemodel.h
    qtquick/controls/embedimagelistmodel.cpp
    qtquick/controls/embedimagelistmodel.h
    qtquick/controls/emojilistmodel.cpp
    qtquick/controls/emojilistmodel.h
    qtquick/controls/languagelistmodel.cpp
    qtquick/controls/languagelistmodel.h
    qtquick/feedgenerator/actorfeedgeneratorlistmodel.cpp
    qtquick/feedgenerator/actorfeedgeneratorlistmodel.h
    qtquick/feedgenerator/feedgeneratorlistmodel.cpp
    qtquick/feedgenerator/feedgeneratorlistmodel.h
    qtquick/link/externallink.cpp
    qtquick/link/externallink.h
    qtquick/link/feedgeneratorlink.cpp
    qtquick/link/feedgeneratorlink.h
    qtquick/link/listlink.cpp
    qtquick/link/listlink.h
    qtquick/link/postlink.cpp
    qtquick/link/postlink.h
    qtquick/list/listblockslistmodel.cpp
    qtquick/list/listblockslistmodel.h
    qtquick/list/listfeedlistmodel.cpp
    qtquick/list/listfeedlistmodel.h
    qtquick/list/listitemlistmodel.cpp
    qtquick/list/listitemlistmodel.h
    qtquick/list/listmuteslistmodel.cpp
    qtquick/list/listmuteslistmodel.h
    qtquick/list/listslistmodel.cpp
    qtquick/list/listslistmodel.h
    qtquick/log/logdailylistmodel.cpp
    qtquick/log/logdailylistmodel.h
    qtquick/log/logfeedlistmodel.cpp
    qtquick/log/logfeedlistmodel.h
    qtquick/log/logmonthlylistmodel.cpp
    qtquick/log/logmonthlylistmodel.h
    qtquick/log/logoperator.cpp
    qtquick/log/logoperator.h
    qtquick/log/logstatisticslistmodel.cpp
    qtquick/log/logstatisticslistmodel.h
    qtquick/moderation/contentfiltersettinglistmodel.cpp
    qtquick/moderation/contentfiltersettinglistmodel.h
    qtquick/moderation/labelerlistmodel.cpp
    qtquick/moderation/labelerlistmodel.h
    qtquick/moderation/mutedwordlistmodel.cpp
    qtquick/moderation/mutedwordlistmodel.h
    qtquick/moderation/reporter.cpp
    qtquick/moderation/reporter.h
    qtquick/notification/notificationlistmodel.cpp
    qtquick/notification/notificationlistmodel.h
    qtquick/operation/recordoperator.cpp
    qtquick/operation/recordoperator.h
    qtquick/operation/translator.cpp
    qtquick/operation/translator.h
    qtquick/profile/anyprofilelistmodel.cpp
    qtquick/profile/anyprofilelistmodel.h
    qtquick/profile/blockslistmodel.cpp
    qtquick/profile/blockslistmodel.h
    qtquick/profile/followerslistmodel.cpp
    qtquick/profile/followerslistmodel.h
    qtquick/profile/followslistmodel.cpp
    qtquick/profile/followslistmodel.h
    qtquick/profile/knownfollowerslistmodel.cpp
    qtquick/profile/knownfollowerslistmodel.h
    qtquick/profile/muteslistmodel.cpp
    qtquick/profile/muteslistmodel.h
    qtquick/profile/searchprofilelistmodel.cpp
    qtquick/profile/searchprofilelistmodel.h
    qtquick/profile/userprofile.cpp
    qtquick/profile/userprofile.h
    qtquick/realtime/editselectorlistmodel.cpp
    qtquick/realtime/editselectorlistmodel.h
    qtquick/realtime/realtimefeedlistmodel.cpp
    qtquick/realtime/realtimefeedlistmodel.h
    qtquick/realtime/realtimefeedstatusgraph.cpp
    qtquick/realtime/realtimefeedstatusgraph.h
    qtquick/realtime/realtimefeedstatuslistmodel.cpp
    qtquick/realtime/realtimefeedstatuslistmodel.h
    qtquick/systemtool.cpp
    qtquick/systemtool.h
    qtquick/thumbnailprovider.h
    qtquick/timeline/actorlikelistmodel.cpp
    qtquick/timeline/actorlikelistmodel.h
    qtquick/timeline/anyfeedlistmodel.cpp
    qtquick/timeline/anyfeedlistmodel.h
    qtquick/timeline/authorfeedlistmodel.cpp
    qtquick/timeline/authorfeedlistmodel.h
    qtquick/timeline/customfeedlistmodel.cpp
    qtquick/timeline/customfeedlistmodel.h
    qtquick/timeline/postthreadlistmodel.cpp
    qtquick/timeline/postthreadlistmodel.h
    qtquick/timeline/quotedpostlistmodel.cpp
    qtquick/timeline/quotedpostlistmodel.h
    qtquick/timeline/searchpostlistmodel.cpp
    qtquick/timeline/searchpostlistmodel.h
    qtquick/timeline/timelinelistmodel.cpp
    qtquick/timeline/timelinelistmodel.h
    qtquick/timeline/userpost.cpp
    qtquick/timeline/userpost.h
    tools/globalsettings.cpp
    tools/globalsettings.h
    tools/translatorchanger.cpp
    tools/translatorchanger.h
    ${RESOUCE_FILES}
)


qt_add_qml_module(Hagoromo
    URI Hagoromo
    VERSION 1.0
    QML_FILES
        qml/data/ColumnSettings.qml
        qml/data/AdjustedValues.qml
        qml/data/Account.qml
        qml/parts/ImagePreview.qml
        qml/parts/ListLinkCard.qml
        qml/parts/QuoteRecord.qml
        qml/parts/ProfileTabButton.qml
        qml/parts/KnownFollowersInfo.qml
        qml/parts/AggregatedAvatarImages.qml
        qml/parts/CursorRect.qml
        qml/parts/TagLabel.qml
        qml/parts/PostInformation.qml
        qml/parts/VersionInfomation.qml
        qml/parts/HandleHistoryPopup.qml
        qml/parts/SelectorDelegate.qml
        qml/parts/RealtimeFeedStatus.qml
        qml/parts/ExternalLinkCard.qml
        qml/parts/PinnedIndicator.qml
        qml/parts/PostControls.qml
        qml/parts/Author.qml
        qml/parts/CoverFrame.qml
        qml/parts/PostDelegate.qml
        qml/parts/AvatarImage.qml
        qml/parts/SelfLabelPopup.qml
        qml/parts/AccountList.qml
        qml/parts/HashTagMenu.qml
        qml/parts/ChatNotificationFrame.qml
        qml/parts/NotificationDelegate.qml
        qml/parts/ChatErrorMessage.qml
        qml/parts/VideoFrame.qml
        qml/parts/LabelerComboBox.qml
        qml/parts/ProfileTabBar.qml
        qml/parts/SideBar.qml
        qml/parts/ReactionAuthor.qml
        qml/parts/TagLabelLayout.qml
        qml/parts/MentionSuggestionView.qml
        qml/parts/AccountLayout.qml
        qml/parts/VerificationInfoPopup.qml
        qml/parts/CursorRectHandle.qml
        qml/parts/ApplicationShortcut.qml
        qml/parts/FeedGeneratorLinkCard.qml
        qml/dialogs/SelectThreadGateDialog.qml
        qml/dialogs/SearchDialog.qml
        qml/dialogs/AddToListDialog.qml
        qml/dialogs/EditProfileDialog.qml
        qml/dialogs/MutedListsDialog.qml
        qml/dialogs/AddColumnDialog.qml
        qml/dialogs/DiscoverFeedsDialog.qml
        qml/dialogs/SettingDialog.qml
        qml/dialogs/ReportMessageDialog.qml
        qml/dialogs/ColumnSettingDialog.qml
        qml/dialogs/AddMutedWordDialog.qml
        qml/dialogs/ImageClipDialog.qml
        qml/dialogs/BlockedListsDialog.qml
        qml/dialogs/PostDialog.qml
        qml/dialogs/MessageDialog.qml
        qml/dialogs/AltEditDialog.qml
        qml/dialogs/LoginDialog.qml
        qml/dialogs/LanguageSelectionDialog.qml
        qml/dialogs/ContentFilterSettingDialog.qml
        qml/dialogs/AccountDialog.qml
        qml/dialogs/ReportAccountDialog.qml
        qml/dialogs/RealtimeFeedEditorDialog.qml
        qml/dialogs/MutedAccountsDialog.qml
        qml/dialogs/ReportPostDialog.qml
        qml/dialogs/LogViewDialog.qml
        qml/dialogs/BlockedAccountsDialog.qml
        qml/dialogs/AddListDialog.qml
        qml/controls/ComboBoxEx.qml
        qml/controls/IconLabelFrame.qml
        qml/controls/CalendarPicker.qml
        qml/controls/MessageBubble.qml
        qml/controls/EmojiPicker.qml
        qml/controls/MenuEx.qml
        qml/controls/ClickableFrame.qml
        qml/controls/ImageWithIndicator.qml
        qml/controls/DragAndDropArea.qml
        qml/controls/IconButton.qml
        qml/controls/MoreButton.qml
        qml/controls/ProgressCircle.qml
        qml/controls/RadioButtonEx.qml
        qml/main.qml
        qml/compat/GlowC.qml
        qml/compat/SettingsC.qml
        qml/compat/OpacityMaskC.qml
        qml/compat/ColorOverlayC.qml
        qml/view/PostThreadView.qml
        qml/view/ImageFullView.qml
        qml/view/AnyProfileListView.qml
        qml/view/ColumnView.qml
        qml/view/ListsListView.qml
        qml/view/LogTotalListView.qml
        qml/view/ChatMessageListView.qml
        qml/view/ProfileListView.qml
        qml/view/SuggestionProfileListView.qml
        qml/view/FeedGeneratorListView.qml
        qml/view/BlogEntryListView.qml
        qml/view/ListDetailView.qml
        qml/view/ProfileView.qml
        qml/view/ChatListView.qml
        qml/view/TimelineView.qml
        qml/view/NotificationListView.qml
)

# Qt for iOS sets MACOSX_BUNDLE_GUI_IDENTIFIER automatically since Qt 6.1.
# If you are developing for iOS or macOS you should consider setting an
# explicit, fixed bundle identifier manually though.
set_target_properties(Hagoromo PROPERTIES
#    MACOSX_BUNDLE_GUI_IDENTIFIER com.example.Hagoromo
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)

target_include_directories(Hagoromo PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/qtquick
)
target_link_libraries(Hagoromo PRIVATE
    Qt${QT_VERSION_MAJOR}::Core
    Qt6::Core5Compat
    Qt6::Gui
    Qt6::Network
    Qt6::Quick
    Qt6::QuickControls2
    HagoromoLib
    # libのd pointerにしたら消せる
    Qt6::Sql
    Qt6::Xml
)

if(CMAKE_BUILD_TYPE STREQUAL "Release")
    target_compile_definitions(Hagoromo PRIVATE
        QT_NO_DEBUG_OUTPUT
    )
    if(HAGOROMO_RELEASE_BUILD)
        target_compile_definitions(Hagoromo PRIVATE HAGOROMO_RELEASE_BUILD)
    endif()
endif()

qt_add_translations(Hagoromo
    QT_TRANSLATION_CATALOGS qtbase)

include(GNUInstallDirs)
install(TARGETS Hagoromo
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
