cmake_minimum_required(VERSION 3.16)

project(HagoromoLib LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(WIN32)
set(OPENSSL_ROOT_DIR "${CMAKE_PREFIX_PATH}/../../Tools/OpenSSLv3/Win_x64")
else()
set(OPENSSL_ROOT_DIR "../openssl")
endif()
set(ZLIB_ROOT "../zlib")

find_package(Qt6 REQUIRED COMPONENTS
    Core
    Core5Compat
    Gui
    HttpServer
    Network
    LinguistTools
    Sql
    Xml
    WebSockets
)
find_package(OpenSSL REQUIRED)
find_package(ZLIB REQUIRED)
qt_standard_project_setup(
    I18N_TRANSLATED_LANGUAGES ja
)

add_library(HagoromoLib STATIC
    atprotocol/accessatprotocol.cpp
    atprotocol/accessatprotocol.h
    atprotocol/app/bsky/actor/appbskyactorgetpreferences.cpp
    atprotocol/app/bsky/actor/appbskyactorgetpreferences.h
    atprotocol/app/bsky/actor/appbskyactorgetprofile.cpp
    atprotocol/app/bsky/actor/appbskyactorgetprofile.h
    atprotocol/app/bsky/actor/appbskyactorgetprofiles.cpp
    atprotocol/app/bsky/actor/appbskyactorgetprofiles.h
    atprotocol/app/bsky/actor/appbskyactorputpreferences.cpp
    atprotocol/app/bsky/actor/appbskyactorputpreferences.h
    atprotocol/app/bsky/actor/appbskyactorsearchactors.cpp
    atprotocol/app/bsky/actor/appbskyactorsearchactors.h
    atprotocol/app/bsky/actor/appbskyactorsearchactorstypeahead.cpp
    atprotocol/app/bsky/actor/appbskyactorsearchactorstypeahead.h
    atprotocol/app/bsky/feed/appbskyfeedgetactorfeeds.cpp
    atprotocol/app/bsky/feed/appbskyfeedgetactorfeeds.h
    atprotocol/app/bsky/feed/appbskyfeedgetactorlikes.cpp
    atprotocol/app/bsky/feed/appbskyfeedgetactorlikes.h
    atprotocol/app/bsky/feed/appbskyfeedgetauthorfeed.cpp
    atprotocol/app/bsky/feed/appbskyfeedgetauthorfeed.h
    atprotocol/app/bsky/feed/appbskyfeedgetfeed.cpp
    atprotocol/app/bsky/feed/appbskyfeedgetfeed.h
    atprotocol/app/bsky/feed/appbskyfeedgetfeedgenerator.cpp
    atprotocol/app/bsky/feed/appbskyfeedgetfeedgenerator.h
    atprotocol/app/bsky/feed/appbskyfeedgetfeedgenerators.cpp
    atprotocol/app/bsky/feed/appbskyfeedgetfeedgenerators.h
    atprotocol/app/bsky/feed/appbskyfeedgetlikes.cpp
    atprotocol/app/bsky/feed/appbskyfeedgetlikes.h
    atprotocol/app/bsky/feed/appbskyfeedgetlistfeed.cpp
    atprotocol/app/bsky/feed/appbskyfeedgetlistfeed.h
    atprotocol/app/bsky/feed/appbskyfeedgetposts.cpp
    atprotocol/app/bsky/feed/appbskyfeedgetposts.h
    atprotocol/app/bsky/feed/appbskyfeedgetpostthread.cpp
    atprotocol/app/bsky/feed/appbskyfeedgetpostthread.h
    atprotocol/app/bsky/feed/appbskyfeedgetquotes.cpp
    atprotocol/app/bsky/feed/appbskyfeedgetquotes.h
    atprotocol/app/bsky/feed/appbskyfeedgetrepostedby.cpp
    atprotocol/app/bsky/feed/appbskyfeedgetrepostedby.h
    atprotocol/app/bsky/feed/appbskyfeedgettimeline.cpp
    atprotocol/app/bsky/feed/appbskyfeedgettimeline.h
    atprotocol/app/bsky/feed/appbskyfeedsearchposts.cpp
    atprotocol/app/bsky/feed/appbskyfeedsearchposts.h
    atprotocol/app/bsky/graph/appbskygraphgetactorstarterpacks.cpp
    atprotocol/app/bsky/graph/appbskygraphgetactorstarterpacks.h
    atprotocol/app/bsky/graph/appbskygraphgetblocks.cpp
    atprotocol/app/bsky/graph/appbskygraphgetblocks.h
    atprotocol/app/bsky/graph/appbskygraphgetfollowers.cpp
    atprotocol/app/bsky/graph/appbskygraphgetfollowers.h
    atprotocol/app/bsky/graph/appbskygraphgetfollows.cpp
    atprotocol/app/bsky/graph/appbskygraphgetfollows.h
    atprotocol/app/bsky/graph/appbskygraphgetknownfollowers.cpp
    atprotocol/app/bsky/graph/appbskygraphgetknownfollowers.h
    atprotocol/app/bsky/graph/appbskygraphgetlist.cpp
    atprotocol/app/bsky/graph/appbskygraphgetlist.h
    atprotocol/app/bsky/graph/appbskygraphgetlistblocks.cpp
    atprotocol/app/bsky/graph/appbskygraphgetlistblocks.h
    atprotocol/app/bsky/graph/appbskygraphgetlistmutes.cpp
    atprotocol/app/bsky/graph/appbskygraphgetlistmutes.h
    atprotocol/app/bsky/graph/appbskygraphgetlists.cpp
    atprotocol/app/bsky/graph/appbskygraphgetlists.h
    atprotocol/app/bsky/graph/appbskygraphgetmutes.cpp
    atprotocol/app/bsky/graph/appbskygraphgetmutes.h
    atprotocol/app/bsky/graph/appbskygraphgetstarterpack.cpp
    atprotocol/app/bsky/graph/appbskygraphgetstarterpack.h
    atprotocol/app/bsky/graph/appbskygraphgetstarterpacks.cpp
    atprotocol/app/bsky/graph/appbskygraphgetstarterpacks.h
    atprotocol/app/bsky/graph/appbskygraphmuteactor.cpp
    atprotocol/app/bsky/graph/appbskygraphmuteactor.h
    atprotocol/app/bsky/graph/appbskygraphmuteactorlist.cpp
    atprotocol/app/bsky/graph/appbskygraphmuteactorlist.h
    atprotocol/app/bsky/graph/appbskygraphmutethread.cpp
    atprotocol/app/bsky/graph/appbskygraphmutethread.h
    atprotocol/app/bsky/graph/appbskygraphunmuteactor.cpp
    atprotocol/app/bsky/graph/appbskygraphunmuteactor.h
    atprotocol/app/bsky/graph/appbskygraphunmuteactorlist.cpp
    atprotocol/app/bsky/graph/appbskygraphunmuteactorlist.h
    atprotocol/app/bsky/graph/appbskygraphunmutethread.cpp
    atprotocol/app/bsky/graph/appbskygraphunmutethread.h
    atprotocol/app/bsky/labeler/appbskylabelergetservices.cpp
    atprotocol/app/bsky/labeler/appbskylabelergetservices.h
    atprotocol/app/bsky/notification/appbskynotificationlistnotifications.cpp
    atprotocol/app/bsky/notification/appbskynotificationlistnotifications.h
    atprotocol/app/bsky/notification/appbskynotificationputpreferences.cpp
    atprotocol/app/bsky/notification/appbskynotificationputpreferences.h
    atprotocol/app/bsky/notification/appbskynotificationupdateseen.cpp
    atprotocol/app/bsky/notification/appbskynotificationupdateseen.h
    atprotocol/app/bsky/unspecced/appbskyunspeccedgetconfig.cpp
    atprotocol/app/bsky/unspecced/appbskyunspeccedgetconfig.h
    atprotocol/app/bsky/unspecced/appbskyunspeccedgetpopularfeedgenerators.cpp
    atprotocol/app/bsky/unspecced/appbskyunspeccedgetpopularfeedgenerators.h
    atprotocol/app/bsky/video/appbskyvideogetjobstatus.cpp
    atprotocol/app/bsky/video/appbskyvideogetjobstatus.h
    atprotocol/app/bsky/video/appbskyvideogetuploadlimits.cpp
    atprotocol/app/bsky/video/appbskyvideogetuploadlimits.h
    atprotocol/app/bsky/video/appbskyvideouploadvideo.cpp
    atprotocol/app/bsky/video/appbskyvideouploadvideo.h
    atprotocol/chat/bsky/actor/chatbskyactordeleteaccount.cpp
    atprotocol/chat/bsky/actor/chatbskyactordeleteaccount.h
    atprotocol/chat/bsky/actor/chatbskyactorexportaccountdata.cpp
    atprotocol/chat/bsky/actor/chatbskyactorexportaccountdata.h
    atprotocol/chat/bsky/convo/chatbskyconvoacceptconvo.cpp
    atprotocol/chat/bsky/convo/chatbskyconvoacceptconvo.h
    atprotocol/chat/bsky/convo/chatbskyconvoaddreaction.cpp
    atprotocol/chat/bsky/convo/chatbskyconvoaddreaction.h
    atprotocol/chat/bsky/convo/chatbskyconvodeletemessageforself.cpp
    atprotocol/chat/bsky/convo/chatbskyconvodeletemessageforself.h
    atprotocol/chat/bsky/convo/chatbskyconvogetconvo.cpp
    atprotocol/chat/bsky/convo/chatbskyconvogetconvo.h
    atprotocol/chat/bsky/convo/chatbskyconvogetconvoavailability.cpp
    atprotocol/chat/bsky/convo/chatbskyconvogetconvoavailability.h
    atprotocol/chat/bsky/convo/chatbskyconvogetconvoformembers.cpp
    atprotocol/chat/bsky/convo/chatbskyconvogetconvoformembers.h
    atprotocol/chat/bsky/convo/chatbskyconvogetlog.cpp
    atprotocol/chat/bsky/convo/chatbskyconvogetlog.h
    atprotocol/chat/bsky/convo/chatbskyconvogetmessages.cpp
    atprotocol/chat/bsky/convo/chatbskyconvogetmessages.h
    atprotocol/chat/bsky/convo/chatbskyconvoleaveconvo.cpp
    atprotocol/chat/bsky/convo/chatbskyconvoleaveconvo.h
    atprotocol/chat/bsky/convo/chatbskyconvolistconvos.cpp
    atprotocol/chat/bsky/convo/chatbskyconvolistconvos.h
    atprotocol/chat/bsky/convo/chatbskyconvomuteconvo.cpp
    atprotocol/chat/bsky/convo/chatbskyconvomuteconvo.h
    atprotocol/chat/bsky/convo/chatbskyconvoremovereaction.cpp
    atprotocol/chat/bsky/convo/chatbskyconvoremovereaction.h
    atprotocol/chat/bsky/convo/chatbskyconvosendmessage.cpp
    atprotocol/chat/bsky/convo/chatbskyconvosendmessage.h
    atprotocol/chat/bsky/convo/chatbskyconvosendmessagebatch.cpp
    atprotocol/chat/bsky/convo/chatbskyconvosendmessagebatch.h
    atprotocol/chat/bsky/convo/chatbskyconvounmuteconvo.cpp
    atprotocol/chat/bsky/convo/chatbskyconvounmuteconvo.h
    atprotocol/chat/bsky/convo/chatbskyconvoupdateallread.cpp
    atprotocol/chat/bsky/convo/chatbskyconvoupdateallread.h
    atprotocol/chat/bsky/convo/chatbskyconvoupdateread.cpp
    atprotocol/chat/bsky/convo/chatbskyconvoupdateread.h
    atprotocol/chat/bsky/moderation/chatbskymoderationgetactormetadata.cpp
    atprotocol/chat/bsky/moderation/chatbskymoderationgetactormetadata.h
    atprotocol/chat/bsky/moderation/chatbskymoderationgetmessagecontext.cpp
    atprotocol/chat/bsky/moderation/chatbskymoderationgetmessagecontext.h
    atprotocol/chat/bsky/moderation/chatbskymoderationupdateactoraccess.cpp
    atprotocol/chat/bsky/moderation/chatbskymoderationupdateactoraccess.h
    atprotocol/com/atproto/moderation/comatprotomoderationcreatereport.cpp
    atprotocol/com/atproto/moderation/comatprotomoderationcreatereport.h
    atprotocol/com/atproto/repo/comatprotorepocreaterecord.cpp
    atprotocol/com/atproto/repo/comatprotorepocreaterecord.h
    atprotocol/com/atproto/repo/comatprotorepodeleterecord.cpp
    atprotocol/com/atproto/repo/comatprotorepodeleterecord.h
    atprotocol/com/atproto/repo/comatprotorepodescriberepo.cpp
    atprotocol/com/atproto/repo/comatprotorepodescriberepo.h
    atprotocol/com/atproto/repo/comatprotorepogetrecord.cpp
    atprotocol/com/atproto/repo/comatprotorepogetrecord.h
    atprotocol/com/atproto/repo/comatprotorepolistrecords.cpp
    atprotocol/com/atproto/repo/comatprotorepolistrecords.h
    atprotocol/com/atproto/repo/comatprotorepoputrecord.cpp
    atprotocol/com/atproto/repo/comatprotorepoputrecord.h
    atprotocol/com/atproto/repo/comatprotorepouploadblob.cpp
    atprotocol/com/atproto/repo/comatprotorepouploadblob.h
    atprotocol/com/atproto/server/comatprotoservercreatesession.cpp
    atprotocol/com/atproto/server/comatprotoservercreatesession.h
    atprotocol/com/atproto/server/comatprotoserverrefreshsession.cpp
    atprotocol/com/atproto/server/comatprotoserverrefreshsession.h
    atprotocol/com/atproto/sync/comatprotosyncgetblob.cpp
    atprotocol/com/atproto/sync/comatprotosyncgetblob.h
    atprotocol/com/atproto/sync/comatprotosyncgetrepo.cpp
    atprotocol/com/atproto/sync/comatprotosyncgetrepo.h
    atprotocol/lexicons.h
    atprotocol/lexicons_func.cpp
    atprotocol/lexicons_func.h
    atprotocol/lexicons_func_unknown.cpp
    atprotocol/lexicons_func_unknown.h
    common.h
    extension/app/bsky/actor/appbskyactorputpreferencesex.cpp
    extension/app/bsky/actor/appbskyactorputpreferencesex.h
    extension/com/atproto/moderation/comatprotomoderationcreatereportex.cpp
    extension/com/atproto/moderation/comatprotomoderationcreatereportex.h
    extension/com/atproto/repo/comatprotorepocreaterecordex.cpp
    extension/com/atproto/repo/comatprotorepocreaterecordex.h
    extension/com/atproto/repo/comatprotorepodeleterecordex.cpp
    extension/com/atproto/repo/comatprotorepodeleterecordex.h
    extension/com/atproto/repo/comatprotorepogetrecordex.cpp
    extension/com/atproto/repo/comatprotorepogetrecordex.h
    extension/com/atproto/repo/comatprotorepolistrecordsex.cpp
    extension/com/atproto/repo/comatprotorepolistrecordsex.h
    extension/com/atproto/repo/comatprotorepoputrecordex.cpp
    extension/com/atproto/repo/comatprotorepoputrecordex.h
    extension/com/atproto/server/comatprotoservercreatesessionex.cpp
    extension/com/atproto/server/comatprotoservercreatesessionex.h
    extension/com/atproto/server/comatprotoserverrefreshsessionex.cpp
    extension/com/atproto/server/comatprotoserverrefreshsessionex.h
    extension/com/atproto/sync/comatprotosyncsubscribereposex.cpp
    extension/com/atproto/sync/comatprotosyncsubscribereposex.h
    extension/directory/plc/directoryplc.cpp
    extension/directory/plc/directoryplc.h
    extension/directory/plc/directoryplclogaudit.cpp
    extension/directory/plc/directoryplclogaudit.h
    extension/oauth/oauthpushedauthorizationrequest.cpp
    extension/oauth/oauthpushedauthorizationrequest.h
    extension/oauth/oauthrequesttoken.cpp
    extension/oauth/oauthrequesttoken.h
    extension/well-known/wellknownoauthauthorizationserver.cpp
    extension/well-known/wellknownoauthauthorizationserver.h
    extension/well-known/wellknownoauthprotectedresource.cpp
    extension/well-known/wellknownoauthprotectedresource.h
    http/httpaccess.cpp
    http/httpaccess.h
    http/httpaccessmanager.cpp
    http/httpaccessmanager.h
    http/httpreply.cpp
    http/httpreply.h
    http/simplehttpserver.cpp
    http/simplehttpserver.h
    log/logaccess.cpp
    log/logaccess.h
    log/logmanager.cpp
    log/logmanager.h
    realtime/abstractpostselector.cpp
    realtime/abstractpostselector.h
    realtime/andpostselector.cpp
    realtime/andpostselector.h
    realtime/firehosereceiver.cpp
    realtime/firehosereceiver.h
    realtime/followerspostselector.cpp
    realtime/followerspostselector.h
    realtime/followingpostselector.cpp
    realtime/followingpostselector.h
    realtime/listpostsselector.cpp
    realtime/listpostsselector.h
    realtime/mepostselector.cpp
    realtime/mepostselector.h
    realtime/notpostselector.cpp
    realtime/notpostselector.h
    realtime/orpostselector.cpp
    realtime/orpostselector.h
    realtime/xorpostselector.cpp
    realtime/xorpostselector.h
    search/search.h
    tools/accountmanager.cpp
    tools/accountmanager.h
    tools/authorization.cpp
    tools/authorization.h
    tools/base32.cpp
    tools/base32.h
    tools/cardecoder.cpp
    tools/cardecoder.h
    tools/chatlogsubscriber.cpp
    tools/chatlogsubscriber.h
    tools/configurablelabels.cpp
    tools/configurablelabels.h
    tools/encryption.cpp
    tools/encryption.h
    tools/encryption_seed.h
    tools/es256.cpp
    tools/es256.h
    tools/imagecompressor.cpp
    tools/imagecompressor.h
    tools/jsonwebtoken.cpp
    tools/jsonwebtoken.h
    tools/labelerprovider.cpp
    tools/labelerprovider.h
    tools/labelprovider.cpp
    tools/labelprovider.h
    tools/leb128.cpp
    tools/leb128.h
    tools/listitemscache.cpp
    tools/listitemscache.h
    tools/opengraphprotocol.cpp
    tools/opengraphprotocol.h
    tools/pinnedpostcache.cpp
    tools/pinnedpostcache.h
    tools/qstringex.h
    tools/tid.cpp
    tools/tid.h
    # resources
    lib.qrc
)

target_include_directories(HagoromoLib PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_include_directories(HagoromoLib PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/../3rdparty/cpp-httplib
    ${OPENSSL_INCLUDE_DIR}
)
target_link_libraries(HagoromoLib PRIVATE
    Qt${QT_VERSION_MAJOR}::Core
    Qt6::Core5Compat
    Qt6::Gui
    Qt6::HttpServer
    Qt6::Network
    Qt6::Sql
    Qt6::Xml
    Qt6::WebSockets
    OpenSSL::SSL
    OpenSSL::Crypto
    ZLIB::ZLIB
)

target_compile_definitions(HagoromoLib PRIVATE
    HAGOROMOLIB_LIBRARY
    CPPHTTPLIB_ZLIB_SUPPORT # zlib support for cpp-httplib
)

qt_add_translations(HagoromoLib QM_OUTPUT_DIRECTORY translations)

if (ZLIB_FOUND)
    target_include_directories(HagoromoLib PRIVATE ${ZLIB_INCLUDE_DIRS})
    target_link_libraries(HagoromoLib PRIVATE ${ZLIB_LIBRARIES})

    get_filename_component(ZLIB_ROOT_ABS ${ZLIB_ROOT} ABSOLUTE)
    if(WIN32)
        if(CMAKE_BUILD_TYPE STREQUAL "Debug")
            set(ZLIB_DLL_PATH "${ZLIB_ROOT_ABS}/bin/zlibd.dll")
        else()
            set(ZLIB_DLL_PATH "${ZLIB_ROOT_ABS}/bin/zlib.dll")
        endif()
    else()
        # set(ZLIB_DLL_PATH "${ZLIB_ROOT_ABS}/lib/libz.so")
    endif()
    if (EXISTS "${ZLIB_DLL_PATH}")
        message(STATUS  "ZLIB_FOUND=${ZLIB_FOUND} ZLIB_DLL_PATH=${ZLIB_DLL_PATH}")
        add_custom_command(TARGET HagoromoLib POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
                "${ZLIB_DLL_PATH}"
                "$<TARGET_FILE_DIR:HagoromoLib>"
            COMMENT "Copying zlib.dll to output directory"
        )
        install(FILES "${ZLIB_DLL_PATH}"
            DESTINATION ${CMAKE_INSTALL_LIBDIR}
        )
    endif()
endif()
