#include <QtTest>
#include <QCoreApplication>
#include <QTemporaryFile>

#include "webserver.h"
#include "atprotocol/com/atproto/server/comatprotoservercreatesession.h"
#include "atprotocol/com/atproto/repo/comatprotorepocreaterecord.h"
#include "atprotocol/app/bsky/feed/appbskyfeedgettimeline.h"
#include "atprotocol/app/bsky/feed/appbskyfeedgetfeedgenerator.h"
#include "tools/opengraphprotocol.h"
#include "atprotocol/lexicons_func_unknown.h"
#include "tools/configurablelabels.h"
#include "tools/imagecompressor.h"

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
    void test_ComAtprotoRepoCreateRecord_post();
    void test_AppBskyFeedGetFeedGenerator();
    void test_ImageCompressor();

private:
    void test_putPreferences(const QString &path, const QByteArray &body);
    QJsonDocument loadJson(const QString &path);

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
                if (request.url().path()
                            == "/response/labels/save/1/xrpc/app.bsky.actor.putPreferences"
                    || request.url().path()
                            == "/response/labels/save/2/xrpc/app.bsky.actor.putPreferences"
                    || request.url().path()
                            == "/response/labels/save/3/xrpc/app.bsky.actor.putPreferences") {
                    test_putPreferences(request.url().path(), request.body());
                    json = "{}";
                    result = true;
                } else if (request.url().path() == "/response/xrpc/com.atproto.repo.createRecord") {
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
        QVERIFY(spy.count() == 1);
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
        QVERIFY(spy.count() == 1);
        QList<QVariant> arguments = spy.takeFirst();
        QVERIFY(arguments.at(0).toBool() == false);
    }

    QVERIFY(session.errorCode() == "RateLimitExceeded");
    QVERIFY(session.errorMessage()
            == "Rate Limit "
               "Exceeded\n\nratelimit-limit:30\nratelimit-remaining:10\nratelimit-reset:2023/09/17 "
               "10:31:07\nratelimit-policy:30;w=300");
    QVERIFY(session.replyJson()
            == "{\"error\":\"RateLimitExceeded\",\"message\":\"Rate Limit Exceeded\"}");
}

