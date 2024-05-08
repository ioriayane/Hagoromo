#include <QtTest>
#include <QCoreApplication>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>

#include "webserver.h"
#include "log/logmanager.h"
#include "log/logoperator.h"
#include "log/logstatisticslistmodel.h"
#include "log/logfeedlistmodel.h"

class log_test : public QObject
{
    Q_OBJECT

public:
    log_test();
    ~log_test();

private slots:
    void initTestCase();
    void cleanupTestCase();
    void test_LogManager();
    void test_LogManager_daily();
    void test_LogManager_monthly();
    void test_LogManager_select();
    void test_LogManager_statistics();
    void test_LogOperator();
    void test_LogStatisticsListModel();
    void test_LogFeedListModel();

private:
    WebServer m_mockServer;
    quint16 m_listenPort;
    QString m_service;
};

log_test::log_test()
{
    qRegisterMetaType<QList<TotalItem>>("QList<TotalItem>");

    QCoreApplication::setOrganizationName(QStringLiteral("relog"));
    QCoreApplication::setApplicationName(QStringLiteral("Hagoromo"));

    m_listenPort = m_mockServer.listen(QHostAddress::LocalHost, 0);
    m_service = QString("http://localhost:%1/response").arg(m_listenPort);

    connect(&m_mockServer, &WebServer::receivedPost,
            [=](const QHttpServerRequest &request, bool &result, QString &json) {
                QVERIFY(false);
            });
}

log_test::~log_test() { }

void log_test::initTestCase() { }

void log_test::cleanupTestCase() { }

void log_test::test_LogManager()
{
    LogManager manager;
    QString did("did:plc:log_manager_test");

    manager.clearDb(did);

    {
        QSignalSpy spy(&manager, SIGNAL(finished(bool)));
        manager.update("", "");
        spy.wait();
        QVERIFY2(spy.count() == 1, QString("spy.count()=%1").arg(spy.count()).toUtf8());

        QList<QVariant> arguments = spy.takeFirst();
        QVERIFY(arguments.at(0).toBool() == false);
    }

    {
        QSignalSpy spy(&manager, SIGNAL(finished(bool)));
        manager.update(m_service, did);
        spy.wait();
        QVERIFY2(spy.count() == 1, QString("spy.count()=%1").arg(spy.count()).toUtf8());

        QList<QVariant> arguments = spy.takeFirst();
        QVERIFY(arguments.at(0).toBool() == true);
    }

    {
        QSignalSpy spy(&manager, SIGNAL(finished(bool)));
        manager.update(m_service, did);
        spy.wait();
        QVERIFY2(spy.count() == 1, QString("spy.count()=%1").arg(spy.count()).toUtf8());

        QList<QVariant> arguments = spy.takeFirst();
        QVERIFY(arguments.at(0).toBool() == true);
    }
}

void log_test::test_LogManager_daily()
{
    LogManager manager;
    QString did("did:plc:log_manager_test");

    QFile file(":/data/repo/daily.json");
    QVERIFY(file.open(QFile::ReadOnly));
    QJsonDocument doc = QJsonDocument::fromJson(file.readAll());
    file.close();

    int max = 0;
    QList<TotalItem> except;
    for (const auto &value : doc.array()) {
        TotalItem item;
        item.name = value.toObject().value("name").toString();
        item.count = value.toObject().value("count").toInt();
        max = (item.count > max) ? item.count : max;
        except.append(item);
    }

    {
        QSignalSpy spy(&manager, SIGNAL(finishedTotals(const QList<TotalItem> &, const int)));
        emit manager.dailyTotals(did);
        spy.wait();
        QVERIFY2(spy.count() == 1, QString("spy.count()=%1").arg(spy.count()).toUtf8());

        QList<QVariant> arguments = spy.takeFirst();
        QList<TotalItem> list = qvariant_cast<QList<TotalItem>>(arguments.at(0));

        QVERIFY2(max == arguments.at(1).toInt(),
                 QString("%1 == %2").arg(max).arg(arguments.at(1).toInt()).toLocal8Bit());
        QVERIFY2(except.length() == list.length(),
                 QString("%1 == %2").arg(except.length()).arg(list.length()).toLocal8Bit());
        for (int i = 0; i < list.length(); i++) {
            QVERIFY2(except.at(i).name == list.at(i).name,
                     QString("%1 == %2").arg(except.at(i).name).arg(list.at(0).name).toLocal8Bit());
            QVERIFY2(except.at(i).count == list.at(i).count,
                     QString("%1 == %2")
                             .arg(except.at(i).count)
                             .arg(list.at(0).count)
                             .toLocal8Bit());
        }
    }
}

