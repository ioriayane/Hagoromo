#include <QtTest>
#include <QCoreApplication>
#include <QJsonDocument>

#include "webserver.h"
#include "timelinelistmodel.h"
#include "recordoperator.h"
#include "feedgeneratorlistmodel.h"
#include "columnlistmodel.h"
#include "notificationlistmodel.h"
#include "userprofile.h"
#include "tools/qstringex.h"
#include "feedgeneratorlink.h"
#include "anyprofilelistmodel.h"
#include "accountlistmodel.h"
#include "common.h"
#include "postthreadlistmodel.h"

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
    void test_FeedGeneratorListModel();
    void test_ColumnListModelMove();
    void test_ColumnListModelRemove();
    void test_ColumnListModelInsertNext();
    void test_NotificationListModel();
    void test_NotificationListModel2();
    void test_UserProfile();
    void test_charCount();
    void test_TimelineListModel_quote_warn();
    void test_TimelineListModel_quote_hide();
    void test_TimelineListModel_quote_hide2();
    void test_TimelineListModel_quote_label();
    void test_NotificationListModel_warn();
    void test_TimelineListModel_next();
    void test_FeedGeneratorLink();
    void test_AnyProfileListModel();
    void test_AccountListModel();
    void test_TimelineListModel_text();
    void test_TimelineListModel_reply();
    void test_PostThreadListModel();

private:
    WebServer m_mockServer;
    quint16 m_listenPort;
    QString m_service;

    void test_RecordOperatorCreateRecord(const QByteArray &body);
    void test_putPreferences(const QString &path, const QByteArray &body);
    void verifyStr(const QString &expect, const QString &actual);
    QJsonDocument loadJson(const QString &path);
    QHash<QString, QString> loadPostHash(const QString &path);
    QHash<QString, QJsonObject> loadPostExpectHash(const QString &path);
};