void atprotocol_test::test_OpenGraphProtocol()
{
    OpenGraphProtocol ogp;

    {
        QSignalSpy spy(&ogp, SIGNAL(finished(bool)));
        ogp.getData(m_service + "/ogp/file1.html");
        spy.wait();
        QVERIFY(spy.count() == 1);

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
        QVERIFY(spy.count() == 1);

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
        QVERIFY(spy.count() == 1);

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
        QVERIFY(spy.count() == 1);

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
        QVERIFY(spy.count() == 1);

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
        QVERIFY(spy.count() == 1);

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
        QVERIFY(spy.count() == 1);

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
        QVERIFY(spy.count() == 1);

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
        QVERIFY(spy.count() == 1);

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
        QVERIFY(spy.count() == 1);

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
    QVERIFY(spy.count() == 1);

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
        QVERIFY(spy.count() == 1);
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
    {
        labels.setService(QString("http://localhost:%1/response/labels/show").arg(m_listenPort));
        QSignalSpy spy(&labels, SIGNAL(finished(bool)));
        labels.load();
        spy.wait();
        QVERIFY(spy.count() == 1);
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
    {
        labels.setService(
                QString("http://localhost:%1/response/labels/show_adult_false").arg(m_listenPort));
        QSignalSpy spy(&labels, SIGNAL(finished(bool)));
        labels.load();
        spy.wait();
        QVERIFY(spy.count() == 1);
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
    {
        labels.setService(QString("http://localhost:%1/response/labels/warn").arg(m_listenPort));
        QSignalSpy spy(&labels, SIGNAL(finished(bool)));
        labels.load();
        spy.wait();
        QVERIFY(spy.count() == 1);
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
    labels.setAccount(m_account);
    {
        labels.setService(QString("http://localhost:%1/response/labels/save/1").arg(m_listenPort));
        QSignalSpy spy(&labels, SIGNAL(finished(bool)));
        labels.save();
        spy.wait();
        QVERIFY(spy.count() == 1);
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
        QVERIFY(spy.count() == 1);
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
        QVERIFY(spy.count() == 1);
        QList<QVariant> arguments = spy.takeFirst();
        QVERIFY(arguments.at(0).toBool());
    }
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
    QVERIFY(spy.count() == 1);
    QList<QVariant> arguments = spy.takeFirst();
    QVERIFY(arguments.at(0).toBool());
}

void atprotocol_test::test_AppBskyFeedGetFeedGenerator()
{
    AtProtocolInterface::AppBskyFeedGetFeedGenerator generator;
    generator.setAccount(m_account);
    generator.setService(QString("http://localhost:%1/response").arg(m_listenPort));

    QSignalSpy spy(&generator, SIGNAL(finished(bool)));
    generator.getFeedGenerator("at://did:plc:42fxwa2jeumqzzggx/app.bsky.feed.generator/aaagrsa");
    spy.wait();
    QVERIFY(spy.count() == 1);

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

void atprotocol_test::test_ImageCompressor()
{
    ImageCompressor comp;

    {
        QSignalSpy spy(&comp, SIGNAL(compressed(QString)));
        comp.compress(":/data/images/image01.jpg");
        spy.wait();
        QVERIFY(spy.count() == 1);
        QList<QVariant> arguments = spy.takeFirst();
        QString new_path = arguments.at(0).toString();
        QFileInfo info(new_path);
        QImage img(new_path);
        qDebug() << "new_path" << new_path << info.size() << img.width() << img.height();
        QVERIFY(QFile::exists(new_path));
        QVERIFY(info.size() < 1000000);
        QVERIFY(img.width() == 4000);
        QVERIFY(img.height() == 3000);
        QFile file(new_path);
        QVERIFY(file.open(QFile::ReadOnly));
        QByteArray b = file.readAll();
        QVERIFY(b.size() == info.size());
        file.close();
    }

    {
        QSignalSpy spy(&comp, SIGNAL(compressed(QString)));
        comp.compress(":/data/images/image02.jpg");
        spy.wait();
        QVERIFY(spy.count() == 1);
        QList<QVariant> arguments = spy.takeFirst();
        QString new_path = arguments.at(0).toString();
        QFileInfo info(new_path);
        QImage img(new_path);
        qDebug() << "new_path" << new_path << info.size() << img.width() << img.height();
        QVERIFY(QFile::exists(new_path));
        QVERIFY(info.size() < 1000000);
        QVERIFY(img.width() == 4000);
        QVERIFY(img.height() == 3000);
        QFile file(new_path);
        QVERIFY(file.open(QFile::ReadOnly));
        QByteArray b = file.readAll();
        QVERIFY(b.size() == info.size());
        file.close();
    }

    {
        QSignalSpy spy(&comp, SIGNAL(compressed(QString)));
        comp.compress(":/data/images/image03.png");
        spy.wait();
        QVERIFY(spy.count() == 1);
        QList<QVariant> arguments = spy.takeFirst();
        QString new_path = arguments.at(0).toString();
        QFileInfo info(new_path);
        QImage img(new_path);
        qDebug() << "new_path" << new_path << info.size() << img.width() << img.height();
        QVERIFY(QFile::exists(new_path));
        QVERIFY(info.size() < 1000000);
        QVERIFY(img.width() == 3600);
        QVERIFY(img.height() == 2700);
        QFile file(new_path);
        QVERIFY(file.open(QFile::ReadOnly));
        QByteArray b = file.readAll();
        QVERIFY(b.size() == info.size());
        file.close();
    }
}

void atprotocol_test::test_putPreferences(const QString &path, const QByteArray &body)
{
    QJsonDocument json_doc_expect;
    if (path.contains("/save/1/")) {
        json_doc_expect = loadJson(":/data/labels/save/1/app.bsky.actor.putPreferences");
    } else if (path.contains("/save/1/")) {
        json_doc_expect = loadJson(":/data/labels/save/2/app.bsky.actor.putPreferences");
    } else {
        json_doc_expect = loadJson(":/data/labels/save/3/app.bsky.actor.putPreferences");
    }

    QJsonDocument json_doc = QJsonDocument::fromJson(body);

    if (json_doc_expect.object() != json_doc.object()) {
        qDebug().noquote().nospace() << QString("\nexpect:%1\nactual:%2\n")
                                                .arg(json_doc_expect.toJson(), json_doc.toJson());
    }
    QVERIFY(json_doc_expect.object() == json_doc.object());
}

QJsonDocument atprotocol_test::loadJson(const QString &path)
{
    QFile file(path);
    if (file.open(QFile::ReadOnly)) {
        return QJsonDocument::fromJson(file.readAll());
    } else {
        return QJsonDocument();
    }
}

QTEST_MAIN(atprotocol_test)

#include "tst_atprotocol_test.moc"
