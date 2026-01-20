#include <QtTest>
#include <QCoreApplication>
#include <QTemporaryFile>

#include "webserver.h"
#include "atprotocol/app/bsky/actor/appbskyactorsearchactorstypeahead.h"
#include "atprotocol/app/bsky/feed/appbskyfeedgetactorfeeds.h"
#include "atprotocol/app/bsky/feed/appbskyfeedgetactorlikes.h"
#include "atprotocol/app/bsky/feed/appbskyfeedgetauthorfeed.h"
#include "atprotocol/app/bsky/feed/appbskyfeedgetfeed.h"
#include "atprotocol/app/bsky/feed/appbskyfeedgettimeline.h"
#include "atprotocol/app/bsky/feed/appbskyfeedgetfeedgenerator.h"
#include "atprotocol/app/bsky/feed/appbskyfeedgetfeedgenerators.h"
#include "atprotocol/app/bsky/graph/appbskygraphgetblocks.h"
#include "atprotocol/app/bsky/graph/appbskygraphgetfollowers.h"
#include "atprotocol/app/bsky/graph/appbskygraphgetfollows.h"
#include "atprotocol/app/bsky/graph/appbskygraphgetlistmutes.h"
#include "atprotocol/app/bsky/graph/appbskygraphgetlistblocks.h"
#include "atprotocol/app/bsky/graph/appbskygraphgetmutes.h"
#include "extension/app/bsky/actor/appbskyactorputpreferencesex.h"
#include "extension/com/atproto/repo/comatprotorepocreaterecordex.h"
#include "extension/com/atproto/server/comatprotoserverrefreshsessionex.h"
#include "extension/com/atproto/repo/comatprotorepogetrecordex.h"
#include "extension/com/atproto/repo/comatprotorepoputrecordex.h"
#include "extension/com/atproto/server/comatprotoservercreatesessionex.h"
#include "atprotocol/lexicons_func_unknown.h"
#include "tools/opengraphprotocol.h"
#include "tools/configurablelabels.h"
#include "tools/labelerprovider.h"
#include "tools/labelprovider.h"
#include "tools/listitemscache.h"
#include "tools/pinnedpostcache.h"
#include "unittest_common.h"

using namespace AtProtocolType;
using namespace AtProtocolType::LexiconsTypeUnknown;

class atprotocol_test : public QObject
{
    Q_OBJECT

public:
    atprotocol_test();
    ~atprotocol_test();

private slots:
    void initTestCase();
    void cleanupTestCase();
    void test_ComAtprotoServerCreateSession();
    void test_ComAtprotoServerRefreshSession();
    void test_OpenGraphProtocol();
    void test_ogpDecodeHtml();
    void test_getTimeline();
    void test_ConfigurableLabels();
    void test_ConfigurableLabels_load();
    void test_ConfigurableLabels_copy();
    void test_ConfigurableLabels_save();
    void test_ConfigurableLabels_mutedword();
    void test_ConfigurableLabels_contains_mutedword();
    void test_LabelerProvider();
    void test_LabelProvider();
    void test_ComAtprotoRepoCreateRecord_post();
    void test_ComAtprotoRepoCreateRecord_threadgate();
    void test_ComAtprotoRepoCreateRecord_postgate();
    void test_AppBskyFeedGetFeedGenerator();
    void test_ServiceUrl();
    void test_ComAtprotoRepoGetRecord_profile();
    void test_ComAtprotoRepoPutRecord_profile();
    void test_ListItemsCache();
    void test_checkPartialMatchLanguage();

    void test_AppBskyActorSearchActorsTypeahead();
    void test_AppBskyFeedGetActorFeeds();
    void test_AppBskyFeedGetActorLikes();
    void test_AppBskyFeedGetAuthorFeed();
    void test_AppBskyFeedGetFeed();
    void test_AppBskyFeedGetFeedGenerators();
    void test_AppBskyGraphGetBlocks();
    void test_AppBskyGraphGetFollowers();
    void test_AppBskyGraphGetFollows();
    void test_AppBskyGraphGetListMutes();
    void test_AppBskyGraphGetListBlocks();
    void test_AppBskyGraphGetMutes();

    void test_AppBskyActorPutPreferencesEx();

    void test_PlcDirectory();
    void test_DirectoryPlcLogAudit();
    void test_PinnedPostCache();

    void test_atprotoProxyHttpHeader();

    void test_convertVideoThumb();
    void test_facets();
    void test_postText();

private:
    void test_putPreferences(const QString &path, const QByteArray &body);
    void test_putRecord(const QString &path, const QByteArray &body);
    void test_createRecord(const QString &path, const QByteArray &body);
    void updateCreatedAt(QJsonObject &json_obj);

    WebServer m_mockServer;
    quint16 m_listenPort;
    QString m_service;
    AtProtocolInterface::AccountData m_account;
};

atprotocol_test::atprotocol_test()
{
    QCoreApplication::setOrganizationName(QStringLiteral("relog"));
    QCoreApplication::setApplicationName(QStringLiteral("Hagoromo_unittest"));

    m_listenPort = m_mockServer.listen(QHostAddress::LocalHost, 0);
    m_service = QString("http://localhost:%1/response").arg(m_listenPort);

    connect(&m_mockServer, &WebServer::receivedPost,
            [=](const QHttpServerRequest &request, bool &result, QString &json) {
                if (request.url().path().endsWith("/app.bsky.actor.putPreferences")) {
                    test_putPreferences(request.url().path(), request.body());
                    json = "{}";
                    result = true;
                } else if (request.url().path().endsWith("/com.atproto.repo.putRecord")) {
                    test_putRecord(request.url().path(), request.body());
                    json = "{}";
                    result = true;
                } else if (request.url().path() == "/response/xrpc/com.atproto.repo.createRecord") {
                    json = "{ \"uri\": \"return uri\", \"cid\": \"return cid\" }";
                    result = true;
                } else if (request.url().path().endsWith("com.atproto.repo.createRecord")) {
                    test_createRecord(request.url().path(), request.body());
                    json = "{ \"uri\": \"return uri\", \"cid\": \"return cid\" }";
                    result = true;
                } else {
                    QFile *file = new QFile(":" + request.url().path());
                    if (file->open(QFile::ReadOnly)) {
                        json = file->readAll();
                    } else {
                        json = "{}";
                    }
                    result = true;
                }
            });
}

atprotocol_test::~atprotocol_test() { }

void atprotocol_test::initTestCase()
{
    QCOMPARE_NE(m_listenPort, 0);
    m_account.accessJwt = "dummy";

    m_account.did = "did:plc:ipj5qejfoqu6eukvt72uhyit";
    m_account.handle = "ioriayane.relog.tech";
    m_account.email = "iori.ayane@gmail.com";
    m_account.accessJwt = "hoge hoge accessJwt";
    m_account.refreshJwt = "hoge hoge refreshJwt";
    m_account.service = m_service;
}

void atprotocol_test::cleanupTestCase() { }

void atprotocol_test::test_ComAtprotoServerCreateSession()
{
    m_account.did.clear();
    m_account.handle.clear();
    m_account.email.clear();
    m_account.accessJwt.clear();
    m_account.refreshJwt.clear();
    m_account.service.clear();

    AtProtocolInterface::ComAtprotoServerCreateSessionEx session;
    session.setService(m_service);

    {
        QSignalSpy spy(&session, SIGNAL(finished(bool)));
        session.createSession("hoge", "fuga", QString(), false);
        spy.wait();
        QVERIFY2(spy.count() == 1, QString("spy.count()=%1").arg(spy.count()).toUtf8());
        QList<QVariant> arguments = spy.takeFirst();
        QVERIFY(arguments.at(0).toBool());
    }

    QCOMPARE(session.did(), "did:plc:ipj5qejfoqu6eukvt72uhyit");
    QCOMPARE(session.handle(), "ioriayane.relog.tech");
    QCOMPARE(session.email(), "iori.ayane@gmail.com");
    QCOMPARE(session.accessJwt(), "hoge hoge accessJwt");
    QCOMPARE(session.refreshJwt(), "hoge hoge refreshJwt");

    QCOMPARE(session.did(), session.account().did);
    QCOMPARE(session.handle(), session.account().handle);
    QCOMPARE(session.email(), session.account().email);
    QCOMPARE(session.accessJwt(), session.account().accessJwt);
    QCOMPARE(session.refreshJwt(), session.account().refreshJwt);

    // 後ろのテストで使うアカウント情報
    m_account = session.account();

    QCOMPARE(m_account.did, "did:plc:ipj5qejfoqu6eukvt72uhyit");
    QCOMPARE(m_account.handle, "ioriayane.relog.tech");
    QCOMPARE(m_account.email, "iori.ayane@gmail.com");
    QCOMPARE(m_account.accessJwt, "hoge hoge accessJwt");
    QCOMPARE(m_account.refreshJwt, "hoge hoge refreshJwt");

    session.setService(m_service + "/limit");

    {
        QSignalSpy spy(&session, SIGNAL(finished(bool)));
        session.createSession("hoge", "fuga", QString(), false);
        spy.wait();
        QVERIFY2(spy.count() == 1, QString("spy.count()=%1").arg(spy.count()).toUtf8());
        QList<QVariant> arguments = spy.takeFirst();
        QCOMPARE(arguments.at(0).toBool(), false);
    }

    QVERIFY2(session.errorCode() == "RateLimitExceeded", session.errorCode().toLocal8Bit());
    QVERIFY(session.errorMessage().contains("ratelimit-limit:30"));
    QVERIFY(session.errorMessage().contains("ratelimit-reset:2023/09/17 10:31:07"));
    QVERIFY(session.errorMessage().contains("ratelimit-remaining:10"));
    QVERIFY(session.errorMessage().contains("ratelimit-policy:30;w=300"));
    QVERIFY2(session.replyJson().trimmed()
                     == "{\"error\":\"RateLimitExceeded\",\"message\":\"Rate Limit Exceeded\"}",
             session.replyJson().toLocal8Bit());
}

void atprotocol_test::test_ComAtprotoServerRefreshSession()
{
    AtProtocolInterface::ComAtprotoServerRefreshSessionEx session;
    session.setService(m_service);
    session.setSession("did", "handle", "email", "access", "refresh");
    {
        QSignalSpy spy(&session, SIGNAL(finished(bool)));
        session.refreshSession();
        spy.wait();
        QVERIFY2(spy.count() == 1, QString("spy.count()=%1").arg(spy.count()).toUtf8());
        QList<QVariant> arguments = spy.takeFirst();
        QVERIFY(arguments.at(0).toBool());
    }

    QCOMPARE(session.did(), "did:plc:mqxsuw5b5rhpwo4lw6iwlid5");
    QCOMPARE(session.handle(), "ioriayane2.bsky.social");
    QCOMPARE(session.email(), "email");
    QCOMPARE(session.accessJwt(), "access jwt");
    QCOMPARE(session.refreshJwt(), "refresh jwt");

    QCOMPARE(session.did(), session.account().did);
    QCOMPARE(session.handle(), session.account().handle);
    QCOMPARE(session.email(), session.account().email);
    QCOMPARE(session.accessJwt(), session.account().accessJwt);
    QCOMPARE(session.refreshJwt(), session.account().refreshJwt);
}

void atprotocol_test::test_OpenGraphProtocol()
{
    OpenGraphProtocol ogp;

    {
        QSignalSpy spy(&ogp, SIGNAL(finished(bool)));
        ogp.getData(m_service + "/ogp/file1.html");
        spy.wait();
        QVERIFY2(spy.count() == 1, QString("spy.count()=%1").arg(spy.count()).toUtf8());

        QList<QVariant> arguments = spy.takeFirst();
        QVERIFY(arguments.at(0).toBool());

        QCOMPARE(ogp.uri(), "http://localhost/response/ogp/file1.html");
        QCOMPARE(ogp.title(), "file1 title");
        QCOMPARE(ogp.description(), "file1 description");
        QCOMPARE(ogp.thumb(), "http://localhost:%1/response/ogp/images/file1.jpg");
    }
    {
        QTemporaryFile temp_file;
        temp_file.open();
        ogp.setThumb(ogp.thumb().replace("%1", QString::number(m_listenPort)));

        QSignalSpy spy(&ogp, SIGNAL(finishedDownload(bool)));
        ogp.downloadThumb(temp_file.fileName());
        spy.wait();
        QVERIFY2(spy.count() == 1, QString("spy.count()=%1").arg(spy.count()).toUtf8());

        QList<QVariant> arguments = spy.takeFirst();
        QVERIFY(arguments.at(0).toBool());

        QVERIFY(QFile::exists(temp_file.fileName()));
        QImage img(temp_file.fileName());
        QVERIFY(!img.isNull());
        QCOMPARE(img.width(), 664);
        QCOMPARE(img.height(), 257);
    }

    {
        QSignalSpy spy(&ogp, SIGNAL(finished(bool)));
        ogp.getData(m_service + "/ogp/file2.html");
        spy.wait();
        QVERIFY2(spy.count() == 1, QString("spy.count()=%1").arg(spy.count()).toUtf8());

        QList<QVariant> arguments = spy.takeFirst();
        QVERIFY(arguments.at(0).toBool());

        QCOMPARE(ogp.uri(), "http://localhost/response/ogp/file2.html");
        QVERIFY2(ogp.title()
                         == QString("file2 ")
                                    .append(QChar(0x30bf))
                                    .append(QChar(0x30a4))
                                    .append(QChar(0x30c8))
                                    .append(QChar(0x30eb)),
                 ogp.title().toLocal8Bit());
        QVERIFY2(ogp.description() == QString("file2 ").append(QChar(0x8a73)).append(QChar(0x7d30)),
                 ogp.description().toLocal8Bit());
        QCOMPARE(ogp.thumb(), "http://localhost:%1/response/ogp/images/file2.gif");
    }
    {
        QTemporaryFile temp_file;
        temp_file.open();

        ogp.setThumb(ogp.thumb().replace("%1", QString::number(m_listenPort)));

        QSignalSpy spy(&ogp, SIGNAL(finishedDownload(bool)));
        ogp.downloadThumb(temp_file.fileName());
        spy.wait();
        QVERIFY2(spy.count() == 1, QString("spy.count()=%1").arg(spy.count()).toUtf8());

        QList<QVariant> arguments = spy.takeFirst();
        QVERIFY(arguments.at(0).toBool());

        QVERIFY(QFile::exists(temp_file.fileName()));
        QImage img(temp_file.fileName());
        QVERIFY(!img.isNull());
        QCOMPARE(img.width(), 664);
        QCOMPARE(img.height(), 257);
    }

    {
        QSignalSpy spy(&ogp, SIGNAL(finished(bool)));
        ogp.getData(m_service + "/ogp/file3.html");
        spy.wait();
        QVERIFY2(spy.count() == 1, QString("spy.count()=%1").arg(spy.count()).toUtf8());

        QList<QVariant> arguments = spy.takeFirst();
        QVERIFY(arguments.at(0).toBool());

        QCOMPARE(ogp.uri(), "http://localhost/response/ogp/file3.html");
        QVERIFY2(ogp.title()
                         == QString("file3 ")
                                    .append(QChar(0x30bf))
                                    .append(QChar(0x30a4))
                                    .append(QChar(0x30c8))
                                    .append(QChar(0x30eb)),
                 ogp.title().toLocal8Bit());
        QVERIFY2(ogp.description() == QString("file3 ").append(QChar(0x8a73)).append(QChar(0x7d30)),
                 ogp.description().toLocal8Bit());
        QCOMPARE(ogp.thumb(), "http://localhost:%1/response/ogp/images/file3.png");
    }
    {
        QTemporaryFile temp_file;
        temp_file.open();

        ogp.setThumb(ogp.thumb().replace("%1", QString::number(m_listenPort)));

        QSignalSpy spy(&ogp, SIGNAL(finishedDownload(bool)));
        ogp.downloadThumb(temp_file.fileName());
        spy.wait();
        QVERIFY2(spy.count() == 1, QString("spy.count()=%1").arg(spy.count()).toUtf8());

        QList<QVariant> arguments = spy.takeFirst();
        QVERIFY(arguments.at(0).toBool());

        QVERIFY(QFile::exists(temp_file.fileName()));
        QImage img(temp_file.fileName());
        QVERIFY(!img.isNull());
        QCOMPARE(img.width(), 664);
        QCOMPARE(img.height(), 257);
    }

    {
        QSignalSpy spy(&ogp, SIGNAL(finished(bool)));
        ogp.getData(m_service + "/ogp/file4.html");
        spy.wait();
        QVERIFY2(spy.count() == 1, QString("spy.count()=%1").arg(spy.count()).toUtf8());

        QList<QVariant> arguments = spy.takeFirst();
        QVERIFY(arguments.at(0).toBool());

        QVERIFY2(
                ogp.uri()
                        == QString("http://localhost:%1/response/ogp/file4.html").arg(m_listenPort),
                ogp.uri().toLocal8Bit());
        QVERIFY2(ogp.title()
                         == QString("file4 ")
                                    .append(QChar(0x30bf))
                                    .append(QChar(0x30a4))
                                    .append(QChar(0x30c8))
                                    .append(QChar(0x30eb)),
                 ogp.title().toLocal8Bit());
        QVERIFY2(ogp.description() == QString("file4 ").append(QChar(0x8a73)).append(QChar(0x7d30)),
                 ogp.description().toLocal8Bit());
        QCOMPARE(ogp.thumb(), "http://localhost:%1/response/ogp/images/file3.png");
    }
    {
        QTemporaryFile temp_file;
        temp_file.open();

        ogp.setThumb(ogp.thumb().replace("%1", QString::number(m_listenPort)));

        QSignalSpy spy(&ogp, SIGNAL(finishedDownload(bool)));
        ogp.downloadThumb(temp_file.fileName());
        spy.wait();
        QVERIFY2(spy.count() == 1, QString("spy.count()=%1").arg(spy.count()).toUtf8());

        QList<QVariant> arguments = spy.takeFirst();
        QVERIFY(arguments.at(0).toBool());

        QVERIFY(QFile::exists(temp_file.fileName()));
        QImage img(temp_file.fileName());
        QVERIFY(!img.isNull());
        QCOMPARE(img.width(), 664);
        QCOMPARE(img.height(), 257);
    }

    {
        QSignalSpy spy(&ogp, SIGNAL(finished(bool)));
        ogp.getData(m_service + "/ogp/file5.html");
        spy.wait();
        QVERIFY2(spy.count() == 1, QString("spy.count()=%1").arg(spy.count()).toUtf8());

        QList<QVariant> arguments = spy.takeFirst();
        QVERIFY(arguments.at(0).toBool());

        QVERIFY2(ogp.uri() == "http://localhost/response/ogp/file5.html?id=10186&s=720",
                 ogp.uri().toLocal8Bit());
        QVERIFY2(ogp.title()
                         == QString("file5 ")
                                    .append(QChar(0x30bf))
                                    .append(QChar(0x30a4))
                                    .append(QChar(0x30c8))
                                    .append(QChar(0x30eb)),
                 ogp.title().toLocal8Bit());
        QVERIFY2(ogp.description() == QString("file5 ").append(QChar(0x8a73)).append(QChar(0x7d30)),
                 ogp.description().toLocal8Bit());
        QCOMPARE(ogp.thumb(), "");
    }

    {
        QSignalSpy spy(&ogp, SIGNAL(finished(bool)));
        ogp.getData(m_service + "/ogp/file6.html");
        spy.wait();
        QVERIFY2(spy.count() == 1, QString("spy.count()=%1").arg(spy.count()).toUtf8());

        QList<QVariant> arguments = spy.takeFirst();
        QVERIFY(arguments.at(0).toBool());

        QVERIFY2(ogp.uri()
                         == QString("http://localhost:%1/response/ogp/file6.html")
                                    .arg(QString::number(m_listenPort)),
                 ogp.uri().toLocal8Bit());
        QVERIFY2(ogp.title() == QString("file6 TITLE"), ogp.title().toLocal8Bit());
        QVERIFY2(ogp.description() == QString("file6 ").append(QChar(0x8a73)).append(QChar(0x7d30)),
                 ogp.description().toLocal8Bit());
        QVERIFY2(ogp.thumb()
                         == QString("http://localhost:%1/response/ogp/images/file6.png")
                                    .arg(QString::number(m_listenPort)),
                 ogp.thumb().toLocal8Bit());
    }

    {
        QSignalSpy spy(&ogp, SIGNAL(finished(bool)));
        ogp.getData(m_service + "/ogp/file7.html");
        spy.wait();
        QVERIFY2(spy.count() == 1, QString("spy.count()=%1").arg(spy.count()).toUtf8());

        QList<QVariant> arguments = spy.takeFirst();
        QVERIFY(arguments.at(0).toBool());

        QVERIFY2(ogp.uri()
                         == QString("http://localhost:%1/response/ogp/file7.html")
                                    .arg(QString::number(m_listenPort)),
                 ogp.uri().toLocal8Bit());
        QVERIFY2(ogp.title() == QString("file7 TITLE"), ogp.title().toLocal8Bit());
        QVERIFY2(ogp.description() == QString("file7 ").append(QChar(0x8a73)).append(QChar(0x7d30)),
                 ogp.description().toLocal8Bit());
        QVERIFY2(ogp.thumb()
                         == QString("http://localhost:%1/response/ogp/images/file7.png")
                                    .arg(QString::number(m_listenPort)),
                 ogp.thumb().toLocal8Bit());
    }
}

