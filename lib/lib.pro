QT += xml sql websockets httpserver
greaterThan(QT_MAJOR_VERSION, 5) {
QT += core5compat
}

TEMPLATE = lib
CONFIG += staticlib

CONFIG += c++11

INCLUDEPATH += $$PWD \
    $$PWD/../3rdparty/cpp-httplib \
    $$PWD/../zlib/include
greaterThan(QT_MAJOR_VERSION, 5) {
win32:INCLUDEPATH += $$dirname(QMAKE_QMAKE)/../../../Tools/OpenSSLv3/Win_x64/include
}else{
win32:INCLUDEPATH += $$dirname(QMAKE_QMAKE)/../../../Tools/OpenSSL/Win_x64/include
}
unix:INCLUDEPATH += ../openssl/include

# build arch
mac:QMAKE_APPLE_DEVICE_ARCHS=x86_64

DEFINES += CPPHTTPLIB_ZLIB_SUPPORT # zlib support for cpp-httplib

TRANSLATIONS += i18n/lib_ja.ts

SOURCES += \
    $$PWD/atprotocol/accessatprotocol.cpp \
    $$PWD/atprotocol/app/bsky/actor/appbskyactorgetpreferences.cpp \
    $$PWD/atprotocol/app/bsky/actor/appbskyactorgetprofile.cpp \
    $$PWD/atprotocol/app/bsky/actor/appbskyactorgetprofiles.cpp \
    $$PWD/atprotocol/app/bsky/actor/appbskyactorputpreferences.cpp \
    $$PWD/atprotocol/app/bsky/actor/appbskyactorsearchactors.cpp \
    $$PWD/atprotocol/app/bsky/actor/appbskyactorsearchactorstypeahead.cpp \
    $$PWD/atprotocol/app/bsky/feed/appbskyfeedgetactorfeeds.cpp \
    $$PWD/atprotocol/app/bsky/feed/appbskyfeedgetactorlikes.cpp \
    $$PWD/atprotocol/app/bsky/feed/appbskyfeedgetauthorfeed.cpp \
    $$PWD/atprotocol/app/bsky/feed/appbskyfeedgetfeed.cpp \
    $$PWD/atprotocol/app/bsky/feed/appbskyfeedgetfeedgenerator.cpp \
    $$PWD/atprotocol/app/bsky/feed/appbskyfeedgetfeedgenerators.cpp \
    $$PWD/atprotocol/app/bsky/feed/appbskyfeedgetlikes.cpp \
    $$PWD/atprotocol/app/bsky/feed/appbskyfeedgetlistfeed.cpp \
    $$PWD/atprotocol/app/bsky/feed/appbskyfeedgetposts.cpp \
    $$PWD/atprotocol/app/bsky/feed/appbskyfeedgetpostthread.cpp \
    $$PWD/atprotocol/app/bsky/feed/appbskyfeedgetquotes.cpp \
    $$PWD/atprotocol/app/bsky/feed/appbskyfeedgetrepostedby.cpp \
    $$PWD/atprotocol/app/bsky/feed/appbskyfeedgettimeline.cpp \
    $$PWD/atprotocol/app/bsky/feed/appbskyfeedsearchposts.cpp \
    $$PWD/atprotocol/app/bsky/graph/appbskygraphgetblocks.cpp \
    $$PWD/atprotocol/app/bsky/graph/appbskygraphgetfollowers.cpp \
    $$PWD/atprotocol/app/bsky/graph/appbskygraphgetfollows.cpp \
    $$PWD/atprotocol/app/bsky/graph/appbskygraphgetknownfollowers.cpp \
    $$PWD/atprotocol/app/bsky/graph/appbskygraphgetlist.cpp \
    $$PWD/atprotocol/app/bsky/graph/appbskygraphgetlistblocks.cpp \
    $$PWD/atprotocol/app/bsky/graph/appbskygraphgetlistmutes.cpp \
    $$PWD/atprotocol/app/bsky/graph/appbskygraphgetlists.cpp \
    $$PWD/atprotocol/app/bsky/graph/appbskygraphgetmutes.cpp \
    $$PWD/atprotocol/app/bsky/graph/appbskygraphmuteactor.cpp \
    $$PWD/atprotocol/app/bsky/graph/appbskygraphmuteactorlist.cpp \
    $$PWD/atprotocol/app/bsky/graph/appbskygraphmutethread.cpp \
    $$PWD/atprotocol/app/bsky/graph/appbskygraphunmuteactor.cpp \
    $$PWD/atprotocol/app/bsky/graph/appbskygraphunmuteactorlist.cpp \
    $$PWD/atprotocol/app/bsky/graph/appbskygraphunmutethread.cpp \
    $$PWD/atprotocol/app/bsky/labeler/appbskylabelergetservices.cpp \
    $$PWD/atprotocol/app/bsky/notification/appbskynotificationlistnotifications.cpp \
    $$PWD/atprotocol/app/bsky/notification/appbskynotificationupdateseen.cpp \
    $$PWD/atprotocol/app/bsky/unspecced/appbskyunspeccedgetconfig.cpp \
    $$PWD/atprotocol/app/bsky/unspecced/appbskyunspeccedgetpopularfeedgenerators.cpp \
    $$PWD/atprotocol/chat/bsky/actor/chatbskyactordeleteaccount.cpp \
    $$PWD/atprotocol/chat/bsky/actor/chatbskyactorexportaccountdata.cpp \
    $$PWD/atprotocol/chat/bsky/convo/chatbskyconvodeletemessageforself.cpp \
    $$PWD/atprotocol/chat/bsky/convo/chatbskyconvogetconvo.cpp \
    $$PWD/atprotocol/chat/bsky/convo/chatbskyconvogetconvoformembers.cpp \
    $$PWD/atprotocol/chat/bsky/convo/chatbskyconvogetlog.cpp \
    $$PWD/atprotocol/chat/bsky/convo/chatbskyconvogetmessages.cpp \
    $$PWD/atprotocol/chat/bsky/convo/chatbskyconvoleaveconvo.cpp \
    $$PWD/atprotocol/chat/bsky/convo/chatbskyconvolistconvos.cpp \
    $$PWD/atprotocol/chat/bsky/convo/chatbskyconvomuteconvo.cpp \
    $$PWD/atprotocol/chat/bsky/convo/chatbskyconvosendmessage.cpp \
    $$PWD/atprotocol/chat/bsky/convo/chatbskyconvosendmessagebatch.cpp \
    $$PWD/atprotocol/chat/bsky/convo/chatbskyconvounmuteconvo.cpp \
    $$PWD/atprotocol/chat/bsky/convo/chatbskyconvoupdateread.cpp \
    $$PWD/atprotocol/chat/bsky/moderation/chatbskymoderationgetactormetadata.cpp \
    $$PWD/atprotocol/chat/bsky/moderation/chatbskymoderationgetmessagecontext.cpp \
    $$PWD/atprotocol/chat/bsky/moderation/chatbskymoderationupdateactoraccess.cpp \
    $$PWD/atprotocol/com/atproto/moderation/comatprotomoderationcreatereport.cpp \
    $$PWD/atprotocol/com/atproto/repo/comatprotorepocreaterecord.cpp \
    $$PWD/atprotocol/com/atproto/repo/comatprotorepodeleterecord.cpp \
    $$PWD/atprotocol/com/atproto/repo/comatprotorepodescriberepo.cpp \
    $$PWD/atprotocol/com/atproto/repo/comatprotorepogetrecord.cpp \
    $$PWD/atprotocol/com/atproto/repo/comatprotorepolistrecords.cpp \
    $$PWD/atprotocol/com/atproto/repo/comatprotorepoputrecord.cpp \
    $$PWD/atprotocol/com/atproto/repo/comatprotorepouploadblob.cpp \
    $$PWD/atprotocol/com/atproto/server/comatprotoservercreatesession.cpp \
    $$PWD/atprotocol/com/atproto/server/comatprotoserverrefreshsession.cpp \
    $$PWD/atprotocol/com/atproto/sync/comatprotosyncgetblob.cpp \
    $$PWD/atprotocol/com/atproto/sync/comatprotosyncgetrepo.cpp \
    $$PWD/atprotocol/lexicons_func.cpp \
    $$PWD/atprotocol/lexicons_func_unknown.cpp \
    $$PWD/extension/app/bsky/actor/appbskyactorputpreferencesex.cpp \
    $$PWD/extension/com/atproto/moderation/comatprotomoderationcreatereportex.cpp \
    $$PWD/extension/com/atproto/repo/comatprotorepocreaterecordex.cpp \
    $$PWD/extension/com/atproto/repo/comatprotorepodeleterecordex.cpp \
    $$PWD/extension/com/atproto/repo/comatprotorepogetrecordex.cpp \
    $$PWD/extension/com/atproto/repo/comatprotorepolistrecordsex.cpp \
    $$PWD/extension/com/atproto/repo/comatprotorepoputrecordex.cpp \
    $$PWD/extension/com/atproto/server/comatprotoservercreatesessionex.cpp \
    $$PWD/extension/com/atproto/server/comatprotoserverrefreshsessionex.cpp \
    $$PWD/extension/com/atproto/sync/comatprotosyncsubscribereposex.cpp \
    $$PWD/extension/directory/plc/directoryplc.cpp \
    $$PWD/extension/directory/plc/directoryplclogaudit.cpp \
    $$PWD/extension/oauth/oauthpushedauthorizationrequest.cpp \
    $$PWD/extension/oauth/oauthrequesttoken.cpp \
    $$PWD/extension/well-known/wellknownoauthauthorizationserver.cpp \
    $$PWD/extension/well-known/wellknownoauthprotectedresource.cpp \
    $$PWD/http/httpaccess.cpp \
    $$PWD/http/httpaccessmanager.cpp \
    $$PWD/http/httpreply.cpp \
    $$PWD/http/simplehttpserver.cpp \
    $$PWD/log/logaccess.cpp \
    $$PWD/log/logmanager.cpp \
    $$PWD/realtime/abstractpostselector.cpp \
    $$PWD/realtime/andpostselector.cpp \
    $$PWD/realtime/firehosereceiver.cpp \
    $$PWD/realtime/followerspostselector.cpp \
    $$PWD/realtime/followingpostselector.cpp \
    $$PWD/realtime/mepostselector.cpp \
    $$PWD/realtime/notpostselector.cpp \
    $$PWD/realtime/orpostselector.cpp \
    $$PWD/realtime/xorpostselector.cpp \
    $$PWD/tools/accountmanager.cpp \
    $$PWD/tools/authorization.cpp \
    $$PWD/tools/base32.cpp \
    $$PWD/tools/cardecoder.cpp \
    $$PWD/tools/chatlogsubscriber.cpp \
    $$PWD/tools/configurablelabels.cpp \
    $$PWD/tools/encryption.cpp \
    $$PWD/tools/es256.cpp \
    $$PWD/tools/imagecompressor.cpp \
    $$PWD/tools/jsonwebtoken.cpp \
    $$PWD/tools/labelerprovider.cpp \
    $$PWD/tools/leb128.cpp \
    $$PWD/tools/listitemscache.cpp \
    $$PWD/tools/opengraphprotocol.cpp \
    $$PWD/tools/pinnedpostcache.cpp \
    $$PWD/realtime/listpostsselector.cpp \
    $$PWD/tools/labelprovider.cpp \
    $$PWD/tools/tid.cpp

