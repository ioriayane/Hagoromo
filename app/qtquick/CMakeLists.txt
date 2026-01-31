cmake_minimum_required(VERSION 3.19)

project(HagoromoGuiLib LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)


find_package(Qt6 REQUIRED COMPONENTS
    Core
    Gui
    Network
    Quick
    QuickControls2
)
find_package(Qt6 OPTIONAL_COMPONENTS
    Core5Compat
)

add_library(HagoromoGuiLib STATIC
    account/accountlistmodel.cpp
    account/accountlistmodel.h
    account/createsession.cpp
    account/createsession.h
    atpabstractlistmodel.cpp
    atpabstractlistmodel.h
    blog/blogentrylistmodel.cpp
    blog/blogentrylistmodel.h
    chat/atpchatabstractlistmodel.cpp
    chat/atpchatabstractlistmodel.h
    chat/chatlistmodel.cpp
    chat/chatlistmodel.h
    chat/chatmessagelistmodel.cpp
    chat/chatmessagelistmodel.h
    chat/chatnotificationlistmodel.cpp
    chat/chatnotificationlistmodel.h
    column/columnlistmodel.cpp
    column/columnlistmodel.h
    column/feedtypelistmodel.cpp
    column/feedtypelistmodel.h
    controls/calendartablemodel.cpp
    controls/calendartablemodel.h
    controls/embedimagelistmodel.cpp
    controls/embedimagelistmodel.h
    controls/emojilistmodel.cpp
    controls/emojilistmodel.h
    controls/languagelistmodel.cpp
    controls/languagelistmodel.h
    draft/draftlistmodel.cpp
    draft/draftlistmodel.h
    feedgenerator/actorfeedgeneratorlistmodel.cpp
    feedgenerator/actorfeedgeneratorlistmodel.h
    feedgenerator/feedgeneratorlistmodel.cpp
    feedgenerator/feedgeneratorlistmodel.h
    link/externallink.cpp
    link/externallink.h
    link/feedgeneratorlink.cpp
    link/feedgeneratorlink.h
    link/listlink.cpp
    link/listlink.h
    link/postlink.cpp
    link/postlink.h
    list/listblockslistmodel.cpp
    list/listblockslistmodel.h
    list/listfeedlistmodel.cpp
    list/listfeedlistmodel.h
    list/listitemlistmodel.cpp
    list/listitemlistmodel.h
    list/listmuteslistmodel.cpp
    list/listmuteslistmodel.h
    list/listslistmodel.cpp
    list/listslistmodel.h
    log/logdailylistmodel.cpp
    log/logdailylistmodel.h
    log/logfeedlistmodel.cpp
    log/logfeedlistmodel.h
    log/logmonthlylistmodel.cpp
    log/logmonthlylistmodel.h
    log/logoperator.cpp
    log/logoperator.h
    log/logstatisticslistmodel.cpp
    log/logstatisticslistmodel.h
    moderation/contentfiltersettinglistmodel.cpp
    moderation/contentfiltersettinglistmodel.h
    moderation/labelerlistmodel.cpp
    moderation/labelerlistmodel.h
    moderation/mutedwordlistmodel.cpp
    moderation/mutedwordlistmodel.h
    moderation/reporter.cpp
    moderation/reporter.h
    notification/notificationlistmodel.cpp
    notification/notificationlistmodel.h
    notification/notificationpreferencelistmodel.cpp
    notification/notificationpreferencelistmodel.h
    operation/draftoperator.cpp
    operation/draftoperator.h
    operation/recordoperator.cpp
    operation/recordoperator.h
    operation/skybluroperator.cpp
    operation/skybluroperator.h
    operation/tokimekipolloperator.cpp
    operation/tokimekipolloperator.h
    operation/translator.cpp
    operation/translator.h
    profile/anyprofilelistmodel.cpp
    profile/anyprofilelistmodel.h
    profile/blockslistmodel.cpp
    profile/blockslistmodel.h
    profile/followerslistmodel.cpp
    profile/followerslistmodel.h
    profile/followslistmodel.cpp
    profile/followslistmodel.h
    profile/knownfollowerslistmodel.cpp
    profile/knownfollowerslistmodel.h
    profile/muteslistmodel.cpp
    profile/muteslistmodel.h
    profile/searchprofilelistmodel.cpp
    profile/searchprofilelistmodel.h
    profile/userprofile.cpp
    profile/userprofile.h
    realtime/editselectorlistmodel.cpp
    realtime/editselectorlistmodel.h
    realtime/realtimefeedlistmodel.cpp
    realtime/realtimefeedlistmodel.h
    realtime/realtimefeedstatusgraph.cpp
    realtime/realtimefeedstatusgraph.h
    realtime/realtimefeedstatuslistmodel.cpp
    realtime/realtimefeedstatuslistmodel.h
    systemtool.cpp
    systemtool.h
    thumbnailprovider.h
    timeline/actorlikelistmodel.cpp
    timeline/actorlikelistmodel.h
    timeline/anyfeedlistmodel.cpp
    timeline/anyfeedlistmodel.h
    timeline/authorfeedlistmodel.cpp
    timeline/authorfeedlistmodel.h
    timeline/bookmarkpostlistmodel.cpp
    timeline/bookmarkpostlistmodel.h
    timeline/customfeedlistmodel.cpp
    timeline/customfeedlistmodel.h
    timeline/postthreadlistmodel.cpp
    timeline/postthreadlistmodel.h
    timeline/quotedpostlistmodel.cpp
    timeline/quotedpostlistmodel.h
    timeline/searchpostlistmodel.cpp
    timeline/searchpostlistmodel.h
    timeline/timelinelistmodel.cpp
    timeline/timelinelistmodel.h
    timeline/userpost.cpp
    timeline/userpost.h
)

target_include_directories(HagoromoGuiLib PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

target_link_libraries(HagoromoGuiLib PRIVATE
    Qt6::Core
    $<$<TARGET_EXISTS:Qt6::Core5Compat>:Qt6::Core5Compat>
    Qt6::Gui
    Qt6::Network
    Qt6::Quick
    Qt6::QuickControls2
    HagoromoLib
)

if(HAGOROMO_UNIT_TEST_BUILD)
    target_compile_definitions(HagoromoGuiLib PUBLIC
        HAGOROMO_UNIT_TEST
    )
endif()
