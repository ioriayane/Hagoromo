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
    void test_NotificationListModel();
    void test_NotificationListModel2();
    void test_UserProfile();
    void test_charCount();

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
        spy.wait();
        QVERIFY2(spy.count() == 2, QString("spy.count()=%1").arg(spy.count()).toUtf8());
    }
    {
        // remove
        model.setAccount(m_service + "/generator/remove", QString(), QString(), QString(), "dummy",
                         QString());
        QSignalSpy spy(&model, SIGNAL(runningChanged()));
        model.removeGenerator(
                "at://did:plc:z72i7hdynmk6r22z27h6tvur/app.bsky.feed.generator/with-friends");
        spy.wait();
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

void hagoromo_test::test_NotificationListModel()
{
    NotificationListModel model;
    model.setAccount(m_service + "/notifications", QString(), QString(), QString(), "dummy",
                     QString());

    {
        int i = 0;
        QSignalSpy spy(&model, SIGNAL(runningChanged()));
        model.getLatest();
        spy.wait();
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
        spy.wait();
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
        spy.wait();
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
        spy.wait();
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
        spy.wait();
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
        spy.wait();
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
        spy.wait();
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
    model.setAccount(m_service + "/notifications", QString(), QString(), QString(), "dummy",
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
    spy.wait();
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