HEADERS += \
    $$PWD/atprotocol/accessatprotocol.h \
    $$PWD/atprotocol/app/bsky/actor/appbskyactorgetpreferences.h \
    $$PWD/atprotocol/app/bsky/actor/appbskyactorgetprofile.h \
    $$PWD/atprotocol/app/bsky/actor/appbskyactorgetprofiles.h \
    $$PWD/atprotocol/app/bsky/actor/appbskyactorputpreferences.h \
    $$PWD/atprotocol/app/bsky/actor/appbskyactorsearchactors.h \
    $$PWD/atprotocol/app/bsky/actor/appbskyactorsearchactorstypeahead.h \
    $$PWD/atprotocol/app/bsky/feed/appbskyfeedgetactorfeeds.h \
    $$PWD/atprotocol/app/bsky/feed/appbskyfeedgetactorlikes.h \
    $$PWD/atprotocol/app/bsky/feed/appbskyfeedgetauthorfeed.h \
    $$PWD/atprotocol/app/bsky/feed/appbskyfeedgetfeed.h \
    $$PWD/atprotocol/app/bsky/feed/appbskyfeedgetfeedgenerator.h \
    $$PWD/atprotocol/app/bsky/feed/appbskyfeedgetfeedgenerators.h \
    $$PWD/atprotocol/app/bsky/feed/appbskyfeedgetlikes.h \
    $$PWD/atprotocol/app/bsky/feed/appbskyfeedgetlistfeed.h \
    $$PWD/atprotocol/app/bsky/feed/appbskyfeedgetposts.h \
    $$PWD/atprotocol/app/bsky/feed/appbskyfeedgetpostthread.h \
    $$PWD/atprotocol/app/bsky/feed/appbskyfeedgetquotes.h \
    $$PWD/atprotocol/app/bsky/feed/appbskyfeedgetrepostedby.h \
    $$PWD/atprotocol/app/bsky/feed/appbskyfeedgettimeline.h \
    $$PWD/atprotocol/app/bsky/feed/appbskyfeedsearchposts.h \
    $$PWD/atprotocol/app/bsky/graph/appbskygraphgetblocks.h \
    $$PWD/atprotocol/app/bsky/graph/appbskygraphgetfollowers.h \
    $$PWD/atprotocol/app/bsky/graph/appbskygraphgetfollows.h \
    $$PWD/atprotocol/app/bsky/graph/appbskygraphgetknownfollowers.h \
    $$PWD/atprotocol/app/bsky/graph/appbskygraphgetlist.h \
    $$PWD/atprotocol/app/bsky/graph/appbskygraphgetlistblocks.h \
    $$PWD/atprotocol/app/bsky/graph/appbskygraphgetlistmutes.h \
    $$PWD/atprotocol/app/bsky/graph/appbskygraphgetlists.h \
    $$PWD/atprotocol/app/bsky/graph/appbskygraphgetmutes.h \
    $$PWD/atprotocol/app/bsky/graph/appbskygraphmuteactor.h \
    $$PWD/atprotocol/app/bsky/graph/appbskygraphmuteactorlist.h \
    $$PWD/atprotocol/app/bsky/graph/appbskygraphmutethread.h \
    $$PWD/atprotocol/app/bsky/graph/appbskygraphunmuteactor.h \
    $$PWD/atprotocol/app/bsky/graph/appbskygraphunmuteactorlist.h \
    $$PWD/atprotocol/app/bsky/graph/appbskygraphunmutethread.h \
    $$PWD/atprotocol/app/bsky/labeler/appbskylabelergetservices.h \
    $$PWD/atprotocol/app/bsky/notification/appbskynotificationlistnotifications.h \
    $$PWD/atprotocol/app/bsky/notification/appbskynotificationupdateseen.h \
    $$PWD/atprotocol/app/bsky/unspecced/appbskyunspeccedgetconfig.h \
    $$PWD/atprotocol/app/bsky/unspecced/appbskyunspeccedgetpopularfeedgenerators.h \
    $$PWD/atprotocol/chat/bsky/actor/chatbskyactordeleteaccount.h \
    $$PWD/atprotocol/chat/bsky/actor/chatbskyactorexportaccountdata.h \
    $$PWD/atprotocol/chat/bsky/convo/chatbskyconvodeletemessageforself.h \
    $$PWD/atprotocol/chat/bsky/convo/chatbskyconvogetconvo.h \
    $$PWD/atprotocol/chat/bsky/convo/chatbskyconvogetconvoformembers.h \
    $$PWD/atprotocol/chat/bsky/convo/chatbskyconvogetlog.h \
    $$PWD/atprotocol/chat/bsky/convo/chatbskyconvogetmessages.h \
    $$PWD/atprotocol/chat/bsky/convo/chatbskyconvoleaveconvo.h \
    $$PWD/atprotocol/chat/bsky/convo/chatbskyconvolistconvos.h \
    $$PWD/atprotocol/chat/bsky/convo/chatbskyconvomuteconvo.h \
    $$PWD/atprotocol/chat/bsky/convo/chatbskyconvosendmessage.h \
    $$PWD/atprotocol/chat/bsky/convo/chatbskyconvosendmessagebatch.h \
    $$PWD/atprotocol/chat/bsky/convo/chatbskyconvounmuteconvo.h \
    $$PWD/atprotocol/chat/bsky/convo/chatbskyconvoupdateread.h \
    $$PWD/atprotocol/chat/bsky/moderation/chatbskymoderationgetactormetadata.h \
    $$PWD/atprotocol/chat/bsky/moderation/chatbskymoderationgetmessagecontext.h \
    $$PWD/atprotocol/chat/bsky/moderation/chatbskymoderationupdateactoraccess.h \
    $$PWD/atprotocol/com/atproto/moderation/comatprotomoderationcreatereport.h \
    $$PWD/atprotocol/com/atproto/repo/comatprotorepocreaterecord.h \
    $$PWD/atprotocol/com/atproto/repo/comatprotorepodeleterecord.h \
    $$PWD/atprotocol/com/atproto/repo/comatprotorepodescriberepo.h \
    $$PWD/atprotocol/com/atproto/repo/comatprotorepogetrecord.h \
    $$PWD/atprotocol/com/atproto/repo/comatprotorepolistrecords.h \
    $$PWD/atprotocol/com/atproto/repo/comatprotorepoputrecord.h \
    $$PWD/atprotocol/com/atproto/repo/comatprotorepouploadblob.h \
    $$PWD/atprotocol/com/atproto/server/comatprotoservercreatesession.h \
    $$PWD/atprotocol/com/atproto/server/comatprotoserverrefreshsession.h \
    $$PWD/atprotocol/com/atproto/sync/comatprotosyncgetblob.h \
    $$PWD/atprotocol/com/atproto/sync/comatprotosyncgetrepo.h \
    $$PWD/atprotocol/lexicons.h \
    $$PWD/atprotocol/lexicons_func.h \
    $$PWD/atprotocol/lexicons_func_unknown.h \
    $$PWD/extension/app/bsky/actor/appbskyactorputpreferencesex.h \
    $$PWD/extension/com/atproto/moderation/comatprotomoderationcreatereportex.h \
    $$PWD/extension/com/atproto/repo/comatprotorepocreaterecordex.h \
    $$PWD/extension/com/atproto/repo/comatprotorepodeleterecordex.h \
    $$PWD/extension/com/atproto/repo/comatprotorepogetrecordex.h \
    $$PWD/extension/com/atproto/repo/comatprotorepolistrecordsex.h \
    $$PWD/extension/com/atproto/repo/comatprotorepoputrecordex.h \
    $$PWD/extension/com/atproto/server/comatprotoservercreatesessionex.h \
    $$PWD/extension/com/atproto/server/comatprotoserverrefreshsessionex.h \
    $$PWD/extension/com/atproto/sync/comatprotosyncsubscribereposex.h \
    $$PWD/extension/directory/plc/directoryplc.h \
    $$PWD/extension/directory/plc/directoryplclogaudit.h \
    $$PWD/extension/oauth/oauthpushedauthorizationrequest.h \
    $$PWD/extension/oauth/oauthrequesttoken.h \
    $$PWD/extension/well-known/wellknownoauthauthorizationserver.h \
    $$PWD/extension/well-known/wellknownoauthprotectedresource.h \
    $$PWD/http/httpaccess.h \
    $$PWD/http/httpaccessmanager.h \
    $$PWD/http/httpreply.h \
    $$PWD/http/simplehttpserver.h \
    $$PWD/log/logaccess.h \
    $$PWD/log/logmanager.h \
    $$PWD/realtime/abstractpostselector.h \
    $$PWD/realtime/andpostselector.h \
    $$PWD/realtime/firehosereceiver.h \
    $$PWD/realtime/followerspostselector.h \
    $$PWD/realtime/followingpostselector.h \
    $$PWD/realtime/mepostselector.h \
    $$PWD/realtime/notpostselector.h \
    $$PWD/realtime/orpostselector.h \
    $$PWD/realtime/xorpostselector.h \
    $$PWD/search/search.h \
    $$PWD/tools/accountmanager.h \
    $$PWD/tools/authorization.h \
    $$PWD/tools/base32.h \
    $$PWD/tools/cardecoder.h \
    $$PWD/tools/chatlogsubscriber.h \
    $$PWD/tools/configurablelabels.h \
    $$PWD/tools/encryption.h \
    $$PWD/tools/encryption_seed.h \
    $$PWD/tools/es256.h \
    $$PWD/tools/imagecompressor.h \
    $$PWD/tools/jsonwebtoken.h \
    $$PWD/tools/labelerprovider.h \
    $$PWD/tools/leb128.h \
    $$PWD/tools/listitemscache.h \
    $$PWD/tools/opengraphprotocol.h \
    $$PWD/tools/pinnedpostcache.h \
    $$PWD/tools/qstringex.h \
    $$PWD/common.h \
    $$PWD/realtime/listpostsselector.h \
    $$PWD/tools/labelprovider.h \
    $$PWD/tools/tid.h

RESOURCES += \
    $$PWD/lib.qrc