void log_test::test_LogManager_monthly()
{
    LogManager manager;
    QString did("did:plc:log_manager_test");

    QFile file(":/data/repo/monthly.json");
    QVERIFY(file.open(QFile::ReadOnly));
    QJsonDocument doc = QJsonDocument::fromJson(file.readAll());
    file.close();

    int max = 0;
    QList<TotalItem> except;
    for (const auto &value : doc.array()) {
        TotalItem item;
        item.name = value.toObject().value("name").toString();
        item.count = value.toObject().value("count").toInt();
        max = (item.count > max) ? item.count : max;
        except.append(item);
    }

    {
        QSignalSpy spy(&manager, SIGNAL(finishedTotals(const QList<TotalItem> &, const int)));
        emit manager.monthlyTotals(did);
        spy.wait();
        QVERIFY2(spy.count() == 1, QString("spy.count()=%1").arg(spy.count()).toUtf8());

        QList<QVariant> arguments = spy.takeFirst();
        QList<TotalItem> list = qvariant_cast<QList<TotalItem>>(arguments.at(0));

        QVERIFY2(max == arguments.at(1).toInt(),
                 QString("%1 == %2").arg(max).arg(arguments.at(1).toInt()).toLocal8Bit());
        QVERIFY2(except.length() == list.length(),
                 QString("%1 == %2").arg(except.length()).arg(list.length()).toLocal8Bit());
        for (int i = 0; i < list.length(); i++) {
            QVERIFY2(except.at(i).name == list.at(i).name,
                     QString("%1 == %2").arg(except.at(i).name).arg(list.at(0).name).toLocal8Bit());
            QVERIFY2(except.at(i).count == list.at(i).count,
                     QString("%1 == %2")
                             .arg(except.at(i).count)
                             .arg(list.at(0).count)
                             .toLocal8Bit());
        }
    }
}