void atprotocol_test::test_ogpDecodeHtml()
{
    QString actual;

    actual = OpenGraphProtocol::decodeHtml("hoge fuga");
    QVERIFY2(actual == "hoge fuga", actual.toLocal8Bit());

    actual = OpenGraphProtocol::decodeHtml("hoge&amp;fuga");
    QVERIFY2(actual == "hoge&fuga", actual.toLocal8Bit());

    actual = OpenGraphProtocol::decodeHtml("hoge fuga&amp;");
    QVERIFY2(actual == "hoge fuga&", actual.toLocal8Bit());

    actual = OpenGraphProtocol::decodeHtml("&amp;hoge fuga");
    QVERIFY2(actual == "&hoge fuga", actual.toLocal8Bit());

    actual = OpenGraphProtocol::decodeHtml("&amphoge fuga");
    QVERIFY2(actual == "&amphoge fuga", actual.toLocal8Bit());

    actual = OpenGraphProtocol::decodeHtml("&amp;hoge&lt;fuga&gt;&quot;foo&apos;&nbsp;bar");
    QVERIFY2(actual == "&hoge<fuga>\"foo' bar", actual.toLocal8Bit());
}

void atprotocol_test::test_getTimeline()
{
    AtProtocolInterface::AppBskyFeedGetTimeline timeline;
    timeline.setAccount(m_account);

    QSignalSpy spy(&timeline, SIGNAL(finished(bool)));
    timeline.getTimeline(QString(), 0, QString());
    spy.wait();
    QVERIFY2(spy.count() == 1, QString("spy.count()=%1").arg(spy.count()).toUtf8());

    QList<QVariant> arguments = spy.takeFirst();
    QVERIFY(arguments.at(0).toBool());

    QCOMPARE(timeline.feedList().count(), 6);

    QCOMPARE(timeline.feedList().at(0).post.author.did, "did:plc:mqxsuw5b5rhpwo4lw6iwlid5");

    QVERIFY2(timeline.feedList().at(1).post.embed_type
                     == AppBskyFeedDefs::PostViewEmbedType::embed_AppBskyEmbedRecordWithMedia_View,
             QString("%1")
                     .arg(static_cast<int>(timeline.feedList().at(1).post.embed_type))
                     .toLocal8Bit());

    QVERIFY2(timeline.feedList().at(1).post.embed_AppBskyEmbedRecordWithMedia_View.media_type
                     == AppBskyEmbedRecordWithMedia::ViewMediaType::media_AppBskyEmbedImages_View,
             QString("%1")
                     .arg(static_cast<int>(
                             timeline.feedList()
                                     .at(1)
                                     .post.embed_AppBskyEmbedRecordWithMedia_View.media_type))
                     .toLocal8Bit());
    QVERIFY2(timeline.feedList().at(1).post.embed_AppBskyEmbedRecordWithMedia_View.record.isNull()
                     == false,
             QString("%1")
                     .arg(timeline.feedList()
                                  .at(1)
                                  .post.embed_AppBskyEmbedRecordWithMedia_View.record.isNull())
                     .toLocal8Bit());
    QVERIFY2(timeline.feedList()
                             .at(1)
                             .post.embed_AppBskyEmbedRecordWithMedia_View.record->record_type
                     == AppBskyEmbedRecord::ViewRecordType::record_ViewRecord,
             QString("%1")
                     .arg(static_cast<int>(timeline.feedList()
                                                   .at(1)
                                                   .post.embed_AppBskyEmbedRecordWithMedia_View
                                                   .record->record_type))
                     .toLocal8Bit());
    QVERIFY2(timeline.feedList()
                             .at(1)
                             .post.embed_AppBskyEmbedRecordWithMedia_View.record->record_ViewRecord
                             .cid
                     == "bafyreig5ylsbfssvs45welz6o45gbs4rsvg3ek3oi6ygdrl76vchbahhpu",
             timeline.feedList()
                     .at(1)
                     .post.embed_AppBskyEmbedRecordWithMedia_View.record->record_ViewRecord.cid
                     .toLocal8Bit());
    QVERIFY2(timeline.feedList()
                             .at(1)
                             .post.embed_AppBskyEmbedRecordWithMedia_View.record->record_ViewRecord
                             .author.handle
                     == "ioriayane2.bsky.social",
             timeline.feedList()
                     .at(1)
                     .post.embed_AppBskyEmbedRecordWithMedia_View.record->record_ViewRecord.author
                     .handle.toLocal8Bit());

    QVERIFY(LexiconsTypeUnknown::fromQVariant<AppBskyFeedPost::Main>(
                    timeline.feedList()
                            .at(1)
                            .post.embed_AppBskyEmbedRecordWithMedia_View.record->record_ViewRecord
                            .value)
                    .text
            == "quoted post");

    QVERIFY(timeline.feedList()
                    .at(1)
                    .post.embed_AppBskyEmbedRecordWithMedia_View.media_AppBskyEmbedImages_View
                    .images.count()
            == 1);
    QVERIFY(timeline.feedList()
                    .at(1)
                    .post.embed_AppBskyEmbedRecordWithMedia_View.media_AppBskyEmbedImages_View
                    .images.at(0)
                    .thumb
            == "https://cdn.bsky.social/imgproxy/TpkmBHqqdpgUngZqPPE5p-KcoBPORIPEkFfjnmDIufo/"
               "rs:fit:1000:1000:1:0/plain/"
               "bafkreihcwn65qcgr2kq6xpknyhcek4xtiq6c2zum453ov5dtp6bqdt3g24@jpeg");
}

void atprotocol_test::test_ConfigurableLabels()
{
    ConfigurableLabels labels;
    bool for_image = false;
    labels.setEnableAdultContent(true);
    for (int i = 0; i < labels.count(); i++) {
        labels.setStatus(i, ConfigurableLabelStatus::Show);
    }
    for_image = true;
    QCOMPARE(labels.visibility("hoge", for_image), ConfigurableLabelStatus::Show);
    QCOMPARE(labels.visibility("porn", for_image), ConfigurableLabelStatus::Show);
    QCOMPARE(labels.visibility("sexual", for_image), ConfigurableLabelStatus::Show);
    QCOMPARE(labels.visibility("graphic-media", for_image), ConfigurableLabelStatus::Show);
    QCOMPARE(labels.visibility("nudity", for_image), ConfigurableLabelStatus::Show);

    labels.setEnableAdultContent(false);
    QCOMPARE(labels.visibility("hoge", for_image), ConfigurableLabelStatus::Show);
    QCOMPARE(labels.visibility("porn", for_image), ConfigurableLabelStatus::Hide);
    QCOMPARE(labels.visibility("sexual", for_image), ConfigurableLabelStatus::Hide);
    QCOMPARE(labels.visibility("graphic-media", for_image), ConfigurableLabelStatus::Hide);
    QCOMPARE(labels.visibility("nudity", for_image), ConfigurableLabelStatus::Show);

    labels.setEnableAdultContent(true);
    for (int i = 0; i < labels.count(); i++) {
        labels.setStatus(i, ConfigurableLabelStatus::Hide);
    }
    QCOMPARE(labels.visibility("porn", true), ConfigurableLabelStatus::Hide);
    QCOMPARE(labels.visibility("sexual", true), ConfigurableLabelStatus::Hide);
    QCOMPARE(labels.visibility("graphic-media", true), ConfigurableLabelStatus::Hide);
    QCOMPARE(labels.visibility("nudity", true), ConfigurableLabelStatus::Hide);

    QCOMPARE(labels.message("porn", true), "Sexually Explicit");
    QCOMPARE(labels.message("sexual", true), "Sexually Suggestive");
    QCOMPARE(labels.message("graphic-media", true), "Graphic Media");
    QCOMPARE(labels.message("nudity", true), "Non-sexual Nudity");

    labels.setEnableAdultContent(true);
    for (int i = 0; i < labels.count(); i++) {
        labels.setStatus(i, ConfigurableLabelStatus::Warning);
    }
    QCOMPARE(labels.visibility("porn", true), ConfigurableLabelStatus::Warning);
    QCOMPARE(labels.visibility("sexual", true), ConfigurableLabelStatus::Warning);
    QCOMPARE(labels.visibility("graphic-media", true), ConfigurableLabelStatus::Warning);
    QCOMPARE(labels.visibility("nudity", true), ConfigurableLabelStatus::Warning);

    QCOMPARE(labels.message("porn", true), "Sexually Explicit");
    QCOMPARE(labels.message("sexual", true), "Sexually Suggestive");
    QCOMPARE(labels.message("graphic-media", true), "Graphic Media");
    QCOMPARE(labels.message("nudity", true), "Non-sexual Nudity");

    // 設定変更できない項目を変更していないか確認
    for (int i = 0; i < labels.count(); i++) {
        labels.setStatus(i, ConfigurableLabelStatus::Show);
    }
    QCOMPARE(labels.visibility("!hide", false), ConfigurableLabelStatus::Hide);
    QCOMPARE(labels.visibility("doxxing", false), ConfigurableLabelStatus::Hide);
    QCOMPARE(labels.visibility("!warn", false), ConfigurableLabelStatus::Warning);
    QCOMPARE(labels.visibility("!warn", true), ConfigurableLabelStatus::Show);
    QCOMPARE(labels.message("!warn", false), "Content warning");
    QCOMPARE(labels.message("!warn", true), QString());

    QCOMPARE(labels.visibility("dmca-violation", false), ConfigurableLabelStatus::Hide);
    QCOMPARE(labels.visibility("doxxing", false), ConfigurableLabelStatus::Hide);
    QCOMPARE(labels.visibility("gore", true), ConfigurableLabelStatus::Hide);
    QCOMPARE(labels.visibility("self-harm", true), ConfigurableLabelStatus::Hide);
    QCOMPARE(labels.visibility("torture", true), ConfigurableLabelStatus::Hide);
    QCOMPARE(labels.visibility("nsfl", true), ConfigurableLabelStatus::Hide);
    QCOMPARE(labels.visibility("corpse", true), ConfigurableLabelStatus::Hide);
    QCOMPARE(labels.visibility("icon-kkk", false), ConfigurableLabelStatus::Hide);
    QCOMPARE(labels.visibility("icon-nazi", false), ConfigurableLabelStatus::Hide);
    QCOMPARE(labels.visibility("icon-intolerant", false), ConfigurableLabelStatus::Hide);
    QCOMPARE(labels.visibility("behavior-intolerant", false), ConfigurableLabelStatus::Hide);
    QCOMPARE(labels.visibility("spam", false), ConfigurableLabelStatus::Hide);
    QCOMPARE(labels.visibility("impersonation", false), ConfigurableLabelStatus::Hide);

    QVERIFY(labels.indexOf("sexual", "") >= 0);
    QCOMPARE(labels.indexOf("sexual", "did:plc:hoge"), -1);
    QVERIFY(labels.indexOf("sexual", "unkown") >= 0);
}

