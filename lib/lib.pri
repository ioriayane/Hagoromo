QT += xml

INCLUDEPATH += $$PWD \
    $$PWD/../3rdparty/cpp-httplib

SOURCES += \
    $$PWD/atprotocol/accessatprotocol.cpp \
    $$PWD/atprotocol/app/bsky/actor/appbskyactorgetpreferences.cpp \
    $$PWD/atprotocol/app/bsky/actor/appbskyactorgetprofile.cpp \
    $$PWD/atprotocol/app/bsky/actor/appbskyactorgetprofiles.cpp \
    $$PWD/atprotocol/app/bsky/actor/appbskyactorputpreferences.cpp \
    $$PWD/atprotocol/app/bsky/actor/appbskyactorsearchactors.cpp \
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
    $$PWD/atprotocol/app/bsky/feed/appbskyfeedgetrepostedby.cpp \
    $$PWD/atprotocol/app/bsky/feed/appbskyfeedgettimeline.cpp \
    $$PWD/atprotocol/app/bsky/feed/appbskyfeedsearchposts.cpp \
    $$PWD/atprotocol/app/bsky/graph/appbskygraphgetfollowers.cpp \
    $$PWD/atprotocol/app/bsky/graph/appbskygraphgetfollows.cpp \
    $$PWD/atprotocol/app/bsky/graph/appbskygraphgetlist.cpp \
    $$PWD/atprotocol/app/bsky/graph/appbskygraphgetlists.cpp \
    $$PWD/atprotocol/app/bsky/graph/appbskygraphmuteactor.cpp \
    $$PWD/atprotocol/app/bsky/graph/appbskygraphunmuteactor.cpp \
    $$PWD/atprotocol/app/bsky/notification/appbskynotificationlistnotifications.cpp \
    $$PWD/atprotocol/app/bsky/unspecced/appbskyunspeccedgetpopularfeedgenerators.cpp \
    $$PWD/atprotocol/com/atproto/moderation/comatprotomoderationcreatereport.cpp \
    $$PWD/atprotocol/com/atproto/repo/comatprotorepocreaterecord.cpp \
    $$PWD/atprotocol/com/atproto/repo/comatprotorepodeleterecord.cpp \
    $$PWD/atprotocol/com/atproto/repo/comatprotorepogetrecord.cpp \
    $$PWD/atprotocol/com/atproto/repo/comatprotorepolistrecords.cpp \
    $$PWD/atprotocol/com/atproto/repo/comatprotorepoputrecord.cpp \
    $$PWD/atprotocol/com/atproto/repo/comatprotorepouploadblob.cpp \
    $$PWD/atprotocol/com/atproto/server/comatprotoservercreatesession.cpp \
    $$PWD/atprotocol/com/atproto/server/comatprotoserverrefreshsession.cpp \
    $$PWD/atprotocol/com/atproto/sync/comatprotosyncgetblob.cpp \
    $$PWD/atprotocol/lexicons_func.cpp \
    $$PWD/atprotocol/lexicons_func_unknown.cpp \
    $$PWD/http/httpaccess.cpp \
    $$PWD/http/httpaccessmanager.cpp \
    $$PWD/http/httpreply.cpp \
    $$PWD/tools/configurablelabels.cpp \
    $$PWD/tools/imagecompressor.cpp \
    $$PWD/tools/opengraphprotocol.cpp

HEADERS += \
    $$PWD/atprotocol/accessatprotocol.h \
    $$PWD/atprotocol/app/bsky/actor/appbskyactorgetpreferences.h \
    $$PWD/atprotocol/app/bsky/actor/appbskyactorgetprofile.h \
    $$PWD/atprotocol/app/bsky/actor/appbskyactorgetprofiles.h \
    $$PWD/atprotocol/app/bsky/actor/appbskyactorputpreferences.h \
    $$PWD/atprotocol/app/bsky/actor/appbskyactorsearchactors.h \
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
    $$PWD/atprotocol/app/bsky/feed/appbskyfeedgetrepostedby.h \
    $$PWD/atprotocol/app/bsky/feed/appbskyfeedgettimeline.h \
    $$PWD/atprotocol/app/bsky/feed/appbskyfeedsearchposts.h \
    $$PWD/atprotocol/app/bsky/graph/appbskygraphgetfollowers.h \
    $$PWD/atprotocol/app/bsky/graph/appbskygraphgetfollows.h \
    $$PWD/atprotocol/app/bsky/graph/appbskygraphgetlist.h \
    $$PWD/atprotocol/app/bsky/graph/appbskygraphgetlists.h \
    $$PWD/atprotocol/app/bsky/graph/appbskygraphmuteactor.h \
    $$PWD/atprotocol/app/bsky/graph/appbskygraphunmuteactor.h \
    $$PWD/atprotocol/app/bsky/notification/appbskynotificationlistnotifications.h \
    $$PWD/atprotocol/app/bsky/unspecced/appbskyunspeccedgetpopularfeedgenerators.h \
    $$PWD/atprotocol/com/atproto/moderation/comatprotomoderationcreatereport.h \
    $$PWD/atprotocol/com/atproto/repo/comatprotorepocreaterecord.h \
    $$PWD/atprotocol/com/atproto/repo/comatprotorepodeleterecord.h \
    $$PWD/atprotocol/com/atproto/repo/comatprotorepogetrecord.h \
    $$PWD/atprotocol/com/atproto/repo/comatprotorepolistrecords.h \
    $$PWD/atprotocol/com/atproto/repo/comatprotorepoputrecord.h \
    $$PWD/atprotocol/com/atproto/repo/comatprotorepouploadblob.h \
    $$PWD/atprotocol/com/atproto/server/comatprotoservercreatesession.h \
    $$PWD/atprotocol/com/atproto/server/comatprotoserverrefreshsession.h \
    $$PWD/atprotocol/com/atproto/sync/comatprotosyncgetblob.h \
    $$PWD/atprotocol/lexicons.h \
    $$PWD/atprotocol/lexicons_func.h \
    $$PWD/atprotocol/lexicons_func_unknown.h \
    $$PWD/http/httpaccess.h \
    $$PWD/http/httpaccessmanager.h \
    $$PWD/http/httpreply.h \
    $$PWD/search/search.h \
    $$PWD/tools/configurablelabels.h \
    $$PWD/tools/imagecompressor.h \
    $$PWD/tools/opengraphprotocol.h \
    $$PWD/tools/qstringex.h
