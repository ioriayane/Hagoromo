#include <QtTest>
#include <QCoreApplication>
#include <QJsonDocument>

#include "webserver.h"
#include "timeline/searchpostlistmodel.h"
#include "profile/searchprofilelistmodel.h"
#include "tools/accountmanager.h"

class search_test : public QObject
{
    Q_OBJECT

public:
    search_test();
    ~search_test();

private slots:
    void initTestCase();
    void cleanupTestCase();
    void test_SearchPostListModel();
    void test_SearchPostListModel_fallback_data();
    void test_SearchPostListModel_fallback();
    void test_SearchPostListModel_noFallback();
    void test_SearchProfileListModel();

private:
    WebServer m_mockServer;
    quint16 m_listenPort;
    QString m_service;
};

search_test::search_test()
{
    QCoreApplication::setOrganizationName(QStringLiteral("relog"));
    QCoreApplication::setApplicationName(QStringLiteral("Hagoromo_unittest"));

    m_listenPort = m_mockServer.listen(QHostAddress::LocalHost, 0);
    m_service = QString("http://127.0.0.1:%1/response").arg(m_listenPort);
}

search_test::~search_test() { }

void search_test::initTestCase()
{
    QCOMPARE_NE(m_listenPort, 0);
}

void search_test::cleanupTestCase() { }

void search_test::test_SearchPostListModel()
{
    QString uuid = AccountManager::getInstance()->updateAccount(
            QString(), m_service, "id", "pass", "did:plc:mqxsuw5b5rhpwo4lw6iwlid5", "handle",
            "email", "accessJwt", "refreshJwt", true);

    SearchPostListModel model;
    model.setAccount(uuid);
    model.setText("epub");

    QSignalSpy spy(&model, SIGNAL(runningChanged()));
    model.getLatest();
    spy.wait(10 * 1000);
    QCOMPARE(spy.count(), 2);

    QCOMPARE(model.rowCount(), 2);
    QStringList uris;
    for (int row = 0; row < model.rowCount(); row++) {
        uris.append(model.item(row, TimelineListModel::UriRole).toString());
    }
    uris.sort();
    QCOMPARE(uris,
             QStringList()
                     << "at://did:plc:ipj5qejfoqu6eukvt72uhyit/app.bsky.feed.post/3jvmr45wspt2j"
                     << "at://did:plc:ipj5qejfoqu6eukvt72uhyit/app.bsky.feed.post/3jvobyrhgem24");
}

void search_test::test_SearchPostListModel_fallback_data()
{
    QTest::addColumn<QString>("path");
    QTest::addColumn<QString>("did");

    // searchPostsV2が無いサービス(404)
    QTest::newRow("404") << "/v1only"
                         << "did:plc:search_fallback";
    // searchPostsV2を実装していないサービス(501 MethodNotImplemented)
    QTest::newRow("501") << "/v2unimplemented"
                         << "did:plc:search_fallback_501";
}

void search_test::test_SearchPostListModel_fallback()
{
    QFETCH(QString, path);
    QFETCH(QString, did);

    QString uuid = AccountManager::getInstance()->updateAccount(QString(), m_service + path, "id",
                                                                "pass", did, "handle", "email",
                                                                "accessJwt", "refreshJwt", true);

    SearchPostListModel model;
    model.setAccount(uuid);
    model.setText("epub");

    {
        QSignalSpy spy(&model, SIGNAL(runningChanged()));
        QSignalSpy spy_error(&model, SIGNAL(errorOccurred(const QString &, const QString &)));
        model.getLatest();
        spy.wait(10 * 1000);
        QCOMPARE(spy.count(), 2);
        // searchPostsに切り替えるのでエラーは通知しない
        QCOMPARE(spy_error.count(), 0);
    }
    QCOMPARE(model.rowCount(), 2);
    QStringList uris;
    for (int row = 0; row < model.rowCount(); row++) {
        uris.append(model.item(row, TimelineListModel::UriRole).toString());
    }
    uris.sort();
    QCOMPARE(uris,
             QStringList()
                     << "at://did:plc:73l5atmh7p3fn3xigbp6ao5x/app.bsky.feed.post/3kd4zxs3ovv26"
                     << "at://did:plc:73l5atmh7p3fn3xigbp6ao5x/app.bsky.feed.post/3kdvx4mkpb22f");

    // searchPostsV2が使えるサービスになっても一度切り替えたらsearchPostsを使い続ける
    AccountManager::getInstance()->updateAccount(uuid, m_service, "id", "pass", did, "handle",
                                                 "email", "accessJwt", "refreshJwt", true);
    {
        QSignalSpy spy(&model, SIGNAL(runningChanged()));
        model.getLatest();
        spy.wait(10 * 1000);
        QCOMPARE(spy.count(), 2);
    }
    QCOMPARE(model.rowCount(), 2);
    QCOMPARE(model.item(0, TimelineListModel::UriRole)
                     .toString()
                     .contains("did:plc:73l5atmh7p3fn3xigbp6ao5x"),
             true);
    QCOMPARE(model.item(1, TimelineListModel::UriRole)
                     .toString()
                     .contains("did:plc:73l5atmh7p3fn3xigbp6ao5x"),
             true);
}

void search_test::test_SearchPostListModel_noFallback()
{
    // searchPostsV2は実装されているがエラーになるサービス
    QString uuid = AccountManager::getInstance()->updateAccount(
            QString(), m_service + "/v2error", "id", "pass", "did:plc:search_no_fallback", "handle",
            "email", "accessJwt", "refreshJwt", true);

    SearchPostListModel model;
    model.setAccount(uuid);
    model.setText("epub");

    for (int i = 0; i < 2; i++) {
        QSignalSpy spy(&model, SIGNAL(runningChanged()));
        QSignalSpy spy_error(&model, SIGNAL(errorOccurred(const QString &, const QString &)));
        model.getLatest();
        spy.wait(10 * 1000);
        QCOMPARE(spy.count(), 2);
        // searchPostsに切り替えずにエラーを通知する
        QCOMPARE(spy_error.count(), 1);
        QCOMPARE(spy_error.takeFirst().at(0).toString(), "ContentParseError");
        QCOMPARE(model.rowCount(), 0);
    }
}

void search_test::test_SearchProfileListModel()
{
    QString uuid = AccountManager::getInstance()->updateAccount(
            QString(), m_service, "id", "pass", "did:plc:mqxsuw5b5rhpwo4lw6iwlid5", "handle",
            "email", "accessJwt", "refreshJwt", true);

    SearchProfileListModel model;
    model.setAccount(uuid);
    model.setText("epub");

    QSignalSpy spy(&model, SIGNAL(runningChanged()));
    model.getLatest();
    spy.wait(10 * 1000);
    QCOMPARE(spy.count(), 2);

    QCOMPARE(model.rowCount(), 3);
}

QTEST_MAIN(search_test)

#include "tst_search_test.moc"