void atprotocol_test::test_ConfigurableLabels_load()
{
    QString labeler_did;
    ConfigurableLabels labels;
    labels.setAccount(m_account);
    labels.setRefreshLabelers(true);
    {
        labels.setService(QString("http://localhost:%1/response/labels/hide").arg(m_listenPort));
        QSignalSpy spy(&labels, SIGNAL(finished(bool)));
        labels.load();
        spy.wait();
        QVERIFY2(spy.count() == 1, QString("spy.count()=%1").arg(spy.count()).toUtf8());
        QList<QVariant> arguments = spy.takeFirst();
        QVERIFY(arguments.at(0).toBool());

        QCOMPARE(labels.enableAdultContent(), true);
        QCOMPARE(labels.visibility("porn", true), ConfigurableLabelStatus::Hide);
        QCOMPARE(labels.visibility("sexual", true), ConfigurableLabelStatus::Hide);
        QCOMPARE(labels.visibility("graphic-media", true), ConfigurableLabelStatus::Hide);
        QCOMPARE(labels.visibility("nudity", true), ConfigurableLabelStatus::Hide);

        QCOMPARE(labels.visibility("nsfl", true), ConfigurableLabelStatus::Hide);
        QCOMPARE(labels.visibility("gore", true), ConfigurableLabelStatus::Hide);
        QCOMPARE(labels.visibility("self-harm", true), ConfigurableLabelStatus::Hide);
        QCOMPARE(labels.visibility("torture", true), ConfigurableLabelStatus::Hide);
        QCOMPARE(labels.visibility("corpse", true), ConfigurableLabelStatus::Hide);
        QCOMPARE(labels.visibility("icon-kkk", false), ConfigurableLabelStatus::Hide);
        QCOMPARE(labels.visibility("icon-nazi", false), ConfigurableLabelStatus::Hide);
        QCOMPARE(labels.visibility("icon-intolerant", false), ConfigurableLabelStatus::Hide);
        QCOMPARE(labels.visibility("behavior-intolerant", false), ConfigurableLabelStatus::Hide);
        QCOMPARE(labels.visibility("spam", false), ConfigurableLabelStatus::Hide);
        QCOMPARE(labels.visibility("impersonation", false), ConfigurableLabelStatus::Hide);

        labeler_did = "did:plc:ar7c4by46qjdydhdevvrndac";
        QVERIFY(labels.visibility("inauthentic", false, labeler_did)
                == ConfigurableLabelStatus::Hide);
        QVERIFY(labels.visibility("misleading", false, labeler_did)
                == ConfigurableLabelStatus::Hide);
        QCOMPARE(labels.visibility("rumor", false, labeler_did), ConfigurableLabelStatus::Hide);
        // 以下デフォルト設定
        QCOMPARE(labels.visibility("spam", false, labeler_did), ConfigurableLabelStatus::Hide);
        QVERIFY(labels.visibility("self-harm", false, labeler_did)
                == ConfigurableLabelStatus::Warning);
        QVERIFY(labels.visibility("sexual-figurative", false, labeler_did)
                == ConfigurableLabelStatus::Show);

        QCOMPARE(labels.hasAdultOnly(""), true);
        QCOMPARE(labels.hasAdultOnly("did:plc:ar7c4by46qjdydhdevvrndac"), true);
    }
    //
    {
        labels.setService(QString("http://localhost:%1/response/labels/show").arg(m_listenPort));
        QSignalSpy spy(&labels, SIGNAL(finished(bool)));
        labels.load();
        spy.wait();
        QVERIFY2(spy.count() == 1, QString("spy.count()=%1").arg(spy.count()).toUtf8());
        QList<QVariant> arguments = spy.takeFirst();
        QVERIFY(arguments.at(0).toBool());

        QCOMPARE(labels.enableAdultContent(), true);
        QCOMPARE(labels.visibility("porn", true), ConfigurableLabelStatus::Show);
        QCOMPARE(labels.visibility("sexual", true), ConfigurableLabelStatus::Show);
        QCOMPARE(labels.visibility("graphic-media", true), ConfigurableLabelStatus::Show);
        QCOMPARE(labels.visibility("nudity", true), ConfigurableLabelStatus::Show);

        QCOMPARE(labels.visibility("nsfl", true), ConfigurableLabelStatus::Hide);
        QCOMPARE(labels.visibility("gore", true), ConfigurableLabelStatus::Hide);
        QCOMPARE(labels.visibility("self-harm", true), ConfigurableLabelStatus::Hide);
        QCOMPARE(labels.visibility("torture", true), ConfigurableLabelStatus::Hide);
        QCOMPARE(labels.visibility("corpse", true), ConfigurableLabelStatus::Hide);
        QCOMPARE(labels.visibility("icon-kkk", false), ConfigurableLabelStatus::Hide);
        QCOMPARE(labels.visibility("icon-nazi", false), ConfigurableLabelStatus::Hide);
        QCOMPARE(labels.visibility("icon-intolerant", false), ConfigurableLabelStatus::Hide);
        QCOMPARE(labels.visibility("behavior-intolerant", false), ConfigurableLabelStatus::Hide);
        QCOMPARE(labels.visibility("spam", false), ConfigurableLabelStatus::Hide);
        QCOMPARE(labels.visibility("impersonation", false), ConfigurableLabelStatus::Hide);

        labeler_did = "did:plc:ar7c4by46qjdydhdevvrndac";
        QVERIFY(labels.visibility("inauthentic", false, labeler_did)
                == ConfigurableLabelStatus::Show);
        QVERIFY(labels.visibility("misleading", false, labeler_did)
                == ConfigurableLabelStatus::Show);
        QCOMPARE(labels.visibility("rumor", false, labeler_did), ConfigurableLabelStatus::Show);

        QCOMPARE(labels.hasAdultOnly(""), true);
        QCOMPARE(labels.hasAdultOnly("did:plc:ar7c4by46qjdydhdevvrndac"), false);
    }
    //
    {
        labels.setService(
                QString("http://localhost:%1/response/labels/show_adult_false").arg(m_listenPort));
        QSignalSpy spy(&labels, SIGNAL(finished(bool)));
        labels.load();
        spy.wait();
        QVERIFY2(spy.count() == 1, QString("spy.count()=%1").arg(spy.count()).toUtf8());
        QList<QVariant> arguments = spy.takeFirst();
        QVERIFY(arguments.at(0).toBool());

        QCOMPARE(labels.enableAdultContent(), false);
        QCOMPARE(labels.visibility("porn", true), ConfigurableLabelStatus::Hide);
        QCOMPARE(labels.visibility("sexual", true), ConfigurableLabelStatus::Hide);
        QCOMPARE(labels.visibility("graphic-media", true), ConfigurableLabelStatus::Hide);
        QCOMPARE(labels.visibility("nudity", true), ConfigurableLabelStatus::Show);

        QCOMPARE(labels.visibility("nsfl", true), ConfigurableLabelStatus::Hide);
        QCOMPARE(labels.visibility("gore", true), ConfigurableLabelStatus::Hide);
        QCOMPARE(labels.visibility("self-harm", true), ConfigurableLabelStatus::Hide);
        QCOMPARE(labels.visibility("torture", true), ConfigurableLabelStatus::Hide);
        QCOMPARE(labels.visibility("corpse", true), ConfigurableLabelStatus::Hide);
        QCOMPARE(labels.visibility("icon-kkk", false), ConfigurableLabelStatus::Hide);
        QCOMPARE(labels.visibility("icon-nazi", false), ConfigurableLabelStatus::Hide);
        QCOMPARE(labels.visibility("icon-intolerant", false), ConfigurableLabelStatus::Hide);
        QCOMPARE(labels.visibility("behavior-intolerant", false), ConfigurableLabelStatus::Hide);
        QCOMPARE(labels.visibility("spam", false), ConfigurableLabelStatus::Hide);
        QCOMPARE(labels.visibility("impersonation", false), ConfigurableLabelStatus::Hide);

        labeler_did = "did:plc:ar7c4by46qjdydhdevvrndac";
        QVERIFY(labels.visibility("inauthentic", false, labeler_did)
                == ConfigurableLabelStatus::Show);
        QVERIFY(labels.visibility("misleading", false, labeler_did)
                == ConfigurableLabelStatus::Show);
        QCOMPARE(labels.visibility("rumor", false, labeler_did), ConfigurableLabelStatus::Show);
    }
    //
    {
        labels.setService(QString("http://localhost:%1/response/labels/warn").arg(m_listenPort));
        QSignalSpy spy(&labels, SIGNAL(finished(bool)));
        labels.load();
        spy.wait();
        QVERIFY2(spy.count() == 1, QString("spy.count()=%1").arg(spy.count()).toUtf8());
        QList<QVariant> arguments = spy.takeFirst();
        QVERIFY(arguments.at(0).toBool());

        QCOMPARE(labels.enableAdultContent(), true);
        QCOMPARE(labels.visibility("porn", true), ConfigurableLabelStatus::Warning);
        QCOMPARE(labels.visibility("sexual", true), ConfigurableLabelStatus::Warning);
        QCOMPARE(labels.visibility("graphic-media", true), ConfigurableLabelStatus::Warning);
        QCOMPARE(labels.visibility("nudity", true), ConfigurableLabelStatus::Warning);

        QCOMPARE(labels.visibility("nsfl", true), ConfigurableLabelStatus::Hide);
        QCOMPARE(labels.visibility("gore", true), ConfigurableLabelStatus::Hide);
        QCOMPARE(labels.visibility("self-harm", true), ConfigurableLabelStatus::Hide);
        QCOMPARE(labels.visibility("torture", true), ConfigurableLabelStatus::Hide);
        QCOMPARE(labels.visibility("corpse", true), ConfigurableLabelStatus::Hide);
        QCOMPARE(labels.visibility("icon-kkk", false), ConfigurableLabelStatus::Hide);
        QCOMPARE(labels.visibility("icon-nazi", false), ConfigurableLabelStatus::Hide);
        QCOMPARE(labels.visibility("icon-intolerant", false), ConfigurableLabelStatus::Hide);
        QCOMPARE(labels.visibility("behavior-intolerant", false), ConfigurableLabelStatus::Hide);
        QCOMPARE(labels.visibility("spam", false), ConfigurableLabelStatus::Hide);
        QCOMPARE(labels.visibility("impersonation", false), ConfigurableLabelStatus::Hide);

        labeler_did = "did:plc:ar7c4by46qjdydhdevvrndac";
        QVERIFY(labels.visibility("inauthentic", false, labeler_did)
                == ConfigurableLabelStatus::Warning);
        QVERIFY(labels.visibility("misleading", false, labeler_did)
                == ConfigurableLabelStatus::Warning);
        QCOMPARE(labels.visibility("rumor", false, labeler_did), ConfigurableLabelStatus::Warning);
    }
    //
    {
        int i = 0;
        labels.setService(
                QString("http://localhost:%1/response/labels/mutedword/1").arg(m_listenPort));
        QSignalSpy spy(&labels, SIGNAL(finished(bool)));
        labels.load();
        spy.wait();
        QVERIFY2(spy.count() == 1, QString("spy.count()=%1").arg(spy.count()).toUtf8());
        QList<QVariant> arguments = spy.takeFirst();
        QVERIFY(arguments.at(0).toBool());

        QVERIFY2(labels.mutedWordCount() == 4,
                 QString::number(labels.mutedWordCount()).toLocal8Bit());

        i = 0;
        QVERIFY2(labels.getMutedWordItem(i).group == 0,
                 QString("%1").arg(labels.getMutedWordItem(i).group).toLocal8Bit());
        QVERIFY2(labels.getMutedWordItem(i).value == "test1-1",
                 QString("%1").arg(labels.getMutedWordItem(i).value).toLocal8Bit());
        QVERIFY2(labels.getMutedWordItem(i).targets.length() == 2,
                 QString("%1").arg(labels.getMutedWordItem(i).targets.length()).toLocal8Bit());
        QVERIFY2(labels.getMutedWordItem(i).targets
                         == QList<MutedWordTarget>()
                                 << MutedWordTarget::Content << MutedWordTarget::Tag,
                 QString("%1,%2")
                         .arg(int(labels.getMutedWordItem(i).targets.at(0)),
                              int(labels.getMutedWordItem(i).targets.at(1)))
                         .toLocal8Bit());
        i = 1;
        QVERIFY2(labels.getMutedWordItem(i).group == 0,
                 QString("%1").arg(labels.getMutedWordItem(i).group).toLocal8Bit());
        QVERIFY2(labels.getMutedWordItem(i).value == "test1-2",
                 QString("%1").arg(labels.getMutedWordItem(i).value).toLocal8Bit());
        QVERIFY2(labels.getMutedWordItem(i).targets.length() == 1,
                 QString("%1").arg(labels.getMutedWordItem(i).targets.length()).toLocal8Bit());
        QVERIFY2(labels.getMutedWordItem(i).targets
                         == QList<MutedWordTarget>() << MutedWordTarget::Tag,
                 QString("%1").arg(int(labels.getMutedWordItem(i).targets.at(0))).toLocal8Bit());
        i = 2;
        QVERIFY2(labels.getMutedWordItem(i).group == 0,
                 QString("%1").arg(labels.getMutedWordItem(i).group).toLocal8Bit());
        QVERIFY2(labels.getMutedWordItem(i).value == "test1-3",
                 QString("%1").arg(labels.getMutedWordItem(i).value).toLocal8Bit());
        QVERIFY2(labels.getMutedWordItem(i).targets.length() == 0,
                 QString("%1").arg(labels.getMutedWordItem(i).targets.length()).toLocal8Bit());
        QCOMPARE(labels.getMutedWordItem(i).targets, QList<MutedWordTarget>());
        i = 3;
        QVERIFY2(labels.getMutedWordItem(i).group == 0,
                 QString("%1").arg(labels.getMutedWordItem(i).group).toLocal8Bit());
        QVERIFY2(labels.getMutedWordItem(i).value == "test1-4",
                 QString("%1").arg(labels.getMutedWordItem(i).value).toLocal8Bit());
        QVERIFY2(labels.getMutedWordItem(i).targets.length() == 1,
                 QString("%1").arg(labels.getMutedWordItem(i).targets.length()).toLocal8Bit());
        QVERIFY2(labels.getMutedWordItem(i).targets
                         == QList<MutedWordTarget>() << MutedWordTarget::Content,
                 QString("%1").arg(int(labels.getMutedWordItem(i).targets.at(0))).toLocal8Bit());
    }
    //
    {
        int i = 0;
        labels.setService(
                QString("http://localhost:%1/response/labels/mutedword/2").arg(m_listenPort));
        QSignalSpy spy(&labels, SIGNAL(finished(bool)));
        labels.load();
        spy.wait();
        QVERIFY2(spy.count() == 1, QString("spy.count()=%1").arg(spy.count()).toUtf8());
        QList<QVariant> arguments = spy.takeFirst();
        QVERIFY(arguments.at(0).toBool());

        QVERIFY2(labels.mutedWordCount() == 8,
                 QString::number(labels.mutedWordCount()).toLocal8Bit());

        i = 0;
        QCOMPARE(labels.getMutedWordItem(i).group, 0);
        QCOMPARE(labels.getMutedWordItem(i).value, "test1-1");
        QCOMPARE(labels.getMutedWordItem(i).targets.length(), 2);
        QVERIFY(labels.getMutedWordItem(i).targets
                == QList<MutedWordTarget>() << MutedWordTarget::Content << MutedWordTarget::Tag);
        i = 1;
        QCOMPARE(labels.getMutedWordItem(i).group, 0);
        QCOMPARE(labels.getMutedWordItem(i).value, "test1-2");
        QCOMPARE(labels.getMutedWordItem(i).targets.length(), 1);
        QVERIFY(labels.getMutedWordItem(i).targets
                == QList<MutedWordTarget>() << MutedWordTarget::Tag);
        i = 2;
        QCOMPARE(labels.getMutedWordItem(i).group, 0);
        QCOMPARE(labels.getMutedWordItem(i).value, "test1-3");
        QCOMPARE(labels.getMutedWordItem(i).targets.length(), 0);
        QCOMPARE(labels.getMutedWordItem(i).targets, QList<MutedWordTarget>());
        i = 3;
        QCOMPARE(labels.getMutedWordItem(i).group, 0);
        QCOMPARE(labels.getMutedWordItem(i).value, "test1-4");
        QCOMPARE(labels.getMutedWordItem(i).targets.length(), 1);
        QVERIFY(labels.getMutedWordItem(i).targets
                == QList<MutedWordTarget>() << MutedWordTarget::Content);

        i = 4;
        QCOMPARE(labels.getMutedWordItem(i).group, 1);
        QCOMPARE(labels.getMutedWordItem(i).value, "test2-1");
        QCOMPARE(labels.getMutedWordItem(i).targets.length(), 1);
        QVERIFY(labels.getMutedWordItem(i).targets
                == QList<MutedWordTarget>() << MutedWordTarget::Content);
        i = 5;
        QCOMPARE(labels.getMutedWordItem(i).group, 1);
        QCOMPARE(labels.getMutedWordItem(i).value, "test2-2");
        QCOMPARE(labels.getMutedWordItem(i).targets.length(), 2);
        QVERIFY(labels.getMutedWordItem(i).targets
                == QList<MutedWordTarget>() << MutedWordTarget::Content << MutedWordTarget::Tag);
        i = 6;
        QCOMPARE(labels.getMutedWordItem(i).group, 1);
        QCOMPARE(labels.getMutedWordItem(i).value, "test2-3");
        QCOMPARE(labels.getMutedWordItem(i).targets.length(), 1);
        QVERIFY(labels.getMutedWordItem(i).targets
                == QList<MutedWordTarget>() << MutedWordTarget::Tag);
        i = 7;
        QCOMPARE(labels.getMutedWordItem(i).group, 1);
        QCOMPARE(labels.getMutedWordItem(i).value, "test2-4");
        QCOMPARE(labels.getMutedWordItem(i).targets.length(), 0);
        QCOMPARE(labels.getMutedWordItem(i).targets, QList<MutedWordTarget>());

        // ------------
        labeler_did = "";
        QCOMPARE(labels.count(labeler_did), 11);
        QCOMPARE(labels.hasAdultOnly(labeler_did), true);
        i = 0;
        QCOMPARE(labels.title(i, labeler_did), "Content hidden");
        QCOMPARE(labels.description(i, labeler_did), "Moderator overrides for special cases.");
        QCOMPARE(labels.isAdultImagery(i, labeler_did), false);
        QCOMPARE(labels.foldableRange(i, labeler_did), ConfigurableLabelFoldableRange::Content);
        QCOMPARE(labels.status(i, labeler_did), ConfigurableLabelStatus::Hide);
        QCOMPARE(labels.configurable(i, labeler_did), false);
        //
        i = 1;
        QCOMPARE(labels.title(i, labeler_did), "Content warning");
        QCOMPARE(labels.description(i, labeler_did), "Moderator overrides for special cases.");
        QCOMPARE(labels.isAdultImagery(i, labeler_did), false);
        QCOMPARE(labels.foldableRange(i, labeler_did), ConfigurableLabelFoldableRange::Content);
        QCOMPARE(labels.status(i, labeler_did), ConfigurableLabelStatus::Warning);
        QCOMPARE(labels.configurable(i, labeler_did), false);
        //
        i = 2;
        QCOMPARE(labels.title(i, labeler_did), "Pornography");
        QCOMPARE(labels.description(i, labeler_did), "Explicit sexual images.");
        QCOMPARE(labels.isAdultImagery(i, labeler_did), true);
        QCOMPARE(labels.foldableRange(i, labeler_did), ConfigurableLabelFoldableRange::Media);
        QCOMPARE(labels.status(i, labeler_did), ConfigurableLabelStatus::Hide);
        QCOMPARE(labels.configurable(i, labeler_did), true);
        //
        i = 3;
        QCOMPARE(labels.title(i, labeler_did), "Sexually Suggestive");
        QCOMPARE(labels.description(i, labeler_did), "Does not include nudity");
        QCOMPARE(labels.isAdultImagery(i, labeler_did), true);
        QCOMPARE(labels.foldableRange(i, labeler_did), ConfigurableLabelFoldableRange::Media);
        QCOMPARE(labels.status(i, labeler_did), ConfigurableLabelStatus::Warning);
        QCOMPARE(labels.configurable(i, labeler_did), true);
        //
        i = 4;
        QCOMPARE(labels.title(i, labeler_did), "Graphic Media");
        QCOMPARE(labels.description(i, labeler_did), "Explicit or potentially disturbing media.");
        QCOMPARE(labels.isAdultImagery(i, labeler_did), true);
        QCOMPARE(labels.foldableRange(i, labeler_did), ConfigurableLabelFoldableRange::Media);
        QCOMPARE(labels.status(i, labeler_did), ConfigurableLabelStatus::Warning);
        QCOMPARE(labels.configurable(i, labeler_did), true);
        //
        i = 5;
        QCOMPARE(labels.title(i, labeler_did), "Non-sexual Nudity");
        QCOMPARE(labels.description(i, labeler_did), "E.g. artistic nudes.");
        QCOMPARE(labels.isAdultImagery(i, labeler_did), false);
        QCOMPARE(labels.foldableRange(i, labeler_did), ConfigurableLabelFoldableRange::Media);
        QCOMPARE(labels.status(i, labeler_did), ConfigurableLabelStatus::Show);
        QCOMPARE(labels.configurable(i, labeler_did), true);
        //
        i = 6;
        QCOMPARE(labels.title(i, labeler_did), "Legal");
        QCOMPARE(labels.description(i, labeler_did), "Content removed for legal reasons.");
        QCOMPARE(labels.isAdultImagery(i, labeler_did), false);
        QCOMPARE(labels.foldableRange(i, labeler_did), ConfigurableLabelFoldableRange::Content);
        QCOMPARE(labels.status(i, labeler_did), ConfigurableLabelStatus::Hide);
        QCOMPARE(labels.configurable(i, labeler_did), false);
        //
        i = 7;
        QCOMPARE(labels.title(i, labeler_did), "Violent / Bloody");
        QCOMPARE(labels.description(i, labeler_did), "Gore, self-harm, torture");
        QCOMPARE(labels.isAdultImagery(i, labeler_did), true);
        QCOMPARE(labels.foldableRange(i, labeler_did), ConfigurableLabelFoldableRange::Media);
        QCOMPARE(labels.status(i, labeler_did), ConfigurableLabelStatus::Hide);
        QCOMPARE(labels.configurable(i, labeler_did), false);
        //
        i = 8;
        QCOMPARE(labels.title(i, labeler_did), "Hate Group Iconography");
        QVERIFY(labels.description(i, labeler_did)
                == "Images of terror groups, articles covering events, etc.");
        QCOMPARE(labels.isAdultImagery(i, labeler_did), false);
        QCOMPARE(labels.foldableRange(i, labeler_did), ConfigurableLabelFoldableRange::Content);
        QCOMPARE(labels.status(i, labeler_did), ConfigurableLabelStatus::Hide);
        QCOMPARE(labels.configurable(i, labeler_did), false);
        //
        i = 9;
        QCOMPARE(labels.title(i, labeler_did), "Spam");
        QCOMPARE(labels.description(i, labeler_did), "Excessive unwanted interactions");
        QCOMPARE(labels.isAdultImagery(i, labeler_did), false);
        QCOMPARE(labels.foldableRange(i, labeler_did), ConfigurableLabelFoldableRange::Content);
        QCOMPARE(labels.status(i, labeler_did), ConfigurableLabelStatus::Hide);
        QCOMPARE(labels.configurable(i, labeler_did), false);
        //
        i = 10;
        QCOMPARE(labels.title(i, labeler_did), "Impersonation / Scam");
        QVERIFY(labels.description(i, labeler_did)
                == "Accounts falsely claiming to be people or orgs");
        QCOMPARE(labels.isAdultImagery(i, labeler_did), false);
        QCOMPARE(labels.foldableRange(i, labeler_did), ConfigurableLabelFoldableRange::Content);
        QCOMPARE(labels.status(i, labeler_did), ConfigurableLabelStatus::Hide);
        QCOMPARE(labels.configurable(i, labeler_did), false);

        // ------------
        labeler_did = "did:plc:ar7c4by46qjdydhdevvrndac";
        QCOMPARE(labels.count(labeler_did), 18);
        QCOMPARE(labels.hasAdultOnly(labeler_did), true);
        i = 0;
        QCOMPARE(labels.title(i, labeler_did), "Spam");
        QVERIFY(labels.description(i, labeler_did)
                == "Unwanted, repeated, or unrelated actions that bother users.");
        QCOMPARE(labels.isAdultImagery(i, labeler_did), false);
        QCOMPARE(labels.foldableRange(i, labeler_did), ConfigurableLabelFoldableRange::Content);
        QCOMPARE(labels.status(i, labeler_did), ConfigurableLabelStatus::Hide);
        QCOMPARE(labels.configurable(i, labeler_did), true);
        //
        i = 3;
        QCOMPARE(labels.title(i, labeler_did), "Intolerance");
        QCOMPARE(labels.description(i, labeler_did), "Discrimination against protected groups.");
        QCOMPARE(labels.isAdultImagery(i, labeler_did), false);
        QCOMPARE(labels.foldableRange(i, labeler_did), ConfigurableLabelFoldableRange::Content);
        QCOMPARE(labels.status(i, labeler_did), ConfigurableLabelStatus::Warning);
        QCOMPARE(labels.configurable(i, labeler_did), true);
        //
        i = 17;
        QCOMPARE(labels.title(i, labeler_did), "Sexually Suggestive (Cartoon)");
        QVERIFY(labels.description(i, labeler_did)
                == "Art with explicit or suggestive sexual themes, including provocative imagery "
                   "or partial nudity.");
        QCOMPARE(labels.isAdultImagery(i, labeler_did), true);
        QCOMPARE(labels.foldableRange(i, labeler_did), ConfigurableLabelFoldableRange::Media);
        QCOMPARE(labels.status(i, labeler_did), ConfigurableLabelStatus::Show);
        QCOMPARE(labels.configurable(i, labeler_did), true);

        // ------------
        labeler_did = "did:plc:original_labeler_did";
        QCOMPARE(labels.count(labeler_did), 1);
        QCOMPARE(labels.hasAdultOnly(labeler_did), false);
    }

    {
        // loadしないとラベラー情報無いのでコピーテスト
        ConfigurableLabels dest;
        QString did = "did:plc:ar7c4by46qjdydhdevvrndac";
        QCOMPARE(dest.targetLabelerCount(), 0);
        QCOMPARE(dest.labelerCount(), 1);
        dest = labels;
        QCOMPARE(dest.targetLabelerCount(), 2);
        QCOMPARE(dest.targetLabelerDid(0), "did:plc:ar7c4by46qjdydhdevvrndac");
        QCOMPARE(dest.targetLabelerDid(1), "did:plc:original_labeler_did");
        QCOMPARE(dest.labelerCount(), 3);
        QVERIFY(dest.labelerDids()
                == QStringList() << "__globally__"
                                 << "did:plc:ar7c4by46qjdydhdevvrndac"
                                 << "did:plc:original_labeler_did");
        QCOMPARE(dest.labelerHandle(did), "moderation.bsky.app");
        QVERIFY(dest.labelerDescription(did)
                == "Official Bluesky Moderation Service. Required part of the Bluesky Social app.");
        QCOMPARE(dest.labelerDisplayName(did), "Bluesky Moderation Service");
    }
}