void log_test::test_LogManager_select()
{
    int i = 0;
    AtProtocolInterface::AccountData account;
    account.service = m_service;
    account.did = "did:plc:log_manager_test";
    account.handle = "log.manager.test";
    account.accessJwt = "access jwt";
    LogManager manager;
    QString did("did:plc:log_manager_test");
    manager.setAccount(account);

    manager.setService(account.service + "/posts/1");
    {
        QSignalSpy spy(&manager, SIGNAL(finishedSelection(const QString &)));
        QSignalSpy spy2(&manager, SIGNAL(finishedSelectionPosts()));
        emit manager.selectRecords(did, 0, "2024/04/18", QString(), 5);
        spy.wait();
        spy2.wait();
        QVERIFY2(spy.count() == 1, QString("spy.count()=%1").arg(spy.count()).toUtf8());
        QVERIFY2(spy2.count() == 1, QString("spy2.count()=%1").arg(spy2.count()).toUtf8());

        QList<QVariant> arguments = spy.takeFirst();

        QFile file(":/data/repo/select_1.json");
        QVERIFY(file.open(QFile::ReadOnly));
        QJsonDocument except = QJsonDocument::fromJson(file.readAll());
        file.close();

        bool check = (except.object()
                      == QJsonDocument::fromJson(arguments.at(0).toString().toUtf8()).object());
        if (!check) {
            qDebug().noquote().nospace()
                    << "except:" << except.toJson(QJsonDocument::JsonFormat::Compact);
            qDebug().noquote().nospace() << "actual:" << arguments.at(0).toString();
        }
        QVERIFY(check);

        qDebug() << "feedViewPosts:" << manager.feedViewPosts().length();
        QVERIFY2(manager.feedViewPosts().length() == 5,
                 QString::number(manager.feedViewPosts().length()).toLocal8Bit());

        i = 0;
        QVERIFY2(
                manager.feedViewPosts().at(i).post.uri
                        == "at://did:plc:mqxsuw5b5rhpwo4lw6iwlid5/app.bsky.feed.post/3kqfxfddc6u22",
                manager.feedViewPosts().at(i).post.uri.toLocal8Bit());
        i = 1;
        QVERIFY2(
                manager.feedViewPosts().at(i).post.uri
                        == "at://did:plc:mqxsuw5b5rhpwo4lw6iwlid5/app.bsky.feed.post/3kqfxf5lbjo2b",
                manager.feedViewPosts().at(i).post.uri.toLocal8Bit());
        i = 2;
        QVERIFY2(
                manager.feedViewPosts().at(i).post.uri
                        == "at://did:plc:mqxsuw5b5rhpwo4lw6iwlid5/app.bsky.feed.post/3kqfxex2a5c2a",
                manager.feedViewPosts().at(i).post.uri.toLocal8Bit());
        i = 3;
        QVERIFY2(
                manager.feedViewPosts().at(i).post.uri
                        == "at://did:plc:mqxsuw5b5rhpwo4lw6iwlid5/app.bsky.feed.post/3kqfxegyxbg2b",
                manager.feedViewPosts().at(i).post.uri.toLocal8Bit());
        i = 4;
        QVERIFY2(
                manager.feedViewPosts().at(i).post.uri
                        == "at://did:plc:mqxsuw5b5rhpwo4lw6iwlid5/app.bsky.feed.post/3kqfweqao2e2r",
                manager.feedViewPosts().at(i).post.uri.toLocal8Bit());
    }

    manager.setService(account.service + "/posts/2");
    {
        QSignalSpy spy(&manager, SIGNAL(finishedSelection(const QString &)));
        QSignalSpy spy2(&manager, SIGNAL(finishedSelectionPosts()));
        emit manager.selectRecords(did, 0, "2024/04/18", "2024-04-18T13:38:16.029Z", 5);
        spy.wait();
        spy2.wait();
        QVERIFY2(spy.count() == 1, QString("spy.count()=%1").arg(spy.count()).toUtf8());
        QVERIFY2(spy2.count() == 1, QString("spy2.count()=%1").arg(spy2.count()).toUtf8());

        QList<QVariant> arguments = spy.takeFirst();

        QFile file(":/data/repo/select_2.json");
        QVERIFY(file.open(QFile::ReadOnly));
        QJsonDocument except = QJsonDocument::fromJson(file.readAll());
        file.close();

        bool check = (except.object()
                      == QJsonDocument::fromJson(arguments.at(0).toString().toUtf8()).object());
        if (!check) {
            qDebug().noquote().nospace()
                    << "except:" << except.toJson(QJsonDocument::JsonFormat::Compact);
            qDebug().noquote().nospace() << "actual:" << arguments.at(0).toString();
        }
        QVERIFY(check);

        qDebug() << "feedViewPosts:" << manager.feedViewPosts().length();
        QVERIFY2(manager.feedViewPosts().length() == 5,
                 QString::number(manager.feedViewPosts().length()).toLocal8Bit());

        i = 0;
        QVERIFY2(
                manager.feedViewPosts().at(i).post.uri
                        == "at://did:plc:mqxsuw5b5rhpwo4lw6iwlid5/app.bsky.feed.post/3kqfweerkyx27",
                manager.feedViewPosts().at(i).post.uri.toLocal8Bit());
        i = 1;
        QVERIFY2(
                manager.feedViewPosts().at(i).post.uri
                        == "at://did:plc:mqxsuw5b5rhpwo4lw6iwlid5/app.bsky.feed.post/3kqfvyabmpg2n",
                manager.feedViewPosts().at(i).post.uri.toLocal8Bit());
        i = 2;
        QVERIFY2(
                manager.feedViewPosts().at(i).post.uri
                        == "at://did:plc:mqxsuw5b5rhpwo4lw6iwlid5/app.bsky.feed.post/3kqfvvohpdc2q",
                manager.feedViewPosts().at(i).post.uri.toLocal8Bit());
        i = 3;
        QVERIFY2(
                manager.feedViewPosts().at(i).post.uri
                        == "at://did:plc:mqxsuw5b5rhpwo4lw6iwlid5/app.bsky.feed.post/3kqfvuzmjoc2a",
                manager.feedViewPosts().at(i).post.uri.toLocal8Bit());
        i = 4;
        QVERIFY2(
                manager.feedViewPosts().at(i).post.uri
                        == "at://did:plc:mqxsuw5b5rhpwo4lw6iwlid5/app.bsky.feed.post/3kqfvsj5wx42r",
                manager.feedViewPosts().at(i).post.uri.toLocal8Bit());
    }

    // レコードにポストの情報が正しくフィードバックできていればREST
    // APIは呼ばないのでエラーにならないはず
    manager.setService(account.service + "/posts/1_dummy");
    {
        QSignalSpy spy(&manager, SIGNAL(finishedSelection(const QString &)));
        QSignalSpy spy2(&manager, SIGNAL(finishedSelectionPosts()));
        emit manager.selectRecords(did, 0, "2024/04/18", QString(), 5);
        spy.wait();
        spy2.wait();
        QVERIFY2(spy.count() == 1, QString("spy.count()=%1").arg(spy.count()).toUtf8());
        QVERIFY2(spy2.count() == 1, QString("spy2.count()=%1").arg(spy2.count()).toUtf8());

        QList<QVariant> arguments = spy.takeFirst();

        QFile file(":/data/repo/select_1.json");
        QVERIFY(file.open(QFile::ReadOnly));
        QJsonDocument except = QJsonDocument::fromJson(file.readAll());
        file.close();

        bool check = (except.object()
                      == QJsonDocument::fromJson(arguments.at(0).toString().toUtf8()).object());
        if (!check) {
            qDebug().noquote().nospace()
                    << "except:" << except.toJson(QJsonDocument::JsonFormat::Compact);
            qDebug().noquote().nospace() << "actual:" << arguments.at(0).toString();
        }
        QVERIFY(check);

        qDebug() << "feedViewPosts:" << manager.feedViewPosts().length();
        QVERIFY2(manager.feedViewPosts().length() == 5,
                 QString::number(manager.feedViewPosts().length()).toLocal8Bit());

        i = 0;
        QVERIFY2(
                manager.feedViewPosts().at(i).post.uri
                        == "at://did:plc:mqxsuw5b5rhpwo4lw6iwlid5/app.bsky.feed.post/3kqfxfddc6u22",
                manager.feedViewPosts().at(i).post.uri.toLocal8Bit());
        i = 1;
        QVERIFY2(
                manager.feedViewPosts().at(i).post.uri
                        == "at://did:plc:mqxsuw5b5rhpwo4lw6iwlid5/app.bsky.feed.post/3kqfxf5lbjo2b",
                manager.feedViewPosts().at(i).post.uri.toLocal8Bit());
        i = 2;
        QVERIFY2(
                manager.feedViewPosts().at(i).post.uri
                        == "at://did:plc:mqxsuw5b5rhpwo4lw6iwlid5/app.bsky.feed.post/3kqfxex2a5c2a",
                manager.feedViewPosts().at(i).post.uri.toLocal8Bit());
        i = 3;
        QVERIFY2(
                manager.feedViewPosts().at(i).post.uri
                        == "at://did:plc:mqxsuw5b5rhpwo4lw6iwlid5/app.bsky.feed.post/3kqfxegyxbg2b",
                manager.feedViewPosts().at(i).post.uri.toLocal8Bit());
        i = 4;
        QVERIFY2(
                manager.feedViewPosts().at(i).post.uri
                        == "at://did:plc:mqxsuw5b5rhpwo4lw6iwlid5/app.bsky.feed.post/3kqfweqao2e2r",
                manager.feedViewPosts().at(i).post.uri.toLocal8Bit());
    }

    manager.setService(account.service + "/posts/3");
    {
        QSignalSpy spy(&manager, SIGNAL(finishedSelection(const QString &)));
        QSignalSpy spy2(&manager, SIGNAL(finishedSelectionPosts()));
        emit manager.selectRecords(did, 1, "2023/07", QString(), 5);
        spy.wait();
        spy2.wait();
        QVERIFY2(spy.count() == 1, QString("spy.count()=%1").arg(spy.count()).toUtf8());
        QVERIFY2(spy2.count() == 1, QString("spy2.count()=%1").arg(spy2.count()).toUtf8());

        QList<QVariant> arguments = spy.takeFirst();

        QFile file(":/data/repo/select_3.json");
        QVERIFY(file.open(QFile::ReadOnly));
        QJsonDocument except = QJsonDocument::fromJson(file.readAll());
        file.close();

        bool check = (except.object()
                      == QJsonDocument::fromJson(arguments.at(0).toString().toUtf8()).object());
        if (!check) {
            qDebug().noquote().nospace()
                    << "except:" << except.toJson(QJsonDocument::JsonFormat::Compact);
            qDebug().noquote().nospace() << "actual:" << arguments.at(0).toString();
        }
        QVERIFY(check);

        qDebug() << "feedViewPosts:" << manager.feedViewPosts().length();
        QVERIFY2(manager.feedViewPosts().length() == 5,
                 QString::number(manager.feedViewPosts().length()).toLocal8Bit());

        i = 0;
        QVERIFY2(
                manager.feedViewPosts().at(i).post.uri
                        == "at://did:plc:mqxsuw5b5rhpwo4lw6iwlid5/app.bsky.feed.post/3k3qetyacqs2g",
                manager.feedViewPosts().at(i).post.uri.toLocal8Bit());
        i = 1;
        QVERIFY2(
                manager.feedViewPosts().at(i).post.uri
                        == "at://did:plc:mqxsuw5b5rhpwo4lw6iwlid5/app.bsky.feed.post/3k3ofnldrp52u",
                manager.feedViewPosts().at(i).post.uri.toLocal8Bit());
        i = 2;
        QVERIFY2(
                manager.feedViewPosts().at(i).post.uri
                        == "at://did:plc:mqxsuw5b5rhpwo4lw6iwlid5/app.bsky.feed.post/3k3e7aciteg2m",
                manager.feedViewPosts().at(i).post.uri.toLocal8Bit());
        i = 3;
        QVERIFY2(
                manager.feedViewPosts().at(i).post.uri
                        == "at://did:plc:mqxsuw5b5rhpwo4lw6iwlid5/app.bsky.feed.post/3k34hcgybdx27",
                manager.feedViewPosts().at(i).post.uri.toLocal8Bit());
        i = 4;
        QVERIFY2(
                manager.feedViewPosts().at(i).post.uri
                        == "at://did:plc:mqxsuw5b5rhpwo4lw6iwlid5/app.bsky.feed.post/3k32c2xekas2h",
                manager.feedViewPosts().at(i).post.uri.toLocal8Bit());
    }
}

