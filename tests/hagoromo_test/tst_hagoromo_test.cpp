#include <QtTest>
#include <QCoreApplication>
#include <QJsonDocument>

#include "webserver.h"
#include "timelinelistmodel.h"
#include "recordoperator.h"

class hagoromo_test : public QObject
{
    Q_OBJECT

public:
    hagoromo_test();
    ~hagoromo_test();

private slots:
    void initTestCase();
    void cleanupTestCase();
    void test_TimelineListModelFacet();
    void test_RecordOperator();

private:
    WebServer m_mockServer;
    quint16 m_listenPort;
    QString m_service;

    void test_RecordOperatorCreateRecord(const QByteArray &body);
    void verifyStr(const QString &expect, const QString &actual);
    QJsonDocument loadJson(const QString &path);
    QHash<QString, QString> loadPostHash(const QString &path);
    QHash<QString, QJsonObject> loadPostExpectHash(const QString &path);
};

hagoromo_test::hagoromo_test()
{
    m_listenPort = m_mockServer.listen(QHostAddress::LocalHost, 0);
    m_service = QString("http://localhost:%1/response").arg(m_listenPort);

    connect(&m_mockServer, &WebServer::receivedPost,
            [=](const QHttpServerRequest &request, bool &result, QString &json) {
                if (request.url().path() == "/response/facet/xrpc/com.atproto.repo.createRecord") {
                    test_RecordOperatorCreateRecord(request.body());
                    json = "{\"cid\":\"CID\",\"uri\":\"URI\"}";
                    result = true;
                } else {
                    json = "{}";
                    result = false;
                }
            });
}

hagoromo_test::~hagoromo_test() { }

void hagoromo_test::initTestCase()
{
    QVERIFY(m_listenPort != 0);
}

void hagoromo_test::cleanupTestCase() { }

void hagoromo_test::test_TimelineListModelFacet()
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

void hagoromo_test::test_RecordOperator()
{
    RecordOperator ope;
    QHash<QString, QString> hash = loadPostHash(":/data/com.atproto.repo.createRecord_post.expect");

    QHashIterator<QString, QString> i(hash);
    while (i.hasNext()) {
        i.next();

        ope.clear();
        ope.setAccount(m_service + "/facet", i.key(), "handle", "email", "accessJwt", "refreshJwt");
        ope.setText(i.value());

        QSignalSpy spy(&ope, SIGNAL(finished(bool, const QString &, const QString &)));
        ope.post();
        spy.wait();
        QVERIFY2(spy.count() == 1, QString("spy.count()=%1").arg(spy.count()).toUtf8());

        QList<QVariant> arguments = spy.takeFirst();
        QVERIFY(arguments.at(0).type() == QVariant::Bool);
        QVERIFY(arguments.at(0).toBool() == true);
    }
}

void hagoromo_test::test_RecordOperatorCreateRecord(const QByteArray &body)
{
    QJsonDocument json_doc = QJsonDocument::fromJson(body);
    QString collection = json_doc.object().value("collection").toString();

    if (collection == "app.bsky.feed.post") {
        QString did = json_doc.object().value("repo").toString();
        QString record = json_doc.object().value("record").toObject().value("text").toString();

        QHash<QString, QJsonObject> hash =
                loadPostExpectHash(":/data/com.atproto.repo.createRecord_post.expect");

        QVERIFY2(hash.contains(did), QString("Unknown test pattern: %1").arg(did).toLocal8Bit());
        verifyStr(hash[did].value("record").toObject().value("text").toString(), record);
        if (!hash[did].value("record").toObject().value("facets").isNull()) {
            QVERIFY2(json_doc.object().value("record").toObject().value("facets")
                             == hash[did].value("record").toObject().value("facets"),
                     did.toLocal8Bit());
        }
    }
}

void hagoromo_test::verifyStr(const QString &expect, const QString &actual)
{
    QVERIFY2(expect == actual,
             QString("\nexpect:%1\nactual:%2\n").arg(expect, actual).toLocal8Bit());
}

QJsonDocument hagoromo_test::loadJson(const QString &path)
{
    QFile file(path);
    if (file.open(QFile::ReadOnly)) {
        return QJsonDocument::fromJson(file.readAll());
    } else {
        return QJsonDocument();
    }
}

QHash<QString, QString> hagoromo_test::loadPostHash(const QString &path)
{
    QHash<QString, QString> hash;
    QJsonDocument json_doc = loadJson(path);
    for (const auto &key : json_doc.object().keys()) {
        hash[key] = json_doc.object().value(key).toObject().value("text").toString();
    }
    return hash;
}

QHash<QString, QJsonObject> hagoromo_test::loadPostExpectHash(const QString &path)
{
    QHash<QString, QJsonObject> hash;
    QJsonDocument json_doc = loadJson(path);
    for (const auto &key : json_doc.object().keys()) {
        hash[key] = json_doc.object().value(key).toObject().value("expect").toObject();
    }
    return hash;
}

QTEST_MAIN(hagoromo_test)

#include "tst_hagoromo_test.moc"