void atprotocol_test::test_ConfigurableLabels_copy()
{
    ConfigurableLabels src, dest;
    src.setEnableAdultContent(true);
    for (int i = 0; i < src.count(); i++) {
        src.setStatus(i, ConfigurableLabelStatus::Show);
    }
    src.insertMutedWord(0, "word3", QList<MutedWordTarget>() << MutedWordTarget::Content);
    src.insertMutedWord(0, "word1", QList<MutedWordTarget>() << MutedWordTarget::Tag);
    src.insertMutedWord(1, "word2",
                        QList<MutedWordTarget>()
                                << MutedWordTarget::Content << MutedWordTarget::Tag);

    dest.setEnableAdultContent(false);
    for (int i = 0; i < dest.count(); i++) {
        dest.setStatus(i, ConfigurableLabelStatus::Hide);
    }

    QCOMPARE(dest.enableAdultContent(), false);
    QCOMPARE(dest.visibility("porn", true), ConfigurableLabelStatus::Hide);
    QCOMPARE(dest.visibility("sexual", true), ConfigurableLabelStatus::Hide);
    QCOMPARE(dest.visibility("graphic-media", true), ConfigurableLabelStatus::Hide);
    QCOMPARE(dest.visibility("nudity", true), ConfigurableLabelStatus::Hide);

    QCOMPARE(dest.visibility("nsfl", true), ConfigurableLabelStatus::Hide);
    QCOMPARE(dest.visibility("gore", true), ConfigurableLabelStatus::Hide);
    QCOMPARE(dest.visibility("self-harm", true), ConfigurableLabelStatus::Hide);
    QCOMPARE(dest.visibility("torture", true), ConfigurableLabelStatus::Hide);
    QCOMPARE(dest.visibility("corpse", true), ConfigurableLabelStatus::Hide);
    QCOMPARE(dest.visibility("icon-kkk", false), ConfigurableLabelStatus::Hide);
    QCOMPARE(dest.visibility("icon-nazi", false), ConfigurableLabelStatus::Hide);
    QCOMPARE(dest.visibility("icon-intolerant", false), ConfigurableLabelStatus::Hide);
    QCOMPARE(dest.visibility("behavior-intolerant", false), ConfigurableLabelStatus::Hide);
    QCOMPARE(dest.visibility("spam", false), ConfigurableLabelStatus::Hide);
    QCOMPARE(dest.visibility("impersonation", false), ConfigurableLabelStatus::Hide);

    QCOMPARE(dest.labelerCount(), 1);
    QCOMPARE(dest.mutedWordCount(), 0);

    dest = src;
    QCOMPARE(dest.enableAdultContent(), true);
    QCOMPARE(dest.visibility("porn", true), ConfigurableLabelStatus::Show);
    QCOMPARE(dest.visibility("sexual", true), ConfigurableLabelStatus::Show);
    QCOMPARE(dest.visibility("graphic-media", true), ConfigurableLabelStatus::Show);
    QCOMPARE(dest.visibility("nudity", true), ConfigurableLabelStatus::Show);

    QCOMPARE(dest.visibility("nsfl", true), ConfigurableLabelStatus::Hide);
    QCOMPARE(dest.visibility("gore", true), ConfigurableLabelStatus::Hide);
    QCOMPARE(dest.visibility("self-harm", true), ConfigurableLabelStatus::Hide);
    QCOMPARE(dest.visibility("torture", true), ConfigurableLabelStatus::Hide);
    QCOMPARE(dest.visibility("corpse", true), ConfigurableLabelStatus::Hide);
    QCOMPARE(dest.visibility("icon-kkk", false), ConfigurableLabelStatus::Hide);
    QCOMPARE(dest.visibility("icon-nazi", false), ConfigurableLabelStatus::Hide);
    QCOMPARE(dest.visibility("icon-intolerant", false), ConfigurableLabelStatus::Hide);
    QCOMPARE(dest.visibility("behavior-intolerant", false), ConfigurableLabelStatus::Hide);
    QCOMPARE(dest.visibility("spam", false), ConfigurableLabelStatus::Hide);
    QCOMPARE(dest.visibility("impersonation", false), ConfigurableLabelStatus::Hide);

    QCOMPARE(dest.labelerCount(), 1);
    QCOMPARE(dest.mutedWordCount(), 3);
    QCOMPARE(dest.getMutedWordItem(0).value, "word1");
    QCOMPARE(dest.getMutedWordItem(0).group, 0);
    QCOMPARE(dest.getMutedWordItem(0).targets, QList<MutedWordTarget>() << MutedWordTarget::Tag);
    QCOMPARE(dest.getMutedWordItem(1).value, "word2");
    QCOMPARE(dest.getMutedWordItem(1).group, 0);
    QVERIFY(dest.getMutedWordItem(1).targets
            == QList<MutedWordTarget>() << MutedWordTarget::Content << MutedWordTarget::Tag);
    QCOMPARE(dest.getMutedWordItem(2).value, "word3");
    QCOMPARE(dest.getMutedWordItem(2).group, 0);
    QVERIFY(dest.getMutedWordItem(2).targets
            == QList<MutedWordTarget>() << MutedWordTarget::Content);

    src.setEnableAdultContent(false);
    for (int i = 0; i < src.count(); i++) {
        src.setStatus(i, ConfigurableLabelStatus::Warning);
    }
    src.removeMutedWordItem(2);
    src.removeMutedWordItem(1);
    src.removeMutedWordItem(0);
    src.insertMutedWord(0, "word3-2",
                        QList<MutedWordTarget>()
                                << MutedWordTarget::Content << MutedWordTarget::Tag);
    src.insertMutedWord(0, "word1-2", QList<MutedWordTarget>() << MutedWordTarget::Content);
    src.insertMutedWord(1, "word2-2", QList<MutedWordTarget>() << MutedWordTarget::Tag);

    dest = src;
    QCOMPARE(dest.enableAdultContent(), false);
    QCOMPARE(dest.visibility("porn", true), ConfigurableLabelStatus::Hide);
    QCOMPARE(dest.visibility("sexual", true), ConfigurableLabelStatus::Hide);
    QCOMPARE(dest.visibility("graphic-media", true), ConfigurableLabelStatus::Hide);
    QCOMPARE(dest.visibility("nudity", true), ConfigurableLabelStatus::Warning);

    QCOMPARE(dest.visibility("nsfl", true), ConfigurableLabelStatus::Hide);
    QCOMPARE(dest.visibility("gore", true), ConfigurableLabelStatus::Hide);
    QCOMPARE(dest.visibility("self-harm", true), ConfigurableLabelStatus::Hide);
    QCOMPARE(dest.visibility("torture", true), ConfigurableLabelStatus::Hide);
    QCOMPARE(dest.visibility("corpse", true), ConfigurableLabelStatus::Hide);
    QCOMPARE(dest.visibility("icon-kkk", false), ConfigurableLabelStatus::Hide);
    QCOMPARE(dest.visibility("icon-nazi", false), ConfigurableLabelStatus::Hide);
    QCOMPARE(dest.visibility("icon-intolerant", false), ConfigurableLabelStatus::Hide);
    QCOMPARE(dest.visibility("behavior-intolerant", false), ConfigurableLabelStatus::Hide);
    QCOMPARE(dest.visibility("spam", false), ConfigurableLabelStatus::Hide);
    QCOMPARE(dest.visibility("impersonation", false), ConfigurableLabelStatus::Hide);

    QCOMPARE(dest.labelerCount(), 1);
    QCOMPARE(dest.mutedWordCount(), 3);
    QCOMPARE(dest.getMutedWordItem(0).value, "word1-2");
    QCOMPARE(dest.getMutedWordItem(0).group, 0);
    QVERIFY(dest.getMutedWordItem(0).targets
            == QList<MutedWordTarget>() << MutedWordTarget::Content);
    QCOMPARE(dest.getMutedWordItem(1).value, "word2-2");
    QCOMPARE(dest.getMutedWordItem(1).group, 0);
    QCOMPARE(dest.getMutedWordItem(1).targets, QList<MutedWordTarget>() << MutedWordTarget::Tag);
    QCOMPARE(dest.getMutedWordItem(2).value, "word3-2");
    QCOMPARE(dest.getMutedWordItem(2).group, 0);
    QVERIFY(dest.getMutedWordItem(2).targets
            == QList<MutedWordTarget>() << MutedWordTarget::Content << MutedWordTarget::Tag);
}

void atprotocol_test::test_ConfigurableLabels_save()
{
    ConfigurableLabels labels;
    m_account.accessJwt = "aaaa";
    labels.setAccount(m_account);
    {
        labels.setService(QString("http://localhost:%1/response/labels/save/1").arg(m_listenPort));
        QSignalSpy spy(&labels, SIGNAL(finished(bool)));
        labels.save();
        spy.wait(10 * 1000);
        QVERIFY2(spy.count() == 1, QString("spy.count()=%1").arg(spy.count()).toUtf8());
        QList<QVariant> arguments = spy.takeFirst();
        QVERIFY(arguments.at(0).toBool());
    }
    {
        labels.setEnableAdultContent(false);
        for (int i = 0; i < labels.count(); i++) {
            labels.setStatus(i, ConfigurableLabelStatus::Warning);
        }
        labels.setService(QString("http://localhost:%1/response/labels/save/2").arg(m_listenPort));
        QSignalSpy spy(&labels, SIGNAL(finished(bool)));
        labels.save();
        spy.wait();
        QVERIFY2(spy.count() == 1, QString("spy.count()=%1").arg(spy.count()).toUtf8());
        QList<QVariant> arguments = spy.takeFirst();
        QVERIFY(arguments.at(0).toBool());
    }
    {
        labels.setEnableAdultContent(true);
        for (int i = 0; i < labels.count(); i++) {
            labels.setStatus(i, ConfigurableLabelStatus::Warning);
        }
        labels.setService(QString("http://localhost:%1/response/labels/save/3").arg(m_listenPort));
        QSignalSpy spy(&labels, SIGNAL(finished(bool)));
        labels.save();
        spy.wait();
        QVERIFY2(spy.count() == 1, QString("spy.count()=%1").arg(spy.count()).toUtf8());
        QList<QVariant> arguments = spy.takeFirst();
        QVERIFY(arguments.at(0).toBool());
    }
    {
        labels.setEnableAdultContent(false);
        for (int i = 0; i < labels.count(); i++) {
            labels.setStatus(i, ConfigurableLabelStatus::Warning);
        }
        labels.setService(QString("http://localhost:%1/response/labels/save/4").arg(m_listenPort));
        QVERIFY2(labels.mutedWordCount() == 0,
                 QString("mutedWordCount=%1").arg(labels.mutedWordCount()).toLocal8Bit());
        QSignalSpy spy(&labels, SIGNAL(finished(bool)));
        labels.save();
        spy.wait();
        QVERIFY2(spy.count() == 1, QString("spy.count()=%1").arg(spy.count()).toUtf8());
        QList<QVariant> arguments = spy.takeFirst();
        QVERIFY(arguments.at(0).toBool());
    }
    {
        {
            labels.setService(
                    QString("http://localhost:%1/response/labels/save/5.1").arg(m_listenPort));
            QSignalSpy spy(&labels, SIGNAL(finished(bool)));
            labels.load();
            spy.wait();
            QVERIFY2(spy.count() == 1, QString("spy.count()=%1").arg(spy.count()).toUtf8());
            QList<QVariant> arguments = spy.takeFirst();
            QVERIFY(arguments.at(0).toBool());
            QVERIFY2(labels.mutedWordCount() == 8,
                     QString("mutedWordCount=%1").arg(labels.mutedWordCount()).toLocal8Bit());
        }
        labels.setEnableAdultContent(false);
        for (int i = 0; i < labels.count(); i++) {
            labels.setStatus(i, ConfigurableLabelStatus::Warning);
        }
        labels.setService(QString("http://localhost:%1/response/labels/save/5").arg(m_listenPort));
        QSignalSpy spy(&labels, SIGNAL(finished(bool)));
        labels.save();
        spy.wait();
        QVERIFY2(spy.count() == 1, QString("spy.count()=%1").arg(spy.count()).toUtf8());
        QList<QVariant> arguments = spy.takeFirst();
        QVERIFY(arguments.at(0).toBool());
    }
    {
        labels.setEnableAdultContent(false);
        for (int i = 0; i < labels.count(); i++) {
            labels.setStatus(i, ConfigurableLabelStatus::Warning);
        }
        labels.clearMutedWord();
        labels.setService(QString("http://localhost:%1/response/labels/save/6").arg(m_listenPort));
        QVERIFY2(labels.mutedWordCount() == 0,
                 QString("mutedWordCount=%1").arg(labels.mutedWordCount()).toLocal8Bit());
        QSignalSpy spy(&labels, SIGNAL(finished(bool)));
        labels.save();
        spy.wait();
        QVERIFY2(spy.count() == 1, QString("spy.count()=%1").arg(spy.count()).toUtf8());
        QList<QVariant> arguments = spy.takeFirst();
        QVERIFY(arguments.at(0).toBool());
    }
}

