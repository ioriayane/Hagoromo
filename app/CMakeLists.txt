cmake_minimum_required(VERSION 3.19)

project(Hagoromo VERSION 0.1 LANGUAGES CXX C)

add_subdirectory(qtquick)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(RESOUCE_FILES
    app.qrc
    ../lib/lib.qrc
)
if(WIN32)
    list(APPEND RESOUCE_FILES ${CMAKE_CURRENT_SOURCE_DIR}/app.rc)
elseif(APPLE)
    set(MACOSX_BUNDLE_ICON_FILE app.icns)
    set(app_icon_macos "${CMAKE_CURRENT_SOURCE_DIR}/app.icns")
    set_source_files_properties(${app_icon_macos} PROPERTIES
        MACOSX_PACKAGE_LOCATION "Resources")
    list(APPEND RESOUCE_FILES ${app_icon_macos})
endif()
message("App  RESOUCE_FILES=${RESOUCE_FILES}")

find_package(Qt6 REQUIRED COMPONENTS
    Core
    LinguistTools
    Quick
    QuickControls2
)
qt_policy(
    SET QTP0004 NEW
)
qt_standard_project_setup(REQUIRES 6.8)
qt_standard_project_setup(
    I18N_TRANSLATED_LANGUAGES ja
)

qt_add_executable(Hagoromo
    main.cpp
    tools/globalsettings.cpp
    tools/globalsettings.h
    tools/translatorchanger.cpp
    tools/translatorchanger.h
    ${RESOUCE_FILES}
    # for openSSL
    $<$<BOOL:${WIN32}>:${CMAKE_SOURCE_DIR}/3rdparty/openssl/ms/applink.c>
)

set_source_files_properties(qml/data/AdjustedValues.qml
    PROPERTIES QT_QML_SINGLETON_TYPE TRUE)

qt_add_qml_module(Hagoromo
    URI tech.relog.hagoromo
    VERSION 1.0
    RESOURCE_PREFIX /
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
        qml/parts/ReportingOptions.qml
        qml/parts/TagLabelLayout.qml
        qml/parts/MentionSuggestionView.qml
        qml/parts/AccountLayout.qml
        qml/parts/VerificationInfoPopup.qml
        qml/parts/CursorRectHandle.qml
        qml/parts/ApplicationShortcut.qml
        qml/parts/FeedGeneratorLinkCard.qml
        qml/parts/SkyblurContent.qml
        qml/parts/ExternalLinkCardPopup.qml
        qml/parts/LiveIconCover.qml
        qml/parts/PollLayout.qml
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
        qml/dialogs/NotificationPreferenceSettingDialog.qml
        qml/dialogs/UpdateActivitySubscriptionDialog.qml
        qml/dialogs/UpdateNotificationDeclarationDialog.qml
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

target_link_libraries(Hagoromo PRIVATE
    Qt6::Core
    Qt6::Quick
    Qt6::QuickControls2
    HagoromoLib
    HagoromoGuiLib
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

get_target_property(HAGOROMO_ADDITIONAL_LIBRARIES HagoromoLib HAGOROMO_ADDITIONAL_LIBRARIES)
if(WIN32)
    set(post_copy_destination_dir "$<TARGET_FILE_DIR:Hagoromo>")
elseif(APPLE)
    set(post_copy_destination_dir "$<TARGET_FILE_DIR:Hagoromo>/../Frameworks/")
else()
    set(post_copy_destination_dir "$<TARGET_FILE_DIR:Hagoromo>")
endif()
foreach(library_path IN LISTS HAGOROMO_ADDITIONAL_LIBRARIES)
    message(STATUS "  path=${library_path}")
    add_custom_command(TARGET Hagoromo POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E make_directory
            "${post_copy_destination_dir}"
    )
    add_custom_command(TARGET Hagoromo POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${library_path}"
            "${post_copy_destination_dir}"
            COMMENT "POST BUILD: copy ${library_path}"
    )
endforeach()