void log_test::test_LogManager_statistics()
{
    LogManager manager;
    QString did("did:plc:log_manager_test");

    QFile file(":/data/repo/statistics.json");
    QVERIFY(file.open(QFile::ReadOnly));
    QJsonDocument doc = QJsonDocument::fromJson(file.readAll());
    file.close();

    int max = 0;
    QList<TotalItem> except;
    for (const auto &value : doc.array()) {
        TotalItem item;
        item.group = value.toObject().value("group").toString();
        item.name = value.toObject().value("name").toString();
        item.count = value.toObject().value("count").toInt();
        except.append(item);
    }

    {
        QSignalSpy spy(&manager, SIGNAL(finishedTotals(const QList<TotalItem> &, const int)));
        emit manager.statistics(did);
        spy.wait();
        QVERIFY2(spy.count() == 1, QString("spy.count()=%1").arg(spy.count()).toUtf8());

        QList<QVariant> arguments = spy.takeFirst();
        QList<TotalItem> list = qvariant_cast<QList<TotalItem>>(arguments.at(0));

        QVERIFY2(max == arguments.at(1).toInt(),
                 QString("%1 == %2").arg(max).arg(arguments.at(1).toInt()).toLocal8Bit());
        QVERIFY2(except.length() == list.length(),
                 QString("%1 == %2").arg(except.length()).arg(list.length()).toLocal8Bit());
        for (int i = 0; i < list.length(); i++) {
            QVERIFY2(except.at(i).group == list.at(i).group,
                     QString("%1, %2 == %3")
                             .arg(i)
                             .arg(except.at(i).group)
                             .arg(list.at(i).group)
                             .toLocal8Bit());
            QVERIFY2(except.at(i).name == list.at(i).name,
                     QString("%1, %2 == %3")
                             .arg(i)
                             .arg(except.at(i).name)
                             .arg(list.at(i).name)
                             .toLocal8Bit());
            QVERIFY2(except.at(i).count == list.at(i).count,
                     QString("%1, %2 == %3")
                             .arg(i)
                             .arg(except.at(i).count)
                             .arg(list.at(i).count)
                             .toLocal8Bit());
        }
    }
}