void atprotocol_test::test_ConfigurableLabels_mutedword()
{
    ConfigurableLabels labels;

    labels.insertMutedWord(0, "word3", QList<MutedWordTarget>() << MutedWordTarget::Content);
    labels.insertMutedWord(0, "word1", QList<MutedWordTarget>() << MutedWordTarget::Tag);
    labels.insertMutedWord(1, "word2",
                           QList<MutedWordTarget>()
                                   << MutedWordTarget::Content << MutedWordTarget::Tag);

    QCOMPARE(labels.getMutedWordItem(0).value, "word1");
    QCOMPARE(labels.getMutedWordItem(0).group, 0);
    QCOMPARE(labels.getMutedWordItem(0).targets, QList<MutedWordTarget>() << MutedWordTarget::Tag);
    QCOMPARE(labels.getMutedWordItem(1).value, "word2");
    QCOMPARE(labels.getMutedWordItem(1).group, 0);
    QVERIFY(labels.getMutedWordItem(1).targets
            == QList<MutedWordTarget>() << MutedWordTarget::Content << MutedWordTarget::Tag);
    QCOMPARE(labels.getMutedWordItem(2).value, "word3");
    QCOMPARE(labels.getMutedWordItem(2).group, 0);
    QVERIFY(labels.getMutedWordItem(2).targets
            == QList<MutedWordTarget>() << MutedWordTarget::Content);

    labels.moveMutedWordItem(2, 0);
    QCOMPARE(labels.getMutedWordItem(1).value, "word1");
    QCOMPARE(labels.getMutedWordItem(1).group, 0);
    QCOMPARE(labels.getMutedWordItem(1).targets, QList<MutedWordTarget>() << MutedWordTarget::Tag);
    QCOMPARE(labels.getMutedWordItem(2).value, "word2");
    QCOMPARE(labels.getMutedWordItem(2).group, 0);
    QVERIFY(labels.getMutedWordItem(2).targets
            == QList<MutedWordTarget>() << MutedWordTarget::Content << MutedWordTarget::Tag);
    QCOMPARE(labels.getMutedWordItem(0).value, "word3");
    QCOMPARE(labels.getMutedWordItem(0).group, 0);
    QVERIFY(labels.getMutedWordItem(0).targets
            == QList<MutedWordTarget>() << MutedWordTarget::Content);
}

void atprotocol_test::test_ConfigurableLabels_contains_mutedword()
{
    ConfigurableLabels labels, labels2;

    labels.insertMutedWord(0, "word3", QList<MutedWordTarget>() << MutedWordTarget::Content);
    labels.insertMutedWord(0, "word1", QList<MutedWordTarget>() << MutedWordTarget::Tag);
    labels.insertMutedWord(1, "word2",
                           QList<MutedWordTarget>()
                                   << MutedWordTarget::Content << MutedWordTarget::Tag);
    labels.insertMutedWord(1, "#word4",
                           QList<MutedWordTarget>()
                                   << MutedWordTarget::Content << MutedWordTarget::Tag);
    labels2 = labels;

    QCOMPARE(labels.containsMutedWords("hoge\nfuga\tpiyo foooo", QStringList(), false), false);
    // word1
    QVERIFY(labels.containsMutedWords("hoge\nword1 fuga\tpiyo foooo", QStringList() << "hoge",
                                      false)
            == false);
    QVERIFY(labels.containsMutedWords("hoge\nword fuga\tpiyo foooo", QStringList() << "word1",
                                      false)
            == true);
    // word2
    QVERIFY(labels.containsMutedWords("hoge\nword2 fuga\tpiyo foooo",
                                      QStringList() << "HOGE"
                                                    << "word1",
                                      false)
            == true);
    QVERIFY(labels.containsMutedWords("hoge\nword1 fuga\tpiyo foooo",
                                      QStringList() << "FUGA"
                                                    << "word2",
                                      false)
            == true);
    QVERIFY(labels.containsMutedWords("hoge #word2 fuga\tpiyo foooo", QStringList() << "hoge",
                                      false)
            == false);
    QVERIFY(labels.containsMutedWords("hoge #word2 fuga\tpiyo foooo",
                                      QStringList() << "hoge"
                                                    << "word2",
                                      false)
            == true);
    // word3
    QVERIFY(labels.containsMutedWords("hoge\nword3 fuga\tpiyo foooo", QStringList() << "hoge",
                                      false)
            == true);
    QVERIFY(labels.containsMutedWords("hoge\nword fuga\tpiyo foooo", QStringList() << "word3",
                                      false)
            == false);
    // word1
    QVERIFY(labels.containsMutedWords("hogeword1fuga\tpiyo foooo", QStringList() << "hoge", true)
            == false);
    QVERIFY(labels.containsMutedWords("hoge\nwordfuga\tpiyo foooo", QStringList() << "word1", true)
            == true);
    // word2
    QVERIFY(labels.containsMutedWords("hogeword2fuga\tpiyo foooo",
                                      QStringList() << "HOGE"
                                                    << "word1",
                                      false)
            == true);
    QVERIFY(labels.containsMutedWords("hoge\nword1 fuga\tpiyo foooo",
                                      QStringList() << "FUGA"
                                                    << "word2",
                                      false)
            == true);
    // word3
    QVERIFY(labels.containsMutedWords("hogeword3fuga\tpiyo foooo", QStringList() << "hoge", true)
            == true);
    QVERIFY(labels.containsMutedWords("hoge\nword fuga\tpiyo foooo", QStringList() << "word3",
                                      false)
            == false);

    // #word4
    QVERIFY(labels.containsMutedWords("hoge\nword4 fuga\tpiyo foooo",
                                      QStringList() << "HOGE"
                                                    << "word",
                                      false)
            == false);
    QVERIFY(labels.containsMutedWords("hoge\n#word4 fuga\tpiyo foooo",
                                      QStringList() << "HOGE"
                                                    << "word",
                                      false)
            == true);
    QVERIFY(labels.containsMutedWords("hoge\nword fuga\tpiyo foooo",
                                      QStringList() << "FUGA"
                                                    << "#word4",
                                      false)
            == false);
    QVERIFY(labels.containsMutedWords("hoge\nword1 fuga\tpiyo foooo",
                                      QStringList() << "FUGA"
                                                    << "word4",
                                      false)
            == true);
    // #word4
    QVERIFY(labels2.containsMutedWords("hoge\nword4 fuga\tpiyo foooo",
                                       QStringList() << "HOGE"
                                                     << "word",
                                       false)
            == false);
    QVERIFY(labels2.containsMutedWords("hoge\n#word4 fuga\tpiyo foooo",
                                       QStringList() << "HOGE"
                                                     << "word",
                                       false)
            == true);
    QVERIFY(labels2.containsMutedWords("hoge\nword fuga\tpiyo foooo",
                                       QStringList() << "FUGA"
                                                     << "#word4",
                                       false)
            == false);
    QVERIFY(labels2.containsMutedWords("hoge\nword1 fuga\tpiyo foooo",
                                       QStringList() << "FUGA"
                                                     << "word4",
                                       false)
            == true);

    labels.removeMutedWordItem(1);
    // #word4
    QVERIFY(labels.containsMutedWords("hoge\nword4 fuga\tpiyo foooo",
                                      QStringList() << "HOGE"
                                                    << "word",
                                      false)
            == false);
    QVERIFY(labels.containsMutedWords("hoge\n#word4 fuga\tpiyo foooo",
                                      QStringList() << "HOGE"
                                                    << "word",
                                      false)
            == false);
    QVERIFY(labels.containsMutedWords("hoge\nword fuga\tpiyo foooo",
                                      QStringList() << "FUGA"
                                                    << "#word4",
                                      false)
            == false);
    QVERIFY(labels.containsMutedWords("hoge\nword1 fuga\tpiyo foooo",
                                      QStringList() << "FUGA"
                                                    << "word4",
                                      false)
            == false);

    labels.clearMutedWord();
    QCOMPARE(labels.mutedWordCount(), 0);
    {

        m_account.accessJwt = "aaaa";
        labels.setAccount(m_account);
        labels.setService(
                QString("http://localhost:%1/response/labels/mutedword/3").arg(m_listenPort));
        QSignalSpy spy(&labels, SIGNAL(finished(bool)));
        labels.load();
        spy.wait(10 * 1000);
        QVERIFY2(spy.count() == 1, QString("spy.count()=%1").arg(spy.count()).toUtf8());
        QList<QVariant> arguments = spy.takeFirst();
        QVERIFY(arguments.at(0).toBool());
    }

    // #word4
    QVERIFY(labels.containsMutedWords("hoge\nword4 fuga\tpiyo foooo",
                                      QStringList() << "HOGE"
                                                    << "word",
                                      false)
            == false);
    QVERIFY(labels.containsMutedWords("hoge\n#word4 fuga\tpiyo foooo",
                                      QStringList() << "HOGE"
                                                    << "word",
                                      false)
            == true);
    QVERIFY(labels.containsMutedWords("hoge\nword fuga\tpiyo foooo",
                                      QStringList() << "FUGA"
                                                    << "#word4",
                                      false)
            == false);
    QVERIFY(labels.containsMutedWords("hoge\nword1 fuga\tpiyo foooo",
                                      QStringList() << "FUGA"
                                                    << "word4",
                                      false)
            == true);
}

void atprotocol_test::test_LabelerProvider()
{
    AtProtocolInterface::AccountData account = m_account;
    LabelerProvider *provider = LabelerProvider::getInstance();
    LabelerConnector *connector = new LabelerConnector(this);

    account.did = "did:plc:user";
    account.handle = "user_handle";
    account.accessJwt = "dummy_jwt";
    account.service_endpoint =
            QString("http://localhost:%1/response/labels/provider").arg(m_listenPort);

    qDebug().noquote() << "-- 1 ----------------------------------------";

    account.service = QString("http://localhost:%1/response/labels/provider/1").arg(m_listenPort);
    account.service_endpoint = account.service;
    provider->setAccount(account);
    {
        QSignalSpy spy(connector, SIGNAL(finished(bool)));
        provider->update(account, connector, LabelerProvider::RefleshMode::RefleshAuto);
        spy.wait();
        QVERIFY2(spy.count() == 1, QString("spy.count()=%1").arg(spy.count()).toUtf8());
    }

    QVERIFY(provider->labelerDids(account)
            == QStringList() << "__globally__"
                             << "did:plc:ar7c4by46qjdydhdevvrndac"
                             << "did:plc:original_labeler_did");

    delete connector;
    connector = new LabelerConnector(this);

    qDebug().noquote() << "-- 2 ----------------------------------------";

    account.service = QString("http://localhost:%1/response/labels/provider/2").arg(m_listenPort);
    account.service_endpoint = account.service;
    provider->setAccount(account);
    {
        QSignalSpy spy(connector, SIGNAL(finished(bool)));
        provider->update(account, connector, LabelerProvider::RefleshMode::RefleshAuto);
        spy.wait();
        QVERIFY2(spy.count() == 1, QString("spy.count()=%1").arg(spy.count()).toUtf8());
    }

    QVERIFY2(provider->labelerDids(account)
                     == QStringList() << "__globally__"
                                      << "did:plc:ar7c4by46qjdydhdevvrndac"
                                      << "did:plc:original_labeler_did",
             QString(provider->labelerDids(account).join(",")).toLocal8Bit());

    qDebug().noquote() << "-- 3 ----------------------------------------";

    provider->setRefreshInterval(account, 0);

    {
        QSignalSpy spy(connector, SIGNAL(finished(bool)));
        provider->update(account, connector, LabelerProvider::RefleshMode::RefleshAuto);
        spy.wait();
        QVERIFY2(spy.count() == 1, QString("spy.count()=%1").arg(spy.count()).toUtf8());
    }

    QVERIFY2(provider->labelerDids(account)
                     == QStringList() << "__globally__"
                                      << "did:plc:ar7c4by46qjdydhdevvrndac"
                                      << "did:plc:original_labeler_did"
                                      << "did:plc:original_labeler_did_2",
             QString(provider->labelerDids(account).join(",")).toLocal8Bit());

    QCOMPARE(provider->visibility(account, "hoge", true), ConfigurableLabelStatus::Show);
    QCOMPARE(provider->visibility(account, "porn", true), ConfigurableLabelStatus::Show);
    QCOMPARE(provider->visibility(account, "sexual", true), ConfigurableLabelStatus::Warning);
    QCOMPARE(provider->visibility(account, "graphic-media", true), ConfigurableLabelStatus::Hide);
    QCOMPARE(provider->visibility(account, "nudity", true), ConfigurableLabelStatus::Warning);

    QString labeler_did = "did:plc:ar7c4by46qjdydhdevvrndac";
    QVERIFY(provider->visibility(account, "rumor", false, labeler_did)
            == ConfigurableLabelStatus::Hide);

    QCOMPARE(provider->message(account, "porn", true), "Sexually Explicit");
    QCOMPARE(provider->message(account, "sexual", true), "Sexually Suggestive");
    QCOMPARE(provider->message(account, "graphic-media", true), "Graphic Media");
    QCOMPARE(provider->message(account, "nudity", true), "Non-sexual Nudity");

    QVERIFY(provider->containsMutedWords(account, "hoge\ntest3-4 fuga\tpiyo foooo",
                                         QStringList() << "hoge", false)
            == true);
}

void atprotocol_test::test_LabelProvider()
{
    AtProtocolInterface::AccountData account;
    QStringList labeler_dids;
    LabelProvider *provider = LabelProvider::getInstance();

    LabelConnector *connector1 = new LabelConnector(this);
    LabelConnector *connector2 = new LabelConnector(this);

    account.did = "did:plc:user";
    account.handle = "user_handle";
    account.accessJwt = "dummy_jwt";
    account.service_endpoint =
            QString("http://localhost:%1/response/labels/provider/10").arg(m_listenPort);

    QString labeler_did;
    QString id;
    LabelData label;

    labeler_dids << "did:plc:ar7c4by46qjdydhdevvrndac"
                 << "did:plc:original_labeler_did";
    {
        QSignalSpy spy(connector2, SIGNAL(finished(const QString &)));
        account.service =
                QString("http://localhost:%1/response/labels/provider/11").arg(m_listenPort);
        account.service_endpoint = account.service;
        provider->update(QStringList() << "did:plc:original_labeler_did", account, connector1);
        account.service =
                QString("http://localhost:%1/response/labels/provider/12").arg(m_listenPort);
        account.service_endpoint = account.service;
        provider->update(QStringList() << "did:plc:original_labeler_did"
                                       << "did:plc:ar7c4by46qjdydhdevvrndac",
                         account, connector2);
        spy.wait();
        spy.wait();
        QVERIFY2(spy.count() == 2, QString("spy.count()=%1").arg(spy.count()).toUtf8());
    }

    labeler_did = "did:plc:original_labeler_did";
    id = "test_label2";
    provider->setLanguage("en");
    label = provider->getLabel(labeler_did, id);
    QVERIFY2(label.name == "label2", label.name.toLocal8Bit());
    QVERIFY2(label.avatar
                     == "https://cdn.bsky.app/img/avatar/plain/did:plc:original_labeler_did/"
                        "avatar@jpeg",
             label.name.toLocal8Bit());
    provider->setLanguage("ja");
    label = provider->getLabel(labeler_did, id);
    QVERIFY2(label.name == "raberu2", label.name.toLocal8Bit());
    provider->setLanguage("unknown");
    label = provider->getLabel(labeler_did, id);
    QVERIFY2(label.name == "label2", label.name.toLocal8Bit());

    labeler_did = "did:plc:ar7c4by46qjdydhdevvrndac";
    id = "extremist";
    label = provider->getLabel(labeler_did, id);
    QVERIFY2(label.name == "Extremist", label.name.toLocal8Bit());
    QVERIFY2(label.avatar == "", label.name.toLocal8Bit());

    // 3連続で3カ所からリクエストしたときに、下記のようにするとgetServicesが3回発行されて
    // 3回目のリクエスト元に対しては3回コールバック必要
    // 1回目
    // did:plc:labeler_1
    // 2回目
    // did:plc:labeler_1
    // did:plc:labeler_2
    // 3回目
    // did:plc:labeler_1
    // did:plc:labeler_2
    // did:plc:labeler_3
}

