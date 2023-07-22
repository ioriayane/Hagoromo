#include <QtTest>
#include <QCoreApplication>
#include <QTemporaryFile>

#include "webserver.h"
#include "atprotocol/com/atproto/server/comatprotoservercreatesession.h"
#include "atprotocol/app/bsky/feed/appbskyfeedgettimeline.h"
#include "tools/opengraphprotocol.h"
#include "atprotocol/lexicons_func_unknown.h"

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

private:
    WebServer m_mockServer;
    quint16 m_listenPort;
    QString m_service;
    AtProtocolInterface::AccountData m_account;
};

atprotocol_test::atprotocol_test()
{
    m_listenPort = m_mockServer.listen(QHostAddress::LocalHost, 0);
    m_service = QString("http://localhost:%1/response").arg(m_listenPort);

    connect(&m_mockServer, &WebServer::receivedPost,
            [=](const QHttpServerRequest &request, bool &result, QString &json) {
                QFile *file = new QFile(":" + request.url().path());
                if (file->open(QFile::ReadOnly)) {
                    json = file->readAll();
                } else {
                    json = "{}";
                }
                result = true;
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

    QSignalSpy spy(&session, SIGNAL(finished(bool)));
    session.create("hoge", "fuga");
    spy.wait();
    QVERIFY(spy.count() == 1);

    QList<QVariant> arguments = spy.takeFirst();
    QVERIFY(arguments.at(0).toBool());

    QVERIFY(session.did() == "did:plc:ipj5qejfoqu6eukvt72uhyit");
    QVERIFY(session.handle() == "ioriayane.relog.tech");
    QVERIFY(session.email() == "iori.ayane@gmail.com");
    QVERIFY(session.accessJwt() == "hoge hoge accessJwt");
    QVERIFY(session.refreshJwt() == "hoge hoge refreshJwt");

    m_account = session.account();
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

        QVERIFY2(ogp.uri() == "http://localhost/response/ogp/file5.html", ogp.uri().toLocal8Bit());
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

QTEST_MAIN(atprotocol_test)

#include "tst_atprotocol_test.moc"
