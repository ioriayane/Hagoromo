#include <QtTest>
#include <QCoreApplication>
#include <QJsonDocument>

#include "webserver.h"
#include "timelinelistmodel.h"

class hagoromo_test : public QObject
{
    Q_OBJECT

public:
    hagoromo_test();
    ~hagoromo_test();

private slots:
    void initTestCase();
    void cleanupTestCase();
    void test_case1();

private:
    WebServer m_mockServer;
    quint16 m_listenPort;
    QString m_service;

    void verifyStr(const QString &expect, const QString &actual);
};

hagoromo_test::hagoromo_test()
{
    m_listenPort = m_mockServer.listen(QHostAddress::LocalHost, 52224);
    m_service = QString("http://localhost:%1/response").arg(m_listenPort);
}

hagoromo_test::~hagoromo_test() { }

void hagoromo_test::initTestCase()
{
    QVERIFY(m_listenPort != 0);
}

void hagoromo_test::cleanupTestCase() { }

void hagoromo_test::test_case1()
{
    TimelineListModel model;
    model.setAccount(m_service + "/facet", QString(), QString(), QString(), "dummy", QString());
    model.setDisplayInterval(0);

    QSignalSpy spy(&model, SIGNAL(runningChanged()));
    model.getLatest();
    spy.wait();
    QVERIFY2(spy.count() == 2, QString("spy.count()=%1").arg(spy.count()).toUtf8());

    QFile file(":/response/facet/xrpc/app.bsky.feed.getTimeline.expect");
    QVERIFY(file.open(QFile::ReadOnly));
    QTextStream ts;

    QJsonDocument json_doc = QJsonDocument::fromJson(file.readAll());
    QVERIFY(json_doc.isArray());

    QVERIFY(json_doc.array().count() == model.rowCount());

    for (int i = 0; i < model.rowCount(); i++) {
        verifyStr(json_doc.array().at(i).toObject().value("recordText").toString(),
                  model.item(i, TimelineListModel::TimelineListModelRoles::RecordTextRole)
                          .toString());
    }
}

void hagoromo_test::verifyStr(const QString &expect, const QString &actual)
{
    QVERIFY2(expect == actual,
             QString("\nexpect:%1\nactual:%2\n").arg(expect, actual).toLocal8Bit());
}

QTEST_MAIN(hagoromo_test)

#include "tst_hagoromo_test.moc"