void atprotocol_test::test_ComAtprotoRepoCreateRecord_post()
{
    AtProtocolInterface::ComAtprotoRepoCreateRecordEx createrecord;
    createrecord.setAccount(m_account);
    createrecord.setSelfLabels(QStringList() << "!warn"
                                             << "spam");
    QSignalSpy spy(&createrecord, SIGNAL(finished(bool)));
    createrecord.post("hello world");
    spy.wait();
    QVERIFY2(spy.count() == 1, QString("spy.count()=%1").arg(spy.count()).toUtf8());
    QList<QVariant> arguments = spy.takeFirst();
    QVERIFY(arguments.at(0).toBool());
}

void atprotocol_test::test_ComAtprotoRepoCreateRecord_threadgate()
{
    QString temp_did = m_account.did;
    m_account.did = "did:plc:mqxsuw5b5rhpwo4lw6iwlid5";
    AtProtocolInterface::ComAtprotoRepoCreateRecordEx createrecord;
    createrecord.setAccount(m_account);

    //    QVERIFY(createrecord.threadGate("at://uri", AtProtocolType::ThreadGateType::Everybody,
    //                                    QList<AtProtocolType::ThreadGateAllow>()));

    {
        createrecord.setService(
                QString("http://localhost:%1/response/threadgate/1").arg(m_listenPort));
        QSignalSpy spy(&createrecord, SIGNAL(finished(bool)));
        createrecord.threadGate(
                "at://did:plc:mqxsuw5b5rhpwo4lw6iwlid5/app.bsky.feed.post/3kggopmh3kd2s",
                AtProtocolType::ThreadGateType::Nobody, QList<AtProtocolType::ThreadGateAllow>());
        spy.wait();
        QVERIFY2(spy.count() == 1, QString("spy.count()=%1").arg(spy.count()).toUtf8());
        QList<QVariant> arguments = spy.takeFirst();
        QVERIFY(arguments.at(0).toBool());
    }

    QList<AtProtocolType::ThreadGateAllow> rules;
    AtProtocolType::ThreadGateAllow rule;

    rule.type = AtProtocolType::ThreadGateAllowType::Mentioned;
    rules.append(rule);
    {
        createrecord.setService(
                QString("http://localhost:%2/response/threadgate/2").arg(m_listenPort));
        QSignalSpy spy(&createrecord, SIGNAL(finished(bool)));
        createrecord.threadGate(
                "at://did:plc:mqxsuw5b5rhpwo4lw6iwlid5/app.bsky.feed.post/3kggopmh3kd2s",
                AtProtocolType::ThreadGateType::Choice, rules);
        spy.wait();
        QVERIFY2(spy.count() == 1, QString("spy.count()=%1").arg(spy.count()).toUtf8());
        QList<QVariant> arguments = spy.takeFirst();
        QVERIFY(arguments.at(0).toBool());
    }

    rules.clear();
    rule.type = AtProtocolType::ThreadGateAllowType::Followed;
    rules.append(rule);
    {
        createrecord.setService(
                QString("http://localhost:%2/response/threadgate/3").arg(m_listenPort));
        QSignalSpy spy(&createrecord, SIGNAL(finished(bool)));
        createrecord.threadGate(
                "at://did:plc:mqxsuw5b5rhpwo4lw6iwlid5/app.bsky.feed.post/3kggopmh3kd2s",
                AtProtocolType::ThreadGateType::Choice, rules);
        spy.wait();
        QVERIFY2(spy.count() == 1, QString("spy.count()=%1").arg(spy.count()).toUtf8());
        QList<QVariant> arguments = spy.takeFirst();
        QVERIFY(arguments.at(0).toBool());
    }

    rules.clear();
    rule.type = AtProtocolType::ThreadGateAllowType::List;
    rule.uri = "at://did:plc:mqxsuw5b5rhpwo4lw6iwlid5/app.bsky.graph.list/3kflf2r3lwg2x";
    rules.append(rule);
    {
        createrecord.setService(
                QString("http://localhost:%2/response/threadgate/4").arg(m_listenPort));
        QSignalSpy spy(&createrecord, SIGNAL(finished(bool)));
        createrecord.threadGate(
                "at://did:plc:mqxsuw5b5rhpwo4lw6iwlid5/app.bsky.feed.post/3kggopmh3kd2s",
                AtProtocolType::ThreadGateType::Choice, rules);
        spy.wait();
        QVERIFY2(spy.count() == 1, QString("spy.count()=%1").arg(spy.count()).toUtf8());
        QList<QVariant> arguments = spy.takeFirst();
        QVERIFY(arguments.at(0).toBool());
    }

    rules.clear();
    rule.type = AtProtocolType::ThreadGateAllowType::Mentioned;
    rules.append(rule);
    rule.type = AtProtocolType::ThreadGateAllowType::Followed;
    rules.append(rule);
    rule.type = AtProtocolType::ThreadGateAllowType::List;
    rule.uri = "at://did:plc:mqxsuw5b5rhpwo4lw6iwlid5/app.bsky.graph.list/3kflf2r3lwg2x";
    rules.append(rule);
    {
        createrecord.setService(
                QString("http://localhost:%2/response/threadgate/5").arg(m_listenPort));
        QSignalSpy spy(&createrecord, SIGNAL(finished(bool)));
        createrecord.threadGate(
                "at://did:plc:mqxsuw5b5rhpwo4lw6iwlid5/app.bsky.feed.post/3kggopmh3kd2s",
                AtProtocolType::ThreadGateType::Choice, rules);
        spy.wait();
        QVERIFY2(spy.count() == 1, QString("spy.count()=%1").arg(spy.count()).toUtf8());
        QList<QVariant> arguments = spy.takeFirst();
        QVERIFY(arguments.at(0).toBool());
    }
}

void atprotocol_test::test_ComAtprotoRepoCreateRecord_postgate()
{

    QString temp_did = m_account.did;
    m_account.did = "did:plc:mqxsuw5b5rhpwo4lw6iwlid5";
    AtProtocolInterface::ComAtprotoRepoCreateRecordEx createrecord;
    createrecord.setAccount(m_account);

    {
        createrecord.setService(
                QString("http://localhost:%1/response/postgate/1").arg(m_listenPort));
        QSignalSpy spy(&createrecord, SIGNAL(finished(bool)));
        createrecord.postGate(
                "at://did:plc:mqxsuw5b5rhpwo4lw6iwlid5/app.bsky.feed.post/3l44kjwogjq2q",
                AtProtocolType::AppBskyFeedPostgate::MainEmbeddingRulesType::
                        embeddingRules_DisableRule,
                QStringList());
        spy.wait();
        QVERIFY2(spy.count() == 1, QString("spy.count()=%1").arg(spy.count()).toUtf8());
        QList<QVariant> arguments = spy.takeFirst();
        QVERIFY(arguments.at(0).toBool());
    }

    {
        createrecord.setService(
                QString("http://localhost:%1/response/postgate/2").arg(m_listenPort));
        QSignalSpy spy(&createrecord, SIGNAL(finished(bool)));
        createrecord.postGate(
                "at://did:plc:mqxsuw5b5rhpwo4lw6iwlid5/app.bsky.feed.post/3l2zk2fehlz24",
                AtProtocolType::AppBskyFeedPostgate::MainEmbeddingRulesType::none,
                QStringList() << "at://did:plc:l4fsx4ujos7uw7n4ijq2ulgs/app.bsky.feed.post/"
                                 "3l44nnbwfcq2q");
        spy.wait();
        QVERIFY2(spy.count() == 1, QString("spy.count()=%1").arg(spy.count()).toUtf8());
        QList<QVariant> arguments = spy.takeFirst();
        QVERIFY(arguments.at(0).toBool());
    }
}

void atprotocol_test::test_AppBskyFeedGetFeedGenerator()
{
    AtProtocolInterface::AppBskyFeedGetFeedGenerator generator;
    generator.setAccount(m_account);
    generator.setService(QString("http://localhost:%1/response").arg(m_listenPort));

    QSignalSpy spy(&generator, SIGNAL(finished(bool)));
    generator.getFeedGenerator("at://did:plc:42fxwa2jeumqzzggx/app.bsky.feed.generator/aaagrsa");
    spy.wait();
    QVERIFY2(spy.count() == 1, QString("spy.count()=%1").arg(spy.count()).toUtf8());

    QVERIFY(generator.view().uri
            == "at://did:plc:42fxwa2jeumqzzggx/app.bsky.feed.generator/aaagrsa");
    QCOMPARE(generator.view().cid, "bafyreib7pgajpklwexy4lidm");
    QCOMPARE(generator.view().did, "did:web:view.bsky.social");
    QCOMPARE(generator.view().displayName, "view:displayName");
    QCOMPARE(generator.view().description, "view:description");
    QCOMPARE(generator.view().avatar, "https://cdn.bsky.social/view_avator.jpeg");
    QCOMPARE(generator.view().creator.did, "did:plc:42fxwa2jeumqzzggxj");
    QCOMPARE(generator.view().creator.handle, "creator.bsky.social");
    QCOMPARE(generator.view().creator.displayName, "creator:displayName");
    QCOMPARE(generator.view().creator.avatar, "https://cdn.bsky.social/creator_avator.jpeg");
}

void atprotocol_test::test_ServiceUrl()
{
    AtProtocolInterface::ComAtprotoServerCreateSessionEx session;
    session.setService(m_service + "/");

    {
        QSignalSpy spy(&session, SIGNAL(finished(bool)));
        session.createSession("hoge", "fuga", QString(), false);
        spy.wait();
        QVERIFY2(spy.count() == 1, QString("spy.count()=%1").arg(spy.count()).toUtf8());
        QList<QVariant> arguments = spy.takeFirst();
        QVERIFY(arguments.at(0).toBool());
    }
}

void atprotocol_test::test_ComAtprotoRepoGetRecord_profile()
{
    AtProtocolInterface::ComAtprotoRepoGetRecordEx record;
    record.setAccount(m_account);
    record.setService(QString("http://localhost:%1/response/profile").arg(m_listenPort));

    {
        QSignalSpy spy(&record, SIGNAL(finished(bool)));
        record.profile("did:plc:ipj5qejfoqu6eukvt72uhyit");
        spy.wait();
        QVERIFY2(spy.count() == 1, QString("spy.count()=%1").arg(spy.count()).toUtf8());
        QList<QVariant> arguments = spy.takeFirst();
        QVERIFY(arguments.at(0).toBool());
    }

    QVERIFY2(record.uri() == "at://did:plc:ipj5qejfoqu6eukvt72uhyit/app.bsky.actor.profile/self",
             record.cid().toLocal8Bit());
    QVERIFY2(record.cid() == "bafyreif4chy7iugq3blmvqt6sgqeo72pxkkr4v4fnzjqii2yriijh545ei",
             record.cid().toLocal8Bit());
    AppBskyActorProfile::Main value =
            LexiconsTypeUnknown::fromQVariant<AppBskyActorProfile::Main>(record.value());
    QVERIFY2(value.description == "epub\nLeME", value.description.toLocal8Bit());
    QVERIFY2(value.displayName == "IoriAYANE", value.displayName.toLocal8Bit());
    QVERIFY2(value.avatar.cid == "bafkreifjldy2fbgjfli7dson343u2bepzwypt7vlffb45ipsll6bjklphy",
             value.avatar.cid.toLocal8Bit());
    QVERIFY2(value.avatar.alt == "", value.avatar.alt.toLocal8Bit());
    QVERIFY2(value.avatar.mimeType == "image/jpeg", value.avatar.mimeType.toLocal8Bit());
    QVERIFY2(value.avatar.size == 68308, QString::number(value.avatar.size).toLocal8Bit());
    QVERIFY2(value.banner.cid == "bafkreifbbm2bipst4ekqtxckcv56vbqsee5q5pckkqvubz3gcoi243a6ni",
             value.banner.cid.toLocal8Bit());
    QVERIFY2(value.banner.alt == "", value.banner.alt.toLocal8Bit());
    QVERIFY2(value.banner.mimeType == "image/jpeg", value.banner.mimeType.toLocal8Bit());
    QVERIFY2(value.banner.size == 51567, QString::number(value.banner.size).toLocal8Bit());
}

void atprotocol_test::test_ComAtprotoRepoPutRecord_profile()
{
    AtProtocolInterface::ComAtprotoRepoPutRecordEx record;
    AtProtocolType::ComAtprotoRepoStrongRef::Main pinned_post;

    AtProtocolType::Blob avatar;
    AtProtocolType::Blob banner;

    m_account.did = "did:plc:mqxsuw5b5rhpwo4lw6iwlid5";
    record.setAccount(m_account);
    record.setService(QString("http://localhost:%1/response/profile/1").arg(m_listenPort));
    avatar.cid = "bafkreiayv34bulrnm5gsnx73b46s2plh76k7fvwcewqrdur7eelf7u6c3u";
    avatar.mimeType = "image/jpeg";
    avatar.size = 52880;
    {
        QSignalSpy spy(&record, SIGNAL(finished(bool)));
        record.profile(avatar, banner, "description", "display name", QString(), QString(),
                       pinned_post, "bafyreie3ckzfk5xadlunbotovrffkhsfb2hdnr7bujofy2bb5ro45elcmy");
        spy.wait();
        QVERIFY2(spy.count() == 1, QString("spy.count()=%1").arg(spy.count()).toUtf8());
        QList<QVariant> arguments = spy.takeFirst();
        QVERIFY(arguments.at(0).toBool());
    }

    m_account.did = "did:plc:ipj5qejfoqu6eukvt72uhyit";
    record.setAccount(m_account);
    record.setService(QString("http://localhost:%1/response/profile/2").arg(m_listenPort));
    avatar.cid = "bafkreifjldy2fbgjfli7dson343u2bepzwypt7vlffb45ipsll6bjklphy";
    avatar.mimeType = "image/jpeg";
    avatar.size = 68308;
    banner.cid = "bafkreifbbm2bipst4ekqtxckcv56vbqsee5q5pckkqvubz3gcoi243a6ni";
    banner.mimeType = "image/jpeg";
    banner.size = 51567;
    {
        QSignalSpy spy(&record, SIGNAL(finished(bool)));
        record.profile(avatar, banner, "epub\nLeME", "IoriAYANE", QString(), QString(), pinned_post,
                       "bafyreif4chy7iugq3blmvqt6sgqeo72pxkkr4v4fnzjqii2yriijh545ei");
        spy.wait();
        QVERIFY2(spy.count() == 1, QString("spy.count()=%1").arg(spy.count()).toUtf8());
        QList<QVariant> arguments = spy.takeFirst();
        QVERIFY(arguments.at(0).toBool());
    }
}

