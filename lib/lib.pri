
INCLUDEPATH += $$PWD

SOURCES += \
    $$PWD/atprotocol/accessatprotocol.cpp \
    $$PWD/atprotocol/app/bsky/actor/appbskyactorgetprofile.cpp \
    $$PWD/atprotocol/app/bsky/actor/appbskyactorgetprofiles.cpp \
    $$PWD/atprotocol/app/bsky/feed/appbskyfeedgetauthorfeed.cpp \
    $$PWD/atprotocol/app/bsky/feed/appbskyfeedgetposts.cpp \
    $$PWD/atprotocol/app/bsky/feed/appbskyfeedgetpostthread.cpp \
    $$PWD/atprotocol/app/bsky/feed/appbskyfeedgettimeline.cpp \
    $$PWD/atprotocol/app/bsky/graph/appbskygraphgetfollowers.cpp \
    $$PWD/atprotocol/app/bsky/graph/appbskygraphgetfollows.cpp \
    $$PWD/atprotocol/app/bsky/notification/appbskynotificationlistnotifications.cpp \
    $$PWD/atprotocol/com/atproto/repo/comatprotorepocreaterecord.cpp \
    $$PWD/atprotocol/com/atproto/repo/comatprotorepodeleterecord.cpp \
    $$PWD/atprotocol/com/atproto/repo/comatprotorepolistrecords.cpp \
    $$PWD/atprotocol/com/atproto/repo/comatprotorepouploadblob.cpp \
    $$PWD/atprotocol/com/atproto/server/comatprotoservercreatesession.cpp \
    $$PWD/atprotocol/com/atproto/server/comatprotoserverrefreshsession.cpp \
    $$PWD/atprotocol/lexicons_func.cpp \
    $$PWD/atprotocol/lexicons_func_unknown.cpp \
    $$PWD/search/search_func.cpp \
    $$PWD/search/searchposts.cpp \
    $$PWD/search/searchprofiles.cpp \
    $$PWD/tools/imagecompressor.cpp

HEADERS += \
    $$PWD/atprotocol/accessatprotocol.h \
    $$PWD/atprotocol/app/bsky/actor/appbskyactorgetprofile.h \
    $$PWD/atprotocol/app/bsky/actor/appbskyactorgetprofiles.h \
    $$PWD/atprotocol/app/bsky/feed/appbskyfeedgetauthorfeed.h \
    $$PWD/atprotocol/app/bsky/feed/appbskyfeedgetposts.h \
    $$PWD/atprotocol/app/bsky/feed/appbskyfeedgetpostthread.h \
    $$PWD/atprotocol/app/bsky/feed/appbskyfeedgettimeline.h \
    $$PWD/atprotocol/app/bsky/graph/appbskygraphgetfollowers.h \
    $$PWD/atprotocol/app/bsky/graph/appbskygraphgetfollows.h \
    $$PWD/atprotocol/app/bsky/notification/appbskynotificationlistnotifications.h \
    $$PWD/atprotocol/com/atproto/repo/comatprotorepocreaterecord.h \
    $$PWD/atprotocol/com/atproto/repo/comatprotorepodeleterecord.h \
    $$PWD/atprotocol/com/atproto/repo/comatprotorepolistrecords.h \
    $$PWD/atprotocol/com/atproto/repo/comatprotorepouploadblob.h \
    $$PWD/atprotocol/com/atproto/server/comatprotoservercreatesession.h \
    $$PWD/atprotocol/com/atproto/server/comatprotoserverrefreshsession.h \
    $$PWD/atprotocol/lexicons.h \
    $$PWD/atprotocol/lexicons_func.h \
    $$PWD/atprotocol/lexicons_func_unknown.h \
    $$PWD/search/search.h \
    $$PWD/search/search_func.h \
    $$PWD/search/searchposts.h \
    $$PWD/search/searchprofiles.h \
    $$PWD/tools/imagecompressor.h