hagoromo_test::hagoromo_test()
{
    QCoreApplication::setOrganizationName(QStringLiteral("relog"));
    QCoreApplication::setApplicationName(QStringLiteral("Hagoromo"));

    m_listenPort = m_mockServer.listen(QHostAddress::LocalHost, 0);
    m_service = QString("http://localhost:%1/response").arg(m_listenPort);

    connect(&m_mockServer, &WebServer::receivedPost,
            [=](const QHttpServerRequest &request, bool &result, QString &json) {
                if (request.url().path() == "/response/facet/xrpc/com.atproto.repo.createRecord") {
                    test_RecordOperatorCreateRecord(request.body());
                    json = "{\"cid\":\"CID\",\"uri\":\"URI\"}";
                    result = true;
                } else if (request.url().path()
                                   == "/response/generator/save/xrpc/app.bsky.actor.putPreferences"
                           || request.url().path()
                                   == "/response/generator/remove/xrpc/"
                                      "app.bsky.actor.putPreferences") {
                    test_putPreferences(request.url().path(), request.body());
                    json = "{}";
                    result = true;
                } else if (request.url().path().endsWith(
                                   "/xrpc/com.atproto.server.refreshSession")) {
                    QFileInfo file_info(request.url().path());
                    QString path = ":" + file_info.filePath();
                    QFile file(path);
                    if (file.open(QFile::ReadOnly)) {
                        json = file.readAll();
                        file.close();
                        result = true;
                    } else {
                        json = "{}";
                        result = false;
                    }
                } else {
                    json = "{}";
                    result = false;
                    QVERIFY(false);
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
    ope.setAccount(m_service + "/facet", QString(), QString(), QString(), "dummy", QString());
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

void hagoromo_test::test_FeedGeneratorListModel()
{
    FeedGeneratorListModel model;
    model.setAccount(m_service + "/generator", QString(), QString(), QString(), "dummy", QString());
    model.setDisplayInterval(0);
    {
        QSignalSpy spy(&model, SIGNAL(runningChanged()));
        model.getLatest();
        spy.wait();
        QVERIFY2(spy.count() == 2, QString("spy.count()=%1").arg(spy.count()).toUtf8());

        QVERIFY(model.rowCount() == 8);
        QVERIFY(model.item(0, FeedGeneratorListModel::SavingRole) == true);
        QVERIFY(model.item(1, FeedGeneratorListModel::DisplayNameRole)
                == QStringLiteral("What's Hot"));
        QVERIFY(model.item(1, FeedGeneratorListModel::SavingRole) == true);
        QVERIFY(model.item(2, FeedGeneratorListModel::SavingRole) == false);
        QVERIFY(model.item(3, FeedGeneratorListModel::SavingRole) == true);
        QVERIFY(model.item(4, FeedGeneratorListModel::SavingRole) == false);
        QVERIFY(model.item(5, FeedGeneratorListModel::SavingRole) == false);
        QVERIFY(model.item(6, FeedGeneratorListModel::SavingRole) == false);
        QVERIFY(model.item(7, FeedGeneratorListModel::DisplayNameRole)
                == QStringLiteral("mostpop"));
        QVERIFY(model.item(7, FeedGeneratorListModel::SavingRole) == false);
    }
    {
        // save
        model.setAccount(m_service + "/generator/save", QString(), QString(), QString(), "dummy",
                         QString());
        QSignalSpy spy(&model, SIGNAL(runningChanged()));
        model.saveGenerator(
                "at://did:plc:z72i7hdynmk6r22z27h6tvur/app.bsky.feed.generator/hot-classic");
        spy.wait(10 * 1000);
        QVERIFY2(spy.count() == 2, QString("spy.count()=%1").arg(spy.count()).toUtf8());
    }
    {
        // remove
        model.setAccount(m_service + "/generator/remove", QString(), QString(), QString(), "dummy",
                         QString());
        QSignalSpy spy(&model, SIGNAL(runningChanged()));
        model.removeGenerator(
                "at://did:plc:z72i7hdynmk6r22z27h6tvur/app.bsky.feed.generator/with-friends");
        spy.wait(10 * 1000);
        QVERIFY2(spy.count() == 2, QString("spy.count()=%1").arg(spy.count()).toUtf8());
    }
}

void hagoromo_test::test_ColumnListModelMove()
{
    ColumnListModel model;

    model.append("uuid_1", 0, false, 10000, 400, "column 1", "value 1");
    model.append("uuid_2", 1, false, 20000, 500, "column 2", "value 2");
    model.append("uuid_3", 2, false, 30000, 600, "column 3", "value 3");
    model.append("uuid_4", 3, false, 40000, 700, "column 4", "value 4");

    QVERIFY2(model.getPreviousRow(0) == -1,
             QString("left pos=%1").arg(model.getPreviousRow(0)).toLocal8Bit());
    QVERIFY2(model.getPreviousRow(1) == 0,
             QString("left pos=%1").arg(model.getPreviousRow(1)).toLocal8Bit());
    QVERIFY2(model.getPreviousRow(2) == 1,
             QString("left pos=%1").arg(model.getPreviousRow(2)).toLocal8Bit());
    QVERIFY2(model.getPreviousRow(3) == 2,
             QString("left pos=%1").arg(model.getPreviousRow(3)).toLocal8Bit());
    QVERIFY(model.getRowListInOrderOfPosition() == QList<int>() << 0 << 1 << 2 << 3);

    model.move(model.item(1, ColumnListModel::KeyRole).toString(), ColumnListModel::MoveLeft);
    QVERIFY2(model.getPreviousRow(0) == 1,
             QString("left pos=%1").arg(model.getPreviousRow(0)).toLocal8Bit());
    QVERIFY2(model.getPreviousRow(1) == -1,
             QString("left pos=%1").arg(model.getPreviousRow(1)).toLocal8Bit());
    QVERIFY2(model.getPreviousRow(2) == 0,
             QString("left pos=%1").arg(model.getPreviousRow(2)).toLocal8Bit());
    QVERIFY2(model.getPreviousRow(3) == 2,
             QString("left pos=%1").arg(model.getPreviousRow(3)).toLocal8Bit());
    QVERIFY(model.getRowListInOrderOfPosition() == QList<int>() << 1 << 0 << 2 << 3);

    model.move(model.item(2, ColumnListModel::KeyRole).toString(), ColumnListModel::MoveLeft);
    QVERIFY2(model.getPreviousRow(0) == 2,
             QString("left pos=%1").arg(model.getPreviousRow(0)).toLocal8Bit());
    QVERIFY2(model.getPreviousRow(1) == -1,
             QString("left pos=%1").arg(model.getPreviousRow(1)).toLocal8Bit());
    QVERIFY2(model.getPreviousRow(2) == 1,
             QString("left pos=%1").arg(model.getPreviousRow(2)).toLocal8Bit());
    QVERIFY2(model.getPreviousRow(3) == 0,
             QString("left pos=%1").arg(model.getPreviousRow(3)).toLocal8Bit());
    QVERIFY(model.getRowListInOrderOfPosition() == QList<int>() << 1 << 2 << 0 << 3);

    model.move(model.item(3, ColumnListModel::KeyRole).toString(), ColumnListModel::MoveLeft);
    QVERIFY2(model.getPreviousRow(0) == 3,
             QString("left pos=%1").arg(model.getPreviousRow(0)).toLocal8Bit());
    QVERIFY2(model.getPreviousRow(1) == -1,
             QString("left pos=%1").arg(model.getPreviousRow(1)).toLocal8Bit());
    QVERIFY2(model.getPreviousRow(2) == 1,
             QString("left pos=%1").arg(model.getPreviousRow(2)).toLocal8Bit());
    QVERIFY2(model.getPreviousRow(3) == 2,
             QString("left pos=%1").arg(model.getPreviousRow(3)).toLocal8Bit());
    QVERIFY(model.getRowListInOrderOfPosition() == QList<int>() << 1 << 2 << 3 << 0);

    model.move(model.item(1, ColumnListModel::KeyRole).toString(), ColumnListModel::MoveLeft);
    QVERIFY2(model.getPreviousRow(0) == 3,
             QString("left pos=%1").arg(model.getPreviousRow(0)).toLocal8Bit());
    QVERIFY2(model.getPreviousRow(1) == -1,
             QString("left pos=%1").arg(model.getPreviousRow(1)).toLocal8Bit());
    QVERIFY2(model.getPreviousRow(2) == 1,
             QString("left pos=%1").arg(model.getPreviousRow(2)).toLocal8Bit());
    QVERIFY2(model.getPreviousRow(3) == 2,
             QString("left pos=%1").arg(model.getPreviousRow(3)).toLocal8Bit());
    QVERIFY(model.getRowListInOrderOfPosition() == QList<int>() << 1 << 2 << 3 << 0);

    model.move(model.item(3, ColumnListModel::KeyRole).toString(), ColumnListModel::MoveRight);
    QVERIFY2(model.getPreviousRow(0) == 2,
             QString("left pos=%1").arg(model.getPreviousRow(0)).toLocal8Bit());
    QVERIFY2(model.getPreviousRow(1) == -1,
             QString("left pos=%1").arg(model.getPreviousRow(1)).toLocal8Bit());
    QVERIFY2(model.getPreviousRow(2) == 1,
             QString("left pos=%1").arg(model.getPreviousRow(2)).toLocal8Bit());
    QVERIFY2(model.getPreviousRow(3) == 0,
             QString("left pos=%1").arg(model.getPreviousRow(3)).toLocal8Bit());
    QVERIFY(model.getRowListInOrderOfPosition() == QList<int>() << 1 << 2 << 0 << 3);

    model.move(model.item(2, ColumnListModel::KeyRole).toString(), ColumnListModel::MoveRight);
    QVERIFY2(model.getPreviousRow(0) == 1,
             QString("left pos=%1").arg(model.getPreviousRow(0)).toLocal8Bit());
    QVERIFY2(model.getPreviousRow(1) == -1,
             QString("left pos=%1").arg(model.getPreviousRow(1)).toLocal8Bit());
    QVERIFY2(model.getPreviousRow(2) == 0,
             QString("left pos=%1").arg(model.getPreviousRow(2)).toLocal8Bit());
    QVERIFY2(model.getPreviousRow(3) == 2,
             QString("left pos=%1").arg(model.getPreviousRow(3)).toLocal8Bit());
    QVERIFY(model.getRowListInOrderOfPosition() == QList<int>() << 1 << 0 << 2 << 3);

    model.move(model.item(1, ColumnListModel::KeyRole).toString(), ColumnListModel::MoveRight);
    QVERIFY2(model.getPreviousRow(0) == -1,
             QString("left pos=%1").arg(model.getPreviousRow(0)).toLocal8Bit());
    QVERIFY2(model.getPreviousRow(1) == 0,
             QString("left pos=%1").arg(model.getPreviousRow(1)).toLocal8Bit());
    QVERIFY2(model.getPreviousRow(2) == 1,
             QString("left pos=%1").arg(model.getPreviousRow(2)).toLocal8Bit());
    QVERIFY2(model.getPreviousRow(3) == 2,
             QString("left pos=%1").arg(model.getPreviousRow(3)).toLocal8Bit());
    QVERIFY(model.getRowListInOrderOfPosition() == QList<int>() << 0 << 1 << 2 << 3);
}

void hagoromo_test::test_ColumnListModelRemove()
{
    ColumnListModel model;

    model.append("uuid_1", 0, false, 10000, 400, "column 1", "value 1");
    model.append("uuid_2", 1, false, 20000, 500, "column 2", "value 2");
    model.append("uuid_3", 2, false, 30000, 600, "column 3", "value 3");
    model.append("uuid_4", 3, false, 40000, 700, "column 4", "value 4");

    model.move(model.item(1, ColumnListModel::KeyRole).toString(), ColumnListModel::MoveLeft);
    model.move(model.item(2, ColumnListModel::KeyRole).toString(), ColumnListModel::MoveLeft);
    model.move(model.item(3, ColumnListModel::KeyRole).toString(), ColumnListModel::MoveLeft);
    QVERIFY2(model.getPreviousRow(0) == 3,
             QString("left pos=%1").arg(model.getPreviousRow(0)).toLocal8Bit());
    QVERIFY2(model.getPreviousRow(1) == -1,
             QString("left pos=%1").arg(model.getPreviousRow(1)).toLocal8Bit());
    QVERIFY2(model.getPreviousRow(2) == 1,
             QString("left pos=%1").arg(model.getPreviousRow(2)).toLocal8Bit());
    QVERIFY2(model.getPreviousRow(3) == 2,
             QString("left pos=%1").arg(model.getPreviousRow(3)).toLocal8Bit());
    QVERIFY(model.getRowListInOrderOfPosition() == QList<int>() << 1 << 2 << 3 << 0);

    model.removeByKey(model.item(2, ColumnListModel::KeyRole).toString());
    QVERIFY2(model.getPreviousRow(0) == 2,
             QString("left pos=%1").arg(model.getPreviousRow(0)).toLocal8Bit());
    QVERIFY2(model.getPreviousRow(1) == -1,
             QString("left pos=%1").arg(model.getPreviousRow(1)).toLocal8Bit());
    QVERIFY2(model.getPreviousRow(2) == 1,
             QString("left pos=%1").arg(model.getPreviousRow(2)).toLocal8Bit());
    QVERIFY(model.getRowListInOrderOfPosition() == QList<int>() << 1 << 2 << 0);
}

void hagoromo_test::test_ColumnListModelInsertNext()
{
    ColumnListModel model;

    model.append("uuid_1", 0, false, 10000, 400, "column 1", "value 1");
    model.append("uuid_2", 1, false, 20000, 500, "column 2", "value 2");
    model.append("uuid_3", 2, false, 30000, 600, "column 3", "value 3");
    model.append("uuid_4", 3, false, 40000, 700, "column 4", "value 4");

    QVERIFY2(model.getPreviousRow(0) == -1,
             QString("left pos=%1").arg(model.getPreviousRow(0)).toLocal8Bit());
    QVERIFY2(model.getPreviousRow(1) == 0,
             QString("left pos=%1").arg(model.getPreviousRow(1)).toLocal8Bit());
    QVERIFY2(model.getPreviousRow(2) == 1,
             QString("left pos=%1").arg(model.getPreviousRow(2)).toLocal8Bit());
    QVERIFY2(model.getPreviousRow(3) == 2,
             QString("left pos=%1").arg(model.getPreviousRow(3)).toLocal8Bit());
    QVERIFY(model.getRowListInOrderOfPosition() == QList<int>() << 0 << 1 << 2 << 3);

    model.insertNext(model.item(0, ColumnListModel::KeyRole).toString(), "uuid_10", 10, false, 10,
                     800, "column 1 next", "value 10");
    QVERIFY2(model.getPreviousRow(0) == -1,
             QString("left pos=%1").arg(model.getPreviousRow(0)).toLocal8Bit());
    QVERIFY2(model.getPreviousRow(1) == 4,
             QString("left pos=%1").arg(model.getPreviousRow(1)).toLocal8Bit());
    QVERIFY2(model.getPreviousRow(2) == 1,
             QString("left pos=%1").arg(model.getPreviousRow(2)).toLocal8Bit());
    QVERIFY2(model.getPreviousRow(3) == 2,
             QString("left pos=%1").arg(model.getPreviousRow(3)).toLocal8Bit());
    QVERIFY2(model.getPreviousRow(4) == 0,
             QString("left pos=%1").arg(model.getPreviousRow(4)).toLocal8Bit());
    QVERIFY(model.getRowListInOrderOfPosition() == QList<int>() << 0 << 4 << 1 << 2 << 3);

    model.insertNext(model.item(0, ColumnListModel::KeyRole).toString(), "uuid_11", 11, false, 11,
                     900, "column 1 next2", "value 11");
    QVERIFY2(model.getPreviousRow(0) == -1,
             QString("left pos=%1").arg(model.getPreviousRow(0)).toLocal8Bit());
    QVERIFY2(model.getPreviousRow(1) == 4,
             QString("left pos=%1").arg(model.getPreviousRow(1)).toLocal8Bit());
    QVERIFY2(model.getPreviousRow(2) == 1,
             QString("left pos=%1").arg(model.getPreviousRow(2)).toLocal8Bit());
    QVERIFY2(model.getPreviousRow(3) == 2,
             QString("left pos=%1").arg(model.getPreviousRow(3)).toLocal8Bit());
    QVERIFY2(model.getPreviousRow(4) == 5,
             QString("left pos=%1").arg(model.getPreviousRow(4)).toLocal8Bit());
    QVERIFY2(model.getPreviousRow(5) == 0,
             QString("left pos=%1").arg(model.getPreviousRow(5)).toLocal8Bit());
    QVERIFY(model.getRowListInOrderOfPosition() == QList<int>() << 0 << 5 << 4 << 1 << 2 << 3);

    model.insertNext(model.item(3, ColumnListModel::KeyRole).toString(), "uuid_12", 12, false, 12,
                     1000, "column 4 next", "value 12");
    QVERIFY2(model.getPreviousRow(0) == -1,
             QString("left pos=%1").arg(model.getPreviousRow(0)).toLocal8Bit());
    QVERIFY2(model.getPreviousRow(1) == 4,
             QString("left pos=%1").arg(model.getPreviousRow(1)).toLocal8Bit());
    QVERIFY2(model.getPreviousRow(2) == 1,
             QString("left pos=%1").arg(model.getPreviousRow(2)).toLocal8Bit());
    QVERIFY2(model.getPreviousRow(3) == 2,
             QString("left pos=%1").arg(model.getPreviousRow(3)).toLocal8Bit());
    QVERIFY2(model.getPreviousRow(4) == 5,
             QString("left pos=%1").arg(model.getPreviousRow(4)).toLocal8Bit());
    QVERIFY2(model.getPreviousRow(5) == 0,
             QString("left pos=%1").arg(model.getPreviousRow(5)).toLocal8Bit());
    QVERIFY2(model.getPreviousRow(6) == 3,
             QString("left pos=%1").arg(model.getPreviousRow(6)).toLocal8Bit());
    QVERIFY(model.getRowListInOrderOfPosition() == QList<int>() << 0 << 5 << 4 << 1 << 2 << 3 << 6);
}

void hagoromo_test::test_NotificationListModel()
{
    NotificationListModel model;
    model.setAccount(m_service + "/notifications/visible", QString(), QString(), QString(), "dummy",
                     QString());

    {
        int i = 0;
        QSignalSpy spy(&model, SIGNAL(runningChanged()));
        model.getLatest();
        spy.wait(10 * 1000);
        QVERIFY2(spy.count() == 2, QString("spy.count()=%1").arg(spy.count()).toUtf8());

        QVERIFY2(model.rowCount() == 6, QString("rowCount()=%1").arg(model.rowCount()).toUtf8());
        QVERIFY2(model.item(i, NotificationListModel::ReasonRole)
                         == NotificationListModel::NotificationListModelReason::ReasonLike,
                 QString("reason=%1")
                         .arg(model.item(i++, NotificationListModel::ReasonRole).toInt())
                         .toLocal8Bit());
        QVERIFY2(model.item(i, NotificationListModel::ReasonRole)
                         == NotificationListModel::NotificationListModelReason::ReasonRepost,
                 QString("reason=%1")
                         .arg(model.item(i++, NotificationListModel::ReasonRole).toInt())
                         .toLocal8Bit());
        QVERIFY2(model.item(i, NotificationListModel::ReasonRole)
                         == NotificationListModel::NotificationListModelReason::ReasonFollow,
                 QString("reason=%1")
                         .arg(model.item(i++, NotificationListModel::ReasonRole).toInt())
                         .toLocal8Bit());
        QVERIFY2(model.item(i, NotificationListModel::ReasonRole)
                         == NotificationListModel::NotificationListModelReason::ReasonMention,
                 QString("reason=%1")
                         .arg(model.item(i++, NotificationListModel::ReasonRole).toInt())
                         .toLocal8Bit());
        QVERIFY2(model.item(i, NotificationListModel::ReasonRole)
                         == NotificationListModel::NotificationListModelReason::ReasonReply,
                 QString("reason=%1")
                         .arg(model.item(i++, NotificationListModel::ReasonRole).toInt())
                         .toLocal8Bit());
        QVERIFY2(model.item(i, NotificationListModel::ReasonRole)
                         == NotificationListModel::NotificationListModelReason::ReasonQuote,
                 QString("reason=%1")
                         .arg(model.item(i++, NotificationListModel::ReasonRole).toInt())
                         .toLocal8Bit());
    }

    {
        int i = 0;
        QSignalSpy spy(&model, SIGNAL(runningChanged()));
        model.setVisibleLike(false);
        model.setVisibleRepost(true);
        model.setVisibleFollow(true);
        model.setVisibleMention(true);
        model.setVisibleReply(true);
        model.setVisibleQuote(true);
        model.clear();
        model.getLatest();
        spy.wait(10 * 1000);
        QVERIFY2(spy.count() == 2, QString("spy.count()=%1").arg(spy.count()).toUtf8());

        QVERIFY2(model.rowCount() == 5, QString("rowCount()=%1").arg(model.rowCount()).toUtf8());
        QVERIFY2(model.item(i, NotificationListModel::ReasonRole)
                         == NotificationListModel::NotificationListModelReason::ReasonRepost,
                 QString("reason=%1")
                         .arg(model.item(i++, NotificationListModel::ReasonRole).toInt())
                         .toLocal8Bit());
        QVERIFY2(model.item(i, NotificationListModel::ReasonRole)
                         == NotificationListModel::NotificationListModelReason::ReasonFollow,
                 QString("reason=%1")
                         .arg(model.item(i++, NotificationListModel::ReasonRole).toInt())
                         .toLocal8Bit());
        QVERIFY2(model.item(i, NotificationListModel::ReasonRole)
                         == NotificationListModel::NotificationListModelReason::ReasonMention,
                 QString("reason=%1")
                         .arg(model.item(i++, NotificationListModel::ReasonRole).toInt())
                         .toLocal8Bit());
        QVERIFY2(model.item(i, NotificationListModel::ReasonRole)
                         == NotificationListModel::NotificationListModelReason::ReasonReply,
                 QString("reason=%1")
                         .arg(model.item(i++, NotificationListModel::ReasonRole).toInt())
                         .toLocal8Bit());
        QVERIFY2(model.item(i, NotificationListModel::ReasonRole)
                         == NotificationListModel::NotificationListModelReason::ReasonQuote,
                 QString("reason=%1")
                         .arg(model.item(i++, NotificationListModel::ReasonRole).toInt())
                         .toLocal8Bit());
    }

    {
        int i = 0;
        QSignalSpy spy(&model, SIGNAL(runningChanged()));
        model.setVisibleLike(true);
        model.setVisibleRepost(false);
        model.setVisibleFollow(true);
        model.setVisibleMention(true);
        model.setVisibleReply(true);
        model.setVisibleQuote(true);
        model.clear();
        model.getLatest();
        spy.wait(10 * 1000);
        QVERIFY2(spy.count() == 2, QString("spy.count()=%1").arg(spy.count()).toUtf8());

        QVERIFY2(model.rowCount() == 5, QString("rowCount()=%1").arg(model.rowCount()).toUtf8());
        QVERIFY2(model.item(i, NotificationListModel::ReasonRole)
                         == NotificationListModel::NotificationListModelReason::ReasonLike,
                 QString("reason=%1")
                         .arg(model.item(i++, NotificationListModel::ReasonRole).toInt())
                         .toLocal8Bit());
        QVERIFY2(model.item(i, NotificationListModel::ReasonRole)
                         == NotificationListModel::NotificationListModelReason::ReasonFollow,
                 QString("reason=%1")
                         .arg(model.item(i++, NotificationListModel::ReasonRole).toInt())
                         .toLocal8Bit());
        QVERIFY2(model.item(i, NotificationListModel::ReasonRole)
                         == NotificationListModel::NotificationListModelReason::ReasonMention,
                 QString("reason=%1")
                         .arg(model.item(i++, NotificationListModel::ReasonRole).toInt())
                         .toLocal8Bit());
        QVERIFY2(model.item(i, NotificationListModel::ReasonRole)
                         == NotificationListModel::NotificationListModelReason::ReasonReply,
                 QString("reason=%1")
                         .arg(model.item(i++, NotificationListModel::ReasonRole).toInt())
                         .toLocal8Bit());
        QVERIFY2(model.item(i, NotificationListModel::ReasonRole)
                         == NotificationListModel::NotificationListModelReason::ReasonQuote,
                 QString("reason=%1")
                         .arg(model.item(i++, NotificationListModel::ReasonRole).toInt())
                         .toLocal8Bit());
    }

    {
        int i = 0;
        QSignalSpy spy(&model, SIGNAL(runningChanged()));
        model.setVisibleLike(true);
        model.setVisibleRepost(true);
        model.setVisibleFollow(false);
        model.setVisibleMention(true);
        model.setVisibleReply(true);
        model.setVisibleQuote(true);
        model.clear();
        model.getLatest();
        spy.wait(10 * 1000);
        QVERIFY2(spy.count() == 2, QString("spy.count()=%1").arg(spy.count()).toUtf8());

        QVERIFY2(model.rowCount() == 5, QString("rowCount()=%1").arg(model.rowCount()).toUtf8());
        QVERIFY2(model.item(i, NotificationListModel::ReasonRole)
                         == NotificationListModel::NotificationListModelReason::ReasonLike,
                 QString("reason=%1")
                         .arg(model.item(i++, NotificationListModel::ReasonRole).toInt())
                         .toLocal8Bit());
        QVERIFY2(model.item(i, NotificationListModel::ReasonRole)
                         == NotificationListModel::NotificationListModelReason::ReasonRepost,
                 QString("reason=%1")
                         .arg(model.item(i++, NotificationListModel::ReasonRole).toInt())
                         .toLocal8Bit());
        QVERIFY2(model.item(i, NotificationListModel::ReasonRole)
                         == NotificationListModel::NotificationListModelReason::ReasonMention,
                 QString("reason=%1")
                         .arg(model.item(i++, NotificationListModel::ReasonRole).toInt())
                         .toLocal8Bit());
        QVERIFY2(model.item(i, NotificationListModel::ReasonRole)
                         == NotificationListModel::NotificationListModelReason::ReasonReply,
                 QString("reason=%1")
                         .arg(model.item(i++, NotificationListModel::ReasonRole).toInt())
                         .toLocal8Bit());
        QVERIFY2(model.item(i, NotificationListModel::ReasonRole)
                         == NotificationListModel::NotificationListModelReason::ReasonQuote,
                 QString("reason=%1")
                         .arg(model.item(i++, NotificationListModel::ReasonRole).toInt())
                         .toLocal8Bit());
    }

    {
        int i = 0;
        QSignalSpy spy(&model, SIGNAL(runningChanged()));
        model.setVisibleLike(true);
        model.setVisibleRepost(true);
        model.setVisibleFollow(true);
        model.setVisibleMention(false);
        model.setVisibleReply(true);
        model.setVisibleQuote(true);
        model.clear();
        model.getLatest();
        spy.wait(10 * 1000);
        QVERIFY2(spy.count() == 2, QString("spy.count()=%1").arg(spy.count()).toUtf8());

        QVERIFY2(model.rowCount() == 5, QString("rowCount()=%1").arg(model.rowCount()).toUtf8());
        QVERIFY2(model.item(i, NotificationListModel::ReasonRole)
                         == NotificationListModel::NotificationListModelReason::ReasonLike,
                 QString("reason=%1")
                         .arg(model.item(i++, NotificationListModel::ReasonRole).toInt())
                         .toLocal8Bit());
        QVERIFY2(model.item(i, NotificationListModel::ReasonRole)
                         == NotificationListModel::NotificationListModelReason::ReasonRepost,
                 QString("reason=%1")
                         .arg(model.item(i++, NotificationListModel::ReasonRole).toInt())
                         .toLocal8Bit());
        QVERIFY2(model.item(i, NotificationListModel::ReasonRole)
                         == NotificationListModel::NotificationListModelReason::ReasonFollow,
                 QString("reason=%1")
                         .arg(model.item(i++, NotificationListModel::ReasonRole).toInt())
                         .toLocal8Bit());
        QVERIFY2(model.item(i, NotificationListModel::ReasonRole)
                         == NotificationListModel::NotificationListModelReason::ReasonReply,
                 QString("reason=%1")
                         .arg(model.item(i++, NotificationListModel::ReasonRole).toInt())
                         .toLocal8Bit());
        QVERIFY2(model.item(i, NotificationListModel::ReasonRole)
                         == NotificationListModel::NotificationListModelReason::ReasonQuote,
                 QString("reason=%1")
                         .arg(model.item(i++, NotificationListModel::ReasonRole).toInt())
                         .toLocal8Bit());
    }

    {
        int i = 0;
        QSignalSpy spy(&model, SIGNAL(runningChanged()));
        model.setVisibleLike(true);
        model.setVisibleRepost(true);
        model.setVisibleFollow(true);
        model.setVisibleMention(true);
        model.setVisibleReply(false);
        model.setVisibleQuote(true);
        model.clear();
        model.getLatest();
        spy.wait(10 * 1000);
        QVERIFY2(spy.count() == 2, QString("spy.count()=%1").arg(spy.count()).toUtf8());

        QVERIFY2(model.rowCount() == 5, QString("rowCount()=%1").arg(model.rowCount()).toUtf8());
        QVERIFY2(model.item(i, NotificationListModel::ReasonRole)
                         == NotificationListModel::NotificationListModelReason::ReasonLike,
                 QString("reason=%1")
                         .arg(model.item(i++, NotificationListModel::ReasonRole).toInt())
                         .toLocal8Bit());
        QVERIFY2(model.item(i, NotificationListModel::ReasonRole)
                         == NotificationListModel::NotificationListModelReason::ReasonRepost,
                 QString("reason=%1")
                         .arg(model.item(i++, NotificationListModel::ReasonRole).toInt())
                         .toLocal8Bit());
        QVERIFY2(model.item(i, NotificationListModel::ReasonRole)
                         == NotificationListModel::NotificationListModelReason::ReasonFollow,
                 QString("reason=%1")
                         .arg(model.item(i++, NotificationListModel::ReasonRole).toInt())
                         .toLocal8Bit());
        QVERIFY2(model.item(i, NotificationListModel::ReasonRole)
                         == NotificationListModel::NotificationListModelReason::ReasonMention,
                 QString("reason=%1")
                         .arg(model.item(i++, NotificationListModel::ReasonRole).toInt())
                         .toLocal8Bit());
        QVERIFY2(model.item(i, NotificationListModel::ReasonRole)
                         == NotificationListModel::NotificationListModelReason::ReasonQuote,
                 QString("reason=%1")
                         .arg(model.item(i++, NotificationListModel::ReasonRole).toInt())
                         .toLocal8Bit());
    }

    {
        int i = 0;
        QSignalSpy spy(&model, SIGNAL(runningChanged()));
        model.setVisibleLike(true);
        model.setVisibleRepost(true);
        model.setVisibleFollow(true);
        model.setVisibleMention(true);
        model.setVisibleReply(true);
        model.setVisibleQuote(false);
        model.clear();
        model.getLatest();
        spy.wait(10 * 1000);
        QVERIFY2(spy.count() == 2, QString("spy.count()=%1").arg(spy.count()).toUtf8());

        QVERIFY2(model.rowCount() == 5, QString("rowCount()=%1").arg(model.rowCount()).toUtf8());
        QVERIFY2(model.item(i, NotificationListModel::ReasonRole)
                         == NotificationListModel::NotificationListModelReason::ReasonLike,
                 QString("reason=%1")
                         .arg(model.item(i++, NotificationListModel::ReasonRole).toInt())
                         .toLocal8Bit());
        QVERIFY2(model.item(i, NotificationListModel::ReasonRole)
                         == NotificationListModel::NotificationListModelReason::ReasonRepost,
                 QString("reason=%1")
                         .arg(model.item(i++, NotificationListModel::ReasonRole).toInt())
                         .toLocal8Bit());
        QVERIFY2(model.item(i, NotificationListModel::ReasonRole)
                         == NotificationListModel::NotificationListModelReason::ReasonFollow,
                 QString("reason=%1")
                         .arg(model.item(i++, NotificationListModel::ReasonRole).toInt())
                         .toLocal8Bit());
        QVERIFY2(model.item(i, NotificationListModel::ReasonRole)
                         == NotificationListModel::NotificationListModelReason::ReasonMention,
                 QString("reason=%1")
                         .arg(model.item(i++, NotificationListModel::ReasonRole).toInt())
                         .toLocal8Bit());
        QVERIFY2(model.item(i, NotificationListModel::ReasonRole)
                         == NotificationListModel::NotificationListModelReason::ReasonReply,
                 QString("reason=%1")
                         .arg(model.item(i++, NotificationListModel::ReasonRole).toInt())
                         .toLocal8Bit());
    }
}

void hagoromo_test::test_NotificationListModel2()
{
    NotificationListModel model;
    model.setAccount(m_service + "/notifications/visible", QString(), QString(), QString(), "dummy",
                     QString());

    int i = 0;
    QSignalSpy spy(&model, SIGNAL(runningChanged()));
    model.setVisibleLike(false);
    model.setVisibleRepost(false);
    model.setVisibleFollow(false);
    model.setVisibleMention(false);
    model.setVisibleReply(false);
    model.setVisibleQuote(false);
    model.getLatest();
    spy.wait(10 * 1000);
    QVERIFY2(spy.count() == 2, QString("spy.count()=%1").arg(spy.count()).toUtf8());

    QVERIFY2(model.rowCount() == 0, QString("rowCount()=%1").arg(model.rowCount()).toUtf8());

    //   like
    //   repost
    //   follow
    //   mention
    // * reply
    //   quote
    model.setVisibleReply(true);
    //    model.reflectVisibility();
    QVERIFY2(model.rowCount() == 1, QString("rowCount()=%1").arg(model.rowCount()).toUtf8());
    QVERIFY2(model.item(i, NotificationListModel::ReasonRole)
                     == NotificationListModel::NotificationListModelReason::ReasonReply,
             QString("reason=%1")
                     .arg(model.item(i++, NotificationListModel::ReasonRole).toInt())
                     .toLocal8Bit());

    //   like
    //   repost
    // * follow
    //   mention
    // * reply
    //   quote
    i = 0;
    model.setVisibleFollow(true);
    //    model.reflectVisibility();
    QVERIFY2(model.rowCount() == 2, QString("rowCount()=%1").arg(model.rowCount()).toUtf8());
    QVERIFY2(model.item(i, NotificationListModel::ReasonRole)
                     == NotificationListModel::NotificationListModelReason::ReasonFollow,
             QString("reason=%1")
                     .arg(model.item(i++, NotificationListModel::ReasonRole).toInt())
                     .toLocal8Bit());
    QVERIFY2(model.item(i, NotificationListModel::ReasonRole)
                     == NotificationListModel::NotificationListModelReason::ReasonReply,
             QString("reason=%1")
                     .arg(model.item(i++, NotificationListModel::ReasonRole).toInt())
                     .toLocal8Bit());

    //   like
    //   repost
    // * follow
    // * mention
    // * reply
    //   quote
    i = 0;
    model.setVisibleMention(true);
    //    model.reflectVisibility();
    QVERIFY2(model.rowCount() == 3, QString("rowCount()=%1").arg(model.rowCount()).toUtf8());
    QVERIFY2(model.item(i, NotificationListModel::ReasonRole)
                     == NotificationListModel::NotificationListModelReason::ReasonFollow,
             QString("reason=%1")
                     .arg(model.item(i++, NotificationListModel::ReasonRole).toInt())
                     .toLocal8Bit());
    QVERIFY2(model.item(i, NotificationListModel::ReasonRole)
                     == NotificationListModel::NotificationListModelReason::ReasonMention,
             QString("reason=%1")
                     .arg(model.item(i++, NotificationListModel::ReasonRole).toInt())
                     .toLocal8Bit());
    QVERIFY2(model.item(i, NotificationListModel::ReasonRole)
                     == NotificationListModel::NotificationListModelReason::ReasonReply,
             QString("reason=%1")
                     .arg(model.item(i++, NotificationListModel::ReasonRole).toInt())
                     .toLocal8Bit());

    // * like
    // * repost
    // * follow
    // * mention
    // * reply
    //   quote
    i = 0;
    model.setVisibleLike(true);
    model.setVisibleRepost(true);
    //    model.reflectVisibility();
    QVERIFY2(model.rowCount() == 5, QString("rowCount()=%1").arg(model.rowCount()).toUtf8());
    QVERIFY2(model.item(i, NotificationListModel::ReasonRole)
                     == NotificationListModel::NotificationListModelReason::ReasonLike,
             QString("reason=%1")
                     .arg(model.item(i++, NotificationListModel::ReasonRole).toInt())
                     .toLocal8Bit());
    QVERIFY2(model.item(i, NotificationListModel::ReasonRole)
                     == NotificationListModel::NotificationListModelReason::ReasonRepost,
             QString("reason=%1")
                     .arg(model.item(i++, NotificationListModel::ReasonRole).toInt())
                     .toLocal8Bit());
    QVERIFY2(model.item(i, NotificationListModel::ReasonRole)
                     == NotificationListModel::NotificationListModelReason::ReasonFollow,
             QString("reason=%1")
                     .arg(model.item(i++, NotificationListModel::ReasonRole).toInt())
                     .toLocal8Bit());
    QVERIFY2(model.item(i, NotificationListModel::ReasonRole)
                     == NotificationListModel::NotificationListModelReason::ReasonMention,
             QString("reason=%1")
                     .arg(model.item(i++, NotificationListModel::ReasonRole).toInt())
                     .toLocal8Bit());
    QVERIFY2(model.item(i, NotificationListModel::ReasonRole)
                     == NotificationListModel::NotificationListModelReason::ReasonReply,
             QString("reason=%1")
                     .arg(model.item(i++, NotificationListModel::ReasonRole).toInt())
                     .toLocal8Bit());

    // * like
    // * repost
    // * follow
    // * mention
    // * reply
    // * quote
    i = 0;
    model.setVisibleQuote(true);
    //    model.reflectVisibility();
    QVERIFY2(model.rowCount() == 6, QString("rowCount()=%1").arg(model.rowCount()).toUtf8());
    QVERIFY2(model.item(i, NotificationListModel::ReasonRole)
                     == NotificationListModel::NotificationListModelReason::ReasonLike,
             QString("reason=%1")
                     .arg(model.item(i++, NotificationListModel::ReasonRole).toInt())
                     .toLocal8Bit());
    QVERIFY2(model.item(i, NotificationListModel::ReasonRole)
                     == NotificationListModel::NotificationListModelReason::ReasonRepost,
             QString("reason=%1")
                     .arg(model.item(i++, NotificationListModel::ReasonRole).toInt())
                     .toLocal8Bit());
    QVERIFY2(model.item(i, NotificationListModel::ReasonRole)
                     == NotificationListModel::NotificationListModelReason::ReasonFollow,
             QString("reason=%1")
                     .arg(model.item(i++, NotificationListModel::ReasonRole).toInt())
                     .toLocal8Bit());
    QVERIFY2(model.item(i, NotificationListModel::ReasonRole)
                     == NotificationListModel::NotificationListModelReason::ReasonMention,
             QString("reason=%1")
                     .arg(model.item(i++, NotificationListModel::ReasonRole).toInt())
                     .toLocal8Bit());
    QVERIFY2(model.item(i, NotificationListModel::ReasonRole)
                     == NotificationListModel::NotificationListModelReason::ReasonReply,
             QString("reason=%1")
                     .arg(model.item(i++, NotificationListModel::ReasonRole).toInt())
                     .toLocal8Bit());
    QVERIFY2(model.item(i, NotificationListModel::ReasonRole)
                     == NotificationListModel::NotificationListModelReason::ReasonQuote,
             QString("reason=%1")
                     .arg(model.item(i++, NotificationListModel::ReasonRole).toInt())
                     .toLocal8Bit());

    // * like
    // * repost
    //   follow
    //   mention
    // * reply
    // * quote
    i = 0;
    model.setVisibleFollow(false);
    model.setVisibleMention(false);
    //    model.reflectVisibility();
    QVERIFY2(model.rowCount() == 4, QString("rowCount()=%1").arg(model.rowCount()).toUtf8());
    QVERIFY2(model.item(i, NotificationListModel::ReasonRole)
                     == NotificationListModel::NotificationListModelReason::ReasonLike,
             QString("reason=%1")
                     .arg(model.item(i++, NotificationListModel::ReasonRole).toInt())
                     .toLocal8Bit());
    QVERIFY2(model.item(i, NotificationListModel::ReasonRole)
                     == NotificationListModel::NotificationListModelReason::ReasonRepost,
             QString("reason=%1")
                     .arg(model.item(i++, NotificationListModel::ReasonRole).toInt())
                     .toLocal8Bit());
    QVERIFY2(model.item(i, NotificationListModel::ReasonRole)
                     == NotificationListModel::NotificationListModelReason::ReasonReply,
             QString("reason=%1")
                     .arg(model.item(i++, NotificationListModel::ReasonRole).toInt())
                     .toLocal8Bit());
    QVERIFY2(model.item(i, NotificationListModel::ReasonRole)
                     == NotificationListModel::NotificationListModelReason::ReasonQuote,
             QString("reason=%1")
                     .arg(model.item(i++, NotificationListModel::ReasonRole).toInt())
                     .toLocal8Bit());

    // * like
    //   repost
    //   follow
    // * mention
    // * reply
    // * quote
    i = 0;
    model.setVisibleRepost(false);
    model.setVisibleMention(true);
    //    model.reflectVisibility();
    QVERIFY2(model.rowCount() == 4, QString("rowCount()=%1").arg(model.rowCount()).toUtf8());
    QVERIFY2(model.item(i, NotificationListModel::ReasonRole)
                     == NotificationListModel::NotificationListModelReason::ReasonLike,
             QString("reason=%1")
                     .arg(model.item(i++, NotificationListModel::ReasonRole).toInt())
                     .toLocal8Bit());
    QVERIFY2(model.item(i, NotificationListModel::ReasonRole)
                     == NotificationListModel::NotificationListModelReason::ReasonMention,
             QString("reason=%1")
                     .arg(model.item(i++, NotificationListModel::ReasonRole).toInt())
                     .toLocal8Bit());
    QVERIFY2(model.item(i, NotificationListModel::ReasonRole)
                     == NotificationListModel::NotificationListModelReason::ReasonReply,
             QString("reason=%1")
                     .arg(model.item(i++, NotificationListModel::ReasonRole).toInt())
                     .toLocal8Bit());
    QVERIFY2(model.item(i, NotificationListModel::ReasonRole)
                     == NotificationListModel::NotificationListModelReason::ReasonQuote,
             QString("reason=%1")
                     .arg(model.item(i++, NotificationListModel::ReasonRole).toInt())
                     .toLocal8Bit());
}

void hagoromo_test::test_UserProfile()
{
    UserProfile profile;

    profile.setDescription("test \r\ntest\ntest");
    QVERIFY(profile.description() == "test <br/>test<br/>test");

    profile.setDescription("https://github.com/ioriayane/Hagoromo");
    QVERIFY(profile.description()
            == "<a "
               "href=\"https://github.com/ioriayane/Hagoromo\">https://github.com/ioriayane/"
               "Hagoromo</a>");

    profile.setDescription("hoge https://github.com/ioriayane/Hagoromo");
    QVERIFY(profile.description()
            == "hoge <a "
               "href=\"https://github.com/ioriayane/Hagoromo\">https://github.com/ioriayane/"
               "Hagoromo</a>");

    profile.setDescription("https://github.com/ioriayane/Hagoromo hoge");
    QVERIFY(profile.description()
            == "<a "
               "href=\"https://github.com/ioriayane/Hagoromo\">https://github.com/ioriayane/"
               "Hagoromo</a> hoge");

    profile.setDescription("hoge\nhttps://github.com/ioriayane/Hagoromo");
    QVERIFY(profile.description()
            == "hoge<br/><a "
               "href=\"https://github.com/ioriayane/Hagoromo\">https://github.com/ioriayane/"
               "Hagoromo</a>");

    profile.setDescription("https://github.com/ioriayane/Hagoromo\nhoge");
    QVERIFY(profile.description()
            == "<a "
               "href=\"https://github.com/ioriayane/Hagoromo\">https://github.com/ioriayane/"
               "Hagoromo</a><br/>hoge");
}

void hagoromo_test::test_charCount()
{
    QFile file(":/string/length.txt");
    QVERIFY(file.open(QFile::ReadOnly));

    QTextStream ts(&file);
    ts.setCodec("utf-8");
    QStringEx text = ts.readAll();
    qDebug() << text << text.length() << text.count();
    for (int i = 0; i < text.count(); i++) {
        qDebug() << i << text.at(i) << text.at(i).isHighSurrogate() << text.at(i).isLowSurrogate()
                 << QString::number(text.at(i).unicode(), 16) << text.at(i).combiningClass()
                 << text.at(i).joiningType();
    }
}

void hagoromo_test::test_TimelineListModel_quote_warn()
{
    int row = 0;
    TimelineListModel model;
    model.setAccount(m_service + "/timeline/warn", QString(), QString(), QString(), "dummy",
                     QString());
    model.setDisplayInterval(0);

    QSignalSpy spy(&model, SIGNAL(runningChanged()));
    model.getLatest();
    spy.wait(10 * 1000);
    QVERIFY2(spy.count() == 2, QString("spy.count()=%1").arg(spy.count()).toUtf8());

    QVERIFY(model.rowCount() == 7);

    row = 0;
    QVERIFY(model.item(row, TimelineListModel::RecordTextRole).toString()
            == "quoted mute user's post");
    QVERIFY(model.item(row, TimelineListModel::QuoteFilterMatchedRole).toBool() == true);
    QVERIFY(model.item(row, TimelineListModel::QuoteRecordBlockedRole).toBool() == false);

    row = 1;
    QVERIFY(model.item(row, TimelineListModel::RecordTextRole).toString()
            == "quote mute user test with image");
    QVERIFY(model.item(row, TimelineListModel::QuoteFilterMatchedRole).toBool() == true);
    QVERIFY(model.item(row, TimelineListModel::QuoteRecordBlockedRole).toBool() == false);

    row = 2;
    QVERIFY(model.item(row, TimelineListModel::RecordTextRole).toString()
            == "quote a post with warn label added");
    QVERIFY(model.item(row, TimelineListModel::QuoteFilterMatchedRole).toBool() == true);
    QVERIFY(model.item(row, TimelineListModel::QuoteRecordBlockedRole).toBool() == false);

    row = 3;
    QVERIFY(model.item(row, TimelineListModel::RecordTextRole).toString()
            == "quote a post with labeling image");
    QVERIFY(model.item(row, TimelineListModel::QuoteFilterMatchedRole).toBool() == true);
    QVERIFY(model.item(row, TimelineListModel::QuoteRecordBlockedRole).toBool() == false);

    row = 4;
    QVERIFY(model.item(row, TimelineListModel::RecordTextRole).toString()
            == "quote a post with warn label added with image");
    QVERIFY(model.item(row, TimelineListModel::QuoteFilterMatchedRole).toBool() == true);
    QVERIFY(model.item(row, TimelineListModel::QuoteRecordBlockedRole).toBool() == false);

    row = 5;
    QVERIFY(model.item(row, TimelineListModel::RecordTextRole).toString()
            == "quote a post with labeling image with image");
    QVERIFY(model.item(row, TimelineListModel::QuoteFilterMatchedRole).toBool() == true);
    QVERIFY(model.item(row, TimelineListModel::QuoteRecordBlockedRole).toBool() == false);

    row = 6;
    QVERIFY(model.item(row, TimelineListModel::RecordTextRole).toString()
            == "quote blocked user's post");
    QVERIFY(model.item(row, TimelineListModel::QuoteFilterMatchedRole).toBool() == false);
    QVERIFY(model.item(row, TimelineListModel::QuoteRecordBlockedRole).toBool() == true);
}

void hagoromo_test::test_TimelineListModel_quote_hide()
{
    int row = 0;
    TimelineListModel model;
    model.setAccount(m_service + "/timeline/hide", "did:plc:l4fsx4ujos7uw7n4ijq2ulgs",
                     "ioriayane.bsky.social", QString(), "dummy", QString());
    model.setDisplayInterval(0);

    QSignalSpy spy(&model, SIGNAL(runningChanged()));
    model.getLatest();
    spy.wait();
    QVERIFY2(spy.count() == 2, QString("spy.count()=%1").arg(spy.count()).toUtf8());

    QVERIFY(model.rowCount() == 7);

    row = 0;
    QVERIFY(model.item(row, TimelineListModel::RecordTextRole).toString()
            == "quoted mute user's post");
    QVERIFY(model.item(row, TimelineListModel::QuoteFilterMatchedRole).toBool() == true);
    QVERIFY(model.item(row, TimelineListModel::QuoteRecordBlockedRole).toBool() == false);

    row = 1;
    QVERIFY(model.item(row, TimelineListModel::RecordTextRole).toString()
            == "quote mute user test with image");
    QVERIFY(model.item(row, TimelineListModel::QuoteFilterMatchedRole).toBool() == true);
    QVERIFY(model.item(row, TimelineListModel::QuoteRecordBlockedRole).toBool() == false);

    row = 2;
    QVERIFY(model.item(row, TimelineListModel::RecordTextRole).toString()
            == "quote a post with warn label added");
    QVERIFY(model.item(row, TimelineListModel::QuoteFilterMatchedRole).toBool() == true);
    QVERIFY(model.item(row, TimelineListModel::QuoteRecordBlockedRole).toBool() == false);

    row = 3;
    QVERIFY(model.item(row, TimelineListModel::RecordTextRole).toString()
            == "quote a post with labeling image");
    QVERIFY(model.item(row, TimelineListModel::QuoteFilterMatchedRole).toBool() == false);
    QVERIFY(model.item(row, TimelineListModel::QuoteRecordBlockedRole).toBool() == true);

    row = 4;
    QVERIFY(model.item(row, TimelineListModel::RecordTextRole).toString()
            == "quote a post with warn label added with image");
    QVERIFY(model.item(row, TimelineListModel::QuoteFilterMatchedRole).toBool() == true);
    QVERIFY(model.item(row, TimelineListModel::QuoteRecordBlockedRole).toBool() == false);

    row = 5;
    QVERIFY(model.item(row, TimelineListModel::RecordTextRole).toString()
            == "quote a post with labeling image with image");
    QVERIFY(model.item(row, TimelineListModel::QuoteFilterMatchedRole).toBool() == false);
    QVERIFY(model.item(row, TimelineListModel::QuoteRecordBlockedRole).toBool() == true);

    row = 6;
    QVERIFY(model.item(row, TimelineListModel::RecordTextRole).toString()
            == "quote blocked user's post");
    QVERIFY(model.item(row, TimelineListModel::QuoteFilterMatchedRole).toBool() == false);
    QVERIFY(model.item(row, TimelineListModel::QuoteRecordBlockedRole).toBool() == true);
}

void hagoromo_test::test_TimelineListModel_quote_hide2()
{
    // 自分のポストが引用されているのを見るイメージ
    // 自分のポストの引用はHide設定でも隠さない
    int row = 0;
    TimelineListModel model;
    model.setAccount(m_service + "/timeline/hide", "did:plc:mqxsuw5b5rhpwo4lw6iwlid5",
                     "ioriayane2.bsky.social", QString(), "dummy", QString());
    model.setDisplayInterval(0);

    QSignalSpy spy(&model, SIGNAL(runningChanged()));
    model.getLatest();
    spy.wait();
    QVERIFY2(spy.count() == 2, QString("spy.count()=%1").arg(spy.count()).toUtf8());

    QVERIFY(model.rowCount() == 7);

    row = 0;
    QVERIFY(model.item(row, TimelineListModel::RecordTextRole).toString()
            == "quoted mute user's post");
    QVERIFY(model.item(row, TimelineListModel::QuoteFilterMatchedRole).toBool() == true);
    QVERIFY(model.item(row, TimelineListModel::QuoteRecordBlockedRole).toBool() == false);

    row = 1;
    QVERIFY(model.item(row, TimelineListModel::RecordTextRole).toString()
            == "quote mute user test with image");
    QVERIFY(model.item(row, TimelineListModel::QuoteFilterMatchedRole).toBool() == true);
    QVERIFY(model.item(row, TimelineListModel::QuoteRecordBlockedRole).toBool() == false);

    row = 2;
    QVERIFY(model.item(row, TimelineListModel::RecordTextRole).toString()
            == "quote a post with warn label added");
    QVERIFY(model.item(row, TimelineListModel::QuoteFilterMatchedRole).toBool() == false);
    QVERIFY(model.item(row, TimelineListModel::QuoteRecordBlockedRole).toBool() == false);

    row = 3;
    QVERIFY(model.item(row, TimelineListModel::RecordTextRole).toString()
            == "quote a post with labeling image");
    QVERIFY(model.item(row, TimelineListModel::QuoteFilterMatchedRole).toBool() == false);
    QVERIFY(model.item(row, TimelineListModel::QuoteRecordBlockedRole).toBool() == false);
    QVERIFY(model.item(row, TimelineListModel::QuoteRecordEmbedImagesRole).toStringList()
            == QStringList()
                    << "https://cdn.bsky.social/imgproxy/"
                       "5Yw3gWICYYm-gCp6LP206jY_NGm3iPn2iH9BD4pw1ZU/rs:fit:1000:1000:1:0/plain/"
                       "bafkreibmmux3wklplvddwjqszdzx3vnvfllhjrbqsnlgtt6fax7ajdjy5y@jpeg");
    QVERIFY(model.item(row, TimelineListModel::QuoteRecordEmbedImagesFullRole).toStringList()
            == QStringList()
                    << "https://cdn.bsky.social/imgproxy/"
                       "k46B3Cqu4IiOyilM2gKVFXUWl_6epvzX6d_v6OnyuE0/rs:fit:2000:2000:1:0/plain/"
                       "bafkreibmmux3wklplvddwjqszdzx3vnvfllhjrbqsnlgtt6fax7ajdjy5y@jpeg");

    row = 4;
    QVERIFY(model.item(row, TimelineListModel::RecordTextRole).toString()
            == "quote a post with warn label added with image");
    QVERIFY(model.item(row, TimelineListModel::QuoteFilterMatchedRole).toBool() == false);
    QVERIFY(model.item(row, TimelineListModel::QuoteRecordBlockedRole).toBool() == false);

    row = 5;
    QVERIFY(model.item(row, TimelineListModel::RecordTextRole).toString()
            == "quote a post with labeling image with image");
    QVERIFY(model.item(row, TimelineListModel::QuoteFilterMatchedRole).toBool() == false);
    QVERIFY(model.item(row, TimelineListModel::QuoteRecordBlockedRole).toBool() == false);

    row = 6;
    QVERIFY(model.item(row, TimelineListModel::RecordTextRole).toString()
            == "quote blocked user's post");
    QVERIFY(model.item(row, TimelineListModel::QuoteFilterMatchedRole).toBool() == false);
    QVERIFY(model.item(row, TimelineListModel::QuoteRecordBlockedRole).toBool() == true);
    QVERIFY(model.item(row, TimelineListModel::LabelsRole).toStringList() == QStringList());
}

void hagoromo_test::test_TimelineListModel_quote_label()
{
    int row = 0;
    TimelineListModel model;
    model.setAccount(m_service + "/timeline/labels", QString(), QString(), QString(), "dummy",
                     QString());
    model.setDisplayInterval(0);

    QSignalSpy spy(&model, SIGNAL(runningChanged()));
    model.getLatest();
    spy.wait();
    QVERIFY2(spy.count() == 2, QString("spy.count()=%1").arg(spy.count()).toUtf8());

    QVERIFY(model.rowCount() == 9);

    row = 0;
    QVERIFY(model.item(row, TimelineListModel::RecordTextRole).toString()
            == "quoted mute user's post");
    QVERIFY(model.item(row, TimelineListModel::QuoteFilterMatchedRole).toBool() == true);
    QVERIFY(model.item(row, TimelineListModel::QuoteRecordBlockedRole).toBool() == false);
    QVERIFY(model.item(row, TimelineListModel::LabelsRole).toStringList() == QStringList());
    QVERIFY(model.item(row, TimelineListModel::ReplyRootCidRole).toString().isEmpty());
    QVERIFY(model.item(row, TimelineListModel::ReplyRootUriRole).toString().isEmpty());

    row = 3;
    QVERIFY(model.item(row, TimelineListModel::RecordTextRole).toString()
            == "quote a post with labeling image");
    QVERIFY(model.item(row, TimelineListModel::LabelsRole).toStringList()
            == QStringList() << "sexual");

    row = 4;
    QVERIFY(model.item(row, TimelineListModel::RecordTextRole).toString()
            == "quote a post with warn label added with image");
    QVERIFY(model.item(row, TimelineListModel::LabelsRole).toStringList()
            == QStringList() << "sexual"
                             << "!warn");

    row = 7;
    QVERIFY(model.item(row, TimelineListModel::RecordTextRole).toString() == "test reply full");
    QVERIFY(model.item(row, TimelineListModel::ReplyRootCidRole).toString()
            == "bafyreievv2yz3obnigwjix5kr2icycfkqdobrfufd3cm4wfavnjfeqhxbe");
    QVERIFY(model.item(row, TimelineListModel::ReplyRootUriRole).toString()
            == "at://did:plc:mqxsuw5b5rhpwo4lw6iwlid5/app.bsky.feed.post/3k7n55v57bn26");

    row = 8;
    QVERIFY(model.item(row, TimelineListModel::RecordTextRole).toString() == "test reply simple");
    QVERIFY(model.item(row, TimelineListModel::ReplyRootCidRole).toString()
            == "bafyreievv2yz3obnigwjix5kr2icycfkqdobrfufd3cm4wfavnjfeqhxbe");
    QVERIFY(model.item(row, TimelineListModel::ReplyRootUriRole).toString()
            == "at://did:plc:mqxsuw5b5rhpwo4lw6iwlid5/app.bsky.feed.post/3k7n55v57bn26");
}

void hagoromo_test::test_NotificationListModel_warn()
{
    int row = 0;
    NotificationListModel model;
    model.setAccount(m_service + "/notifications/warn", QString(), QString(), QString(), "dummy",
                     QString());
    model.setDisplayInterval(0);

    QSignalSpy spy(&model, SIGNAL(runningChanged()));
    model.getLatest();
    spy.wait(10 * 1000);
    QVERIFY2(spy.count() == 2, QString("spy.count()=%1").arg(spy.count()).toUtf8());

    QVERIFY(model.rowCount() == 2);

    row = 0;
    QVERIFY(model.item(row, NotificationListModel::RecordTextPlainRole).toString()
            == "@ioriayane2.bsky.social labeling sexual with image");
    QVERIFY(model.item(row, NotificationListModel::ContentFilterMatchedRole).toBool() == false);
    QVERIFY(model.item(row, NotificationListModel::ContentMediaFilterMatchedRole).toBool() == true);
    QVERIFY(model.item(row, NotificationListModel::EmbedImagesRole).toStringList()
            == QStringList()
                    << "https://cdn.bsky.social/imgproxy/"
                       "5Yw3gWICYYm-gCp6LP206jY_NGm3iPn2iH9BD4pw1ZU/rs:fit:1000:1000:1:0/plain/"
                       "bafkreibmmux3wklplvddwjqszdzx3vnvfllhjrbqsnlgtt6fax7ajdjy5y@jpeg");
    QVERIFY(model.item(row, NotificationListModel::EmbedImagesFullRole).toStringList()
            == QStringList()
                    << "https://cdn.bsky.social/imgproxy/"
                       "k46B3Cqu4IiOyilM2gKVFXUWl_6epvzX6d_v6OnyuE0/rs:fit:2000:2000:1:0/plain/"
                       "bafkreibmmux3wklplvddwjqszdzx3vnvfllhjrbqsnlgtt6fax7ajdjy5y@jpeg");

    row = 1;
    QVERIFY(model.item(row, NotificationListModel::RecordTextPlainRole).toString()
            == "@ioriayane2.bsky.social labeling warn");
    QVERIFY(model.item(row, NotificationListModel::ContentFilterMatchedRole).toBool() == true);
    QVERIFY(model.item(row, NotificationListModel::ContentMediaFilterMatchedRole).toBool()
            == false);
}

void hagoromo_test::test_TimelineListModel_next()
{
    int row = 0;
    TimelineListModel model;
    model.setAccount(m_service + "/timeline/next/1st", QString(), QString(), QString(), "dummy",
                     QString());
    model.setDisplayInterval(0);
    {
        QSignalSpy spy(&model, SIGNAL(runningChanged()));
        model.getLatest();
        spy.wait(10 * 1000);
        QVERIFY2(spy.count() == 2, QString("spy.count()=%1").arg(spy.count()).toUtf8());
    }
    QVERIFY(model.rowCount() == 6);

    row = 0;
    QVERIFY(model.item(row, TimelineListModel::CidRole).toString()
            == "bafyreig7i2uyva4rpgxv3slogiwf5fvlwy2wx4bjvwuoywy6e7ojjvcrky_1");
    row = 1;
    QVERIFY(model.item(row, TimelineListModel::CidRole).toString()
            == "bafyreiajxwbfoa5cbnphxcwvunisgjiqkjjqkqxpnr4mgfu3vqqupr6wca_2");
    row = 2;
    QVERIFY(model.item(row, TimelineListModel::CidRole).toString()
            == "bafyreif2ux6qo4b2ez266iewichrjvqgeeehui5c7lo3gcglrzdi54pjja_3");
    QVERIFY(model.item(row, TimelineListModel::IsRepostedRole).toBool() == true);

    row = 3;
    QVERIFY(model.item(row, TimelineListModel::CidRole).toString()
            == "bafyreidmpxa2bdcohuxa4p62q3d6oja75mohihxsliuasvmsxds3utokqa_4");
    row = 4;
    QVERIFY(model.item(row, TimelineListModel::CidRole).toString()
            == "bafyreif2ux6qo4b2ez266iewichrjvqgeeehui5c7lo3gcglrzdi54pjjb_5");
    row = 5;
    QVERIFY(model.item(row, TimelineListModel::CidRole).toString()
            == "bafyreihr2hrmavhzdpmnc65udreph5vfmd3xceqtw2jm3b4clbfbacgsqe_6");

    model.setAccount(m_service + "/timeline/next/2nd", QString(), QString(), QString(), "dummy",
                     QString());
    model.setDisplayInterval(0);
    {
        QSignalSpy spy(&model, SIGNAL(runningChanged()));
        model.getNext();
        spy.wait();
        QVERIFY2(spy.count() == 2, QString("spy.count()=%1").arg(spy.count()).toUtf8());
    }
    QVERIFY(model.rowCount() == 13);

    row = 6;
    QVERIFY(model.item(row, TimelineListModel::CidRole).toString()
            == "bafyreiejog3yvjc2tdg4muknodbplaib2yqftukwurd4qjcnal3zdxu4ni_7");
    row = 7;
    QVERIFY2(model.item(row, TimelineListModel::CidRole).toString()
                     == "bafyreib67ewj54g6maljtbclhno7mrkquf3w7wbex2woedj5m23mjwyite_8",
             model.item(row, TimelineListModel::CidRole).toString().toLocal8Bit());
    row = 8;
    QVERIFY(model.item(row, TimelineListModel::CidRole).toString()
            == "bafyreigj7v4cnmqpu5jiaqk2e4z7lele7toehjjbzbgmnaydufkayrsrly_9");
    row = 9;
    QVERIFY(model.item(row, TimelineListModel::CidRole).toString()
            == "bafyreiemuasu6a6snzjjhke5tr3f462bfz62t7yqlidkxrpnedbzbrnnou_10");
    row = 10;
    QVERIFY(model.item(row, TimelineListModel::CidRole).toString()
            == "bafyreiegferq3itlq4qapotqm3udyi3eobdigoptoyxubvudv5ttrjf5ka_11");
    row = 11;
    QVERIFY(model.item(row, TimelineListModel::CidRole).toString()
            == "bafyreia3gadukf62bq3pq46kr3ewjpsao2ltbrlaios332oxqfggbaqha4_12");
    row = 12;
    QVERIFY(model.item(row, TimelineListModel::CidRole).toString()
            == "bafyreiejog3yvjc2tdg4muknodbplaib2yqftukwurd4qjcnal3zdxu4ni11_13");
}

void hagoromo_test::test_FeedGeneratorLink()
{
    FeedGeneratorLink link;

    QVERIFY(link.checkUri("https://bsky.app/profile/did:plc:hoge/feed/aaaaaaaa") == true);
    QVERIFY(link.checkUri("https://staging.bsky.app/profile/did:plc:hoge/feed/aaaaaaaa") == false);
    QVERIFY(link.checkUri("https://bsky.app/feeds/did:plc:hoge/feed/aaaaaaaa") == false);
    QVERIFY(link.checkUri("https://bsky.app/profile/did:plc:hoge/feeds/aaaaaaaa") == false);
    QVERIFY(link.checkUri("https://bsky.app/profile/did:plc:hoge/feed/") == false);
    QVERIFY(link.checkUri("https://bsky.app/profile/handle/feed/aaaaaaaa") == false);

    QVERIFY(link.convertToAtUri("https://bsky.app/profile/did:plc:hoge/feed/aaaaaaaa")
            == "at://did:plc:hoge/app.bsky.feed.generator/aaaaaaaa");

    link.setAccount(m_service + "/generator", QString(), QString(), QString(), "dummy", QString());
    {
        QSignalSpy spy(&link, SIGNAL(runningChanged()));
        link.getFeedGenerator("https://bsky.app/profile/did:plc:hoge/feed/aaaaaaaa");
        spy.wait();
        QVERIFY2(spy.count() == 2, QString("spy.count()=%1").arg(spy.count()).toUtf8());
    }

    QVERIFY(link.avatar() == "https://cdn.bsky.social/view_avator.jpeg");
    QVERIFY(link.displayName() == "view:displayName");
    QVERIFY(link.creatorHandle() == "creator.bsky.social");
    QVERIFY(link.likeCount() == 9);
}

void hagoromo_test::test_AnyProfileListModel()
{
    AnyProfileListModel model;
    model.setAccount(m_service + "/anyprofile", QString(), QString(), QString(), "dummy",
                     QString());
    model.setTargetUri("at://did:plc:ipj5qejfoqu6eukvt72uhyit/app.bsky.feed.post/3k6tpw4xr4d27");

    model.setType(AnyProfileListModel::AnyProfileListModelType::Like);
    {
        QSignalSpy spy(&model, SIGNAL(runningChanged()));
        model.getLatest();
        spy.wait();
        QVERIFY2(spy.count() == 2, QString("spy.count()=%1").arg(spy.count()).toUtf8());
    }

    QVERIFY(model.rowCount() == 5);

    model.setType(AnyProfileListModel::AnyProfileListModelType::Repost);
    {
        QSignalSpy spy(&model, SIGNAL(runningChanged()));
        model.getLatest();
        spy.wait();
        QVERIFY2(spy.count() == 2, QString("spy.count()=%1").arg(spy.count()).toUtf8());
    }

    QVERIFY(model.rowCount() == 2);
}

void hagoromo_test::test_AccountListModel()
{
    QString temp_path = Common::appDataFolder() + "/account.json";
    if (QFile::exists(temp_path)) {
        QFile::remove(temp_path);
    }

    AccountListModel model;

    model.updateAccount(m_service + "/account/account1", "id1", "password1", "did:plc:account1",
                        "account1.relog.tech", "account1@relog.tech", "accessJwt_account1",
                        "refreshJwt_account1", true);
    model.updateAccount(m_service + "/account/account2", "id2", "password2", "did:plc:account2",
                        "account2.relog.tech", "account2@relog.tech", "accessJwt_account2",
                        "refreshJwt_account2", true);

    AccountListModel model2;
    {
        QSignalSpy spy(&model2, SIGNAL(updatedAccount(int, const QString &)));
        model2.load();
        spy.wait(10 * 1000);
        spy.wait(10 * 1000);
        QVERIFY2(spy.count() == 2, QString("spy.count()=%1").arg(spy.count()).toUtf8());
    }
    QVERIFY2(model2.rowCount() == 2, QString::number(model2.rowCount()).toLocal8Bit());
    int row = 0;
    QVERIFY2(model2.item(row, AccountListModel::DidRole).toString() == "did:plc:account1_refresh",
             model2.item(row, AccountListModel::DidRole).toString().toLocal8Bit());
    QVERIFY2(model2.item(row, AccountListModel::RefreshJwtRole).toString()
                     == "refreshJwt_account1_refresh",
             model2.item(row, AccountListModel::RefreshJwtRole).toString().toLocal8Bit());
    row = 1;
    QVERIFY2(model2.item(row, AccountListModel::DidRole).toString() == "did:plc:account2_refresh",
             model2.item(row, AccountListModel::DidRole).toString().toLocal8Bit());
    QVERIFY2(model2.item(row, AccountListModel::RefreshJwtRole).toString()
                     == "refreshJwt_account2_refresh",
             model2.item(row, AccountListModel::RefreshJwtRole).toString().toLocal8Bit());
}

void hagoromo_test::test_TimelineListModel_text()
{
    int row = 0;
    TimelineListModel model;
    model.setAccount(m_service + "/timeline/text", QString(), QString(), QString(), "dummy",
                     QString());
    model.setDisplayInterval(0);

    QSignalSpy spy(&model, SIGNAL(runningChanged()));
    model.getLatest();
    spy.wait(10 * 1000);
    QVERIFY2(spy.count() == 2, QString("spy.count()=%1").arg(spy.count()).toUtf8());

    QVERIFY(model.rowCount() == 4);

    qDebug() << QString("hoge\nfuga<>\"foo").toHtmlEscaped();

    row = 0;
    QVERIFY(model.item(row, TimelineListModel::RecordTextPlainRole).toString() == "test < data");
    QVERIFY(model.item(row, TimelineListModel::RecordTextRole).toString() == "test &lt; data");

    row = 1;
    QVERIFY(model.item(row, TimelineListModel::RecordTextPlainRole).toString()
            == "@ioriayane.relog.tech resp.remainingPoints < lowest.remainingPoints");
    QVERIFY(model.item(row, TimelineListModel::RecordTextRole).toString()
            == "<a href=\"did:plc:ipj5qejfoqu6eukvt72uhyit\">@ioriayane.relog.tech</a> "
               "resp.remainingPoints &lt; lowest.remainingPoints");

    row = 2;
    qDebug().noquote()
            << model.item(row, TimelineListModel::RecordTextPlainRole).toString().toLocal8Bit();
    qDebug().noquote()
            << model.item(row, TimelineListModel::RecordTextRole).toString().toLocal8Bit();
}

void hagoromo_test::test_TimelineListModel_reply()
{
    int row = 0;
    TimelineListModel model;
    model.setAccount(m_service + "/timeline/reply", "did:plc:ipj5qejfoqu6eukvt72uhyit", QString(),
                     QString(), "dummy", QString());
    model.setDisplayInterval(0);

    model.setVisibleReplyToUnfollowedUsers(true);
    {
        QSignalSpy spy(&model, SIGNAL(runningChanged()));
        model.getLatest();
        spy.wait();
        QVERIFY2(spy.count() == 2, QString("spy.count()=%1").arg(spy.count()).toUtf8());
    }

    QVERIFY2(model.rowCount() == 4, QString("rowCount()=%1").arg(model.rowCount()).toLocal8Bit());
    QVERIFY(model.item(0, TimelineListModel::CidRole)
            == "bafyreidsfmfwqaoud3c64lzuxoblcfgcnkmurjcvgx67hmzgp5vhm3tcia_1");
    QVERIFY(model.item(1, TimelineListModel::CidRole)
            == "bafyreienuzckap2uhylwn2sq2pq5k7rj445exfxtzkrk7xby7dvfy4muoa_2");
    QVERIFY(model.item(2, TimelineListModel::CidRole)
            == "bafyreiayu2dsqvwfyumuepg6b62a24dwtntagoh5lkym7h3lawxcpvz7f4_3");
    QVERIFY(model.item(3, TimelineListModel::CidRole)
            == "bafyreievv2yz3obnigwjix5kr2icycfkqdobrfufd3cm4wfavnjfeqhxbe_4");

    model.setVisibleReplyToUnfollowedUsers(false);
    {
        QSignalSpy spy(&model, SIGNAL(runningChanged()));
        model.getLatest();
        spy.wait();
        QVERIFY2(spy.count() == 2, QString("spy.count()=%1").arg(spy.count()).toUtf8());
    }

    QVERIFY2(model.rowCount() == 3, QString("rowCount()=%1").arg(model.rowCount()).toLocal8Bit());
    QVERIFY(model.item(0, TimelineListModel::CidRole)
            == "bafyreidsfmfwqaoud3c64lzuxoblcfgcnkmurjcvgx67hmzgp5vhm3tcia_1");
    QVERIFY(model.item(1, TimelineListModel::CidRole)
            == "bafyreienuzckap2uhylwn2sq2pq5k7rj445exfxtzkrk7xby7dvfy4muoa_2");
    QVERIFY(model.item(2, TimelineListModel::CidRole)
            == "bafyreievv2yz3obnigwjix5kr2icycfkqdobrfufd3cm4wfavnjfeqhxbe_4");
}

void hagoromo_test::test_PostThreadListModel()
{
    PostThreadListModel model;
    model.setAccount(m_service + "/postthread/1", QString(), QString(), QString(), "dummy",
                     QString());
    model.setDisplayInterval(0);
    model.setPostThreadUri("at://uri");

    {
        QSignalSpy spy(&model, SIGNAL(runningChanged()));
        model.getLatest();
        spy.wait(10 * 1000);
        QVERIFY2(spy.count() == 2, QString("spy.count()=%1").arg(spy.count()).toUtf8());
    }

    int row;

    QVERIFY2(model.rowCount() == 4, QString("rowCount()=%1").arg(model.rowCount()).toLocal8Bit());
    row = 0;
    QVERIFY(model.item(row, PostThreadListModel::RecordTextPlainRole).toString() == "test");
    QVERIFY(model.item(row, PostThreadListModel::ThreadConnectorTopRole).toBool() == false);
    QVERIFY(model.item(row, PostThreadListModel::ThreadConnectorBottomRole).toBool() == true);
    row = 1;
    QVERIFY(model.item(row, PostThreadListModel::RecordTextPlainRole).toString() == "reply 2");
    QVERIFY(model.item(row, PostThreadListModel::ThreadConnectorTopRole).toBool() == true);
    QVERIFY(model.item(row, PostThreadListModel::ThreadConnectorBottomRole).toBool() == true);
    row = 2;
    QVERIFY(model.item(row, PostThreadListModel::RecordTextPlainRole).toString() == "reply 3");
    QVERIFY(model.item(row, PostThreadListModel::ThreadConnectorTopRole).toBool() == true);
    QVERIFY(model.item(row, PostThreadListModel::ThreadConnectorBottomRole).toBool() == true);
    row = 3;
    QVERIFY(model.item(row, PostThreadListModel::RecordTextPlainRole).toString() == "reply 4");
    QVERIFY(model.item(row, PostThreadListModel::ThreadConnectorTopRole).toBool() == true);
    QVERIFY(model.item(row, PostThreadListModel::ThreadConnectorBottomRole).toBool() == false);

    model.clear();
    model.setAccount(m_service + "/postthread/2", QString(), QString(), QString(), "dummy",
                     QString());
    {
        QSignalSpy spy(&model, SIGNAL(runningChanged()));
        model.getLatest();
        spy.wait();
        QVERIFY2(spy.count() == 2, QString("spy.count()=%1").arg(spy.count()).toUtf8());
    }

    QVERIFY2(model.rowCount() == 6, QString("rowCount()=%1").arg(model.rowCount()).toLocal8Bit());
    row = 0;
    QVERIFY(model.item(row, PostThreadListModel::RecordTextPlainRole).toString() == "test");
    QVERIFY(model.item(row, PostThreadListModel::ThreadConnectorTopRole).toBool() == false);
    QVERIFY(model.item(row, PostThreadListModel::ThreadConnectorBottomRole).toBool() == true);
    row = 1;
    QVERIFY(model.item(row, PostThreadListModel::RecordTextPlainRole).toString() == "reply 2");
    QVERIFY(model.item(row, PostThreadListModel::ThreadConnectorTopRole).toBool() == true);
    QVERIFY(model.item(row, PostThreadListModel::ThreadConnectorBottomRole).toBool() == true);
    row = 2;
    QVERIFY(model.item(row, PostThreadListModel::RecordTextPlainRole).toString() == "reply 3 - 2");
    QVERIFY(model.item(row, PostThreadListModel::ThreadConnectorTopRole).toBool() == true);
    QVERIFY(model.item(row, PostThreadListModel::ThreadConnectorBottomRole).toBool() == true);
    row = 3;
    QVERIFY(model.item(row, PostThreadListModel::RecordTextPlainRole).toString() == "reply 4 - 2");
    QVERIFY(model.item(row, PostThreadListModel::ThreadConnectorTopRole).toBool() == true);
    QVERIFY(model.item(row, PostThreadListModel::ThreadConnectorBottomRole).toBool() == false);
    row = 4;
    QVERIFY(model.item(row, PostThreadListModel::RecordTextPlainRole).toString() == "reply 3");
    QVERIFY(model.item(row, PostThreadListModel::ThreadConnectorTopRole).toBool() == true);
    QVERIFY(model.item(row, PostThreadListModel::ThreadConnectorBottomRole).toBool() == true);
    row = 5;
    QVERIFY(model.item(row, PostThreadListModel::RecordTextPlainRole).toString() == "reply 4");
    QVERIFY(model.item(row, PostThreadListModel::ThreadConnectorTopRole).toBool() == true);
    QVERIFY(model.item(row, PostThreadListModel::ThreadConnectorBottomRole).toBool() == false);

    model.clear();
    model.setAccount(m_service + "/postthread/3", QString(), QString(), QString(), "dummy",
                     QString());
    {
        QSignalSpy spy(&model, SIGNAL(runningChanged()));
        model.getLatest();
        spy.wait();
        QVERIFY2(spy.count() == 2, QString("spy.count()=%1").arg(spy.count()).toUtf8());
    }

    QVERIFY2(model.rowCount() == 6, QString("rowCount()=%1").arg(model.rowCount()).toLocal8Bit());
    row = 0;
    QVERIFY(model.item(row, PostThreadListModel::RecordTextPlainRole).toString() == "test");
    QVERIFY(model.item(row, PostThreadListModel::ThreadConnectorTopRole).toBool() == false);
    QVERIFY(model.item(row, PostThreadListModel::ThreadConnectorBottomRole).toBool() == true);
    row = 1;
    QVERIFY(model.item(row, PostThreadListModel::RecordTextPlainRole).toString() == "reply 2");
    QVERIFY(model.item(row, PostThreadListModel::ThreadConnectorTopRole).toBool() == true);
    QVERIFY(model.item(row, PostThreadListModel::ThreadConnectorBottomRole).toBool() == true);
    row = 2;
    QVERIFY(model.item(row, PostThreadListModel::RecordTextPlainRole).toString() == "reply 3 - 2");
    QVERIFY(model.item(row, PostThreadListModel::ThreadConnectorTopRole).toBool() == true);
    QVERIFY(model.item(row, PostThreadListModel::ThreadConnectorBottomRole).toBool() == true);
    row = 3;
    QVERIFY(model.item(row, PostThreadListModel::RecordTextPlainRole).toString() == "reply 4 - 2");
    QVERIFY(model.item(row, PostThreadListModel::ThreadConnectorTopRole).toBool() == true);
    QVERIFY(model.item(row, PostThreadListModel::ThreadConnectorBottomRole).toBool() == false);
    row = 4;
    QVERIFY(model.item(row, PostThreadListModel::RecordTextPlainRole).toString() == "reply 3");
    QVERIFY(model.item(row, PostThreadListModel::ThreadConnectorTopRole).toBool() == true);
    QVERIFY(model.item(row, PostThreadListModel::ThreadConnectorBottomRole).toBool() == true);
    row = 5;
    QVERIFY(model.item(row, PostThreadListModel::RecordTextPlainRole).toString() == "reply 4");
    QVERIFY(model.item(row, PostThreadListModel::ThreadConnectorTopRole).toBool() == true);
    QVERIFY(model.item(row, PostThreadListModel::ThreadConnectorBottomRole).toBool() == false);
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

void hagoromo_test::test_putPreferences(const QString &path, const QByteArray &body)
{
    QJsonDocument json_doc_expect;
    if (path.contains("/save/")) {
        json_doc_expect = loadJson(":/data/generator/save/app.bsky.actor.putPreferences");
    } else {
        json_doc_expect = loadJson(":/data/generator/remove/app.bsky.actor.putPreferences");
    }

    QJsonDocument json_doc = QJsonDocument::fromJson(body);

    if (json_doc_expect.object() != json_doc.object()) {
        qDebug().noquote().nospace() << QString("\nexpect:%1\nactual:%2\n")
                                                .arg(json_doc_expect.toJson(), json_doc.toJson());
    }
    QVERIFY(json_doc_expect.object() == json_doc.object());
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