void atprotocol_test::test_ListItemsCache()
{
    ListItemsCache *cache = ListItemsCache::getInstance();

    QCOMPARE(cache->getListNames("hoge", "hoge"), QStringList());

    QCOMPARE(cache->has("account1"), false);
    QCOMPARE(cache->has("account2"), false);
    QCOMPARE(cache->has("account3"), false);

    cache->addItem("account1", "user1", "list1", "list1_uri", "list1_item1_uri");
    QCOMPARE(cache->getListNames("account1", "user1"), QStringList() << "list1");
    QCOMPARE(cache->getListUris("account1", "user1"), QStringList() << "list1_uri");

    cache->addItem("account1", "user1", "list1", "list1_uri", "list1_itemX_uri");
    QCOMPARE(cache->getListNames("account1", "user1"), QStringList() << "list1");
    QCOMPARE(cache->getListUris("account1", "user1"), QStringList() << "list1_uri");

    ListInfo info = cache->getListInfo("account1", "user1", "list1_uri");
    QVERIFY2(info.item_uri == "list1_item1_uri", info.item_uri.toLocal8Bit());

    cache->addItem("account1", "user1", "list2", "list2_uri", "list2_item1_uri");
    QVERIFY(cache->getListNames("account1", "user1")
            == QStringList() << "list1"
                             << "list2");
    QVERIFY(cache->getListUris("account1", "user1")
            == QStringList() << "list1_uri"
                             << "list2_uri");

    cache->addItem("account1", "user1", "list3", "list3_uri", "list3_item1_uri");
    QVERIFY(cache->getListNames("account1", "user1")
            == QStringList() << "list1"
                             << "list2"
                             << "list3");
    QVERIFY(cache->getListUris("account1", "user1")
            == QStringList() << "list1_uri"
                             << "list2_uri"
                             << "list3_uri");

    cache->addItem("account3", "user1", "account3list1", "account3list1_uri",
                   "account3list1_item1_uri");
    QCOMPARE(cache->getListNames("account3", "user1"), QStringList() << "account3list1");
    QCOMPARE(cache->getListUris("account3", "user1"), QStringList() << "account3list1_uri");

    cache->addItem("account3", "user1", "account3list2", "account3list2_uri",
                   "account3list2_item1_uri");
    QVERIFY(cache->getListNames("account3", "user1")
            == QStringList() << "account3list1"
                             << "account3list2");
    QVERIFY(cache->getListUris("account3", "user1")
            == QStringList() << "account3list1_uri"
                             << "account3list2_uri");

    cache->addItem("account3", "user2", "account3list1", "account3list1_uri",
                   "account3list1_item1_uri");
    QCOMPARE(cache->getListNames("account3", "user2"), QStringList() << "account3list1");
    QCOMPARE(cache->getListUris("account3", "user2"), QStringList() << "account3list1_uri");

    cache->addItem("account3", "user1", "account3list3", "account3list3_uri",
                   "account3list3_item1_uri");
    QVERIFY(cache->getListNames("account3", "user1")
            == QStringList() << "account3list1"
                             << "account3list2"
                             << "account3list3");
    QVERIFY(cache->getListUris("account3", "user1")
            == QStringList() << "account3list1_uri"
                             << "account3list2_uri"
                             << "account3list3_uri");

    cache->addItem("account2", "user1", "a2list1", "a2list1_uri", "a2list1_item1_uri");
    QCOMPARE(cache->getListNames("account2", "user1"), QStringList() << "a2list1");
    QCOMPARE(cache->getListUris("account2", "user1"), QStringList() << "a2list1_uri");

    cache->addItem("account2", "user1", "a2list2", "a2list2_uri", "a2list2_item1_uri");
    QVERIFY(cache->getListNames("account2", "user1")
            == QStringList() << "a2list1"
                             << "a2list2");
    QVERIFY(cache->getListUris("account2", "user1")
            == QStringList() << "a2list1_uri"
                             << "a2list2_uri");

    cache->addItem("account2", "user2", "a2list2", "a2list2_uri", "a2list2_item1_uri");
    QCOMPARE(cache->getListNames("account2", "user2"), QStringList() << "a2list2");
    QCOMPARE(cache->getListUris("account2", "user2"), QStringList() << "a2list2_uri");

    cache = ListItemsCache::getInstance();

    cache->addItem("account2", "user1", "a2list2", "a2list2_uri", "a2list2_item1_uri");
    QVERIFY(cache->getListNames("account2", "user1")
            == QStringList() << "a2list1"
                             << "a2list2");
    QVERIFY(cache->getListUris("account2", "user1")
            == QStringList() << "a2list1_uri"
                             << "a2list2_uri");

    // //////////////
    cache->removeItem("account3", "user1", "account3list2_uri");
    QVERIFY(cache->getListNames("account3", "user1")
            == QStringList() << "account3list1"
                             << "account3list3");
    QVERIFY(cache->getListUris("account3", "user1")
            == QStringList() << "account3list1_uri"
                             << "account3list3_uri");

    cache->removeItem("account1", "user1", "list3_uri");
    QVERIFY(cache->getListNames("account1", "user1")
            == QStringList() << "list1"
                             << "list2");
    QVERIFY(cache->getListUris("account1", "user1")
            == QStringList() << "list1_uri"
                             << "list2_uri");
    // //////////////

    cache->clear("account2");
    QCOMPARE(cache->has("account1"), true);
    QCOMPARE(cache->has("account2"), false);
    QCOMPARE(cache->has("account3"), true);
    QVERIFY(cache->getListNames("account1", "user1")
            == QStringList() << "list1"
                             << "list2");
    QVERIFY(cache->getListUris("account1", "user1")
            == QStringList() << "list1_uri"
                             << "list2_uri");
    QCOMPARE(cache->getListNames("account2", "user2"), QStringList());
    QCOMPARE(cache->getListUris("account2", "user2"), QStringList());
    QVERIFY(cache->getListNames("account3", "user1")
            == QStringList() << "account3list1"
                             << "account3list3");
    QVERIFY(cache->getListUris("account3", "user1")
            == QStringList() << "account3list1_uri"
                             << "account3list3_uri");
    cache->clear();
    QCOMPARE(cache->getListNames("account1", "user2"), QStringList());
    QCOMPARE(cache->getListUris("account1", "user2"), QStringList());
    QCOMPARE(cache->getListNames("account2", "user2"), QStringList());
    QCOMPARE(cache->getListUris("account2", "user2"), QStringList());
    QCOMPARE(cache->getListNames("account3", "user2"), QStringList());
    QCOMPARE(cache->getListUris("account3", "user2"), QStringList());

    QCOMPARE(cache->has("account1"), false);
    QCOMPARE(cache->has("account2"), false);
    QCOMPARE(cache->has("account3"), false);

    QCOMPARE(cache->getListNames("hoge", "hoge"), QStringList());
}

void atprotocol_test::test_checkPartialMatchLanguage()
{
    QCOMPARE(LexiconsTypeUnknown::checkPartialMatchLanguage(QStringList() << "ja"), true);
    QCOMPARE(LexiconsTypeUnknown::checkPartialMatchLanguage(QStringList() << "ja_JP"), true);
    QVERIFY(LexiconsTypeUnknown::checkPartialMatchLanguage(QStringList() << "ja"
                                                                         << "en")
            == true);
    QVERIFY(LexiconsTypeUnknown::checkPartialMatchLanguage(QStringList() << "de"
                                                                         << "ja"
                                                                         << "en")
            == true);
    QVERIFY(LexiconsTypeUnknown::checkPartialMatchLanguage(QStringList() << "de"
                                                                         << "en")
            == false);
    QCOMPARE(LexiconsTypeUnknown::checkPartialMatchLanguage(QStringList() << "de"), false);
    QCOMPARE(LexiconsTypeUnknown::checkPartialMatchLanguage(QStringList() << ""), false);
    QCOMPARE(LexiconsTypeUnknown::checkPartialMatchLanguage(QStringList()), false);
}

void atprotocol_test::test_AppBskyActorSearchActorsTypeahead()
{
    AtProtocolInterface::AppBskyActorSearchActorsTypeahead api;
    m_account.did = "did:plc:mqxsuw5b5rhpwo4lw6iwlid5";
    api.setAccount(m_account);
    api.setService(QString("http://localhost:%1/response").arg(m_listenPort));

    {
        QSignalSpy spy(&api, SIGNAL(finished(bool)));
        api.searchActorsTypeahead("hoge", 0);
        spy.wait();
        QVERIFY2(spy.count() == 1, QString("spy.count()=%1").arg(spy.count()).toUtf8());
        QList<QVariant> arguments = spy.takeFirst();
        QVERIFY(arguments.at(0).toBool());
    }

    QCOMPARE(api.followsList().count(), 2);
    QCOMPARE(api.followsList().at(0).did, "did:plc:mqxsuw5b5rhpwo4lw6iwlid5");
    QCOMPARE(api.followsList().at(1).did, "did:plc:ipj5qejfoqu6eukvt72uhyit");
}

void atprotocol_test::test_AppBskyFeedGetActorFeeds()
{
    AtProtocolInterface::AppBskyFeedGetActorFeeds api;

    m_account.did = "did:plc:mqxsuw5b5rhpwo4lw6iwlid5";
    api.setAccount(m_account);
    api.setService(QString("http://localhost:%1/response").arg(m_listenPort));

    {
        QSignalSpy spy(&api, SIGNAL(finished(bool)));
        api.getActorFeeds("", 0, QString());
        spy.wait();
        QVERIFY2(spy.count() == 1, QString("spy.count()=%1").arg(spy.count()).toUtf8());
        QList<QVariant> arguments = spy.takeFirst();
        QVERIFY(arguments.at(0).toBool());
    }

    QCOMPARE(api.feedsList().count(), 2);
    QVERIFY(api.feedsList().at(0).cid
            == "bafyreifiwk54etwnvf276ibod6agn4tl6off4ddxpj5qxgj6vqtm7mi6hy");
    QVERIFY(api.feedsList().at(1).cid
            == "bafyreifnih4vdr3nmwg3nsdkaamb6wnvwgo44v6qi73uxxugele65wswgi");
}

void atprotocol_test::test_AppBskyFeedGetActorLikes()
{
    AtProtocolInterface::AppBskyFeedGetActorLikes api;
    m_account.did = "did:plc:mqxsuw5b5rhpwo4lw6iwlid5";
    api.setAccount(m_account);
    api.setService(QString("http://localhost:%1/response").arg(m_listenPort));

    {
        QSignalSpy spy(&api, SIGNAL(finished(bool)));
        api.getActorLikes("", 0, QString());
        spy.wait();
        QVERIFY2(spy.count() == 1, QString("spy.count()=%1").arg(spy.count()).toUtf8());
        QList<QVariant> arguments = spy.takeFirst();
        QVERIFY(arguments.at(0).toBool());
    }

    QCOMPARE(api.feedList().count(), 1);
    QVERIFY(api.feedList().at(0).post.cid
            == "bafyreigkj2ljajuvh5yg4dxnaduxslxftfvbyshdaastjx6iacjuarmxka");
}

void atprotocol_test::test_AppBskyFeedGetAuthorFeed()
{
    AtProtocolInterface::AppBskyFeedGetAuthorFeed api;
    m_account.did = "did:plc:mqxsuw5b5rhpwo4lw6iwlid5";
    api.setAccount(m_account);
    api.setService(QString("http://localhost:%1/response").arg(m_listenPort));

    {
        QSignalSpy spy(&api, SIGNAL(finished(bool)));
        api.getAuthorFeed("", 0, QString(), QString(), false);
        spy.wait();
        QVERIFY2(spy.count() == 1, QString("spy.count()=%1").arg(spy.count()).toUtf8());
        QList<QVariant> arguments = spy.takeFirst();
        QVERIFY(arguments.at(0).toBool());
    }

    QCOMPARE(api.feedList().count(), 3);
    QVERIFY(api.feedList().at(0).post.cid
            == "bafyreifkt2ejac2rxsj6j3lvslvh6m3pvpcq5l4y5zmisops7hq2g4y5n4");
    QVERIFY(api.feedList().at(1).post.cid
            == "bafyreicdy6ddnhqbhxmzhsyvwxsdqx3zeaybdud5tzl3mnwe3zehaysmei");
    QVERIFY(api.feedList().at(1).reply.root_PostView.cid
            == "bafyreiad5kukibhklryendcsnzfbmqknqzxogwp67vorehabwryizs7to4");
    QVERIFY(api.feedList().at(1).reply.parent_PostView.cid
            == "bafyreiad5kukibhklryendcsnzfbmqknqzxogwp67vorehabwryizs7to4");
    QVERIFY(api.feedList().at(2).post.cid
            == "bafyreibl6vp7vczey2fxae5gqw2xfhb4cmnd6lcez4d6hlbrll3p6fg4o4");
}

void atprotocol_test::test_AppBskyFeedGetFeed()
{
    AtProtocolInterface::AppBskyFeedGetFeed api;
    m_account.did = "did:plc:mqxsuw5b5rhpwo4lw6iwlid5";
    api.setAccount(m_account);
    api.setService(QString("http://localhost:%1/response").arg(m_listenPort));

    {
        QSignalSpy spy(&api, SIGNAL(finished(bool)));
        api.getFeed("", 0, QString());
        spy.wait();
        QVERIFY2(spy.count() == 1, QString("spy.count()=%1").arg(spy.count()).toUtf8());
        QList<QVariant> arguments = spy.takeFirst();
        QVERIFY(arguments.at(0).toBool());
    }

    QCOMPARE(api.feedList().count(), 6);
    QVERIFY(api.feedList().at(0).post.cid
            == "bafyreidmnkhb3ls2t5jog774uncafb5zkzzyjffgs2pd7b2425wqshl2am");
    QVERIFY(api.feedList().at(1).post.cid
            == "bafyreibaqfe5nk4dbr5zcdjqlz6n32gungxhx5skk552n57dquhq4zvw7e");
    QVERIFY(api.feedList().at(2).post.cid
            == "bafyreihq26lx4cgiimgqoj6xekcilnmgzjxiogryaawprk262iqq3ws4vy");
    QVERIFY(api.feedList().at(3).post.cid
            == "bafyreicm2xpzq3aysgnypi7mjtihsptcomqj3ic6ryiq2qsgb2vdfnx6um");
    QVERIFY(api.feedList().at(4).post.cid
            == "bafyreiarsieqm27cswgla3b6z3ywvtpmozhdp67jenl4vz4whpyyqrqzbu");
    QVERIFY(api.feedList().at(5).post.cid
            == "bafyreiabnzvhd6ut7qcsjvofqq62hkicg2swfsn6ftcgfzpakxorwlyt4y");
}

void atprotocol_test::test_AppBskyFeedGetFeedGenerators()
{
    AtProtocolInterface::AppBskyFeedGetFeedGenerators api;
    m_account.did = "did:plc:mqxsuw5b5rhpwo4lw6iwlid5";
    api.setAccount(m_account);
    api.setService(QString("http://localhost:%1/response").arg(m_listenPort));

    {
        QSignalSpy spy(&api, SIGNAL(finished(bool)));
        api.getFeedGenerators(QStringList() << "");
        spy.wait();
        QVERIFY2(spy.count() == 1, QString("spy.count()=%1").arg(spy.count()).toUtf8());
        QList<QVariant> arguments = spy.takeFirst();
        QVERIFY(arguments.at(0).toBool());
    }

    QCOMPARE(api.feedsList().count(), 3);
    QVERIFY(api.feedsList().at(0).cid
            == "bafyreibffkfh5lnndhpmginso7txws4cmqcjrubb3dtfnnwqt2lyrydkua");
    QVERIFY(api.feedsList().at(1).cid
            == "bafyreifiwk54etwnvf276ibod6agn4tl6off4ddxpj5qxgj6vqtm7mi6hy");
    QVERIFY(api.feedsList().at(2).cid
            == "bafyreifnih4vdr3nmwg3nsdkaamb6wnvwgo44v6qi73uxxugele65wswgi");
}

void atprotocol_test::test_AppBskyGraphGetBlocks()
{
    AtProtocolInterface::AppBskyGraphGetBlocks api;
    m_account.did = "did:plc:mqxsuw5b5rhpwo4lw6iwlid5";
    api.setAccount(m_account);
    api.setService(QString("http://localhost:%1/response").arg(m_listenPort));

    {
        QSignalSpy spy(&api, SIGNAL(finished(bool)));
        api.getBlocks(0, QString());
        spy.wait();
        QVERIFY2(spy.count() == 1, QString("spy.count()=%1").arg(spy.count()).toUtf8());
        QList<QVariant> arguments = spy.takeFirst();
        QVERIFY(arguments.at(0).toBool());
    }

    QCOMPARE(api.followsList().count(), 1);
    QCOMPARE(api.followsList().at(0).did, "did:plc:blocked_user_did");
}

void atprotocol_test::test_AppBskyGraphGetFollowers()
{
    AtProtocolInterface::AppBskyGraphGetFollowers api;
    m_account.did = "did:plc:mqxsuw5b5rhpwo4lw6iwlid5";
    api.setAccount(m_account);
    api.setService(QString("http://localhost:%1/response").arg(m_listenPort));

    {
        QSignalSpy spy(&api, SIGNAL(finished(bool)));
        api.getFollowers("", 0, QString());
        spy.wait();
        QVERIFY2(spy.count() == 1, QString("spy.count()=%1").arg(spy.count()).toUtf8());
        QList<QVariant> arguments = spy.takeFirst();
        QVERIFY(arguments.at(0).toBool());
    }

    QCOMPARE(api.followsList().count(), 2);
    QCOMPARE(api.followsList().at(0).did, "did:plc:73l5atmh7p3fn3xigbp6ao5x");
    QCOMPARE(api.followsList().at(1).did, "did:plc:mqxsuw5b5rhpwo4lw6iwlid5");

    QCOMPARE(api.subject().did, "did:plc:l4fsx4ujos7uw7n4ijq2ulgs1");
}

void atprotocol_test::test_AppBskyGraphGetFollows()
{
    AtProtocolInterface::AppBskyGraphGetFollows api;
    m_account.did = "did:plc:mqxsuw5b5rhpwo4lw6iwlid5";
    api.setAccount(m_account);
    api.setService(QString("http://localhost:%1/response").arg(m_listenPort));

    {
        QSignalSpy spy(&api, SIGNAL(finished(bool)));
        api.getFollows("", 0, QString());
        spy.wait();
        QVERIFY2(spy.count() == 1, QString("spy.count()=%1").arg(spy.count()).toUtf8());
        QList<QVariant> arguments = spy.takeFirst();
        QVERIFY(arguments.at(0).toBool());
    }

    QCOMPARE(api.followsList().count(), 3);
    QCOMPARE(api.followsList().at(0).did, "did:plc:mqxsuw5b5rhpwo4lw6iwlid5");
    QCOMPARE(api.followsList().at(1).did, "did:plc:73l5atmh7p3fn3xigbp6ao5x");
    QCOMPARE(api.followsList().at(2).did, "did:plc:ipj5qejfoqu6eukvt72uhyit");

    QCOMPARE(api.subject().did, "did:plc:l4fsx4ujos7uw7n4ijq2ulgs");
}

void atprotocol_test::test_AppBskyGraphGetListMutes()
{
    AtProtocolInterface::AppBskyGraphGetListMutes api;
    m_account.did = "did:plc:mqxsuw5b5rhpwo4lw6iwlid5";
    api.setAccount(m_account);
    api.setService(QString("http://localhost:%1/response").arg(m_listenPort));

    {
        QSignalSpy spy(&api, SIGNAL(finished(bool)));
        api.getListMutes(0, QString());
        spy.wait();
        QVERIFY2(spy.count() == 1, QString("spy.count()=%1").arg(spy.count()).toUtf8());
        QList<QVariant> arguments = spy.takeFirst();
        QVERIFY(arguments.at(0).toBool());
    }

    QCOMPARE(api.listsList().count(), 2);
    QVERIFY(api.listsList().at(0).cid
            == "bafyreiexwt5i5qo4wy6qnn3wxnznjbqjbemmbqq3dwfaf7qbn2wk4cdblm");
    QVERIFY(api.listsList().at(1).cid
            == "bafyreifw2u4s2k6axspo3dppja3ofbwmyhkz6a5igemezmx3spxfvi6mcy");
}

void atprotocol_test::test_AppBskyGraphGetListBlocks()
{
    AtProtocolInterface::AppBskyGraphGetListBlocks api;
    m_account.did = "did:plc:mqxsuw5b5rhpwo4lw6iwlid5";
    api.setAccount(m_account);
    api.setService(QString("http://localhost:%1/response").arg(m_listenPort));

    {
        QSignalSpy spy(&api, SIGNAL(finished(bool)));
        api.getListBlocks(0, QString());
        spy.wait();
        QVERIFY2(spy.count() == 1, QString("spy.count()=%1").arg(spy.count()).toUtf8());
        QList<QVariant> arguments = spy.takeFirst();
        QVERIFY(arguments.at(0).toBool());
    }

    QCOMPARE(api.listsList().count(), 2);
    QVERIFY(api.listsList().at(0).cid
            == "bafyreif73o7vskbe36slr3zalbvitpcl5scr2d27ts3vmuqkyhj4u4pvm4");
    QVERIFY(api.listsList().at(1).cid
            == "bafyreieyd765syuilkovwe3ms3cpegt7wo3xksistzy2v4xmazrwbzlwtm");
}

