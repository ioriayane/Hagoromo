#include <QtTest>
#include <QCoreApplication>
#include <QTemporaryFile>

#include "webserver.h"
#include "atprotocol/com/atproto/server/comatprotoservercreatesession.h"
#include "atprotocol/com/atproto/repo/comatprotorepocreaterecord.h"
#include "atprotocol/app/bsky/feed/appbskyfeedgettimeline.h"
#include "atprotocol/app/bsky/feed/appbskyfeedgetfeedgenerator.h"
#include "atprotocol/com/atproto/repo/comatprotorepogetrecord.h"
#include "atprotocol/com/atproto/repo/comatprotorepoputrecord.h"
#include "tools/opengraphprotocol.h"
#include "atprotocol/lexicons_func_unknown.h"
#include "tools/configurablelabels.h"
#include "tools/listitemscache.h"
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
    void test_OpenGraphProtocol();
    void test_getTimeline();
    void test_ConfigurableLabels();
    void test_ConfigurableLabels_load();
    void test_ConfigurableLabels_copy();
    void test_ConfigurableLabels_save();
    void test_ConfigurableLabels_mutedword();
    void test_ComAtprotoRepoCreateRecord_post();
    void test_ComAtprotoRepoCreateRecord_threadgate();
    void test_AppBskyFeedGetFeedGenerator();
    void test_ServiceUrl();
    void test_ComAtprotoRepoGetRecord_profile();
    void test_ComAtprotoRepoPutRecord_profile();
    void test_ListItemsCache();

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
    QCoreApplication::setApplicationName(QStringLiteral("Hagoromo"));

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
    QVERIFY(m_listenPort != 0);
    m_account.accessJwt = "dummy";
}

void atprotocol_test::cleanupTestCase() { }

void atprotocol_test::test_ComAtprotoServerCreateSession()
{
    AtProtocolInterface::ComAtprotoServerCreateSession session;
    session.setService(m_service);

    {
        QSignalSpy spy(&session, SIGNAL(finished(bool)));
        session.create("hoge", "fuga");
        spy.wait();
        QVERIFY2(spy.count() == 1, QString("spy.count()=%1").arg(spy.count()).toUtf8());
        QList<QVariant> arguments = spy.takeFirst();
        QVERIFY(arguments.at(0).toBool());
    }

    QVERIFY(session.did() == "did:plc:ipj5qejfoqu6eukvt72uhyit");
    QVERIFY(session.handle() == "ioriayane.relog.tech");
    QVERIFY(session.email() == "iori.ayane@gmail.com");
    QVERIFY(session.accessJwt() == "hoge hoge accessJwt");
    QVERIFY(session.refreshJwt() == "hoge hoge refreshJwt");

    // 後ろのテストで使うアカウント情報
    m_account = session.account();

    session.setService(m_service + "/limit");

    {
        QSignalSpy spy(&session, SIGNAL(finished(bool)));
        session.create("hoge", "fuga");
        spy.wait();
        QVERIFY2(spy.count() == 1, QString("spy.count()=%1").arg(spy.count()).toUtf8());
        QList<QVariant> arguments = spy.takeFirst();
        QVERIFY(arguments.at(0).toBool() == false);
    }

    QVERIFY2(session.errorCode() == "RateLimitExceeded", session.errorCode().toLocal8Bit());
    QVERIFY2(session.errorMessage()
                     == "Rate Limit "
                        "Exceeded\n\nratelimit-limit:30\nratelimit-policy:30;w=300\nratelimit-"
                        "remaining:10\nratelimit-reset:2023/09/17 10:31:07",
             session.errorMessage().toLocal8Bit());
    QVERIFY2(session.replyJson()
                     == "{\"error\":\"RateLimitExceeded\",\"message\":\"Rate Limit Exceeded\"}",
             session.replyJson().toLocal8Bit());
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

        QVERIFY(ogp.uri() == "http://localhost/response/ogp/file1.html");
        QVERIFY(ogp.title() == "file1 title");
        QVERIFY(ogp.description() == "file1 description");
        QVERIFY(ogp.thumb() == "http://localhost:%1/response/ogp/images/file1.jpg");
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
        QVERIFY(img.width() == 664);
        QVERIFY(img.height() == 257);
    }

    {
        QSignalSpy spy(&ogp, SIGNAL(finished(bool)));
        ogp.getData(m_service + "/ogp/file2.html");
        spy.wait();
        QVERIFY2(spy.count() == 1, QString("spy.count()=%1").arg(spy.count()).toUtf8());

        QList<QVariant> arguments = spy.takeFirst();
        QVERIFY(arguments.at(0).toBool());

        QVERIFY(ogp.uri() == "http://localhost/response/ogp/file2.html");
        QVERIFY2(ogp.title()
                         == QString("file2 ")
                                    .append(QChar(0x30bf))
                                    .append(QChar(0x30a4))
                                    .append(QChar(0x30c8))
                                    .append(QChar(0x30eb)),
                 ogp.title().toLocal8Bit());
        QVERIFY2(ogp.description() == QString("file2 ").append(QChar(0x8a73)).append(QChar(0x7d30)),
                 ogp.description().toLocal8Bit());
        QVERIFY(ogp.thumb() == "http://localhost:%1/response/ogp/images/file2.gif");
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
        QVERIFY(img.width() == 664);
        QVERIFY(img.height() == 257);
    }

    {
        QSignalSpy spy(&ogp, SIGNAL(finished(bool)));
        ogp.getData(m_service + "/ogp/file3.html");
        spy.wait();
        QVERIFY2(spy.count() == 1, QString("spy.count()=%1").arg(spy.count()).toUtf8());

        QList<QVariant> arguments = spy.takeFirst();
        QVERIFY(arguments.at(0).toBool());

        QVERIFY(ogp.uri() == "http://localhost/response/ogp/file3.html");
        QVERIFY2(ogp.title()
                         == QString("file3 ")
                                    .append(QChar(0x30bf))
                                    .append(QChar(0x30a4))
                                    .append(QChar(0x30c8))
                                    .append(QChar(0x30eb)),
                 ogp.title().toLocal8Bit());
        QVERIFY2(ogp.description() == QString("file3 ").append(QChar(0x8a73)).append(QChar(0x7d30)),
                 ogp.description().toLocal8Bit());
        QVERIFY(ogp.thumb() == "http://localhost:%1/response/ogp/images/file3.png");
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
        QVERIFY(img.width() == 664);
        QVERIFY(img.height() == 257);
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
        QVERIFY(ogp.thumb() == "http://localhost:%1/response/ogp/images/file3.png");
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
        QVERIFY(img.width() == 664);
        QVERIFY(img.height() == 257);
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
        QVERIFY(ogp.thumb() == "");
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
}

