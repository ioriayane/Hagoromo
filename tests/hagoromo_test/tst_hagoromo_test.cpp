#include <QtTest>
#include <QCoreApplication>
#include <QJsonDocument>
#include <QDir>
#include <QFile>

#include "webserver.h"
#include "unittest_common.h"
#include "timelinelistmodel.h"
#include "columnlistmodel.h"
#include "notificationlistmodel.h"
#include "userprofile.h"
#include "tools/qstringex.h"
#include "anyprofilelistmodel.h"
#include "common.h"
#include "postthreadlistmodel.h"
#include "searchprofilelistmodel.h"
#include "searchpostlistmodel.h"

class hagoromo_test : public QObject
{
    Q_OBJECT

public:
    hagoromo_test();
    ~hagoromo_test();

private slots:
    void initTestCase();
    void cleanupTestCase();
    void test_test_TimelineListModelError();
    void test_TimelineListModelFacet();
    void test_ColumnListModelMove();
    void test_ColumnListModelRemove();
    void test_ColumnListModelInsertNext();
    void test_ColumnListModelSelected();
    void test_NotificationListModel();
    void test_NotificationListModel2();
    void test_UserProfile();
    void test_charCount();
    void test_TimelineListModel_quote_warn();
    void test_TimelineListModel_quote_hide();
    void test_TimelineListModel_quote_hide2();
    void test_TimelineListModel_quote_label();
    void test_TimelineListModel_animated_image();
    void test_TimelineListModel_threadgate();
    void test_TimelineListModel_hide_repost();
    void test_TimelineListModel_labelers();
    void test_NotificationListModel_warn();
    void test_TimelineListModel_next();
    void test_AnyProfileListModel();
    void test_TimelineListModel_text();
    void test_TimelineListModel_reply();
    void test_PostThreadListModel();
    void test_SystemTool_ImageClip();
    void test_SearchProfileListModel_suggestion();
    void test_SearchPostListModel_text();

private:
    WebServer m_mockServer;
    quint16 m_listenPort;
    QString m_service;

    void verifyStr(const QString &expect, const QString &actual);
};

