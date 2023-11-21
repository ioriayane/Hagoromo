#include <QtTest>
#include <QCoreApplication>
#include <QJsonDocument>

#include "webserver.h"

#include "search/search.h"
#include "search/search_func.h"
#include "search/searchposts.h"
#include "search/searchprofiles.h"
#include "searchpostlistmodel.h"
#include "searchprofilelistmodel.h"

class search_test : public QObject
{
    Q_OBJECT

public:
    search_test();
    ~search_test();

private slots:
    void initTestCase();
    void cleanupTestCase();
    void test_copyTest();
    void test_SearchPosts();
    void test_SearchPostListModel();
    void test_SearchProfiles();
    void test_SearchProfileListModel();

private:
    WebServer m_mockServer;
    quint16 m_listenPort;
    QString m_service;
};

search_test::search_test()
{
    QCoreApplication::setOrganizationName(QStringLiteral("relog"));
    QCoreApplication::setApplicationName(QStringLiteral("Hagoromo"));

    m_listenPort = m_mockServer.listen(QHostAddress::LocalHost, 0);
    m_service = QString("http://localhost:%1/response").arg(m_listenPort);
}

search_test::~search_test() { }

void search_test::initTestCase()
{
    QVERIFY(m_listenPort != 0);
}

void search_test::cleanupTestCase() { }

void search_test::test_copyTest()
{
    QString json;
    json += "{";
    json += "\"tid\": \"did:plc:ipj5qejfoqu6eukvt72uhyit\",";
    json += "\"cid\": \"bafyreihr2hrmavhzdpmnc65udreph5vfmd3xceqtw2jm3b4clbfbacgsqe\",";
    json += "\"user\": {";
    json += "\"did\": \"did:plc:ipj5qejfoqu6eukvt72uhyit\",";
    json += "\"handle\": \"ioriayane.bsky.social\"";
    json += "},";
    json += "\"post\": {";
    json += "\"createdAt\": 6298000000,";
    //    json += "\"createdAt\": 1682988006298000000,";
    //                             628021800000
    json += "\"text\": \"test test\",";
    json += "\"user\": \"ioriayane.bsky.social\"";
    json += "}";
    json += "}";
    QJsonDocument json_doc = QJsonDocument::fromJson(json.toUtf8());

    SearchType::ViewPost view_post;

    SearchType::copyViewPost(json_doc.object(), view_post);

    QVERIFY(view_post.tid == "did:plc:ipj5qejfoqu6eukvt72uhyit");
    QVERIFY(view_post.cid == "bafyreihr2hrmavhzdpmnc65udreph5vfmd3xceqtw2jm3b4clbfbacgsqe");
    QVERIFY(view_post.user.did == "did:plc:ipj5qejfoqu6eukvt72uhyit");
    QVERIFY(view_post.user.handle == "ioriayane.bsky.social");
    QVERIFY(view_post.post.user == "ioriayane.bsky.social");
    QVERIFY(view_post.post.text == "test test");
}

void search_test::test_SearchPosts()
{
    SearchInterface::SearchPosts search;
    search.setService(m_service);

    QSignalSpy spy(&search, SIGNAL(finished(bool)));
    search.search("text");
    spy.wait();
    QVERIFY(spy.count() == 1);

    QList<QVariant> arguments = spy.takeFirst();
    QVERIFY(arguments.at(0).toBool());

    QVERIFY(search.viewPostList()->length() == 2);
}

void search_test::test_SearchPostListModel()
{
    SearchPostListModel model;
    model.setAccount(m_service, QString(), QString(), QString(), "dummy", QString());
    model.setText("epub");

    QSignalSpy spy(&model, SIGNAL(runningChanged()));
    model.getLatest();
    spy.wait();
    spy.wait();
    QVERIFY2(spy.count() == 2, QString("spy.count()=%1").arg(spy.count()).toUtf8());

    QVERIFY(model.rowCount() == 2);
}

void search_test::test_SearchProfiles()
{
    SearchInterface::SearchProfiles profiles;
    profiles.setService(m_service);

    QSignalSpy spy(&profiles, SIGNAL(finished(bool)));
    profiles.search("text");
    spy.wait();
    QVERIFY(spy.count() == 1);

    QList<QVariant> arguments = spy.takeFirst();
    QVERIFY(arguments.at(0).toBool());

    QVERIFY(profiles.didList()->count() == 2);
}

void search_test::test_SearchProfileListModel()
{
    SearchProfileListModel model;
    model.setAccount(m_service, QString(), QString(), QString(), "dummy", QString());
    model.setText("epub");

    QSignalSpy spy(&model, SIGNAL(runningChanged()));
    model.getLatest();
    spy.wait();
    spy.wait();
    QVERIFY2(spy.count() == 2, QString("spy.count()=%1").arg(spy.count()).toUtf8());

    QVERIFY(model.rowCount() == 2);
}

QTEST_MAIN(search_test)

#include "tst_search_test.moc"