void log_test::test_LogOperator()
{
    LogOperator ope;

    ope.setService(m_service);
    ope.setDid("did:plc:log_operator_test");

    ope.clear();

    {
        QSignalSpy spy(&ope, SIGNAL(finished(bool)));
        emit ope.getLatest();
        spy.wait();
        QVERIFY2(spy.count() == 1, QString("spy.count()=%1").arg(spy.count()).toUtf8());

        QList<QVariant> arguments = spy.takeFirst();
        QVERIFY(arguments.at(0).toBool() == true);
    }
}

void log_test::test_LogStatisticsListModel()
{
    LogStatisticsListModel model;
    model.setDid("did:plc:log_operator_test");

    {
        QSignalSpy spy(&model, SIGNAL(finished()));
        model.getLatest();
        spy.wait();
        QVERIFY2(spy.count() == 1, QString("spy.count()=%1").arg(spy.count()).toUtf8());
    }

    QVERIFY2(model.rowCount() == 16, QString("%1").arg(model.rowCount()).toLocal8Bit());
    QVERIFY2(model.item(0, LogStatisticsListModel::NameRole).toString() == "Total number of posts",
             model.item(0, LogStatisticsListModel::NameRole).toString().toLocal8Bit());
    QVERIFY2(model.item(0, LogStatisticsListModel::CountRole).toInt() == 439,
             model.item(0, LogStatisticsListModel::CountRole).toString().toLocal8Bit());

    QVERIFY2(model.item(3, LogStatisticsListModel::NameRole).toString() == "Total number of likes",
             model.item(3, LogStatisticsListModel::NameRole).toString().toLocal8Bit());
    QVERIFY2(model.item(3, LogStatisticsListModel::CountRole).toInt() == 12,
             model.item(3, LogStatisticsListModel::CountRole).toString().toLocal8Bit());
}