void atprotocol_test::test_getTimeline()
{
    AtProtocolInterface::AppBskyFeedGetTimeline timeline;
    timeline.setAccount(m_account);

    QSignalSpy spy(&timeline, SIGNAL(finished(bool)));
    timeline.getTimeline();
    spy.wait();
    QVERIFY2(spy.count() == 1, QString("spy.count()=%1").arg(spy.count()).toUtf8());

    QList<QVariant> arguments = spy.takeFirst();
    QVERIFY(arguments.at(0).toBool());

    QVERIFY(timeline.feedList()->count() == 6);

    QVERIFY(timeline.feedList()->at(0).post.author.did == "did:plc:mqxsuw5b5rhpwo4lw6iwlid5");

    QVERIFY2(timeline.feedList()->at(1).post.embed_type
                     == AppBskyFeedDefs::PostViewEmbedType::embed_AppBskyEmbedRecordWithMedia_View,
             QString("%1")
                     .arg(static_cast<int>(timeline.feedList()->at(1).post.embed_type))
                     .toLocal8Bit());

    QVERIFY2(timeline.feedList()->at(1).post.embed_AppBskyEmbedRecordWithMedia_View.media_type
                     == AppBskyEmbedRecordWithMedia::ViewMediaType::media_AppBskyEmbedImages_View,
             QString("%1")
                     .arg(static_cast<int>(
                             timeline.feedList()
                                     ->at(1)
                                     .post.embed_AppBskyEmbedRecordWithMedia_View.media_type))
                     .toLocal8Bit());
    QVERIFY2(timeline.feedList()->at(1).post.embed_AppBskyEmbedRecordWithMedia_View.record.isNull()
                     == false,
             QString("%1")
                     .arg(timeline.feedList()
                                  ->at(1)
                                  .post.embed_AppBskyEmbedRecordWithMedia_View.record.isNull())
                     .toLocal8Bit());
    QVERIFY2(timeline.feedList()
                             ->at(1)
                             .post.embed_AppBskyEmbedRecordWithMedia_View.record->record_type
                     == AppBskyEmbedRecord::ViewRecordType::record_ViewRecord,
             QString("%1")
                     .arg(static_cast<int>(timeline.feedList()
                                                   ->at(1)
                                                   .post.embed_AppBskyEmbedRecordWithMedia_View
                                                   .record->record_type))
                     .toLocal8Bit());
    QVERIFY2(timeline.feedList()
                             ->at(1)
                             .post.embed_AppBskyEmbedRecordWithMedia_View.record->record_ViewRecord
                             .cid
                     == "bafyreig5ylsbfssvs45welz6o45gbs4rsvg3ek3oi6ygdrl76vchbahhpu",
             timeline.feedList()
                     ->at(1)
                     .post.embed_AppBskyEmbedRecordWithMedia_View.record->record_ViewRecord.cid
                     .toLocal8Bit());
    QVERIFY2(timeline.feedList()
                             ->at(1)
                             .post.embed_AppBskyEmbedRecordWithMedia_View.record->record_ViewRecord
                             .author.handle
                     == "ioriayane2.bsky.social",
             timeline.feedList()
                     ->at(1)
                     .post.embed_AppBskyEmbedRecordWithMedia_View.record->record_ViewRecord.author
                     .handle.toLocal8Bit());

    QVERIFY(LexiconsTypeUnknown::fromQVariant<AppBskyFeedPost::Main>(
                    timeline.feedList()
                            ->at(1)
                            .post.embed_AppBskyEmbedRecordWithMedia_View.record->record_ViewRecord
                            .value)
                    .text
            == "quoted post");

    QVERIFY(timeline.feedList()
                    ->at(1)
                    .post.embed_AppBskyEmbedRecordWithMedia_View.media_AppBskyEmbedImages_View
                    .images.count()
            == 1);
    QVERIFY(timeline.feedList()
                    ->at(1)
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
    QVERIFY(labels.visibility("hoge", for_image) == ConfigurableLabelStatus::Show);
    QVERIFY(labels.visibility("porn", for_image) == ConfigurableLabelStatus::Show);
    QVERIFY(labels.visibility("nsfl", for_image) == ConfigurableLabelStatus::Show);
    QVERIFY(labels.visibility("nudity", for_image) == ConfigurableLabelStatus::Show);
    QVERIFY(labels.visibility("sexual", for_image) == ConfigurableLabelStatus::Show);
    QVERIFY(labels.visibility("gore", for_image) == ConfigurableLabelStatus::Show);
    QVERIFY(labels.visibility("self-harm", for_image) == ConfigurableLabelStatus::Show);
    QVERIFY(labels.visibility("torture", for_image) == ConfigurableLabelStatus::Show);
    QVERIFY(labels.visibility("corpse", for_image) == ConfigurableLabelStatus::Show);
    QVERIFY(labels.visibility("icon-kkk", for_image) == ConfigurableLabelStatus::Show);
    QVERIFY(labels.visibility("icon-nazi", for_image) == ConfigurableLabelStatus::Show);
    QVERIFY(labels.visibility("icon-intolerant", for_image) == ConfigurableLabelStatus::Show);
    QVERIFY(labels.visibility("behavior-intolerant", for_image) == ConfigurableLabelStatus::Show);
    QVERIFY(labels.visibility("spam", for_image) == ConfigurableLabelStatus::Show);
    QVERIFY(labels.visibility("impersonation", for_image) == ConfigurableLabelStatus::Show);

    labels.setEnableAdultContent(false);
    QVERIFY(labels.visibility("hoge", for_image) == ConfigurableLabelStatus::Show);
    QVERIFY(labels.visibility("porn", for_image) == ConfigurableLabelStatus::Hide);
    QVERIFY(labels.visibility("nsfl", for_image) == ConfigurableLabelStatus::Hide);
    QVERIFY(labels.visibility("nudity", for_image) == ConfigurableLabelStatus::Hide);
    QVERIFY(labels.visibility("sexual", for_image) == ConfigurableLabelStatus::Hide);
    QVERIFY(labels.visibility("gore", for_image) == ConfigurableLabelStatus::Hide);
    QVERIFY(labels.visibility("self-harm", for_image) == ConfigurableLabelStatus::Hide);
    QVERIFY(labels.visibility("torture", for_image) == ConfigurableLabelStatus::Hide);
    QVERIFY(labels.visibility("corpse", for_image) == ConfigurableLabelStatus::Hide);
    QVERIFY(labels.visibility("icon-kkk", for_image) == ConfigurableLabelStatus::Show);
    QVERIFY(labels.visibility("icon-nazi", for_image) == ConfigurableLabelStatus::Show);
    QVERIFY(labels.visibility("icon-intolerant", for_image) == ConfigurableLabelStatus::Show);
    QVERIFY(labels.visibility("behavior-intolerant", for_image) == ConfigurableLabelStatus::Show);
    QVERIFY(labels.visibility("spam", for_image) == ConfigurableLabelStatus::Show);
    QVERIFY(labels.visibility("impersonation", for_image) == ConfigurableLabelStatus::Show);

    labels.setEnableAdultContent(true);
    for (int i = 0; i < labels.count(); i++) {
        labels.setStatus(i, ConfigurableLabelStatus::Hide);
    }
    QVERIFY(labels.visibility("corpse", true) == ConfigurableLabelStatus::Hide);
    QVERIFY(labels.visibility("icon-kkk", true) == ConfigurableLabelStatus::Show);
    QVERIFY(labels.visibility("corpse", false) == ConfigurableLabelStatus::Show);
    QVERIFY(labels.visibility("icon-kkk", false) == ConfigurableLabelStatus::Hide);

    QVERIFY(labels.message("corpse", true) == "Violence");
    QVERIFY(labels.message("icon-kkk", true) == QString());
    QVERIFY(labels.message("corpse", false) == QString());
    QVERIFY(labels.message("icon-kkk", false) == "Hate Groups");

    labels.setEnableAdultContent(true);
    for (int i = 0; i < labels.count(); i++) {
        labels.setStatus(i, ConfigurableLabelStatus::Warning);
    }
    QVERIFY(labels.visibility("corpse", true) == ConfigurableLabelStatus::Warning);
    QVERIFY(labels.visibility("icon-kkk", true) == ConfigurableLabelStatus::Show);
    QVERIFY(labels.visibility("corpse", false) == ConfigurableLabelStatus::Show);
    QVERIFY(labels.visibility("icon-kkk", false) == ConfigurableLabelStatus::Warning);

    QVERIFY(labels.message("corpse", true) == "Violence");
    QVERIFY(labels.message("icon-kkk", true) == QString());
    QVERIFY(labels.message("corpse", false) == QString());
    QVERIFY(labels.message("icon-kkk", false) == "Hate Groups");

    // 設定変更できない項目を変更していないか確認
    for (int i = 0; i < labels.count(); i++) {
        labels.setStatus(i, ConfigurableLabelStatus::Show);
    }
    QVERIFY(labels.visibility("!hide", false) == ConfigurableLabelStatus::Hide);
    QVERIFY(labels.visibility("doxxing", false) == ConfigurableLabelStatus::Hide);
    QVERIFY(labels.visibility("!warn", false) == ConfigurableLabelStatus::Warning);
    QVERIFY(labels.visibility("!warn", true) == ConfigurableLabelStatus::Show);
    QVERIFY(labels.message("!warn", false) == "Content warning");
    QVERIFY(labels.message("!warn", true) == QString());
}

void atprotocol_test::test_ConfigurableLabels_load()
{
    ConfigurableLabels labels;
    labels.setAccount(m_account);
    {
        labels.setService(QString("http://localhost:%1/response/labels/hide").arg(m_listenPort));
        QSignalSpy spy(&labels, SIGNAL(finished(bool)));
        labels.load();
        spy.wait();
        QVERIFY2(spy.count() == 1, QString("spy.count()=%1").arg(spy.count()).toUtf8());
        QList<QVariant> arguments = spy.takeFirst();
        QVERIFY(arguments.at(0).toBool());

        QVERIFY(labels.enableAdultContent() == true);
        QVERIFY(labels.visibility("porn", true) == ConfigurableLabelStatus::Hide);
        QVERIFY(labels.visibility("nsfl", true) == ConfigurableLabelStatus::Hide);
        QVERIFY(labels.visibility("nudity", true) == ConfigurableLabelStatus::Hide);
        QVERIFY(labels.visibility("sexual", true) == ConfigurableLabelStatus::Hide);
        QVERIFY(labels.visibility("gore", true) == ConfigurableLabelStatus::Hide);
        QVERIFY(labels.visibility("self-harm", true) == ConfigurableLabelStatus::Hide);
        QVERIFY(labels.visibility("torture", true) == ConfigurableLabelStatus::Hide);
        QVERIFY(labels.visibility("corpse", true) == ConfigurableLabelStatus::Hide);
        QVERIFY(labels.visibility("icon-kkk", false) == ConfigurableLabelStatus::Hide);
        QVERIFY(labels.visibility("icon-nazi", false) == ConfigurableLabelStatus::Hide);
        QVERIFY(labels.visibility("icon-intolerant", false) == ConfigurableLabelStatus::Hide);
        QVERIFY(labels.visibility("behavior-intolerant", false) == ConfigurableLabelStatus::Hide);
        QVERIFY(labels.visibility("spam", false) == ConfigurableLabelStatus::Hide);
        QVERIFY(labels.visibility("impersonation", false) == ConfigurableLabelStatus::Hide);
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

        QVERIFY(labels.enableAdultContent() == true);
        QVERIFY(labels.visibility("porn", true) == ConfigurableLabelStatus::Show);
        QVERIFY(labels.visibility("nsfl", true) == ConfigurableLabelStatus::Show);
        QVERIFY(labels.visibility("nudity", true) == ConfigurableLabelStatus::Show);
        QVERIFY(labels.visibility("sexual", true) == ConfigurableLabelStatus::Show);
        QVERIFY(labels.visibility("gore", true) == ConfigurableLabelStatus::Show);
        QVERIFY(labels.visibility("self-harm", true) == ConfigurableLabelStatus::Show);
        QVERIFY(labels.visibility("torture", true) == ConfigurableLabelStatus::Show);
        QVERIFY(labels.visibility("corpse", true) == ConfigurableLabelStatus::Show);
        QVERIFY(labels.visibility("icon-kkk", false) == ConfigurableLabelStatus::Show);
        QVERIFY(labels.visibility("icon-nazi", false) == ConfigurableLabelStatus::Show);
        QVERIFY(labels.visibility("icon-intolerant", false) == ConfigurableLabelStatus::Show);
        QVERIFY(labels.visibility("behavior-intolerant", false) == ConfigurableLabelStatus::Show);
        QVERIFY(labels.visibility("spam", false) == ConfigurableLabelStatus::Show);
        QVERIFY(labels.visibility("impersonation", false) == ConfigurableLabelStatus::Show);
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

        QVERIFY(labels.enableAdultContent() == false);
        QVERIFY(labels.visibility("porn", true) == ConfigurableLabelStatus::Hide);
        QVERIFY(labels.visibility("nsfl", true) == ConfigurableLabelStatus::Hide);
        QVERIFY(labels.visibility("nudity", true) == ConfigurableLabelStatus::Hide);
        QVERIFY(labels.visibility("sexual", true) == ConfigurableLabelStatus::Hide);
        QVERIFY(labels.visibility("gore", true) == ConfigurableLabelStatus::Hide);
        QVERIFY(labels.visibility("self-harm", true) == ConfigurableLabelStatus::Hide);
        QVERIFY(labels.visibility("torture", true) == ConfigurableLabelStatus::Hide);
        QVERIFY(labels.visibility("corpse", true) == ConfigurableLabelStatus::Hide);
        QVERIFY(labels.visibility("icon-kkk", false) == ConfigurableLabelStatus::Show);
        QVERIFY(labels.visibility("icon-nazi", false) == ConfigurableLabelStatus::Show);
        QVERIFY(labels.visibility("icon-intolerant", false) == ConfigurableLabelStatus::Show);
        QVERIFY(labels.visibility("behavior-intolerant", false) == ConfigurableLabelStatus::Show);
        QVERIFY(labels.visibility("spam", false) == ConfigurableLabelStatus::Show);
        QVERIFY(labels.visibility("impersonation", false) == ConfigurableLabelStatus::Show);
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

        QVERIFY(labels.enableAdultContent() == true);
        QVERIFY(labels.visibility("porn", true) == ConfigurableLabelStatus::Warning);
        QVERIFY(labels.visibility("nsfl", true) == ConfigurableLabelStatus::Warning);
        QVERIFY(labels.visibility("nudity", true) == ConfigurableLabelStatus::Warning);
        QVERIFY(labels.visibility("sexual", true) == ConfigurableLabelStatus::Warning);
        QVERIFY(labels.visibility("gore", true) == ConfigurableLabelStatus::Warning);
        QVERIFY(labels.visibility("self-harm", true) == ConfigurableLabelStatus::Warning);
        QVERIFY(labels.visibility("torture", true) == ConfigurableLabelStatus::Warning);
        QVERIFY(labels.visibility("corpse", true) == ConfigurableLabelStatus::Warning);
        QVERIFY(labels.visibility("icon-kkk", false) == ConfigurableLabelStatus::Warning);
        QVERIFY(labels.visibility("icon-nazi", false) == ConfigurableLabelStatus::Warning);
        QVERIFY(labels.visibility("icon-intolerant", false) == ConfigurableLabelStatus::Warning);
        QVERIFY(labels.visibility("behavior-intolerant", false)
                == ConfigurableLabelStatus::Warning);
        QVERIFY(labels.visibility("spam", false) == ConfigurableLabelStatus::Warning);
        QVERIFY(labels.visibility("impersonation", false) == ConfigurableLabelStatus::Warning);
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
        QVERIFY(labels.getMutedWordItem(i).targets == QList<MutedWordTarget>());
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
        QVERIFY(labels.getMutedWordItem(i).group == 0);
        QVERIFY(labels.getMutedWordItem(i).value == "test1-1");
        QVERIFY(labels.getMutedWordItem(i).targets.length() == 2);
        QVERIFY(labels.getMutedWordItem(i).targets
                == QList<MutedWordTarget>() << MutedWordTarget::Content << MutedWordTarget::Tag);
        i = 1;
        QVERIFY(labels.getMutedWordItem(i).group == 0);
        QVERIFY(labels.getMutedWordItem(i).value == "test1-2");
        QVERIFY(labels.getMutedWordItem(i).targets.length() == 1);
        QVERIFY(labels.getMutedWordItem(i).targets
                == QList<MutedWordTarget>() << MutedWordTarget::Tag);
        i = 2;
        QVERIFY(labels.getMutedWordItem(i).group == 0);
        QVERIFY(labels.getMutedWordItem(i).value == "test1-3");
        QVERIFY(labels.getMutedWordItem(i).targets.length() == 0);
        QVERIFY(labels.getMutedWordItem(i).targets == QList<MutedWordTarget>());
        i = 3;
        QVERIFY(labels.getMutedWordItem(i).group == 0);
        QVERIFY(labels.getMutedWordItem(i).value == "test1-4");
        QVERIFY(labels.getMutedWordItem(i).targets.length() == 1);
        QVERIFY(labels.getMutedWordItem(i).targets
                == QList<MutedWordTarget>() << MutedWordTarget::Content);

        i = 4;
        QVERIFY(labels.getMutedWordItem(i).group == 1);
        QVERIFY(labels.getMutedWordItem(i).value == "test2-1");
        QVERIFY(labels.getMutedWordItem(i).targets.length() == 1);
        QVERIFY(labels.getMutedWordItem(i).targets
                == QList<MutedWordTarget>() << MutedWordTarget::Content);
        i = 5;
        QVERIFY(labels.getMutedWordItem(i).group == 1);
        QVERIFY(labels.getMutedWordItem(i).value == "test2-2");
        QVERIFY(labels.getMutedWordItem(i).targets.length() == 2);
        QVERIFY(labels.getMutedWordItem(i).targets
                == QList<MutedWordTarget>() << MutedWordTarget::Content << MutedWordTarget::Tag);
        i = 6;
        QVERIFY(labels.getMutedWordItem(i).group == 1);
        QVERIFY(labels.getMutedWordItem(i).value == "test2-3");
        QVERIFY(labels.getMutedWordItem(i).targets.length() == 1);
        QVERIFY(labels.getMutedWordItem(i).targets
                == QList<MutedWordTarget>() << MutedWordTarget::Tag);
        i = 7;
        QVERIFY(labels.getMutedWordItem(i).group == 1);
        QVERIFY(labels.getMutedWordItem(i).value == "test2-4");
        QVERIFY(labels.getMutedWordItem(i).targets.length() == 0);
        QVERIFY(labels.getMutedWordItem(i).targets == QList<MutedWordTarget>());
    }
}

void atprotocol_test::test_ConfigurableLabels_copy()
{
    ConfigurableLabels src, dest;
    src.setEnableAdultContent(true);
    for (int i = 0; i < src.count(); i++) {
        src.setStatus(i, ConfigurableLabelStatus::Show);
    }
    dest.setEnableAdultContent(false);
    for (int i = 0; i < dest.count(); i++) {
        dest.setStatus(i, ConfigurableLabelStatus::Hide);
    }
    QVERIFY(dest.enableAdultContent() == false);
    QVERIFY(dest.visibility("porn", true) == ConfigurableLabelStatus::Hide);
    QVERIFY(dest.visibility("nsfl", true) == ConfigurableLabelStatus::Hide);
    QVERIFY(dest.visibility("nudity", true) == ConfigurableLabelStatus::Hide);
    QVERIFY(dest.visibility("sexual", true) == ConfigurableLabelStatus::Hide);
    QVERIFY(dest.visibility("gore", true) == ConfigurableLabelStatus::Hide);
    QVERIFY(dest.visibility("self-harm", true) == ConfigurableLabelStatus::Hide);
    QVERIFY(dest.visibility("torture", true) == ConfigurableLabelStatus::Hide);
    QVERIFY(dest.visibility("corpse", true) == ConfigurableLabelStatus::Hide);
    QVERIFY(dest.visibility("icon-kkk", false) == ConfigurableLabelStatus::Hide);
    QVERIFY(dest.visibility("icon-nazi", false) == ConfigurableLabelStatus::Hide);
    QVERIFY(dest.visibility("icon-intolerant", false) == ConfigurableLabelStatus::Hide);
    QVERIFY(dest.visibility("behavior-intolerant", false) == ConfigurableLabelStatus::Hide);
    QVERIFY(dest.visibility("spam", false) == ConfigurableLabelStatus::Hide);
    QVERIFY(dest.visibility("impersonation", false) == ConfigurableLabelStatus::Hide);

    dest = src;
    QVERIFY(dest.enableAdultContent() == true);
    QVERIFY(dest.visibility("porn", true) == ConfigurableLabelStatus::Show);
    QVERIFY(dest.visibility("nsfl", true) == ConfigurableLabelStatus::Show);
    QVERIFY(dest.visibility("nudity", true) == ConfigurableLabelStatus::Show);
    QVERIFY(dest.visibility("sexual", true) == ConfigurableLabelStatus::Show);
    QVERIFY(dest.visibility("gore", true) == ConfigurableLabelStatus::Show);
    QVERIFY(dest.visibility("self-harm", true) == ConfigurableLabelStatus::Show);
    QVERIFY(dest.visibility("torture", true) == ConfigurableLabelStatus::Show);
    QVERIFY(dest.visibility("corpse", true) == ConfigurableLabelStatus::Show);
    QVERIFY(dest.visibility("icon-kkk", false) == ConfigurableLabelStatus::Show);
    QVERIFY(dest.visibility("icon-nazi", false) == ConfigurableLabelStatus::Show);
    QVERIFY(dest.visibility("icon-intolerant", false) == ConfigurableLabelStatus::Show);
    QVERIFY(dest.visibility("behavior-intolerant", false) == ConfigurableLabelStatus::Show);
    QVERIFY(dest.visibility("spam", false) == ConfigurableLabelStatus::Show);
    QVERIFY(dest.visibility("impersonation", false) == ConfigurableLabelStatus::Show);

    src.setEnableAdultContent(false);
    for (int i = 0; i < src.count(); i++) {
        src.setStatus(i, ConfigurableLabelStatus::Warning);
    }
    dest = src;
    QVERIFY(dest.enableAdultContent() == false);
    QVERIFY(dest.visibility("porn", true) == ConfigurableLabelStatus::Hide);
    QVERIFY(dest.visibility("nsfl", true) == ConfigurableLabelStatus::Hide);
    QVERIFY(dest.visibility("nudity", true) == ConfigurableLabelStatus::Hide);
    QVERIFY(dest.visibility("sexual", true) == ConfigurableLabelStatus::Hide);
    QVERIFY(dest.visibility("gore", true) == ConfigurableLabelStatus::Hide);
    QVERIFY(dest.visibility("self-harm", true) == ConfigurableLabelStatus::Hide);
    QVERIFY(dest.visibility("torture", true) == ConfigurableLabelStatus::Hide);
    QVERIFY(dest.visibility("corpse", true) == ConfigurableLabelStatus::Hide);
    QVERIFY(dest.visibility("icon-kkk", false) == ConfigurableLabelStatus::Warning);
    QVERIFY(dest.visibility("icon-nazi", false) == ConfigurableLabelStatus::Warning);
    QVERIFY(dest.visibility("icon-intolerant", false) == ConfigurableLabelStatus::Warning);
    QVERIFY(dest.visibility("behavior-intolerant", false) == ConfigurableLabelStatus::Warning);
    QVERIFY(dest.visibility("spam", false) == ConfigurableLabelStatus::Warning);
    QVERIFY(dest.visibility("impersonation", false) == ConfigurableLabelStatus::Warning);
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
        labels.setEnableAdultContent(false);
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
}

void atprotocol_test::test_ConfigurableLabels_mutedword()
{
    ConfigurableLabels labels;

    labels.insertMutedWord(0, "word3", QList<MutedWordTarget>() << MutedWordTarget::Content);
    labels.insertMutedWord(0, "word1", QList<MutedWordTarget>() << MutedWordTarget::Tag);
    labels.insertMutedWord(1, "word2",
                           QList<MutedWordTarget>()
                                   << MutedWordTarget::Content << MutedWordTarget::Tag);

    QVERIFY(labels.getMutedWordItem(0).value == "word1");
    QVERIFY(labels.getMutedWordItem(0).group == 0);
    QVERIFY(labels.getMutedWordItem(0).targets == QList<MutedWordTarget>() << MutedWordTarget::Tag);
    QVERIFY(labels.getMutedWordItem(1).value == "word2");
    QVERIFY(labels.getMutedWordItem(1).group == 0);
    QVERIFY(labels.getMutedWordItem(1).targets
            == QList<MutedWordTarget>() << MutedWordTarget::Content << MutedWordTarget::Tag);
    QVERIFY(labels.getMutedWordItem(2).value == "word3");
    QVERIFY(labels.getMutedWordItem(2).group == 0);
    QVERIFY(labels.getMutedWordItem(2).targets
            == QList<MutedWordTarget>() << MutedWordTarget::Content);

    labels.moveMutedWordItem(2, 0);
    QVERIFY(labels.getMutedWordItem(1).value == "word1");
    QVERIFY(labels.getMutedWordItem(1).group == 0);
    QVERIFY(labels.getMutedWordItem(1).targets == QList<MutedWordTarget>() << MutedWordTarget::Tag);
    QVERIFY(labels.getMutedWordItem(2).value == "word2");
    QVERIFY(labels.getMutedWordItem(2).group == 0);
    QVERIFY(labels.getMutedWordItem(2).targets
            == QList<MutedWordTarget>() << MutedWordTarget::Content << MutedWordTarget::Tag);
    QVERIFY(labels.getMutedWordItem(0).value == "word3");
    QVERIFY(labels.getMutedWordItem(0).group == 0);
    QVERIFY(labels.getMutedWordItem(0).targets
            == QList<MutedWordTarget>() << MutedWordTarget::Content);
}

void atprotocol_test::test_ComAtprotoRepoCreateRecord_post()
{
    AtProtocolInterface::ComAtprotoRepoCreateRecord createrecord;
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
    AtProtocolInterface::ComAtprotoRepoCreateRecord createrecord;
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

void atprotocol_test::test_AppBskyFeedGetFeedGenerator()
{
    AtProtocolInterface::AppBskyFeedGetFeedGenerator generator;
    generator.setAccount(m_account);
    generator.setService(QString("http://localhost:%1/response").arg(m_listenPort));

    QSignalSpy spy(&generator, SIGNAL(finished(bool)));
    generator.getFeedGenerator("at://did:plc:42fxwa2jeumqzzggx/app.bsky.feed.generator/aaagrsa");
    spy.wait();
    QVERIFY2(spy.count() == 1, QString("spy.count()=%1").arg(spy.count()).toUtf8());

    QVERIFY(generator.generatorView().uri
            == "at://did:plc:42fxwa2jeumqzzggx/app.bsky.feed.generator/aaagrsa");
    QVERIFY(generator.generatorView().cid == "bafyreib7pgajpklwexy4lidm");
    QVERIFY(generator.generatorView().did == "did:web:view.bsky.social");
    QVERIFY(generator.generatorView().displayName == "view:displayName");
    QVERIFY(generator.generatorView().description == "view:description");
    QVERIFY(generator.generatorView().avatar == "https://cdn.bsky.social/view_avator.jpeg");
    QVERIFY(generator.generatorView().creator.did == "did:plc:42fxwa2jeumqzzggxj");
    QVERIFY(generator.generatorView().creator.handle == "creator.bsky.social");
    QVERIFY(generator.generatorView().creator.displayName == "creator:displayName");
    QVERIFY(generator.generatorView().creator.avatar
            == "https://cdn.bsky.social/creator_avator.jpeg");
}

void atprotocol_test::test_ServiceUrl()
{
    AtProtocolInterface::ComAtprotoServerCreateSession session;
    session.setService(m_service + "/");

    {
        QSignalSpy spy(&session, SIGNAL(finished(bool)));
        session.create("hoge", "fuga");
        spy.wait();
        QVERIFY2(spy.count() == 1, QString("spy.count()=%1").arg(spy.count()).toUtf8());
        QList<QVariant> arguments = spy.takeFirst();
        QVERIFY(arguments.at(0).toBool());
    }
}

void atprotocol_test::test_ComAtprotoRepoGetRecord_profile()
{
    AtProtocolInterface::ComAtprotoRepoGetRecord record;
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
            LexiconsTypeUnknown::fromQVariant<AppBskyActorProfile::Main>(record.record());
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
    AtProtocolInterface::ComAtprotoRepoPutRecord record;

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
        record.profile(avatar, banner, "description", "display name",
                       "bafyreie3ckzfk5xadlunbotovrffkhsfb2hdnr7bujofy2bb5ro45elcmy");
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
        record.profile(avatar, banner, "epub\nLeME", "IoriAYANE",
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

    QVERIFY(cache->getListNames("hoge", "hoge") == QStringList());

    QVERIFY(cache->has("account1") == false);
    QVERIFY(cache->has("account2") == false);
    QVERIFY(cache->has("account3") == false);

    cache->addItem("account1", "user1", "list1", "list1_uri", "list1_item1_uri");
    QVERIFY(cache->getListNames("account1", "user1") == QStringList() << "list1");
    QVERIFY(cache->getListUris("account1", "user1") == QStringList() << "list1_uri");

    cache->addItem("account1", "user1", "list1", "list1_uri", "list1_itemX_uri");
    QVERIFY(cache->getListNames("account1", "user1") == QStringList() << "list1");
    QVERIFY(cache->getListUris("account1", "user1") == QStringList() << "list1_uri");

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
    QVERIFY(cache->getListNames("account3", "user1") == QStringList() << "account3list1");
    QVERIFY(cache->getListUris("account3", "user1") == QStringList() << "account3list1_uri");

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
    QVERIFY(cache->getListNames("account3", "user2") == QStringList() << "account3list1");
    QVERIFY(cache->getListUris("account3", "user2") == QStringList() << "account3list1_uri");

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
    QVERIFY(cache->getListNames("account2", "user1") == QStringList() << "a2list1");
    QVERIFY(cache->getListUris("account2", "user1") == QStringList() << "a2list1_uri");

    cache->addItem("account2", "user1", "a2list2", "a2list2_uri", "a2list2_item1_uri");
    QVERIFY(cache->getListNames("account2", "user1")
            == QStringList() << "a2list1"
                             << "a2list2");
    QVERIFY(cache->getListUris("account2", "user1")
            == QStringList() << "a2list1_uri"
                             << "a2list2_uri");

    cache->addItem("account2", "user2", "a2list2", "a2list2_uri", "a2list2_item1_uri");
    QVERIFY(cache->getListNames("account2", "user2") == QStringList() << "a2list2");
    QVERIFY(cache->getListUris("account2", "user2") == QStringList() << "a2list2_uri");

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
    QVERIFY(cache->has("account1") == true);
    QVERIFY(cache->has("account2") == false);
    QVERIFY(cache->has("account3") == true);
    QVERIFY(cache->getListNames("account1", "user1")
            == QStringList() << "list1"
                             << "list2");
    QVERIFY(cache->getListUris("account1", "user1")
            == QStringList() << "list1_uri"
                             << "list2_uri");
    QVERIFY(cache->getListNames("account2", "user2") == QStringList());
    QVERIFY(cache->getListUris("account2", "user2") == QStringList());
    QVERIFY(cache->getListNames("account3", "user1")
            == QStringList() << "account3list1"
                             << "account3list3");
    QVERIFY(cache->getListUris("account3", "user1")
            == QStringList() << "account3list1_uri"
                             << "account3list3_uri");
    cache->clear();
    QVERIFY(cache->getListNames("account1", "user2") == QStringList());
    QVERIFY(cache->getListUris("account1", "user2") == QStringList());
    QVERIFY(cache->getListNames("account2", "user2") == QStringList());
    QVERIFY(cache->getListUris("account2", "user2") == QStringList());
    QVERIFY(cache->getListNames("account3", "user2") == QStringList());
    QVERIFY(cache->getListUris("account3", "user2") == QStringList());

    QVERIFY(cache->has("account1") == false);
    QVERIFY(cache->has("account2") == false);
    QVERIFY(cache->has("account3") == false);

    QVERIFY(cache->getListNames("hoge", "hoge") == QStringList());
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
    } else {
        qDebug() << path;
        QVERIFY(false);
    }

    QJsonDocument json_doc = QJsonDocument::fromJson(body);

    if (json_doc_expect.object() != json_doc.object()) {
        qDebug().noquote().nospace() << QString("\nexpect:%1\nactual:%2\n")
                                                .arg(json_doc_expect.toJson(), json_doc.toJson());
    }
    QVERIFY(json_doc_expect.object() == json_doc.object());
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
    QVERIFY(json_doc_expect.object() == json_doc.object());
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
    QVERIFY(json_expect == json_actual);
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