void atprotocol_test::test_AppBskyGraphGetMutes()
{
    AtProtocolInterface::AppBskyGraphGetMutes api;
    m_account.did = "did:plc:mqxsuw5b5rhpwo4lw6iwlid5";
    api.setAccount(m_account);
    api.setService(QString("http://localhost:%1/response").arg(m_listenPort));

    {
        QSignalSpy spy(&api, SIGNAL(finished(bool)));
        api.getMutes(0, QString());
        spy.wait();
        QVERIFY2(spy.count() == 1, QString("spy.count()=%1").arg(spy.count()).toUtf8());
        QList<QVariant> arguments = spy.takeFirst();
        QVERIFY(arguments.at(0).toBool());
    }

    QCOMPARE(api.followsList().count(), 1);
    QCOMPARE(api.followsList().at(0).did, "did:plc:l4fsx4ujos7uw7n4ijq2ulgs");
}

void atprotocol_test::test_AppBskyActorPutPreferencesEx()
{
    AtProtocolInterface::AppBskyActorPutPreferencesEx pref;

    QString path;

    path = ":data/preferences/app.bsky.actor.putPreferences.10.json";
    QFile file(path);
    QVERIFY(file.open(QFile::ReadOnly));

    QString type = "app.bsky.actor.defs#savedFeedsPref";
    QJsonObject part;
    part.insert("$type", type);
    part.insert("saved", QJsonArray());

    QJsonDocument json_doc;
    QJsonObject dest_object;
    dest_object.insert("preferences",
                       pref.updatePreferencesJson(QString(file.readAll()), type, part));
    json_doc.setObject(dest_object);
    QVERIFY2(json_doc
                     == UnitTestCommon::loadJson(
                             ":data/preferences/app.bsky.actor.putPreferences.20.json"),
             json_doc.toJson());

    json_doc.setObject(pref.makePostInteractionSettingsPref("everybody", QStringList(), false));
    QVERIFY2(json_doc
                     == UnitTestCommon::loadJson(
                             ":data/preferences/app.bsky.actor.putPreferences.everybody.1.json"),
             json_doc.toJson());
    json_doc.setObject(pref.makePostInteractionSettingsPref("everybody", QStringList(), true));
    QVERIFY2(json_doc
                     == UnitTestCommon::loadJson(
                             ":data/preferences/app.bsky.actor.putPreferences.everybody.2.json"),
             json_doc.toJson());

    json_doc.setObject(pref.makePostInteractionSettingsPref("nobody", QStringList(), false));
    QVERIFY2(json_doc
                     == UnitTestCommon::loadJson(
                             ":data/preferences/app.bsky.actor.putPreferences.nobody.1.json"),
             json_doc.toJson());
    json_doc.setObject(pref.makePostInteractionSettingsPref("nobody", QStringList(), true));
    QVERIFY2(json_doc
                     == UnitTestCommon::loadJson(
                             ":data/preferences/app.bsky.actor.putPreferences.nobody.2.json"),
             json_doc.toJson());

    json_doc.setObject(pref.makePostInteractionSettingsPref("choice",
                                                            QStringList() << "mentioned"
                                                                          << "followed"
                                                                          << "follower"
                                                                          << "at://path/to/1"
                                                                          << "at://path/to/2",
                                                            false));
    QVERIFY2(json_doc
                     == UnitTestCommon::loadJson(
                             ":data/preferences/app.bsky.actor.putPreferences.choice.1.json"),
             json_doc.toJson());
}

void atprotocol_test::test_PlcDirectory()
{
#if 0
    AtProtocolInterface::DirectoryPlc plc;

    {
        QSignalSpy spy(&plc, SIGNAL(finished(bool)));
        plc.directory("did:plc:ipj5qejfoqu6eukvt72uhyit");
        spy.wait(10 * 1000);
        QVERIFY2(spy.count() == 1, QString("spy.count()=%1").arg(spy.count()).toUtf8());
    }

    QVERIFY(!plc.didDoc().service.isEmpty());
    QCOMPARE(plc.didDoc().service.at(0).id, "#atproto_pds");
    QCOMPARE(plc.didDoc().service.at(0).type, "AtprotoPersonalDataServer");
    QVERIFY(plc.didDoc().service.at(0).serviceEndpoint
            == "https://porcini.us-east.host.bsky.network");

    QCOMPARE(plc.serviceEndpoint(), "https://porcini.us-east.host.bsky.network");

#endif
}

void atprotocol_test::test_DirectoryPlcLogAudit()
{
#if 0
    AtProtocolInterface::DirectoryPlcLogAudit audit;

    {
        QSignalSpy spy(&audit, SIGNAL(finished(bool)));
        audit.audit("did:plc:ipj5qejfoqu6eukvt72uhyit");
        spy.wait();
        QVERIFY2(spy.count() == 1, QString("spy.count()=%1").arg(spy.count()).toUtf8());
    }

    QVERIFY(audit.plcAuditLog().length() >= 3);
    int i = 0;
    QVERIFY2(audit.plcAuditLog().at(i).cid
                     == "bafyreicd2pmbcjlufhrfcvm76vb6ceyhzk77ky66wrew3qhifzgbqkp2we",
             audit.plcAuditLog().at(i).cid.toLocal8Bit());
    QVERIFY2(audit.plcAuditLog().at(i).did == "did:plc:ipj5qejfoqu6eukvt72uhyit",
             audit.plcAuditLog().at(i).did.toLocal8Bit());
    QVERIFY2(audit.plcAuditLog().at(i).createdAt == "2023-04-13T16:33:31.049Z",
             audit.plcAuditLog().at(i).createdAt.toLocal8Bit());
    QVERIFY2(audit.plcAuditLog().at(i).operation_Plc_operation.type == "plc_operation",
             audit.plcAuditLog().at(i).operation_Plc_operation.type.toLocal8Bit());
    QVERIFY2(audit.plcAuditLog().at(i).operation_Plc_operation.services.atproto_pds.type
                     == "AtprotoPersonalDataServer",
             audit.plcAuditLog()
                     .at(i)
                     .operation_Plc_operation.services.atproto_pds.type.toLocal8Bit());
    QVERIFY2(audit.plcAuditLog().at(i).operation_Plc_operation.services.atproto_pds.endpoint
                     == "https://bsky.social",
             audit.plcAuditLog()
                     .at(i)
                     .operation_Plc_operation.services.atproto_pds.endpoint.toLocal8Bit());
    QVERIFY2(audit.plcAuditLog().at(i).operation_Plc_operation.alsoKnownAs.length() == 1,
             QString::number(audit.plcAuditLog().at(i).operation_Plc_operation.alsoKnownAs.length())
                     .toLocal8Bit());
    QVERIFY2(audit.plcAuditLog().at(i).operation_Plc_operation.alsoKnownAs.at(0)
                     == "at://ioriayane.bsky.social",
             audit.plcAuditLog().at(i).operation_Plc_operation.alsoKnownAs.at(0).toLocal8Bit());

    i = 2;
    QVERIFY2(audit.plcAuditLog().at(i).cid
                     == "bafyreigxn7o7fdb4a3rkaqfiqaivvcq4abt2wgjktgtpuicxhzr4qvj64i",
             audit.plcAuditLog().at(i).cid.toLocal8Bit());
    QVERIFY2(audit.plcAuditLog().at(i).did == "did:plc:ipj5qejfoqu6eukvt72uhyit",
             audit.plcAuditLog().at(i).did.toLocal8Bit());
    QVERIFY2(audit.plcAuditLog().at(i).createdAt == "2023-11-10T05:23:30.519Z",
             audit.plcAuditLog().at(i).createdAt.toLocal8Bit());
    QVERIFY2(audit.plcAuditLog().at(i).operation_Plc_operation.type == "plc_operation",
             audit.plcAuditLog().at(i).operation_Plc_operation.type.toLocal8Bit());
    QVERIFY2(audit.plcAuditLog().at(i).operation_Plc_operation.services.atproto_pds.type
                     == "AtprotoPersonalDataServer",
             audit.plcAuditLog()
                     .at(i)
                     .operation_Plc_operation.services.atproto_pds.type.toLocal8Bit());
    QVERIFY2(audit.plcAuditLog().at(i).operation_Plc_operation.services.atproto_pds.endpoint
                     == "https://porcini.us-east.host.bsky.network",
             audit.plcAuditLog()
                     .at(i)
                     .operation_Plc_operation.services.atproto_pds.endpoint.toLocal8Bit());
    QVERIFY2(audit.plcAuditLog().at(i).operation_Plc_operation.alsoKnownAs.length() == 1,
             QString::number(audit.plcAuditLog().at(i).operation_Plc_operation.alsoKnownAs.length())
                     .toLocal8Bit());
    QVERIFY2(audit.plcAuditLog().at(i).operation_Plc_operation.alsoKnownAs.at(0)
                     == "at://ioriayane.relog.tech",
             audit.plcAuditLog().at(i).operation_Plc_operation.alsoKnownAs.at(0).toLocal8Bit());
#endif
}

void atprotocol_test::test_PinnedPostCache()
{
    QCOMPARE(PinnedPostCache::getInstance()->pinned("", ""), false);
    //
    QCOMPARE(PinnedPostCache::getInstance()->pinned("did1", ""), false);
    QCOMPARE(PinnedPostCache::getInstance()->pinned("did1", "uri1"), false);
    PinnedPostCache::getInstance()->update("did1", "uri1");
    QCOMPARE(PinnedPostCache::getInstance()->pinned("did1", "uri1"), true);
    PinnedPostCache::getInstance()->update("did1", "uri1-1");
    QCOMPARE(PinnedPostCache::getInstance()->pinned("did1", "uri1"), false);
    QCOMPARE(PinnedPostCache::getInstance()->pinned("did1", "uri1-1"), true);
    //
    QCOMPARE(PinnedPostCache::getInstance()->pinned("did2", ""), false);
    QCOMPARE(PinnedPostCache::getInstance()->pinned("did2", "uri1"), false);
    PinnedPostCache::getInstance()->update("did2", "uri1");
    QCOMPARE(PinnedPostCache::getInstance()->pinned("did2", "uri1"), true);
    PinnedPostCache::getInstance()->update("did2", "uri1-1");
    QCOMPARE(PinnedPostCache::getInstance()->pinned("did2", "uri1"), false);
    QCOMPARE(PinnedPostCache::getInstance()->pinned("did2", "uri1-1"), true);
    //
    PinnedPostCache::getInstance()->update("did2", "");
    QCOMPARE(PinnedPostCache::getInstance()->pinned("did2", ""), false);
}

void atprotocol_test::test_atprotoProxyHttpHeader()
{
    AtProtocolInterface::AccountData account = m_account;
    AtProtocolInterface::AppBskyFeedGetTimeline timeline;
    timeline.setAccount(account);

    QSignalSpy spy(&timeline, SIGNAL(finished(bool)));
    timeline.getTimeline(QString(), 0, QString());
    spy.wait();
    QVERIFY2(spy.count() == 1, QString("spy.count()=%1").arg(spy.count()).toUtf8());
    QList<QVariant> arguments = spy.takeFirst();
    QVERIFY(arguments.at(0).toBool());
}

void atprotocol_test::test_convertVideoThumb()
{

    QString url;

    url = AtProtocolType::LexiconsTypeUnknown::convertVideoThumb(
            "https://video.bsky.app/watch/did%3Aplc%3Aipj5qejfoqu6eukvt72uhyit/"
            "bafkreie2dwcpkwtx4jqjh4qwcvgf3zs4eis3gt6wjgtd2smrheh5m3s62i/thumbnail.jpg");
#if 0
    QVERIFY2(url
                     == "https://video.cdn.bsky.app/hls/did:plc:ipj5qejfoqu6eukvt72uhyit/"
                        "bafkreie2dwcpkwtx4jqjh4qwcvgf3zs4eis3gt6wjgtd2smrheh5m3s62i/thumbnail.jpg",
             url.toLocal8Bit());
#endif
}

void atprotocol_test::test_facets()
{
    QJsonDocument json_testassets = UnitTestCommon::loadJson(":/data/facets/records.json");
    for (const auto json_testasset : json_testassets.array()) {
        QVariant record;
        LexiconsTypeUnknown::copyUnknown(json_testasset.toObject().value("record").toObject(),
                                         record);
        QString actual = LexiconsTypeUnknown::copyRecordText(record);
        QString expect = json_testasset.toObject().value("expect").toString();
        QVERIFY2(actual == expect, QString("\n%1\n%2\n").arg(actual).arg(expect).toLocal8Bit());
    }
}

void atprotocol_test::test_postText()
{
    QJsonDocument json_testassets = UnitTestCommon::loadJson(":/data/facets/records_plain.json");
    for (const auto json_testasset : json_testassets.array()) {
        QVariant record;
        LexiconsTypeUnknown::copyUnknown(json_testasset.toObject().value("record").toObject(),
                                         record);
        QString actual = LexiconsTypeUnknown::copyRecordText(record);
        QString expect = json_testasset.toObject().value("expect").toString();
        QVERIFY2(actual == expect, QString("\n%1\n%2\n").arg(actual).arg(expect).toLocal8Bit());
    }
}

void atprotocol_test::test_putPreferences(const QString &path, const QByteArray &body)
{
    QJsonDocument json_doc_expect;
    if (path.contains("/save/1/")) {
        json_doc_expect =
                UnitTestCommon::loadJson(":/data/labels/save/1/app.bsky.actor.putPreferences");
    } else if (path.contains("/save/2/")) {
        json_doc_expect =
                UnitTestCommon::loadJson(":/data/labels/save/2/app.bsky.actor.putPreferences");
    } else if (path.contains("/save/3/")) {
        json_doc_expect =
                UnitTestCommon::loadJson(":/data/labels/save/3/app.bsky.actor.putPreferences");
    } else if (path.contains("/save/4/")) {
        json_doc_expect =
                UnitTestCommon::loadJson(":/data/labels/save/4/app.bsky.actor.putPreferences");
    } else if (path.contains("/save/5/")) {
        json_doc_expect =
                UnitTestCommon::loadJson(":/data/labels/save/5/app.bsky.actor.putPreferences");
    } else if (path.contains("/save/5.1/")) {
        json_doc_expect =
                UnitTestCommon::loadJson(":/data/labels/save/5.1/app.bsky.actor.putPreferences");
    } else if (path.contains("/save/6/")) {
        json_doc_expect =
                UnitTestCommon::loadJson(":/data/labels/save/6/app.bsky.actor.putPreferences");
    } else {
        qDebug() << path;
        QVERIFY(false);
    }

    QJsonDocument json_doc = QJsonDocument::fromJson(body);

    if (json_doc_expect.object() != json_doc.object()) {
        qDebug().noquote().nospace() << QString("\nexpect:%1\nactual:%2\n")
                                                .arg(json_doc_expect.toJson(), json_doc.toJson());
    }
    QCOMPARE(json_doc_expect.object(), json_doc.object());
}

void atprotocol_test::test_putRecord(const QString &path, const QByteArray &body)
{
    QJsonDocument json_doc_expect;
    if (path.contains("/profile/1/")) {
        json_doc_expect = UnitTestCommon::loadJson(":/data/profile/1/com.atproto.repo.putRecord");
    } else if (path.contains("/profile/2/")) {
        json_doc_expect = UnitTestCommon::loadJson(":/data/profile/2/com.atproto.repo.putRecord");
    } else {
        qDebug() << path;
        QVERIFY(false);
    }

    QJsonDocument json_doc = QJsonDocument::fromJson(body);
    if (json_doc_expect.object() != json_doc.object()) {
        qDebug().noquote().nospace() << QString("\nexpect:%1\nactual:%2\n")
                                                .arg(json_doc_expect.toJson(), json_doc.toJson());
    }
    QCOMPARE(json_doc_expect.object(), json_doc.object());
}

void atprotocol_test::test_createRecord(const QString &path, const QByteArray &body)
{
    QString data_path = path;
    data_path.replace("/response/", ":/data/");
    QVERIFY(QFile::exists(data_path));

    QJsonDocument json_doc_expect = UnitTestCommon::loadJson(data_path);
    QJsonObject json_expect = json_doc_expect.object();
    updateCreatedAt(json_expect);

    QJsonDocument json_doc = QJsonDocument::fromJson(body);
    QJsonObject json_actual = json_doc.object();
    updateCreatedAt(json_actual);

    if (json_expect != json_actual) {
        qDebug().noquote().nospace() << QString("\nexpect:%1\nactual:%2\n")
                                                .arg(QJsonDocument(json_expect).toJson(),
                                                     QJsonDocument(json_actual).toJson());
    }
    QCOMPARE(json_expect, json_actual);
}

void atprotocol_test::updateCreatedAt(QJsonObject &json_obj)
{
    if (json_obj.contains("createdAt")) {
        json_obj["createdAt"] = "2023-12-13T14:15:16.000Z";
    }
    for (auto it = json_obj.begin(); it != json_obj.end(); ++it) {
        if (it.value().isObject()) {
            QJsonObject temp = it.value().toObject();
            updateCreatedAt(temp);
            it.value() = temp;
        } else if (it.value().isArray()) {
            QJsonArray array = it.value().toArray();
            for (int i = 0; i < array.size(); ++i) {
                if (array[i].isObject()) {
                    QJsonObject temp = array[i].toObject();
                    updateCreatedAt(temp);
                    it.value() = temp;
                }
            }
            it.value() = array;
        }
    }
}

QTEST_MAIN(atprotocol_test)

#include "tst_atprotocol_test.moc"