void log_test::test_LogFeedListModel()
{
    int i = 0;
    AtProtocolInterface::AccountData account;
    account.service = m_service;
    account.did = "did:plc:log_manager_test";
    account.handle = "log.manager.test";
    account.accessJwt = "access jwt";

    LogFeedListModel model;
    model.setAccount(account.service + "/posts/10", account.did, account.handle, account.email,
                     account.accessJwt, account.refreshJwt);
    model.setTargetDid(account.did);
    model.setTargetHandle(account.handle);
    model.setTargetAvatar("test_avatar.jpg");
    model.setSelectCondition("2024/03/28");
    model.setFeedType(LogFeedListModel::LogFeedListModelFeedType::DailyFeedType);

    {
        QSignalSpy spy(&model, SIGNAL(finished(bool)));
        model.getLatest();
        spy.wait();
        QVERIFY2(spy.count() == 1, QString("spy.count()=%1").arg(spy.count()).toUtf8());

        QList<QVariant> arguments = spy.takeFirst();
        QVERIFY(arguments.at(0).toBool() == true);
    }

    QVERIFY(model.running() == false);
    QVERIFY2(model.rowCount() == 5, QString::number(model.rowCount()).toLocal8Bit());

    i = 1;
    QVERIFY2(model.item(i, LogFeedListModel::CidRole).toString()
                     == "bafyreibl6vp7vczey2fxae5gqw2xfhb4cmnd6lcez4d6hlbrll3p6fg4o4",
             model.item(i, LogFeedListModel::CidRole).toString().toLocal8Bit());
    QVERIFY2(model.item(i, LogFeedListModel::DisplayNameRole).toString() == "iori2",
             model.item(i, LogFeedListModel::DisplayNameRole).toString().toLocal8Bit());
    QVERIFY2(model.item(i, LogFeedListModel::HandleRole).toString() == "ioriayane2.bsky.social",
             model.item(i, LogFeedListModel::HandleRole).toString().toLocal8Bit());
    QVERIFY2(model.item(i, LogFeedListModel::AvatarRole).toString()
                     == "https://cdn.bsky.app/img/avatar/plain/did:plc:mqxsuw5b5rhpwo4lw6iwlid5/"
                        "bafkreiaeoiy6fqjypbhbcrb3jdlnjtpnwri5wa6jrvbwxtbtey6synwxr4@jpeg",
             model.item(i, LogFeedListModel::AvatarRole).toString().toLocal8Bit());
    QVERIFY2(model.item(i, LogFeedListModel::RecordTextPlainRole).toString() == "tst",
             model.item(i, LogFeedListModel::RecordTextPlainRole).toString().toLocal8Bit());
    QVERIFY2(model.item(i, LogFeedListModel::HasQuoteRecordRole).toBool() == true,
             model.item(i, LogFeedListModel::HasQuoteRecordRole).toString().toLocal8Bit());
    QVERIFY2(model.item(i, LogFeedListModel::QuoteRecordCidRole).toString()
                     == "bafyreic2q7ofjjen2sgyempgdbpveopqja537j7jauss4epdlmsfplrdsy",
             model.item(i, LogFeedListModel::QuoteRecordCidRole).toString().toLocal8Bit());
    QVERIFY2(model.item(i, LogFeedListModel::QuoteRecordHandleRole).toString()
                     == "ioriayane.relog.tech",
             model.item(i, LogFeedListModel::QuoteRecordHandleRole).toString().toLocal8Bit());
    QVERIFY2(model.item(i, LogFeedListModel::HasExternalLinkRole).toBool() == true,
             model.item(i, LogFeedListModel::HasExternalLinkRole).toString().toLocal8Bit());
    QVERIFY2(model.item(i, LogFeedListModel::ExternalLinkThumbRole).toString()
                     == "https://cdn.bsky.app/img/feed_thumbnail/plain/"
                        "did:plc:mqxsuw5b5rhpwo4lw6iwlid5/"
                        "bafkreicyv4kxm3gjayv2farf4lwhzhh6hdf3l6lsyac2ww3mphgrm6dosq@jpeg",
             model.item(i, LogFeedListModel::ExternalLinkThumbRole).toString().toLocal8Bit());
}

QTEST_MAIN(log_test)

#include "tst_log_test.moc"