hagoromo_test::hagoromo_test()
{
    QCoreApplication::setOrganizationName(QStringLiteral("relog"));
    QCoreApplication::setApplicationName(QStringLiteral("Hagoromo"));

    m_listenPort = m_mockServer.listen(QHostAddress::LocalHost, 0);
    m_service = QString("http://localhost:%1/response").arg(m_listenPort);

    connect(&m_mockServer, &WebServer::receivedPost,
            [=](const QHttpServerRequest &request, bool &result, QString &json) {
                if (request.url().path().endsWith("/xrpc/com.atproto.server.refreshSession")) {
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
                } else if (request.url().path().endsWith(
                                   "/xrpc/app.bsky.notification.updateSeen")) {
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

void hagoromo_test::test_test_TimelineListModelError()
{
    TimelineListModel model;

    model.setRunning(true);
    QVERIFY(model.getLatest() == false);
    QVERIFY(model.getNext() == false);
}

void hagoromo_test::test_TimelineListModelFacet()
{
    TimelineListModel model;
    model.setAccount(m_service + "/facet", QString(), QString(), QString(), "dummy", QString());
    model.setDisplayInterval(0);

    QSignalSpy spy(&model, SIGNAL(runningChanged()));
    model.getLatest();
    spy.wait(10 * 1000);
    QVERIFY2(spy.count() == 2, QString("spy.count()=%1").arg(spy.count()).toUtf8());

    QFile file(":/response/facet/xrpc/app.bsky.feed.getTimeline.expect");
    QVERIFY(file.open(QFile::ReadOnly));
    QTextStream ts;

    QJsonDocument json_doc = QJsonDocument::fromJson(file.readAll());
    QVERIFY(json_doc.isArray());

    QVERIFY2(json_doc.array().count() == model.rowCount(),
             QString("%1 == %2").arg(json_doc.array().count()).arg(model.rowCount()).toLocal8Bit());

    for (int i = 0; i < model.rowCount(); i++) {
        verifyStr(json_doc.array().at(i).toObject().value("recordText").toString(),
                  model.item(i, TimelineListModel::TimelineListModelRoles::RecordTextRole)
                          .toString());
    }
}

void hagoromo_test::test_ColumnListModelMove()
{
    ColumnListModel model;

    model.append("uuid_1", 0, false, 10000, 400, 1, "column 1", "value 1");
    model.append("uuid_2", 1, false, 20000, 500, 1, "column 2", "value 2");
    model.append("uuid_3", 2, false, 30000, 600, 1, "column 3", "value 3");
    model.append("uuid_4", 3, false, 40000, 700, 1, "column 4", "value 4");

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

    model.append("uuid_1", 0, false, 10000, 400, 1, "column 1", "value 1");
    model.append("uuid_2", 1, false, 20000, 500, 1, "column 2", "value 2");
    model.append("uuid_3", 2, false, 30000, 600, 1, "column 3", "value 3");
    model.append("uuid_4", 3, false, 40000, 700, 1, "column 4", "value 4");

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

    model.append("uuid_1", 0, false, 10000, 400, 1, "column 1", "value 1");
    model.append("uuid_2", 1, false, 20000, 500, 1, "column 2", "value 2");
    model.append("uuid_3", 2, false, 30000, 600, 1, "column 3", "value 3");
    model.append("uuid_4", 3, false, 40000, 700, 1, "column 4", "value 4");

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
                     800, 1, "column 1 next", "value 10");
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
                     900, 1, "column 1 next2", "value 11");
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
                     1000, 1, "column 4 next", "value 12");
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

void hagoromo_test::test_ColumnListModelSelected()
{
    int row = 0;
    ColumnListModel model;

    model.append("uuid_1", 0, false, 10000, 400, 1, "column 1", "value 1");
    model.append("uuid_2", 1, false, 20000, 500, 1, "column 2", "value 2");
    model.append("uuid_3", 2, false, 30000, 600, 1, "column 3", "value 3");
    model.append("uuid_4", 3, false, 40000, 700, 1, "column 4", "value 4");

    model.move(model.item(1, ColumnListModel::KeyRole).toString(), ColumnListModel::MoveLeft);
    model.move(model.item(2, ColumnListModel::KeyRole).toString(), ColumnListModel::MoveLeft);
    model.move(model.item(2, ColumnListModel::KeyRole).toString(), ColumnListModel::MoveLeft);
    QVERIFY(model.getRowListInOrderOfPosition() == QList<int>() << 2 << 1 << 0 << 3);

    row = 0;
    QVERIFY2(model.item(row, ColumnListModel::SelectedRole).toBool() == false,
             QString("row(%1)=%2")
                     .arg(row)
                     .arg(model.item(row, ColumnListModel::SelectedRole).toBool())
                     .toLocal8Bit());
    row++;
    QVERIFY2(model.item(row, ColumnListModel::SelectedRole).toBool() == false,
             QString("row(%1)=%2")
                     .arg(row)
                     .arg(model.item(row, ColumnListModel::SelectedRole).toBool())
                     .toLocal8Bit());
    row++;
    QVERIFY2(model.item(row, ColumnListModel::SelectedRole).toBool() == true,
             QString("row(%1)=%2")
                     .arg(row)
                     .arg(model.item(row, ColumnListModel::SelectedRole).toBool())
                     .toLocal8Bit());
    row++;
    QVERIFY2(model.item(row, ColumnListModel::SelectedRole).toBool() == false,
             QString("row(%1)=%2")
                     .arg(row)
                     .arg(model.item(row, ColumnListModel::SelectedRole).toBool())
                     .toLocal8Bit());

    model.moveSelectionToRight();
    row = 1;
    QVERIFY2(model.item(row, ColumnListModel::SelectedRole).toBool() == true,
             QString("row(%1)=%2")
                     .arg(row)
                     .arg(model.item(row, ColumnListModel::SelectedRole).toBool())
                     .toLocal8Bit());
    model.moveSelectionToRight();
    row = 0;
    QVERIFY2(model.item(row, ColumnListModel::SelectedRole).toBool() == true,
             QString("row(%1)=%2")
                     .arg(row)
                     .arg(model.item(row, ColumnListModel::SelectedRole).toBool())
                     .toLocal8Bit());
    model.moveSelectionToRight();
    row = 3;
    QVERIFY2(model.item(row, ColumnListModel::SelectedRole).toBool() == true,
             QString("row(%1)=%2")
                     .arg(row)
                     .arg(model.item(row, ColumnListModel::SelectedRole).toBool())
                     .toLocal8Bit());
    model.moveSelectionToRight();
    row = 3;
    QVERIFY2(model.item(row, ColumnListModel::SelectedRole).toBool() == true,
             QString("row(%1)=%2")
                     .arg(row)
                     .arg(model.item(row, ColumnListModel::SelectedRole).toBool())
                     .toLocal8Bit());

    model.moveSelectionToLeft();
    row = 0;
    QVERIFY2(model.item(row, ColumnListModel::SelectedRole).toBool() == true,
             QString("row(%1)=%2")
                     .arg(row)
                     .arg(model.item(row, ColumnListModel::SelectedRole).toBool())
                     .toLocal8Bit());
    model.moveSelectionToLeft();
    row = 1;
    QVERIFY2(model.item(row, ColumnListModel::SelectedRole).toBool() == true,
             QString("row(%1)=%2")
                     .arg(row)
                     .arg(model.item(row, ColumnListModel::SelectedRole).toBool())
                     .toLocal8Bit());
    model.moveSelectionToLeft();
    row = 2;
    QVERIFY2(model.item(row, ColumnListModel::SelectedRole).toBool() == true,
             QString("row(%1)=%2")
                     .arg(row)
                     .arg(model.item(row, ColumnListModel::SelectedRole).toBool())
                     .toLocal8Bit());
    model.moveSelectionToLeft();
    row = 2;
    QVERIFY2(model.item(row, ColumnListModel::SelectedRole).toBool() == true,
             QString("row(%1)=%2")
                     .arg(row)
                     .arg(model.item(row, ColumnListModel::SelectedRole).toBool())
                     .toLocal8Bit());
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
        spy.wait(20 * 1000);
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
        spy.wait(20 * 1000);
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
        spy.wait(20 * 1000);
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
        spy.wait(20 * 1000);
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
        spy.wait(20 * 1000);
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
        spy.wait(20 * 1000);
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
        spy.wait(20 * 1000);
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
    spy.wait(20 * 1000);
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
    QVERIFY(profile.formattedDescription() == "test <br/>test<br/>test");

    profile.setDescription("https://github.com/ioriayane/Hagoromo");
    QVERIFY(profile.formattedDescription()
            == "<a "
               "href=\"https://github.com/ioriayane/Hagoromo\">https://github.com/ioriayane/"
               "Hagoromo</a>");

    profile.setDescription("hoge https://github.com/ioriayane/Hagoromo");
    QVERIFY(profile.formattedDescription()
            == "hoge <a "
               "href=\"https://github.com/ioriayane/Hagoromo\">https://github.com/ioriayane/"
               "Hagoromo</a>");

    profile.setDescription("https://github.com/ioriayane/Hagoromo hoge");
    QVERIFY(profile.formattedDescription()
            == "<a "
               "href=\"https://github.com/ioriayane/Hagoromo\">https://github.com/ioriayane/"
               "Hagoromo</a> hoge");

    profile.setDescription("hoge\nhttps://github.com/ioriayane/Hagoromo");
    QVERIFY(profile.formattedDescription()
            == "hoge<br/><a "
               "href=\"https://github.com/ioriayane/Hagoromo\">https://github.com/ioriayane/"
               "Hagoromo</a>");

    profile.setDescription("https://github.com/ioriayane/Hagoromo\nhoge");
    QVERIFY(profile.formattedDescription()
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
    spy.wait(10 * 1000);
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

void hagoromo_test::test_TimelineListModel_animated_image()
{
    int row = 0;
    TimelineListModel model;
    model.setAccount(m_service + "/timeline/animated", QString(), QString(), QString(), "dummy",
                     QString());
    model.setDisplayInterval(0);

    QSignalSpy spy(&model, SIGNAL(runningChanged()));
    QVERIFY(model.getLatest());
    spy.wait(10 * 1000);
    QVERIFY2(spy.count() == 2, QString("spy.count()=%1").arg(spy.count()).toUtf8());

    QVERIFY2(model.rowCount() == 2, QString("rowCount()=%1").arg(model.rowCount()).toUtf8());

    QString folder = Common::appTempFolder("ext_images");
    QStringList thumb_paths;
    QStringList full_paths;
    QStringList image_paths;
    QDir dir;
    dir.mkpath(folder);

    image_paths << QString("%1/%2.%3")
                           .arg(folder,
                                "bafkreicwnlvi3jfbpm7wp5rm676gy4epqdvvu4xts4tdvw4axvrejtamuy_",
                                "gif")
                << QString("%1/%2.%3")
                           .arg(folder,
                                "bafkreiau5tvk6ddh34ztxp5q4fjyxspizsbnecv3skttpcppi4ilikbbpu_",
                                "gif");
    for (const auto &image_path : qAsConst(image_paths)) {
        QFile::setPermissions(image_path, QFile::WriteOwner | QFile::ReadOwner);
        QFile::remove(image_path);
    }

    row = 1;
    QVERIFY(model.item(row, TimelineListModel::RecordTextRole).toString() == "animated images");

    // //////////////////////
    thumb_paths.clear();
    full_paths.clear();
    thumb_paths << "https://cdn.bsky.app/img/feed_thumbnail/plain/did:plc:mqxsuw5b5rhpwo4lw6iwlid5/"
                   "bafkreicwnlvi3jfbpm7wp5rm676gy4epqdvvu4xts4tdvw4axvrejtamuy@jpeg"
                << "https://cdn.bsky.app/img/feed_thumbnail/plain/did:plc:mqxsuw5b5rhpwo4lw6iwlid5/"
                   "bafkreiau5tvk6ddh34ztxp5q4fjyxspizsbnecv3skttpcppi4ilikbbpu@jpeg";
    full_paths << "https://cdn.bsky.app/img/feed_fullsize/plain/did:plc:mqxsuw5b5rhpwo4lw6iwlid5/"
                  "bafkreicwnlvi3jfbpm7wp5rm676gy4epqdvvu4xts4tdvw4axvrejtamuy@jpeg"
               << "https://cdn.bsky.app/img/feed_fullsize/plain/did:plc:mqxsuw5b5rhpwo4lw6iwlid5/"
                  "bafkreiau5tvk6ddh34ztxp5q4fjyxspizsbnecv3skttpcppi4ilikbbpu@jpeg";
    QVERIFY2(model.item(row, TimelineListModel::EmbedImagesRole).toStringList() == thumb_paths,
             model.item(row, TimelineListModel::EmbedImagesRole)
                             .toStringList()
                             .join("\n")
                             .toLocal8Bit()
                     + "\n<>\n" + thumb_paths.join("\n").toLocal8Bit());
    QVERIFY2(model.item(row, TimelineListModel::EmbedImagesFullRole).toStringList() == full_paths,
             model.item(row, TimelineListModel::EmbedImagesFullRole)
                             .toStringList()
                             .join("\n")
                             .toLocal8Bit()
                     + "\n<>\n" + full_paths.join("\n").toLocal8Bit());

    // //////////////////
    QVERIFY(QFile::copy(":/data/images/sample.jpg", image_paths.at(0)));
    thumb_paths.clear();
    full_paths.clear();
    thumb_paths << QUrl::fromLocalFile(image_paths.at(0)).toString()
                << "https://cdn.bsky.app/img/feed_thumbnail/plain/did:plc:mqxsuw5b5rhpwo4lw6iwlid5/"
                   "bafkreiau5tvk6ddh34ztxp5q4fjyxspizsbnecv3skttpcppi4ilikbbpu@jpeg";
    full_paths << QUrl::fromLocalFile(image_paths.at(0)).toString()
               << "https://cdn.bsky.app/img/feed_fullsize/plain/did:plc:mqxsuw5b5rhpwo4lw6iwlid5/"
                  "bafkreiau5tvk6ddh34ztxp5q4fjyxspizsbnecv3skttpcppi4ilikbbpu@jpeg";
    QVERIFY2(model.item(row, TimelineListModel::EmbedImagesRole).toStringList() == thumb_paths,
             model.item(row, TimelineListModel::EmbedImagesRole)
                             .toStringList()
                             .join("\n")
                             .toLocal8Bit()
                     + "\n<>\n" + thumb_paths.join("\n").toLocal8Bit());
    QVERIFY2(model.item(row, TimelineListModel::EmbedImagesFullRole).toStringList() == full_paths,
             model.item(row, TimelineListModel::EmbedImagesFullRole)
                             .toStringList()
                             .join("\n")
                             .toLocal8Bit()
                     + "\n<>\n" + full_paths.join("\n").toLocal8Bit());

    for (const auto &image_path : qAsConst(image_paths)) {
        QFile::setPermissions(image_path, QFile::WriteOwner | QFile::ReadOwner);
        QFile::remove(image_path);
    }

    // ///////////////////
    QVERIFY(QFile::copy(":/data/images/sample.jpg", image_paths.at(1)));
    thumb_paths.clear();
    full_paths.clear();
    thumb_paths << "https://cdn.bsky.app/img/feed_thumbnail/plain/did:plc:mqxsuw5b5rhpwo4lw6iwlid5/"
                   "bafkreicwnlvi3jfbpm7wp5rm676gy4epqdvvu4xts4tdvw4axvrejtamuy@jpeg"
                << QUrl::fromLocalFile(image_paths.at(1)).toString();
    full_paths << "https://cdn.bsky.app/img/feed_fullsize/plain/did:plc:mqxsuw5b5rhpwo4lw6iwlid5/"
                  "bafkreicwnlvi3jfbpm7wp5rm676gy4epqdvvu4xts4tdvw4axvrejtamuy@jpeg"
               << QUrl::fromLocalFile(image_paths.at(1)).toString();
    QVERIFY2(model.item(row, TimelineListModel::EmbedImagesRole).toStringList() == thumb_paths,
             model.item(row, TimelineListModel::EmbedImagesRole)
                             .toStringList()
                             .join("\n")
                             .toLocal8Bit()
                     + "\n<>\n" + thumb_paths.join("\n").toLocal8Bit());
    QVERIFY2(model.item(row, TimelineListModel::EmbedImagesFullRole).toStringList() == full_paths,
             model.item(row, TimelineListModel::EmbedImagesFullRole)
                             .toStringList()
                             .join("\n")
                             .toLocal8Bit()
                     + "\n<>\n" + full_paths.join("\n").toLocal8Bit());

    // ////////////////////
    QVERIFY(QFile::copy(":/data/images/sample.jpg", image_paths.at(0)));
    // QVERIFY(QFile::copy(":/data/images/sample.jpg", image_paths.at(1)));
    thumb_paths.clear();
    full_paths.clear();
    thumb_paths << QUrl::fromLocalFile(image_paths.at(0)).toString()
                << QUrl::fromLocalFile(image_paths.at(1)).toString();
    full_paths << QUrl::fromLocalFile(image_paths.at(0)).toString()
               << QUrl::fromLocalFile(image_paths.at(1)).toString();
    QVERIFY2(model.item(row, TimelineListModel::EmbedImagesRole).toStringList() == thumb_paths,
             model.item(row, TimelineListModel::EmbedImagesRole)
                             .toStringList()
                             .join("\n")
                             .toLocal8Bit()
                     + "\n<>\n" + thumb_paths.join("\n").toLocal8Bit());
    QVERIFY2(model.item(row, TimelineListModel::EmbedImagesFullRole).toStringList() == full_paths,
             model.item(row, TimelineListModel::EmbedImagesFullRole)
                             .toStringList()
                             .join("\n")
                             .toLocal8Bit()
                     + "\n<>\n" + full_paths.join("\n").toLocal8Bit());

    for (const auto &image_path : qAsConst(image_paths)) {
        QFile::setPermissions(image_path, QFile::WriteOwner | QFile::ReadOwner);
        QFile::remove(image_path);
    }
}

void hagoromo_test::test_TimelineListModel_threadgate()
{
    int row = 0;
    TimelineListModel model;
    model.setAccount(m_service + "/timeline/threadgate", QString(), QString(), QString(), "dummy",
                     QString());
    model.setDisplayInterval(0);

    QSignalSpy spy(&model, SIGNAL(runningChanged()));
    QVERIFY(model.getLatest());
    spy.wait(10 * 1000);
    QVERIFY2(spy.count() == 2, QString("spy.count()=%1").arg(spy.count()).toUtf8());

    row = 0;
    QVERIFY2(model.item(row, TimelineListModel::ThreadGateTypeRole) == "choice",
             model.item(row, TimelineListModel::ThreadGateTypeRole).toString().toLocal8Bit());
    QVERIFY2(model.item(row, TimelineListModel::ThreadGateRulesRole)
                     == QStringList() << "followed"
                                      << "at://did:plc:mqxsuw5b5rhpwo4lw6iwlid5/"
                                         "app.bsky.graph.list/3kflf2r3lwg2x"
                                      << "at://did:plc:mqxsuw5b5rhpwo4lw6iwlid5/"
                                         "app.bsky.graph.list/3kflbnc4c4o2x",
             model.item(row, TimelineListModel::ThreadGateRulesRole)
                     .toStringList()
                     .join(",")
                     .toLocal8Bit());

    row = 1;
    QVERIFY2(model.item(row, TimelineListModel::ThreadGateTypeRole) == "everybody",
             model.item(row, TimelineListModel::ThreadGateTypeRole).toString().toLocal8Bit());
    QVERIFY2(model.item(row, TimelineListModel::ThreadGateRulesRole) == QStringList(),
             model.item(row, TimelineListModel::ThreadGateRulesRole)
                     .toStringList()
                     .join(",")
                     .toLocal8Bit());

    row = 2;
    QVERIFY2(model.item(row, TimelineListModel::ThreadGateTypeRole) == "nobody",
             model.item(row, TimelineListModel::ThreadGateTypeRole).toString().toLocal8Bit());
    QVERIFY2(model.item(row, TimelineListModel::ThreadGateRulesRole) == QStringList(),
             model.item(row, TimelineListModel::ThreadGateRulesRole)
                     .toStringList()
                     .join(",")
                     .toLocal8Bit());

    row = 3;
    QVERIFY2(model.item(row, TimelineListModel::ThreadGateTypeRole) == "choice",
             model.item(row, TimelineListModel::ThreadGateTypeRole).toString().toLocal8Bit());
    QVERIFY2(model.item(row, TimelineListModel::ThreadGateRulesRole)
                     == QStringList() << "at://did:plc:ipj5qejfoqu6eukvt72uhyit/"
                                         "app.bsky.graph.list/3kdoiwxqcjv2v",
             model.item(row, TimelineListModel::ThreadGateRulesRole)
                     .toStringList()
                     .join(",")
                     .toLocal8Bit());

    row = 4;
    QVERIFY2(model.item(row, TimelineListModel::ThreadGateTypeRole) == "choice",
             model.item(row, TimelineListModel::ThreadGateTypeRole).toString().toLocal8Bit());
    QVERIFY2(model.item(row, TimelineListModel::ThreadGateRulesRole) == QStringList() << "followed",
             model.item(row, TimelineListModel::ThreadGateRulesRole)
                     .toStringList()
                     .join(",")
                     .toLocal8Bit());

    row = 5;
    QVERIFY2(model.item(row, TimelineListModel::ThreadGateTypeRole) == "choice",
             model.item(row, TimelineListModel::ThreadGateTypeRole).toString().toLocal8Bit());
    QVERIFY2(model.item(row, TimelineListModel::ThreadGateRulesRole)
                     == QStringList() << "mentioned",
             model.item(row, TimelineListModel::ThreadGateRulesRole)
                     .toStringList()
                     .join(",")
                     .toLocal8Bit());
}

void hagoromo_test::test_TimelineListModel_hide_repost()
{
    int row = 0;
    TimelineListModel model;
    model.setAccount(m_service + "/timeline/hide2", "did:plc:mqxsuw5b5rhpwo4lw6iwlid5",
                     "ioriayane2.bsky.social", QString(), "dummy", QString());
    model.setDisplayInterval(0);

    QSignalSpy spy(&model, SIGNAL(runningChanged()));
    QVERIFY(model.getLatest());
    spy.wait(10 * 1000);
    QVERIFY2(spy.count() == 2, QString("spy.count()=%1").arg(spy.count()).toUtf8());

    //
    {
        QVERIFY2(model.rowCount() == 7, QString::number(model.rowCount()).toLocal8Bit());
        row = 0;
        QVERIFY2(model.item(row, TimelineListModel::CidRole)
                         == "bafyreifvecqlsp36t7wzs2owv5r2fdpb2tq3mwmocpwkxopyocdnvhip3u",
                 model.item(row, TimelineListModel::CidRole).toString().toLocal8Bit());
        QVERIFY2(model.item(row, TimelineListModel::IsRepostedByRole) == true,
                 model.item(row, TimelineListModel::IsRepostedByRole).toString().toLocal8Bit());
        row = 1;
        QVERIFY2(model.item(row, TimelineListModel::CidRole)
                         == "bafyreidrtozxt3boj6ftbuoacfpu5wjc3hapx4zum6ijmlhakqyfykugja",
                 model.item(row, TimelineListModel::CidRole).toString().toLocal8Bit());
        QVERIFY2(model.item(row, TimelineListModel::IsRepostedByRole) == true,
                 model.item(row, TimelineListModel::IsRepostedByRole).toString().toLocal8Bit());
        row = 2;
        QVERIFY2(model.item(row, TimelineListModel::CidRole)
                         == "bafyreif5g26rkg42hqigvscvwitm46hwa43v35p5hu7iteycntirquzj7e",
                 model.item(row, TimelineListModel::CidRole).toString().toLocal8Bit());
        QVERIFY2(model.item(row, TimelineListModel::IsRepostedByRole) == true,
                 model.item(row, TimelineListModel::IsRepostedByRole).toString().toLocal8Bit());
        row = 3;
        QVERIFY2(model.item(row, TimelineListModel::CidRole)
                         == "bafyreiasmg6ks4c4voo6tsiqx4klq6mfjjdloijf4ewp35nj7dnpkpvpc4",
                 model.item(row, TimelineListModel::CidRole).toString().toLocal8Bit());
        QVERIFY2(model.item(row, TimelineListModel::IsRepostedByRole) == true,
                 model.item(row, TimelineListModel::IsRepostedByRole).toString().toLocal8Bit());
        row = 4;
        QVERIFY2(model.item(row, TimelineListModel::CidRole)
                         == "bafyreicdfgdzc445denxvuzsklv4q4cw5dlm4kishkrqbv6ssfhbn3lfqy",
                 model.item(row, TimelineListModel::CidRole).toString().toLocal8Bit());
        QVERIFY2(model.item(row, TimelineListModel::IsRepostedByRole) == true,
                 model.item(row, TimelineListModel::IsRepostedByRole).toString().toLocal8Bit());
        row = 5;
        QVERIFY2(model.item(row, TimelineListModel::CidRole)
                         == "bafyreiaaysocrjeueqgaxyhk2r4ov34havbiup4ovbf7p4am7wycv6qvwy",
                 model.item(row, TimelineListModel::CidRole).toString().toLocal8Bit());
        QVERIFY2(model.item(row, TimelineListModel::IsRepostedByRole) == true,
                 model.item(row, TimelineListModel::IsRepostedByRole).toString().toLocal8Bit());
        row = 6;
        QVERIFY2(model.item(row, TimelineListModel::CidRole)
                         == "bafyreih77ppn7ykiharfjkqlyq5q6rqzatq5sxj2koomqv22s2z56dk5fe",
                 model.item(row, TimelineListModel::CidRole).toString().toLocal8Bit());
        QVERIFY2(model.item(row, TimelineListModel::IsRepostedByRole) == true,
                 model.item(row, TimelineListModel::IsRepostedByRole).toString().toLocal8Bit());
    }
    //
    {
        model.setVisibleRepostOfOwn(false);
        model.setVisibleRepostOfFollowingUsers(true);
        model.setVisibleRepostOfUnfollowingUsers(true);
        model.setVisibleRepostOfMine(true);
        model.setVisibleRepostByMe(true);

        QVERIFY2(model.rowCount() == 5, QString::number(model.rowCount()).toLocal8Bit());
        row = 0;
        QVERIFY2(model.item(row, TimelineListModel::CidRole)
                         == "bafyreifvecqlsp36t7wzs2owv5r2fdpb2tq3mwmocpwkxopyocdnvhip3u",
                 model.item(row, TimelineListModel::CidRole).toString().toLocal8Bit());
        QVERIFY2(model.item(row, TimelineListModel::IsRepostedByRole) == true,
                 model.item(row, TimelineListModel::IsRepostedByRole).toString().toLocal8Bit());
        row++;
        QVERIFY2(model.item(row, TimelineListModel::CidRole)
                         == "bafyreidrtozxt3boj6ftbuoacfpu5wjc3hapx4zum6ijmlhakqyfykugja",
                 model.item(row, TimelineListModel::CidRole).toString().toLocal8Bit());
        QVERIFY2(model.item(row, TimelineListModel::IsRepostedByRole) == true,
                 model.item(row, TimelineListModel::IsRepostedByRole).toString().toLocal8Bit());
        //    row++;
        //    QVERIFY2(model.item(row, TimelineListModel::CidRole)
        //                     == "bafyreif5g26rkg42hqigvscvwitm46hwa43v35p5hu7iteycntirquzj7e",
        //             model.item(row, TimelineListModel::CidRole).toString().toLocal8Bit());
        //    QVERIFY2(model.item(row, TimelineListModel::IsRepostedByRole) == true,
        //             model.item(row,
        //             TimelineListModel::IsRepostedByRole).toString().toLocal8Bit());
        row++;
        QVERIFY2(model.item(row, TimelineListModel::CidRole)
                         == "bafyreiasmg6ks4c4voo6tsiqx4klq6mfjjdloijf4ewp35nj7dnpkpvpc4",
                 model.item(row, TimelineListModel::CidRole).toString().toLocal8Bit());
        QVERIFY2(model.item(row, TimelineListModel::IsRepostedByRole) == true,
                 model.item(row, TimelineListModel::IsRepostedByRole).toString().toLocal8Bit());
        row++;
        QVERIFY2(model.item(row, TimelineListModel::CidRole)
                         == "bafyreicdfgdzc445denxvuzsklv4q4cw5dlm4kishkrqbv6ssfhbn3lfqy",
                 model.item(row, TimelineListModel::CidRole).toString().toLocal8Bit());
        QVERIFY2(model.item(row, TimelineListModel::IsRepostedByRole) == true,
                 model.item(row, TimelineListModel::IsRepostedByRole).toString().toLocal8Bit());
        row++;
        QVERIFY2(model.item(row, TimelineListModel::CidRole)
                         == "bafyreiaaysocrjeueqgaxyhk2r4ov34havbiup4ovbf7p4am7wycv6qvwy",
                 model.item(row, TimelineListModel::CidRole).toString().toLocal8Bit());
        QVERIFY2(model.item(row, TimelineListModel::IsRepostedByRole) == true,
                 model.item(row, TimelineListModel::IsRepostedByRole).toString().toLocal8Bit());
        //    row++;
        //    QVERIFY2(model.item(row, TimelineListModel::CidRole)
        //                     == "bafyreih77ppn7ykiharfjkqlyq5q6rqzatq5sxj2koomqv22s2z56dk5fe",
        //             model.item(row, TimelineListModel::CidRole).toString().toLocal8Bit());
        //    QVERIFY2(model.item(row, TimelineListModel::IsRepostedByRole) == true,
        //             model.item(row,
        //             TimelineListModel::IsRepostedByRole).toString().toLocal8Bit());
    }
    //
    {
        model.setVisibleRepostOfOwn(true);
        model.setVisibleRepostOfFollowingUsers(false);
        model.setVisibleRepostOfUnfollowingUsers(true);
        model.setVisibleRepostOfMine(true);
        model.setVisibleRepostByMe(true);

        QVERIFY2(model.rowCount() == 4, QString("%1").arg(model.rowCount()).toLocal8Bit());
        row = 0;
        QVERIFY2(model.item(row, TimelineListModel::CidRole)
                         == "bafyreifvecqlsp36t7wzs2owv5r2fdpb2tq3mwmocpwkxopyocdnvhip3u",
                 model.item(row, TimelineListModel::CidRole).toString().toLocal8Bit());
        QVERIFY2(model.item(row, TimelineListModel::IsRepostedByRole) == true,
                 model.item(row, TimelineListModel::IsRepostedByRole).toString().toLocal8Bit());
        //    row++;
        //    QVERIFY2(model.item(row, TimelineListModel::CidRole)
        //                     == "bafyreidrtozxt3boj6ftbuoacfpu5wjc3hapx4zum6ijmlhakqyfykugja",
        //             model.item(row, TimelineListModel::CidRole).toString().toLocal8Bit());
        //    QVERIFY2(model.item(row, TimelineListModel::IsRepostedByRole) == true,
        //             model.item(row,
        //             TimelineListModel::IsRepostedByRole).toString().toLocal8Bit());
        //    row++;
        //    QVERIFY2(model.item(row, TimelineListModel::CidRole)
        //                     == "bafyreif5g26rkg42hqigvscvwitm46hwa43v35p5hu7iteycntirquzj7e",
        //             model.item(row, TimelineListModel::CidRole).toString().toLocal8Bit());
        //    QVERIFY2(model.item(row, TimelineListModel::IsRepostedByRole) == true,
        //             model.item(row,
        //             TimelineListModel::IsRepostedByRole).toString().toLocal8Bit());
        row++;
        QVERIFY2(model.item(row, TimelineListModel::CidRole)
                         == "bafyreiasmg6ks4c4voo6tsiqx4klq6mfjjdloijf4ewp35nj7dnpkpvpc4",
                 model.item(row, TimelineListModel::CidRole).toString().toLocal8Bit());
        QVERIFY2(model.item(row, TimelineListModel::IsRepostedByRole) == true,
                 model.item(row, TimelineListModel::IsRepostedByRole).toString().toLocal8Bit());
        row++;
        QVERIFY2(model.item(row, TimelineListModel::CidRole)
                         == "bafyreicdfgdzc445denxvuzsklv4q4cw5dlm4kishkrqbv6ssfhbn3lfqy",
                 model.item(row, TimelineListModel::CidRole).toString().toLocal8Bit());
        QVERIFY2(model.item(row, TimelineListModel::IsRepostedByRole) == true,
                 model.item(row, TimelineListModel::IsRepostedByRole).toString().toLocal8Bit());
        //    row++;
        //    QVERIFY2(model.item(row, TimelineListModel::CidRole)
        //                     == "bafyreiaaysocrjeueqgaxyhk2r4ov34havbiup4ovbf7p4am7wycv6qvwy",
        //             model.item(row, TimelineListModel::CidRole).toString().toLocal8Bit());
        //    QVERIFY2(model.item(row, TimelineListModel::IsRepostedByRole) == true,
        //             model.item(row,
        //             TimelineListModel::IsRepostedByRole).toString().toLocal8Bit());
        row++;
        QVERIFY2(model.item(row, TimelineListModel::CidRole)
                         == "bafyreih77ppn7ykiharfjkqlyq5q6rqzatq5sxj2koomqv22s2z56dk5fe",
                 model.item(row, TimelineListModel::CidRole).toString().toLocal8Bit());
        QVERIFY2(model.item(row, TimelineListModel::IsRepostedByRole) == true,
                 model.item(row, TimelineListModel::IsRepostedByRole).toString().toLocal8Bit());
    }
    //
    {
        model.setVisibleRepostOfOwn(true);
        model.setVisibleRepostOfFollowingUsers(true);
        model.setVisibleRepostOfUnfollowingUsers(false);
        model.setVisibleRepostOfMine(true);
        model.setVisibleRepostByMe(true);

        QVERIFY2(model.rowCount() == 5, QString("%1").arg(model.rowCount()).toLocal8Bit());
        row = 0;
        // QVERIFY2(model.item(row, TimelineListModel::CidRole)
        //              == "bafyreifvecqlsp36t7wzs2owv5r2fdpb2tq3mwmocpwkxopyocdnvhip3u",
        //          model.item(row, TimelineListModel::CidRole).toString().toLocal8Bit());
        // QVERIFY2(model.item(row, TimelineListModel::IsRepostedByRole) == true,
        //          model.item(row, TimelineListModel::IsRepostedByRole).toString().toLocal8Bit());
        //    row++;
        QVERIFY2(model.item(row, TimelineListModel::CidRole)
                         == "bafyreidrtozxt3boj6ftbuoacfpu5wjc3hapx4zum6ijmlhakqyfykugja",
                 model.item(row, TimelineListModel::CidRole).toString().toLocal8Bit());
        QVERIFY2(model.item(row, TimelineListModel::IsRepostedByRole) == true,
                 model.item(row, TimelineListModel::IsRepostedByRole).toString().toLocal8Bit());
        row++;
        QVERIFY2(model.item(row, TimelineListModel::CidRole)
                         == "bafyreif5g26rkg42hqigvscvwitm46hwa43v35p5hu7iteycntirquzj7e",
                 model.item(row, TimelineListModel::CidRole).toString().toLocal8Bit());
        QVERIFY2(model.item(row, TimelineListModel::IsRepostedByRole) == true,
                 model.item(row, TimelineListModel::IsRepostedByRole).toString().toLocal8Bit());
        row++;
        QVERIFY2(model.item(row, TimelineListModel::CidRole)
                         == "bafyreiasmg6ks4c4voo6tsiqx4klq6mfjjdloijf4ewp35nj7dnpkpvpc4",
                 model.item(row, TimelineListModel::CidRole).toString().toLocal8Bit());
        QVERIFY2(model.item(row, TimelineListModel::IsRepostedByRole) == true,
                 model.item(row, TimelineListModel::IsRepostedByRole).toString().toLocal8Bit());
        row++;
        // QVERIFY2(model.item(row, TimelineListModel::CidRole)
        //              == "bafyreicdfgdzc445denxvuzsklv4q4cw5dlm4kishkrqbv6ssfhbn3lfqy",
        //          model.item(row, TimelineListModel::CidRole).toString().toLocal8Bit());
        // QVERIFY2(model.item(row, TimelineListModel::IsRepostedByRole) == true,
        //          model.item(row, TimelineListModel::IsRepostedByRole).toString().toLocal8Bit());
        // row++;
        QVERIFY2(model.item(row, TimelineListModel::CidRole)
                         == "bafyreiaaysocrjeueqgaxyhk2r4ov34havbiup4ovbf7p4am7wycv6qvwy",
                 model.item(row, TimelineListModel::CidRole).toString().toLocal8Bit());
        QVERIFY2(model.item(row, TimelineListModel::IsRepostedByRole) == true,
                 model.item(row, TimelineListModel::IsRepostedByRole).toString().toLocal8Bit());
        row++;
        QVERIFY2(model.item(row, TimelineListModel::CidRole)
                         == "bafyreih77ppn7ykiharfjkqlyq5q6rqzatq5sxj2koomqv22s2z56dk5fe",
                 model.item(row, TimelineListModel::CidRole).toString().toLocal8Bit());
        QVERIFY2(model.item(row, TimelineListModel::IsRepostedByRole) == true,
                 model.item(row, TimelineListModel::IsRepostedByRole).toString().toLocal8Bit());
    }
    //
    {
        model.setVisibleRepostOfOwn(true);
        model.setVisibleRepostOfFollowingUsers(true);
        model.setVisibleRepostOfUnfollowingUsers(true);
        model.setVisibleRepostOfMine(false);
        model.setVisibleRepostByMe(true);

        QVERIFY2(model.rowCount() == 5, QString("%1").arg(model.rowCount()).toLocal8Bit());
        row = 0;
        QVERIFY2(model.item(row, TimelineListModel::CidRole)
                         == "bafyreifvecqlsp36t7wzs2owv5r2fdpb2tq3mwmocpwkxopyocdnvhip3u",
                 model.item(row, TimelineListModel::CidRole).toString().toLocal8Bit());
        QVERIFY2(model.item(row, TimelineListModel::IsRepostedByRole) == true,
                 model.item(row, TimelineListModel::IsRepostedByRole).toString().toLocal8Bit());
        row++;
        QVERIFY2(model.item(row, TimelineListModel::CidRole)
                         == "bafyreidrtozxt3boj6ftbuoacfpu5wjc3hapx4zum6ijmlhakqyfykugja",
                 model.item(row, TimelineListModel::CidRole).toString().toLocal8Bit());
        QVERIFY2(model.item(row, TimelineListModel::IsRepostedByRole) == true,
                 model.item(row, TimelineListModel::IsRepostedByRole).toString().toLocal8Bit());
        row++;
        QVERIFY2(model.item(row, TimelineListModel::CidRole)
                         == "bafyreif5g26rkg42hqigvscvwitm46hwa43v35p5hu7iteycntirquzj7e",
                 model.item(row, TimelineListModel::CidRole).toString().toLocal8Bit());
        QVERIFY2(model.item(row, TimelineListModel::IsRepostedByRole) == true,
                 model.item(row, TimelineListModel::IsRepostedByRole).toString().toLocal8Bit());
        row++;
        // QVERIFY2(model.item(row, TimelineListModel::CidRole)
        //                  == "bafyreiasmg6ks4c4voo6tsiqx4klq6mfjjdloijf4ewp35nj7dnpkpvpc4",
        //          model.item(row, TimelineListModel::CidRole).toString().toLocal8Bit());
        // QVERIFY2(model.item(row, TimelineListModel::IsRepostedByRole) == true,
        //          model.item(row, TimelineListModel::IsRepostedByRole).toString().toLocal8Bit());
        // row++;
        QVERIFY2(model.item(row, TimelineListModel::CidRole)
                         == "bafyreicdfgdzc445denxvuzsklv4q4cw5dlm4kishkrqbv6ssfhbn3lfqy",
                 model.item(row, TimelineListModel::CidRole).toString().toLocal8Bit());
        QVERIFY2(model.item(row, TimelineListModel::IsRepostedByRole) == true,
                 model.item(row, TimelineListModel::IsRepostedByRole).toString().toLocal8Bit());
        row++;
        QVERIFY2(model.item(row, TimelineListModel::CidRole)
                         == "bafyreiaaysocrjeueqgaxyhk2r4ov34havbiup4ovbf7p4am7wycv6qvwy",
                 model.item(row, TimelineListModel::CidRole).toString().toLocal8Bit());
        QVERIFY2(model.item(row, TimelineListModel::IsRepostedByRole) == true,
                 model.item(row, TimelineListModel::IsRepostedByRole).toString().toLocal8Bit());
        row++;
        // QVERIFY2(model.item(row, TimelineListModel::CidRole)
        //                  == "bafyreih77ppn7ykiharfjkqlyq5q6rqzatq5sxj2koomqv22s2z56dk5fe",
        //          model.item(row, TimelineListModel::CidRole).toString().toLocal8Bit());
        // QVERIFY2(model.item(row, TimelineListModel::IsRepostedByRole) == true,
        //          model.item(row, TimelineListModel::IsRepostedByRole).toString().toLocal8Bit());
    }
    //
    {
        model.setVisibleRepostOfOwn(true);
        model.setVisibleRepostOfFollowingUsers(true);
        model.setVisibleRepostOfUnfollowingUsers(true);
        model.setVisibleRepostOfMine(true);
        model.setVisibleRepostByMe(false);

        QVERIFY2(model.rowCount() == 4, QString("%1").arg(model.rowCount()).toLocal8Bit());
        row = 0;
        QVERIFY2(model.item(row, TimelineListModel::CidRole)
                         == "bafyreifvecqlsp36t7wzs2owv5r2fdpb2tq3mwmocpwkxopyocdnvhip3u",
                 model.item(row, TimelineListModel::CidRole).toString().toLocal8Bit());
        QVERIFY2(model.item(row, TimelineListModel::IsRepostedByRole) == true,
                 model.item(row, TimelineListModel::IsRepostedByRole).toString().toLocal8Bit());
        row++;
        QVERIFY2(model.item(row, TimelineListModel::CidRole)
                         == "bafyreidrtozxt3boj6ftbuoacfpu5wjc3hapx4zum6ijmlhakqyfykugja",
                 model.item(row, TimelineListModel::CidRole).toString().toLocal8Bit());
        QVERIFY2(model.item(row, TimelineListModel::IsRepostedByRole) == true,
                 model.item(row, TimelineListModel::IsRepostedByRole).toString().toLocal8Bit());
        row++;
        QVERIFY2(model.item(row, TimelineListModel::CidRole)
                         == "bafyreif5g26rkg42hqigvscvwitm46hwa43v35p5hu7iteycntirquzj7e",
                 model.item(row, TimelineListModel::CidRole).toString().toLocal8Bit());
        QVERIFY2(model.item(row, TimelineListModel::IsRepostedByRole) == true,
                 model.item(row, TimelineListModel::IsRepostedByRole).toString().toLocal8Bit());
        row++;
        QVERIFY2(model.item(row, TimelineListModel::CidRole)
                         == "bafyreiasmg6ks4c4voo6tsiqx4klq6mfjjdloijf4ewp35nj7dnpkpvpc4",
                 model.item(row, TimelineListModel::CidRole).toString().toLocal8Bit());
        QVERIFY2(model.item(row, TimelineListModel::IsRepostedByRole) == true,
                 model.item(row, TimelineListModel::IsRepostedByRole).toString().toLocal8Bit());
        row++;
        // QVERIFY2(model.item(row, TimelineListModel::CidRole)
        //              == "bafyreicdfgdzc445denxvuzsklv4q4cw5dlm4kishkrqbv6ssfhbn3lfqy",
        //          model.item(row, TimelineListModel::CidRole).toString().toLocal8Bit());
        // QVERIFY2(model.item(row, TimelineListModel::IsRepostedByRole) == true,
        //          model.item(row, TimelineListModel::IsRepostedByRole).toString().toLocal8Bit());
        // row++;
        // QVERIFY2(model.item(row, TimelineListModel::CidRole)
        //              == "bafyreiaaysocrjeueqgaxyhk2r4ov34havbiup4ovbf7p4am7wycv6qvwy",
        //          model.item(row, TimelineListModel::CidRole).toString().toLocal8Bit());
        // QVERIFY2(model.item(row, TimelineListModel::IsRepostedByRole) == true,
        //          model.item(row, TimelineListModel::IsRepostedByRole).toString().toLocal8Bit());
        // row++;
        // QVERIFY2(model.item(row, TimelineListModel::CidRole)
        //                  == "bafyreih77ppn7ykiharfjkqlyq5q6rqzatq5sxj2koomqv22s2z56dk5fe",
        //          model.item(row, TimelineListModel::CidRole).toString().toLocal8Bit());
        // QVERIFY2(model.item(row, TimelineListModel::IsRepostedByRole) == true,
        //          model.item(row, TimelineListModel::IsRepostedByRole).toString().toLocal8Bit());
    }
}

void hagoromo_test::test_TimelineListModel_labelers()
{
    int row = 0;
    TimelineListModel model;
    model.setAccount(m_service + "/timeline/labelers/1", "did:plc:l4fsx4ujos7uw7n4ijq2ulgs",
                     "ioriayane.bsky.social", QString(), "dummy", QString());
    model.setDisplayInterval(0);

    QSignalSpy spy(&model, SIGNAL(runningChanged()));
    QVERIFY(model.getLatest());
    spy.wait(10 * 1000);
    QVERIFY2(spy.count() == 2, QString("spy.count()=%1").arg(spy.count()).toUtf8());

    //
    {
        QVERIFY2(model.rowCount() == 7, QString::number(model.rowCount()).toLocal8Bit());
        row = 0;
        QVERIFY2(model.item(row, TimelineListModel::CidRole)
                         == "bafyreihhe3chrtye3fnzozazoutwevgjtjats7jjhasgqmyzm5eag3kjcm_1",
                 model.item(row, TimelineListModel::CidRole).toString().toLocal8Bit());
        QVERIFY2(model.item(row, TimelineListModel::ContentFilterMatchedRole) == true,
                 model.item(row, TimelineListModel::ContentFilterMatchedRole)
                         .toString()
                         .toLocal8Bit());
        QVERIFY2(model.item(row, TimelineListModel::ContentMediaFilterMatchedRole) == false,
                 model.item(row, TimelineListModel::ContentMediaFilterMatchedRole)
                         .toString()
                         .toLocal8Bit());
        QVERIFY2(model.item(row, TimelineListModel::ContentFilterMessageRole) == "Content warning",
                 model.item(row, TimelineListModel::ContentFilterMessageRole)
                         .toString()
                         .toLocal8Bit());
        QVERIFY2(model.item(row, TimelineListModel::ContentMediaFilterMessageRole) == "",
                 model.item(row, TimelineListModel::ContentMediaFilterMessageRole)
                         .toString()
                         .toLocal8Bit());
        row = 1;
        QVERIFY2(model.item(row, TimelineListModel::CidRole)
                         == "bafyreihhe3chrtye3fnzozazoutwevgjtjats7jjhasgqmyzm5eag3kjcm_2",
                 model.item(row, TimelineListModel::CidRole).toString().toLocal8Bit());
        QVERIFY2(model.item(row, TimelineListModel::ContentFilterMatchedRole) == true,
                 model.item(row, TimelineListModel::ContentFilterMatchedRole)
                         .toString()
                         .toLocal8Bit());
        QVERIFY2(model.item(row, TimelineListModel::ContentMediaFilterMatchedRole) == false,
                 model.item(row, TimelineListModel::ContentMediaFilterMatchedRole)
                         .toString()
                         .toLocal8Bit());
        QVERIFY2(model.item(row, TimelineListModel::ContentFilterMessageRole)
                         == "Inauthentic Account",
                 model.item(row, TimelineListModel::ContentFilterMessageRole)
                         .toString()
                         .toLocal8Bit());
        QVERIFY2(model.item(row, TimelineListModel::ContentMediaFilterMessageRole) == "",
                 model.item(row, TimelineListModel::ContentMediaFilterMessageRole)
                         .toString()
                         .toLocal8Bit());
        row = 2;
        QVERIFY2(model.item(row, TimelineListModel::CidRole)
                         == "bafyreihhe3chrtye3fnzozazoutwevgjtjats7jjhasgqmyzm5eag3kjcm_3",
                 model.item(row, TimelineListModel::CidRole).toString().toLocal8Bit());
        QVERIFY2(model.item(row, TimelineListModel::ContentFilterMatchedRole) == false,
                 model.item(row, TimelineListModel::ContentFilterMatchedRole)
                         .toString()
                         .toLocal8Bit());
        QVERIFY2(model.item(row, TimelineListModel::ContentMediaFilterMatchedRole) == true,
                 model.item(row, TimelineListModel::ContentMediaFilterMatchedRole)
                         .toString()
                         .toLocal8Bit());
        QVERIFY2(model.item(row, TimelineListModel::ContentFilterMessageRole) == "",
                 model.item(row, TimelineListModel::ContentFilterMessageRole)
                         .toString()
                         .toLocal8Bit());
        QVERIFY2(model.item(row, TimelineListModel::ContentMediaFilterMessageRole)
                         == "Sexually Suggestive (Cartoon)",
                 model.item(row, TimelineListModel::ContentMediaFilterMessageRole)
                         .toString()
                         .toLocal8Bit());
        row = 3;
        QVERIFY2(model.item(row, TimelineListModel::CidRole)
                         == "bafyreihhe3chrtye3fnzozazoutwevgjtjats7jjhasgqmyzm5eag3kjcm_4",
                 model.item(row, TimelineListModel::CidRole).toString().toLocal8Bit());
        QVERIFY2(model.item(row, TimelineListModel::ContentFilterMatchedRole) == false,
                 model.item(row, TimelineListModel::ContentFilterMatchedRole)
                         .toString()
                         .toLocal8Bit());
        QVERIFY2(model.item(row, TimelineListModel::ContentMediaFilterMatchedRole) == true,
                 model.item(row, TimelineListModel::ContentMediaFilterMatchedRole)
                         .toString()
                         .toLocal8Bit());
        QVERIFY2(model.item(row, TimelineListModel::ContentFilterMessageRole) == "",
                 model.item(row, TimelineListModel::ContentFilterMessageRole)
                         .toString()
                         .toLocal8Bit());
        QVERIFY2(model.item(row, TimelineListModel::ContentMediaFilterMessageRole) == "Violence",
                 model.item(row, TimelineListModel::ContentMediaFilterMessageRole)
                         .toString()
                         .toLocal8Bit());
        row = 4;
        QVERIFY2(model.item(row, TimelineListModel::CidRole)
                         == "bafyreialfbse4o4ccllwz42n5xhpautvv46oxwonwfwp5fxiwl3a4cyl4a_5",
                 model.item(row, TimelineListModel::CidRole).toString().toLocal8Bit());
        QVERIFY2(model.item(row, TimelineListModel::ContentFilterMatchedRole) == false,
                 model.item(row, TimelineListModel::ContentFilterMatchedRole)
                         .toString()
                         .toLocal8Bit());
        QVERIFY2(model.item(row, TimelineListModel::ContentMediaFilterMatchedRole) == false,
                 model.item(row, TimelineListModel::ContentMediaFilterMatchedRole)
                         .toString()
                         .toLocal8Bit());
        QVERIFY2(model.item(row, TimelineListModel::ContentFilterMessageRole) == "",
                 model.item(row, TimelineListModel::ContentFilterMessageRole)
                         .toString()
                         .toLocal8Bit());
        QVERIFY2(model.item(row, TimelineListModel::ContentMediaFilterMessageRole) == "",
                 model.item(row, TimelineListModel::ContentMediaFilterMessageRole)
                         .toString()
                         .toLocal8Bit());
        QVERIFY2(model.item(row, TimelineListModel::QuoteFilterMatchedRole) == true,
                 model.item(row, TimelineListModel::QuoteFilterMatchedRole)
                         .toString()
                         .toLocal8Bit());
        row = 5;
        QVERIFY2(model.item(row, TimelineListModel::CidRole)
                         == "bafyreialfbse4o4ccllwz42n5xhpautvv46oxwonwfwp5fxiwl3a4cyl4a_6",
                 model.item(row, TimelineListModel::CidRole).toString().toLocal8Bit());
        QVERIFY2(model.item(row, TimelineListModel::ContentFilterMatchedRole) == false,
                 model.item(row, TimelineListModel::ContentFilterMatchedRole)
                         .toString()
                         .toLocal8Bit());
        QVERIFY2(model.item(row, TimelineListModel::ContentMediaFilterMatchedRole) == false,
                 model.item(row, TimelineListModel::ContentMediaFilterMatchedRole)
                         .toString()
                         .toLocal8Bit());
        QVERIFY2(model.item(row, TimelineListModel::ContentFilterMessageRole) == "",
                 model.item(row, TimelineListModel::ContentFilterMessageRole)
                         .toString()
                         .toLocal8Bit());
        QVERIFY2(model.item(row, TimelineListModel::ContentMediaFilterMessageRole) == "",
                 model.item(row, TimelineListModel::ContentMediaFilterMessageRole)
                         .toString()
                         .toLocal8Bit());
        QVERIFY2(model.item(row, TimelineListModel::QuoteFilterMatchedRole) == true,
                 model.item(row, TimelineListModel::QuoteFilterMatchedRole)
                         .toString()
                         .toLocal8Bit());
        row = 6;
        QVERIFY2(model.item(row, TimelineListModel::CidRole)
                         == "bafyreialfbse4o4ccllwz42n5xhpautvv46oxwonwfwp5fxiwl3a4cyl4a_7",
                 model.item(row, TimelineListModel::CidRole).toString().toLocal8Bit());
        QVERIFY2(model.item(row, TimelineListModel::ContentFilterMatchedRole) == false,
                 model.item(row, TimelineListModel::ContentFilterMatchedRole)
                         .toString()
                         .toLocal8Bit());
        QVERIFY2(model.item(row, TimelineListModel::ContentMediaFilterMatchedRole) == false,
                 model.item(row, TimelineListModel::ContentMediaFilterMatchedRole)
                         .toString()
                         .toLocal8Bit());
        QVERIFY2(model.item(row, TimelineListModel::ContentFilterMessageRole) == "",
                 model.item(row, TimelineListModel::ContentFilterMessageRole)
                         .toString()
                         .toLocal8Bit());
        QVERIFY2(model.item(row, TimelineListModel::ContentMediaFilterMessageRole) == "",
                 model.item(row, TimelineListModel::ContentMediaFilterMessageRole)
                         .toString()
                         .toLocal8Bit());
        QVERIFY2(model.item(row, TimelineListModel::QuoteFilterMatchedRole) == true,
                 model.item(row, TimelineListModel::QuoteFilterMatchedRole)
                         .toString()
                         .toLocal8Bit());
    }
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
    spy.wait(20 * 1000);
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
    model.setAccount(m_service + "/timeline/next", QString(), QString(), QString(), "dummy",
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

    model.setDisplayInterval(0);
    {
        QSignalSpy spy(&model, SIGNAL(runningChanged()));
        model.getNext();
        spy.wait(10 * 1000);
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
        spy.wait(10 * 1000);
        QVERIFY2(spy.count() == 2, QString("spy.count()=%1").arg(spy.count()).toUtf8());
    }

    QVERIFY2(model.rowCount() == 5, QString("rowCount()=%1").arg(model.rowCount()).toLocal8Bit());
    QVERIFY(model.item(0, TimelineListModel::CidRole)
            == "bafyreidsfmfwqaoud3c64lzuxoblcfgcnkmurjcvgx67hmzgp5vhm3tcia_1");
    QVERIFY(model.item(1, TimelineListModel::CidRole)
            == "bafyreienuzckap2uhylwn2sq2pq5k7rj445exfxtzkrk7xby7dvfy4muoa_2");
    QVERIFY(model.item(2, TimelineListModel::CidRole)
            == "bafyreiayu2dsqvwfyumuepg6b62a24dwtntagoh5lkym7h3lawxcpvz7f4_3");
    QVERIFY(model.item(3, TimelineListModel::CidRole)
            == "bafyreievv2yz3obnigwjix5kr2icycfkqdobrfufd3cm4wfavnjfeqhxbe_4");
    QVERIFY(model.item(4, TimelineListModel::CidRole)
            == "bafyreiayu2dsqvwfyumuepg6b62a24dwtntagoh5lkym7h3lawxcpvz7f4_5");

    model.setVisibleReplyToUnfollowedUsers(false);
    {
        QSignalSpy spy(&model, SIGNAL(runningChanged()));
        model.getLatest();
        spy.wait(10 * 1000);
        QVERIFY2(spy.count() == 2, QString("spy.count()=%1").arg(spy.count()).toUtf8());
    }

    QVERIFY2(model.rowCount() == 3, QString("rowCount()=%1").arg(model.rowCount()).toLocal8Bit());
    QVERIFY(model.item(0, TimelineListModel::CidRole)
            == "bafyreidsfmfwqaoud3c64lzuxoblcfgcnkmurjcvgx67hmzgp5vhm3tcia_1");
    QVERIFY(model.item(1, TimelineListModel::CidRole)
            == "bafyreienuzckap2uhylwn2sq2pq5k7rj445exfxtzkrk7xby7dvfy4muoa_2");
    QVERIFY(model.item(2, TimelineListModel::CidRole)
            == "bafyreievv2yz3obnigwjix5kr2icycfkqdobrfufd3cm4wfavnjfeqhxbe_4");

    model.setAccount(m_service + "/timeline/reply", "did:plc:mqxsuw5b5rhpwo4lw6iwlid5", QString(),
                     QString(), "dummy", QString());
    model.setVisibleReplyToUnfollowedUsers(false);
    {
        QSignalSpy spy(&model, SIGNAL(runningChanged()));
        model.getLatest();
        spy.wait(10 * 1000);
        QVERIFY2(spy.count() == 2, QString("spy.count()=%1").arg(spy.count()).toUtf8());
    }

    QVERIFY2(model.rowCount() == 4, QString("rowCount()=%1").arg(model.rowCount()).toLocal8Bit());
    QVERIFY(model.item(0, TimelineListModel::CidRole)
            == "bafyreienuzckap2uhylwn2sq2pq5k7rj445exfxtzkrk7xby7dvfy4muoa_2");
    QVERIFY(model.item(1, TimelineListModel::CidRole)
            == "bafyreiayu2dsqvwfyumuepg6b62a24dwtntagoh5lkym7h3lawxcpvz7f4_3");
    QVERIFY(model.item(2, TimelineListModel::CidRole)
            == "bafyreievv2yz3obnigwjix5kr2icycfkqdobrfufd3cm4wfavnjfeqhxbe_4");
    QVERIFY(model.item(3, TimelineListModel::CidRole)
            == "bafyreiayu2dsqvwfyumuepg6b62a24dwtntagoh5lkym7h3lawxcpvz7f4_5");
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
        spy.wait(10 * 1000);
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
        spy.wait(10 * 1000);
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

void hagoromo_test::test_SystemTool_ImageClip()
{
    SystemTool tool;

    qDebug() << tool.clipImage(QUrl::fromLocalFile(":/data/images/sample.jpg"), 1700, 130, 400,
                               250);
}

void hagoromo_test::test_SearchProfileListModel_suggestion()
{
    SearchProfileListModel model;
    model.setAccount(m_service + "/search_profile", QString(), QString(), QString(), "dummy",
                     QString());

    QString actual;

    actual = model.extractHandleBlock("");
    QVERIFY2(actual == "", actual.toLocal8Bit());

    actual = model.extractHandleBlock("\n\n");
    QVERIFY2(actual == "", actual.toLocal8Bit());

    actual = model.extractHandleBlock("hoge fuga @handle.bsky.social ");
    QVERIFY2(actual == "", actual.toLocal8Bit());

    actual = model.extractHandleBlock("hoge fuga @handle.bsky.social\t");
    QVERIFY2(actual == "", actual.toLocal8Bit());

    actual = model.extractHandleBlock("hoge fuga @handle.bsky.social\n");
    QVERIFY2(actual == "", actual.toLocal8Bit());

    actual = model.extractHandleBlock("hoge fuga @handle.bsky.social\n\nhoge");
    QVERIFY2(actual == "", actual.toLocal8Bit());

    actual = model.extractHandleBlock("hoge fuga @handle.bsky.social\n\nhoge@handle2");
    QVERIFY2(actual == "handle2", actual.toLocal8Bit());

    actual = model.extractHandleBlock("hoge fuga @handle.bsky.social\n\nhoge@han.dle2");
    QVERIFY2(actual == "han.dle2", actual.toLocal8Bit());

    actual = model.extractHandleBlock("hoge fuga @handle.bsky.social\n\nhoge@han.dle2[");
    QVERIFY2(actual == "", actual.toLocal8Bit());

    actual = model.extractHandleBlock("hoge fuga @handle.bsky.social hoge");
    QVERIFY2(actual == "", actual.toLocal8Bit());

    // 見つかったところを入力済みデータの中で置換
    // 探すときの候補文字列より入力が先にいってしまってずれたときは置き換えないようにする?
    // ↑ハンドルのインクリメンタルサーチではないので変な入力しても一致するとは限らない

    // 1. user input text
    // 2. text to be extracted
    // 3. selected
    // 3. be replaced user input text
    // [] cursor position

    // 1. hoge @handl[]
    // 2. handl
    // 3. handle.bsky.social
    // 4. hoge @handle.bsky.social
    actual = model.replaceText("hoge @handl", 11, "handle.bsky.social");
    QVERIFY2(actual == "hoge @handle.bsky.social ", actual.toLocal8Bit());

    // 1. hoge @handl[] fuga
    // 2. handl
    // 3. handle.bsky.social
    // 4. hoge @handle.bsky.social  fuga
    actual = model.replaceText("hoge @handl fuga", 11, "handle.bsky.social");
    QVERIFY2(actual == "hoge @handle.bsky.social fuga", actual.toLocal8Bit());

    // 1. hoge @handl[]  fuga
    // 2. handl
    // 3. handle.bsky.social
    // 4. hoge @handle.bsky.social  fuga
    actual = model.replaceText("hoge @handl  fuga", 11, "handle.bsky.social");
    QVERIFY2(actual == "hoge @handle.bsky.social  fuga", actual.toLocal8Bit());

    // 1. hoge @handl[]\tfuga
    // 2. handl
    // 3. handle.bsky.social
    // 4. hoge @handle.bsky.social\tfuga
    actual = model.replaceText("hoge @handl\tfuga", 11, "handle.bsky.social");
    QVERIFY2(actual == "hoge @handle.bsky.social\tfuga", actual.toLocal8Bit());

    // 1. hoge @handl[]\nfuga
    // 2. handl
    // 3. handle.bsky.social
    // 4. hoge @handle.bsky.social\nfuga
    actual = model.replaceText("hoge @handl\nfuga", 11, "handle.bsky.social");
    QVERIFY2(actual == "hoge @handle.bsky.social\nfuga", actual.toLocal8Bit());

    // 1. hoge @handl[] fuga
    // 2. handl
    // 3. another.handle.bsky.social
    // 4. hoge @another.handle.bsky.social fuga
    actual = model.replaceText("hoge @handl fuga", 11, "another.handle.bsky.social");
    QVERIFY2(actual == "hoge @another.handle.bsky.social fuga", actual.toLocal8Bit());

    // 1. hoge @handl[]fuga
    // 2. handl
    // 3. handle.bsky.social
    // 4. hoge @handle.bsky.socialfuga
    actual = model.replaceText("hoge @handlfuga", 11, "handle.bsky.social");
    QVERIFY2(actual == "hoge @handle.bsky.social fuga", actual.toLocal8Bit());

    // 1.  hoge @handl[] fuga
    // 1'. hoge @hande[] fuga
    // 2. handl
    // 3. handle.bsky.social
    // 4. hoge @handle.bsky.social[] fuga
    actual = model.replaceText("hoge @hande fuga", 11, "handle.bsky.social");
    QVERIFY2(actual == "hoge @handle.bsky.social fuga", actual.toLocal8Bit());

    // 1. ho\nge @handl[] fuga
    // 2. handl
    // 3. handle.bsky.social
    // 4. ho\nge @handle.bsky.social fuga
    actual = model.replaceText("ho\nge @handl fuga", 12, "handle.bsky.social");
    QVERIFY2(actual == "ho\nge @handle.bsky.social fuga", actual.toLocal8Bit());

    // 1. ho\nge @handl[] f\nuga
    // 2. handl
    // 3. handle.bsky.social
    // 4. ho\nge @handle.bsky.social f\nuga
    actual = model.replaceText("ho\nge @handl f\nuga", 12, "handle.bsky.social");
    QVERIFY2(actual == "ho\nge @handle.bsky.social f\nuga", actual.toLocal8Bit());
}

void hagoromo_test::test_SearchPostListModel_text()
{
    SearchPostListModel model;
    model.setAccount("", "did:plc:hogehoge", "hogehoge.bsky.sockal", "", "", "");

    QVERIFY2(model.replaceSearchCommand("from:me") == "from:hogehoge.bsky.sockal",
             model.text().toLocal8Bit());

    QVERIFY2(model.replaceSearchCommand("fuga  from:me hoge")
                     == "fuga  from:hogehoge.bsky.sockal hoge",
             model.text().toLocal8Bit());

    QVERIFY2(model.replaceSearchCommand("fuga\tfrom:me\thoge")
                     == "fuga from:hogehoge.bsky.sockal hoge",
             model.text().toLocal8Bit());

    QVERIFY2(model.replaceSearchCommand(QString("fuga%1from:me%1hoge").arg(QChar(0x3000)))
                     == "fuga from:hogehoge.bsky.sockal hoge",
             model.text().toLocal8Bit());
}

void hagoromo_test::verifyStr(const QString &expect, const QString &actual)
{
    QVERIFY2(expect == actual,
             QString("\nexpect:%1\nactual:%2\n").arg(expect, actual).toLocal8Bit());
}

QTEST_MAIN(hagoromo_test)

#include "tst_hagoromo_test.moc"
