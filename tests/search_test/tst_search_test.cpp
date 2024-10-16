#include <QtTest>
#include <QCoreApplication>
#include <QJsonDocument>

#include "webserver.h"

#include "timeline/searchpostlistmodel.h"
#include "profile/searchprofilelistmodel.h"

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
    m_service = QString("http://localhost:%1/response").arg(m_listenPort);
}

search_test::~search_test() { }

void search_test::initTestCase()
{
    QVERIFY(m_listenPort != 0);
}

void search_test::cleanupTestCase() { }

void search_test::test_SearchPostListModel()
{
    SearchPostListModel model;
    model.setAccount(m_service, QString(), QString(), QString(), "dummy", QString());
    model.setText("epub");

    QSignalSpy spy(&model, SIGNAL(runningChanged()));
    model.getLatest();
    spy.wait(10 * 1000);
    QVERIFY2(spy.count() == 2, QString("spy.count()=%1").arg(spy.count()).toUtf8());

    QVERIFY(model.rowCount() == 2);
}

void search_test::test_SearchProfileListModel()
{
    SearchProfileListModel model;
    model.setAccount(m_service, QString(), QString(), QString(), "dummy", QString());
    model.setText("epub");

    QSignalSpy spy(&model, SIGNAL(runningChanged()));
    model.getLatest();
    spy.wait(10 * 1000);
    QVERIFY2(spy.count() == 2, QString("spy.count()=%1").arg(spy.count()).toUtf8());

    QVERIFY(model.rowCount() == 3);
}

QTEST_MAIN(search_test)

#include "tst_search_test.moc"
