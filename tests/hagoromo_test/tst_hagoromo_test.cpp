#include <QtTest>
#include <QCoreApplication>
#include <QDateTime>
#include <QJsonDocument>
#include <QDir>
#include <QFile>
#include <QStringConverter>

#include "webserver.h"
#include "unittest_common.h"
#include "timeline/timelinelistmodel.h"
#include "column/columnlistmodel.h"
#include "notification/notificationlistmodel.h"
#include "profile/userprofile.h"
#include "tools/qstringex.h"
#include "profile/anyprofilelistmodel.h"
#include "common.h"
#include "timeline/postthreadlistmodel.h"
#include "profile/searchprofilelistmodel.h"
#include "timeline/searchpostlistmodel.h"
#include "moderation/contentfiltersettinglistmodel.h"
#include "tools/labelerprovider.h"
#include "controls/calendartablemodel.h"
#include "tools/accountmanager.h"
#include "operation/tokimekipolloperator.h"
#include "tools/tid.h"
#include "draft/draftlistmodel.h"

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
    void test_NotificationList_collecting();
    void test_NotificationList_collecting_next();
    void test_NotificationList_collecting_visibility();
    void test_NotificationList_no_collecting();
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
    void test_TimelineListModel_pinnded();
    void test_NotificationListModel_warn();
    void test_TimelineListModel_next();
    void test_AnyProfileListModel();
    void test_TimelineListModel_text();
    void test_TimelineListModel_reply();
    void test_PostThreadListModel();
    void test_SystemTool_ImageClip();
    void test_SearchProfileListModel_suggestion();
    void test_SearchPostListModel_text();
    void test_ContentFilterSettingListModel();
    void test_CalendarTableModel();

    void test_TokimekiPollOperator_convertUrlToUri();
    void test_TokimekiPollOperator_getPoll();
    void test_TokimekiPollOperator_getPoll_noVote();
    void test_TokimekiPollOperator_getPoll_noHit();
    void test_TokimekiPollOperator_makePollOgpFile();
    void test_TokimekiPollOperator_makeAltUrl();
    void test_TokimekiPollOperator_getOgp();

    void test_DraftListModel();

private:
    WebServer m_mockServer;
    quint16 m_listenPort;
    QString m_service;
};

hagoromo_test::hagoromo_test()
{
    QCoreApplication::setOrganizationName(QStringLiteral("relog"));
    QCoreApplication::setApplicationName(QStringLiteral("Hagoromo_unittest"));

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
    QCOMPARE_NE(m_listenPort, 0);
}

void hagoromo_test::cleanupTestCase() { }

void hagoromo_test::test_test_TimelineListModelError()
{
    TimelineListModel model;

    model.setRunning(true);
    QCOMPARE(model.getLatest(), false);
    QCOMPARE(model.getNext(), false);
}

void hagoromo_test::test_TimelineListModelFacet()
{
    QString uuid = AccountManager::getInstance()->updateAccount(
            QString(), m_service + "/facet", "id", "pass", "did:plc:ipj5qejfoqu6eukvt72uhyit",
            "handle", "email", "accessJwt", "refreshJwt", true);

    TimelineListModel model;
    model.setAccount(uuid);
    model.setDisplayInterval(0);

    QSignalSpy spy(&model, SIGNAL(runningChanged()));
    model.getLatest();
    spy.wait(10 * 1000);
    QCOMPARE(spy.count(), 2);

    QFile file(":/response/facet/xrpc/app.bsky.feed.getTimeline.expect");
    QVERIFY(file.open(QFile::ReadOnly));
    QTextStream ts;

    QJsonDocument json_doc = QJsonDocument::fromJson(file.readAll());
    QVERIFY(json_doc.isArray());

    QCOMPARE(json_doc.array().count(), model.rowCount());

    for (int i = 0; i < model.rowCount(); i++) {
        QCOMPARE(json_doc.array().at(i).toObject().value("recordText").toString(),
                 model.item(i, TimelineListModel::TimelineListModelRoles::RecordTextRole)
                         .toString());
    }
}

void hagoromo_test::test_ColumnListModelMove()
{
    ColumnListModel model;

    model.append("uuid_1", 0, false, 10000, 400, 1, "column 1", "value 1", QStringList());
    model.append("uuid_2", 1, false, 20000, 500, 1, "column 2", "value 2", QStringList());
    model.append("uuid_3", 2, false, 30000, 600, 1, "column 3", "value 3", QStringList());
    model.append("uuid_4", 3, false, 40000, 700, 1, "column 4", "value 4", QStringList());

    QCOMPARE(model.getPreviousRow(0), -1);
    QCOMPARE(model.getPreviousRow(1), 0);
    QCOMPARE(model.getPreviousRow(2), 1);
    QCOMPARE(model.getPreviousRow(3), 2);
    QCOMPARE(model.getRowListInOrderOfPosition(), QList<int>() << 0 << 1 << 2 << 3);

    model.move(model.item(1, ColumnListModel::KeyRole).toString(), ColumnListModel::MoveLeft);
    QCOMPARE(model.getPreviousRow(0), 1);
    QCOMPARE(model.getPreviousRow(1), -1);
    QCOMPARE(model.getPreviousRow(2), 0);
    QCOMPARE(model.getPreviousRow(3), 2);
    QCOMPARE(model.getRowListInOrderOfPosition(), QList<int>() << 1 << 0 << 2 << 3);

    model.move(model.item(2, ColumnListModel::KeyRole).toString(), ColumnListModel::MoveLeft);
    QCOMPARE(model.getPreviousRow(0), 2);
    QCOMPARE(model.getPreviousRow(1), -1);
    QCOMPARE(model.getPreviousRow(2), 1);
    QCOMPARE(model.getPreviousRow(3), 0);
    QCOMPARE(model.getRowListInOrderOfPosition(), QList<int>() << 1 << 2 << 0 << 3);

    model.move(model.item(3, ColumnListModel::KeyRole).toString(), ColumnListModel::MoveLeft);
    QCOMPARE(model.getPreviousRow(0), 3);
    QCOMPARE(model.getPreviousRow(1), -1);
    QCOMPARE(model.getPreviousRow(2), 1);
    QCOMPARE(model.getPreviousRow(3), 2);
    QCOMPARE(model.getRowListInOrderOfPosition(), QList<int>() << 1 << 2 << 3 << 0);

    model.move(model.item(1, ColumnListModel::KeyRole).toString(), ColumnListModel::MoveLeft);
    QCOMPARE(model.getPreviousRow(0), 3);
    QCOMPARE(model.getPreviousRow(1), -1);
    QCOMPARE(model.getPreviousRow(2), 1);
    QCOMPARE(model.getPreviousRow(3), 2);
    QCOMPARE(model.getRowListInOrderOfPosition(), QList<int>() << 1 << 2 << 3 << 0);

    model.move(model.item(3, ColumnListModel::KeyRole).toString(), ColumnListModel::MoveRight);
    QCOMPARE(model.getPreviousRow(0), 2);
    QCOMPARE(model.getPreviousRow(1), -1);
    QCOMPARE(model.getPreviousRow(2), 1);
    QCOMPARE(model.getPreviousRow(3), 0);
    QCOMPARE(model.getRowListInOrderOfPosition(), QList<int>() << 1 << 2 << 0 << 3);

    model.move(model.item(2, ColumnListModel::KeyRole).toString(), ColumnListModel::MoveRight);
    QCOMPARE(model.getPreviousRow(0), 1);
    QCOMPARE(model.getPreviousRow(1), -1);
    QCOMPARE(model.getPreviousRow(2), 0);
    QCOMPARE(model.getPreviousRow(3), 2);
    QCOMPARE(model.getRowListInOrderOfPosition(), QList<int>() << 1 << 0 << 2 << 3);

    model.move(model.item(1, ColumnListModel::KeyRole).toString(), ColumnListModel::MoveRight);
    QCOMPARE(model.getPreviousRow(0), -1);
    QCOMPARE(model.getPreviousRow(1), 0);
    QCOMPARE(model.getPreviousRow(2), 1);
    QCOMPARE(model.getPreviousRow(3), 2);
    QCOMPARE(model.getRowListInOrderOfPosition(), QList<int>() << 0 << 1 << 2 << 3);
}

void hagoromo_test::test_ColumnListModelRemove()
{
    ColumnListModel model;

    model.append("uuid_1", 0, false, 10000, 400, 1, "column 1", "value 1", QStringList());
    model.append("uuid_2", 1, false, 20000, 500, 1, "column 2", "value 2", QStringList());
    model.append("uuid_3", 2, false, 30000, 600, 1, "column 3", "value 3", QStringList());
    model.append("uuid_4", 3, false, 40000, 700, 1, "column 4", "value 4", QStringList());

    model.move(model.item(1, ColumnListModel::KeyRole).toString(), ColumnListModel::MoveLeft);
    model.move(model.item(2, ColumnListModel::KeyRole).toString(), ColumnListModel::MoveLeft);
    model.move(model.item(3, ColumnListModel::KeyRole).toString(), ColumnListModel::MoveLeft);
    QCOMPARE(model.getPreviousRow(0), 3);
    QCOMPARE(model.getPreviousRow(1), -1);
    QCOMPARE(model.getPreviousRow(2), 1);
    QCOMPARE(model.getPreviousRow(3), 2);
    QCOMPARE(model.getRowListInOrderOfPosition(), QList<int>() << 1 << 2 << 3 << 0);

    model.removeByKey(model.item(2, ColumnListModel::KeyRole).toString());
    QCOMPARE(model.getPreviousRow(0), 2);
    QCOMPARE(model.getPreviousRow(1), -1);
    QCOMPARE(model.getPreviousRow(2), 1);
    QCOMPARE(model.getRowListInOrderOfPosition(), QList<int>() << 1 << 2 << 0);
}

void hagoromo_test::test_ColumnListModelInsertNext()
{
    ColumnListModel model;

    model.append("uuid_1", 0, false, 10000, 400, 1, "column 1", "value 1", QStringList());
    model.append("uuid_2", 1, false, 20000, 500, 1, "column 2", "value 2", QStringList());
    model.append("uuid_3", 2, false, 30000, 600, 1, "column 3", "value 3", QStringList());
    model.append("uuid_4", 3, false, 40000, 700, 1, "column 4", "value 4", QStringList());

    QCOMPARE(model.getPreviousRow(0), -1);
    QCOMPARE(model.getPreviousRow(1), 0);
    QCOMPARE(model.getPreviousRow(2), 1);
    QCOMPARE(model.getPreviousRow(3), 2);
    QCOMPARE(model.getRowListInOrderOfPosition(), QList<int>() << 0 << 1 << 2 << 3);

    model.insertNext(model.item(0, ColumnListModel::KeyRole).toString(), "uuid_10", 10, false, 10,
                     800, 1, "column 1 next", "value 10", QStringList());
    QCOMPARE(model.getPreviousRow(0), -1);
    QCOMPARE(model.getPreviousRow(1), 4);
    QCOMPARE(model.getPreviousRow(2), 1);
    QCOMPARE(model.getPreviousRow(3), 2);
    QCOMPARE(model.getPreviousRow(4), 0);
    QCOMPARE(model.getRowListInOrderOfPosition(), QList<int>() << 0 << 4 << 1 << 2 << 3);

    model.insertNext(model.item(0, ColumnListModel::KeyRole).toString(), "uuid_11", 11, false, 11,
                     900, 1, "column 1 next2", "value 11", QStringList());
    QCOMPARE(model.getPreviousRow(0), -1);
    QCOMPARE(model.getPreviousRow(1), 4);
    QCOMPARE(model.getPreviousRow(2), 1);
    QCOMPARE(model.getPreviousRow(3), 2);
    QCOMPARE(model.getPreviousRow(4), 5);
    QCOMPARE(model.getPreviousRow(5), 0);
    QCOMPARE(model.getRowListInOrderOfPosition(), QList<int>() << 0 << 5 << 4 << 1 << 2 << 3);

    model.insertNext(model.item(3, ColumnListModel::KeyRole).toString(), "uuid_12", 12, false, 12,
                     1000, 1, "column 4 next", "value 12", QStringList());
    QCOMPARE(model.getPreviousRow(0), -1);
    QCOMPARE(model.getPreviousRow(1), 4);
    QCOMPARE(model.getPreviousRow(2), 1);
    QCOMPARE(model.getPreviousRow(3), 2);
    QCOMPARE(model.getPreviousRow(4), 5);
    QCOMPARE(model.getPreviousRow(5), 0);
    QCOMPARE(model.getPreviousRow(6), 3);
    QCOMPARE(model.getRowListInOrderOfPosition(), QList<int>() << 0 << 5 << 4 << 1 << 2 << 3 << 6);
}

void hagoromo_test::test_ColumnListModelSelected()
{
    int row = 0;
    ColumnListModel model;

    model.append("uuid_1", 0, false, 10000, 400, 1, "column 1", "value 1", QStringList());
    model.append("uuid_2", 1, false, 20000, 500, 1, "column 2", "value 2", QStringList());
    model.append("uuid_3", 2, false, 30000, 600, 1, "column 3", "value 3", QStringList());
    model.append("uuid_4", 3, false, 40000, 700, 1, "column 4", "value 4", QStringList());

    model.move(model.item(1, ColumnListModel::KeyRole).toString(), ColumnListModel::MoveLeft);
    model.move(model.item(2, ColumnListModel::KeyRole).toString(), ColumnListModel::MoveLeft);
    model.move(model.item(2, ColumnListModel::KeyRole).toString(), ColumnListModel::MoveLeft);
    QCOMPARE(model.getRowListInOrderOfPosition(), QList<int>() << 2 << 1 << 0 << 3);

    row = 0;
    QCOMPARE(model.item(row, ColumnListModel::SelectedRole).toBool(), false);
    row++;
    QCOMPARE(model.item(row, ColumnListModel::SelectedRole).toBool(), false);
    row++;
    QCOMPARE(model.item(row, ColumnListModel::SelectedRole).toBool(), true);
    row++;
    QCOMPARE(model.item(row, ColumnListModel::SelectedRole).toBool(), false);

    model.moveSelectionToRight();
    row = 1;
    QCOMPARE(model.item(row, ColumnListModel::SelectedRole).toBool(), true);
    model.moveSelectionToRight();
    row = 0;
    QCOMPARE(model.item(row, ColumnListModel::SelectedRole).toBool(), true);
    model.moveSelectionToRight();
    row = 3;
    QCOMPARE(model.item(row, ColumnListModel::SelectedRole).toBool(), true);
    model.moveSelectionToRight();
    row = 3;
    QCOMPARE(model.item(row, ColumnListModel::SelectedRole).toBool(), true);

    model.moveSelectionToLeft();
    row = 0;
    QCOMPARE(model.item(row, ColumnListModel::SelectedRole).toBool(), true);
    model.moveSelectionToLeft();
    row = 1;
    QCOMPARE(model.item(row, ColumnListModel::SelectedRole).toBool(), true);
    model.moveSelectionToLeft();
    row = 2;
    QCOMPARE(model.item(row, ColumnListModel::SelectedRole).toBool(), true);
    model.moveSelectionToLeft();
    row = 2;
    QCOMPARE(model.item(row, ColumnListModel::SelectedRole).toBool(), true);
}

void hagoromo_test::test_NotificationListModel()
{
    QString uuid = AccountManager::getInstance()->updateAccount(
            QString(), m_service + "/notifications/visible", "id", "pass",
            "did:plc:mqxsuw5b5rhpwo4lw6iwlid5", "handle", "email", "accessJwt", "refreshJwt", true);

    NotificationListModel model;
    model.setAccount(uuid);

    {
        int i = 0;
        QSignalSpy spy(&model, SIGNAL(runningChanged()));
        model.getLatest();
        spy.wait(20 * 1000);
        QCOMPARE(spy.count(), 2);

        QCOMPARE(model.rowCount(), 6);
        QCOMPARE(model.item(i, NotificationListModel::ReasonRole),
                 NotificationListModel::NotificationListModelReason::ReasonLike);
        i++;
        QCOMPARE(model.item(i, NotificationListModel::ReasonRole),
                 NotificationListModel::NotificationListModelReason::ReasonRepost);
        i++;
        QCOMPARE(model.item(i, NotificationListModel::ReasonRole),
                 NotificationListModel::NotificationListModelReason::ReasonFollow);
        i++;
        QCOMPARE(model.item(i, NotificationListModel::ReasonRole),
                 NotificationListModel::NotificationListModelReason::ReasonMention);
        i++;
        QCOMPARE(model.item(i, NotificationListModel::ReasonRole),
                 NotificationListModel::NotificationListModelReason::ReasonReply);
        i++;
        QCOMPARE(model.item(i, NotificationListModel::ReasonRole),
                 NotificationListModel::NotificationListModelReason::ReasonQuote);
        i++;
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
        QCOMPARE(spy.count(), 2);

        QCOMPARE(model.rowCount(), 5);
        QCOMPARE(model.item(i, NotificationListModel::ReasonRole),
                 NotificationListModel::NotificationListModelReason::ReasonRepost);
        i++;
        QCOMPARE(model.item(i, NotificationListModel::ReasonRole),
                 NotificationListModel::NotificationListModelReason::ReasonFollow);
        i++;
        QCOMPARE(model.item(i, NotificationListModel::ReasonRole),
                 NotificationListModel::NotificationListModelReason::ReasonMention);
        i++;
        QCOMPARE(model.item(i, NotificationListModel::ReasonRole),
                 NotificationListModel::NotificationListModelReason::ReasonReply);
        i++;
        QCOMPARE(model.item(i, NotificationListModel::ReasonRole),
                 NotificationListModel::NotificationListModelReason::ReasonQuote);
        i++;
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
        QCOMPARE(spy.count(), 2);

        QCOMPARE(model.rowCount(), 5);
        QCOMPARE(model.item(i, NotificationListModel::ReasonRole),
                 NotificationListModel::NotificationListModelReason::ReasonLike);
        i++;
        QCOMPARE(model.item(i, NotificationListModel::ReasonRole),
                 NotificationListModel::NotificationListModelReason::ReasonFollow);
        i++;
        QCOMPARE(model.item(i, NotificationListModel::ReasonRole),
                 NotificationListModel::NotificationListModelReason::ReasonMention);
        i++;
        QCOMPARE(model.item(i, NotificationListModel::ReasonRole),
                 NotificationListModel::NotificationListModelReason::ReasonReply);
        i++;
        QCOMPARE(model.item(i, NotificationListModel::ReasonRole),
                 NotificationListModel::NotificationListModelReason::ReasonQuote);
        i++;
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
        QCOMPARE(spy.count(), 2);

        QCOMPARE(model.rowCount(), 5);
        QCOMPARE(model.item(i, NotificationListModel::ReasonRole),
                 NotificationListModel::NotificationListModelReason::ReasonLike);
        i++;
        QCOMPARE(model.item(i, NotificationListModel::ReasonRole),
                 NotificationListModel::NotificationListModelReason::ReasonRepost);
        i++;
        QCOMPARE(model.item(i, NotificationListModel::ReasonRole),
                 NotificationListModel::NotificationListModelReason::ReasonMention);
        i++;
        QCOMPARE(model.item(i, NotificationListModel::ReasonRole),
                 NotificationListModel::NotificationListModelReason::ReasonReply);
        i++;
        QCOMPARE(model.item(i, NotificationListModel::ReasonRole),
                 NotificationListModel::NotificationListModelReason::ReasonQuote);
        i++;
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
        QCOMPARE(spy.count(), 2);

        QCOMPARE(model.rowCount(), 5);
        QCOMPARE(model.item(i, NotificationListModel::ReasonRole),
                 NotificationListModel::NotificationListModelReason::ReasonLike);
        i++;
        QCOMPARE(model.item(i, NotificationListModel::ReasonRole),
                 NotificationListModel::NotificationListModelReason::ReasonRepost);
        i++;
        QCOMPARE(model.item(i, NotificationListModel::ReasonRole),
                 NotificationListModel::NotificationListModelReason::ReasonFollow);
        i++;
        QCOMPARE(model.item(i, NotificationListModel::ReasonRole),
                 NotificationListModel::NotificationListModelReason::ReasonReply);
        i++;
        QCOMPARE(model.item(i, NotificationListModel::ReasonRole),
                 NotificationListModel::NotificationListModelReason::ReasonQuote);
        i++;
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
        QCOMPARE(spy.count(), 2);

        QCOMPARE(model.rowCount(), 5);
        QCOMPARE(model.item(i, NotificationListModel::ReasonRole),
                 NotificationListModel::NotificationListModelReason::ReasonLike);
        i++;
        QCOMPARE(model.item(i, NotificationListModel::ReasonRole),
                 NotificationListModel::NotificationListModelReason::ReasonRepost);
        i++;
        QCOMPARE(model.item(i, NotificationListModel::ReasonRole),
                 NotificationListModel::NotificationListModelReason::ReasonFollow);
        i++;
        QCOMPARE(model.item(i, NotificationListModel::ReasonRole),
                 NotificationListModel::NotificationListModelReason::ReasonMention);
        i++;
        QCOMPARE(model.item(i, NotificationListModel::ReasonRole),
                 NotificationListModel::NotificationListModelReason::ReasonQuote);
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
        QCOMPARE(spy.count(), 2);

        QCOMPARE(model.rowCount(), 5);
        QCOMPARE(model.item(i, NotificationListModel::ReasonRole),
                 NotificationListModel::NotificationListModelReason::ReasonLike);
        i++;
        QCOMPARE(model.item(i, NotificationListModel::ReasonRole),
                 NotificationListModel::NotificationListModelReason::ReasonRepost);
        i++;
        QCOMPARE(model.item(i, NotificationListModel::ReasonRole),
                 NotificationListModel::NotificationListModelReason::ReasonFollow);
        i++;
        QCOMPARE(model.item(i, NotificationListModel::ReasonRole),
                 NotificationListModel::NotificationListModelReason::ReasonMention);
        i++;
        QCOMPARE(model.item(i, NotificationListModel::ReasonRole),
                 NotificationListModel::NotificationListModelReason::ReasonReply);
    }
}

void hagoromo_test::test_NotificationListModel2()
{
    QString uuid = AccountManager::getInstance()->updateAccount(
            QString(), m_service + "/notifications/visible", "id", "pass",
            "did:plc:mqxsuw5b5rhpwo4lw6iwlid5", "handle", "email", "accessJwt", "refreshJwt", true);

    NotificationListModel model;
    model.setAccount(uuid);

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
    QCOMPARE(spy.count(), 2);

    QCOMPARE(model.rowCount(), 0);

    //   like
    //   repost
    //   follow
    //   mention
    // * reply
    //   quote
    model.setVisibleReply(true);
    //    model.reflectVisibility();
    QCOMPARE(model.rowCount(), 1);
    QCOMPARE(model.item(i, NotificationListModel::ReasonRole),
             NotificationListModel::NotificationListModelReason::ReasonReply);

    //   like
    //   repost
    // * follow
    //   mention
    // * reply
    //   quote
    i = 0;
    model.setVisibleFollow(true);
    //    model.reflectVisibility();
    QCOMPARE(model.rowCount(), 2);
    QCOMPARE(model.item(i, NotificationListModel::ReasonRole),
             NotificationListModel::NotificationListModelReason::ReasonFollow);
    i++;
    QCOMPARE(model.item(i, NotificationListModel::ReasonRole),
             NotificationListModel::NotificationListModelReason::ReasonReply);
    i++;

    //   like
    //   repost
    // * follow
    // * mention
    // * reply
    //   quote
    i = 0;
    model.setVisibleMention(true);
    //    model.reflectVisibility();
    QCOMPARE(model.rowCount(), 3);
    QCOMPARE(model.item(i, NotificationListModel::ReasonRole),
             NotificationListModel::NotificationListModelReason::ReasonFollow);
    i++;
    QCOMPARE(model.item(i, NotificationListModel::ReasonRole),
             NotificationListModel::NotificationListModelReason::ReasonMention);
    i++;
    QCOMPARE(model.item(i, NotificationListModel::ReasonRole),
             NotificationListModel::NotificationListModelReason::ReasonReply);

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
    QCOMPARE(model.rowCount(), 5);
    QCOMPARE(model.item(i, NotificationListModel::ReasonRole),
             NotificationListModel::NotificationListModelReason::ReasonLike);
    i++;
    QCOMPARE(model.item(i, NotificationListModel::ReasonRole),
             NotificationListModel::NotificationListModelReason::ReasonRepost);
    i++;
    QCOMPARE(model.item(i, NotificationListModel::ReasonRole),
             NotificationListModel::NotificationListModelReason::ReasonFollow);
    i++;
    QCOMPARE(model.item(i, NotificationListModel::ReasonRole),
             NotificationListModel::NotificationListModelReason::ReasonMention);
    i++;
    QCOMPARE(model.item(i, NotificationListModel::ReasonRole),
             NotificationListModel::NotificationListModelReason::ReasonReply);

    // * like
    // * repost
    // * follow
    // * mention
    // * reply
    // * quote
    i = 0;
    model.setVisibleQuote(true);
    //    model.reflectVisibility();
    QCOMPARE(model.rowCount(), 6);
    QCOMPARE(model.item(i, NotificationListModel::ReasonRole),
             NotificationListModel::NotificationListModelReason::ReasonLike);
    i++;
    QCOMPARE(model.item(i, NotificationListModel::ReasonRole),
             NotificationListModel::NotificationListModelReason::ReasonRepost);
    i++;
    QCOMPARE(model.item(i, NotificationListModel::ReasonRole),
             NotificationListModel::NotificationListModelReason::ReasonFollow);
    i++;
    QCOMPARE(model.item(i, NotificationListModel::ReasonRole),
             NotificationListModel::NotificationListModelReason::ReasonMention);
    i++;
    QCOMPARE(model.item(i, NotificationListModel::ReasonRole),
             NotificationListModel::NotificationListModelReason::ReasonReply);
    i++;
    QCOMPARE(model.item(i, NotificationListModel::ReasonRole),
             NotificationListModel::NotificationListModelReason::ReasonQuote);

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
    QCOMPARE(model.rowCount(), 4);
    QCOMPARE(model.item(i, NotificationListModel::ReasonRole),
             NotificationListModel::NotificationListModelReason::ReasonLike);
    i++;
    QCOMPARE(model.item(i, NotificationListModel::ReasonRole),
             NotificationListModel::NotificationListModelReason::ReasonRepost);
    i++;
    QCOMPARE(model.item(i, NotificationListModel::ReasonRole),
             NotificationListModel::NotificationListModelReason::ReasonReply);
    i++;
    QCOMPARE(model.item(i, NotificationListModel::ReasonRole),
             NotificationListModel::NotificationListModelReason::ReasonQuote);

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
    QCOMPARE(model.rowCount(), 4);
    QCOMPARE(model.item(i, NotificationListModel::ReasonRole),
             NotificationListModel::NotificationListModelReason::ReasonLike);
    i++;
    QCOMPARE(model.item(i, NotificationListModel::ReasonRole),
             NotificationListModel::NotificationListModelReason::ReasonMention);
    i++;
    QCOMPARE(model.item(i, NotificationListModel::ReasonRole),
             NotificationListModel::NotificationListModelReason::ReasonReply);
    i++;
    QCOMPARE(model.item(i, NotificationListModel::ReasonRole),
             NotificationListModel::NotificationListModelReason::ReasonQuote);
}

void hagoromo_test::test_NotificationList_collecting()
{
    QString uuid = AccountManager::getInstance()->updateAccount(
            QString(), m_service + "/notifications/collecting/1", "id", "pass",
            "did:plc:ipj5qejfoqu6eukvt72uhyit", "handle", "email", "accessJwt", "refreshJwt", true);

    int i = 0;
    NotificationListModel model;
    model.setAccount(uuid);

    {
        QSignalSpy spy(&model, SIGNAL(runningChanged()));
        model.getLatest();
        spy.wait(20 * 1000);
        QCOMPARE(spy.count(), 2);
    }

    QCOMPARE(model.rowCount(), 1);
    i = 0;
    QStringList liked_avatars =
            model.item(i, NotificationListModel::AggregatedAvatarsRole).toStringList();
    QCOMPARE(liked_avatars.count(), 3);
    QCOMPARE(liked_avatars,
             QStringList() << "https://cdn.bsky.app/img/avatar/plain/did:plc:user_1_did/"
                              "bafkreid6z7kdocoo6snciauq2fwvzbsr3jjvpgokkekul35hk5jbjhdh5y@jpeg"
                           << "https://cdn.bsky.app/img/avatar/plain/did:plc:user_2_did/"
                              "bafkreibgqvqe5ujw4xxqkwvs7twp2f5vhkm3c4yh6gb5rdruppbhvw4o6u@jpeg"
                           << "https://cdn.bsky.app/img/avatar/plain/did:plc:user_3_did/"
                              "bafkreiah4zikm3wgc73y3efsb4dznet5b6e3zmqyjhqyk44b52jc24j47m@jpeg");

    {
        QSignalSpy spy(&model, SIGNAL(runningChanged()));
        model.getLatest();
        spy.wait(20 * 1000);
        QCOMPARE(spy.count(), 2);
    }

    QCOMPARE(model.rowCount(), 1);
    i = 0;
    liked_avatars = model.item(i, NotificationListModel::AggregatedAvatarsRole).toStringList();
    QCOMPARE(liked_avatars.count(), 3);
    QCOMPARE(liked_avatars,
             QStringList() << "https://cdn.bsky.app/img/avatar/plain/did:plc:user_1_did/"
                              "bafkreid6z7kdocoo6snciauq2fwvzbsr3jjvpgokkekul35hk5jbjhdh5y@jpeg"
                           << "https://cdn.bsky.app/img/avatar/plain/did:plc:user_2_did/"
                              "bafkreibgqvqe5ujw4xxqkwvs7twp2f5vhkm3c4yh6gb5rdruppbhvw4o6u@jpeg"
                           << "https://cdn.bsky.app/img/avatar/plain/did:plc:user_3_did/"
                              "bafkreiah4zikm3wgc73y3efsb4dznet5b6e3zmqyjhqyk44b52jc24j47m@jpeg");

    model.setVisibleLike(false);
    QCOMPARE(model.rowCount(), 0);
    model.setVisibleLike(true);
    QCOMPARE(model.rowCount(), 1);
}

void hagoromo_test::test_NotificationList_collecting_next()
{
    QString uuid = AccountManager::getInstance()->updateAccount(
            QString(), m_service + "/notifications/collecting/2", "id", "pass",
            "did:plc:ipj5qejfoqu6eukvt72uhyit", "handle", "email", "accessJwt", "refreshJwt", true);

    NotificationListModel model;
    model.setAccount(uuid);

    {
        QSignalSpy spy(&model, SIGNAL(runningChanged()));
        model.getLatest();
        spy.wait(20 * 1000);
        QCOMPARE(spy.count(), 2);
    }

    QCOMPARE(model.rowCount(), 4);
    QStringList liked_avatars;

    QCOMPARE(model.item(0, NotificationListModel::CidRole).toString(),
             "bafyreiebakaokbztlhlbb5sr2ekp3fnzf3apodwnyx3ae4bevz7d4s23e4");
    liked_avatars = model.item(0, NotificationListModel::AggregatedAvatarsRole).toStringList();
    QCOMPARE(liked_avatars.count(), 3);
    QCOMPARE(liked_avatars,
             QStringList() << "https://cdn.bsky.app/img/avatar/plain/did:plc:user_1_did/"
                              "bafkreid6z7kdocoo6snciauq2fwvzbsr3jjvpgokkekul35hk5jbjhdh5y@jpeg"
                           << "https://cdn.bsky.app/img/avatar/plain/did:plc:user_2_did/"
                              "bafkreibgqvqe5ujw4xxqkwvs7twp2f5vhkm3c4yh6gb5rdruppbhvw4o6u@jpeg"
                           << "https://cdn.bsky.app/img/avatar/plain/did:plc:user_3_did/"
                              "bafkreiah4zikm3wgc73y3efsb4dznet5b6e3zmqyjhqyk44b52jc24j47m@jpeg");

    QCOMPARE(model.item(1, NotificationListModel::CidRole).toString(),
             "bafyreidofiacyhigmns7av6dvxd27hl27epstaqyp5ml3eg7w4a6lpgapi");
    liked_avatars = model.item(1, NotificationListModel::AggregatedAvatarsRole).toStringList();
    QCOMPARE(liked_avatars.count(), 1);

    QCOMPARE(model.item(2, NotificationListModel::CidRole).toString(),
             "bafyreih7kafwej4ezcr6phztanahjxbegbmix3t3vt5k4nghhqtztnpb6e");
    liked_avatars = model.item(2, NotificationListModel::AggregatedAvatarsRole).toStringList();
    QCOMPARE(liked_avatars.count(), 2);
    QCOMPARE(liked_avatars,
             QStringList() << "https://cdn.bsky.app/img/avatar/plain/"
                              "did:plc:mqxsuw5b5rhpwo4lw6iwlid5/"
                              "bafkreiaeoiy6fqjypbhbcrb3jdlnjtpnwri5wa6jrvbwxtbtey6synwxr4@jpeg"
                           << "https://cdn.bsky.app/img/avatar/plain/did:plc:user_11_did/"
                              "bafkreiblrxnoqf2wxdsmwefvgktbfjvtrrasv666veul6raxgpfjj22jay@jpeg");

    QCOMPARE(model.item(3, NotificationListModel::CidRole).toString(),
             "bafyreid45fiup7gptei6wzna534ydnctgl6hahw3gthz4zlurzr6oozrsu");

    {
        QSignalSpy spy(&model, SIGNAL(runningChanged()));
        model.getNext();
        spy.wait(20 * 1000);
        QCOMPARE(spy.count(), 2);
    }

    QCOMPARE(model.rowCount(), 6);

    QCOMPARE(model.item(0, NotificationListModel::CidRole).toString(),
             "bafyreiebakaokbztlhlbb5sr2ekp3fnzf3apodwnyx3ae4bevz7d4s23e4");
    liked_avatars = model.item(0, NotificationListModel::AggregatedAvatarsRole).toStringList();
    QCOMPARE(liked_avatars.count(), 4);
    QCOMPARE(liked_avatars,
             QStringList() << "https://cdn.bsky.app/img/avatar/plain/did:plc:user_1_did/"
                              "bafkreid6z7kdocoo6snciauq2fwvzbsr3jjvpgokkekul35hk5jbjhdh5y@jpeg"
                           << "https://cdn.bsky.app/img/avatar/plain/did:plc:user_2_did/"
                              "bafkreibgqvqe5ujw4xxqkwvs7twp2f5vhkm3c4yh6gb5rdruppbhvw4o6u@jpeg"
                           << "https://cdn.bsky.app/img/avatar/plain/did:plc:user_3_did/"
                              "bafkreiah4zikm3wgc73y3efsb4dznet5b6e3zmqyjhqyk44b52jc24j47m@jpeg"
                           << "https://cdn.bsky.app/img/avatar/plain/did:plc:user_18_did/"
                              "bafkreideay2fqegla7bx3vm47mnfbcwhplw6mk6s4dmxo5657ygomlpe7i@jpeg");

    QCOMPARE(model.item(1, NotificationListModel::CidRole).toString(),
             "bafyreidofiacyhigmns7av6dvxd27hl27epstaqyp5ml3eg7w4a6lpgapi");
    liked_avatars = model.item(1, NotificationListModel::AggregatedAvatarsRole).toStringList();
    QCOMPARE(liked_avatars.count(), 2);

    QCOMPARE(model.item(2, NotificationListModel::CidRole).toString(),
             "bafyreih7kafwej4ezcr6phztanahjxbegbmix3t3vt5k4nghhqtztnpb6e");
    liked_avatars = model.item(2, NotificationListModel::AggregatedAvatarsRole).toStringList();
    QCOMPARE(liked_avatars.count(), 2);
    QCOMPARE(liked_avatars,
             QStringList() << "https://cdn.bsky.app/img/avatar/plain/"
                              "did:plc:mqxsuw5b5rhpwo4lw6iwlid5/"
                              "bafkreiaeoiy6fqjypbhbcrb3jdlnjtpnwri5wa6jrvbwxtbtey6synwxr4@jpeg"
                           << "https://cdn.bsky.app/img/avatar/plain/did:plc:user_11_did/"
                              "bafkreiblrxnoqf2wxdsmwefvgktbfjvtrrasv666veul6raxgpfjj22jay@jpeg");

    QCOMPARE(model.item(3, NotificationListModel::CidRole).toString(),
             "bafyreid45fiup7gptei6wzna534ydnctgl6hahw3gthz4zlurzr6oozrsu");

    QCOMPARE(model.item(4, NotificationListModel::CidRole).toString(),
             "bafyreihh665zos4rof4cefnzd6iiswzswwnotakmxgjbl44vfdt2a7ysxa");

    QCOMPARE(model.item(5, NotificationListModel::CidRole).toString(),
             "bafyreia67z4x3gmhs6afclhdodq2jvdwdimnykjfhpjgmqqtb2hrpce5bm");
}

void hagoromo_test::test_NotificationList_collecting_visibility()
{
    QString uuid = AccountManager::getInstance()->updateAccount(
            QString(), m_service + "/notifications/collecting/2", "id", "pass",
            "did:plc:ipj5qejfoqu6eukvt72uhyit", "handle", "email", "accessJwt", "refreshJwt", true);
    NotificationListModel model;
    model.setAccount(uuid);
    model.setVisibleLike(false);
    {
        QSignalSpy spy(&model, SIGNAL(runningChanged()));
        model.getLatest();
        spy.wait(20 * 1000);
        QCOMPARE(spy.count(), 2);
    }
    QCOMPARE(model.rowCount(), 2);

    model.setVisibleLike(true);

    QCOMPARE(model.rowCount(), 4);
    QStringList liked_avatars;

    QCOMPARE(model.item(0, NotificationListModel::CidRole).toString(),
             "bafyreiebakaokbztlhlbb5sr2ekp3fnzf3apodwnyx3ae4bevz7d4s23e4");
    liked_avatars = model.item(0, NotificationListModel::AggregatedAvatarsRole).toStringList();
    QCOMPARE(liked_avatars.count(), 3);
    QCOMPARE(liked_avatars,
             QStringList() << "https://cdn.bsky.app/img/avatar/plain/did:plc:user_1_did/"
                              "bafkreid6z7kdocoo6snciauq2fwvzbsr3jjvpgokkekul35hk5jbjhdh5y@jpeg"
                           << "https://cdn.bsky.app/img/avatar/plain/did:plc:user_2_did/"
                              "bafkreibgqvqe5ujw4xxqkwvs7twp2f5vhkm3c4yh6gb5rdruppbhvw4o6u@jpeg"
                           << "https://cdn.bsky.app/img/avatar/plain/did:plc:user_3_did/"
                              "bafkreiah4zikm3wgc73y3efsb4dznet5b6e3zmqyjhqyk44b52jc24j47m@jpeg");

    QCOMPARE(model.item(1, NotificationListModel::CidRole).toString(),
             "bafyreidofiacyhigmns7av6dvxd27hl27epstaqyp5ml3eg7w4a6lpgapi");
    liked_avatars = model.item(1, NotificationListModel::AggregatedAvatarsRole).toStringList();
    QCOMPARE(liked_avatars.count(), 1);

    QCOMPARE(model.item(2, NotificationListModel::CidRole).toString(),
             "bafyreih7kafwej4ezcr6phztanahjxbegbmix3t3vt5k4nghhqtztnpb6e");
    liked_avatars = model.item(2, NotificationListModel::AggregatedAvatarsRole).toStringList();
    QCOMPARE(liked_avatars.count(), 2);
    QCOMPARE(liked_avatars,
             QStringList() << "https://cdn.bsky.app/img/avatar/plain/"
                              "did:plc:mqxsuw5b5rhpwo4lw6iwlid5/"
                              "bafkreiaeoiy6fqjypbhbcrb3jdlnjtpnwri5wa6jrvbwxtbtey6synwxr4@jpeg"
                           << "https://cdn.bsky.app/img/avatar/plain/did:plc:user_11_did/"
                              "bafkreiblrxnoqf2wxdsmwefvgktbfjvtrrasv666veul6raxgpfjj22jay@jpeg");

    QCOMPARE(model.item(3, NotificationListModel::CidRole).toString(),
             "bafyreid45fiup7gptei6wzna534ydnctgl6hahw3gthz4zlurzr6oozrsu");

    {
        QSignalSpy spy(&model, SIGNAL(runningChanged()));
        model.getNext();
        spy.wait(20 * 1000);
        QCOMPARE(spy.count(), 2);
    }

    QCOMPARE(model.rowCount(), 6);

    QCOMPARE(model.item(0, NotificationListModel::CidRole).toString(),
             "bafyreiebakaokbztlhlbb5sr2ekp3fnzf3apodwnyx3ae4bevz7d4s23e4");
    liked_avatars = model.item(0, NotificationListModel::AggregatedAvatarsRole).toStringList();
    QCOMPARE(liked_avatars.count(), 4);
    QCOMPARE(liked_avatars,
             QStringList() << "https://cdn.bsky.app/img/avatar/plain/did:plc:user_1_did/"
                              "bafkreid6z7kdocoo6snciauq2fwvzbsr3jjvpgokkekul35hk5jbjhdh5y@jpeg"
                           << "https://cdn.bsky.app/img/avatar/plain/did:plc:user_2_did/"
                              "bafkreibgqvqe5ujw4xxqkwvs7twp2f5vhkm3c4yh6gb5rdruppbhvw4o6u@jpeg"
                           << "https://cdn.bsky.app/img/avatar/plain/did:plc:user_3_did/"
                              "bafkreiah4zikm3wgc73y3efsb4dznet5b6e3zmqyjhqyk44b52jc24j47m@jpeg"
                           << "https://cdn.bsky.app/img/avatar/plain/did:plc:user_18_did/"
                              "bafkreideay2fqegla7bx3vm47mnfbcwhplw6mk6s4dmxo5657ygomlpe7i@jpeg");

    QCOMPARE(model.item(1, NotificationListModel::CidRole).toString(),
             "bafyreidofiacyhigmns7av6dvxd27hl27epstaqyp5ml3eg7w4a6lpgapi");
    liked_avatars = model.item(1, NotificationListModel::AggregatedAvatarsRole).toStringList();
    QCOMPARE(liked_avatars.count(), 2);

    QCOMPARE(model.item(2, NotificationListModel::CidRole).toString(),
             "bafyreih7kafwej4ezcr6phztanahjxbegbmix3t3vt5k4nghhqtztnpb6e");
    liked_avatars = model.item(2, NotificationListModel::AggregatedAvatarsRole).toStringList();
    QCOMPARE(liked_avatars.count(), 2);
    QCOMPARE(liked_avatars,
             QStringList() << "https://cdn.bsky.app/img/avatar/plain/"
                              "did:plc:mqxsuw5b5rhpwo4lw6iwlid5/"
                              "bafkreiaeoiy6fqjypbhbcrb3jdlnjtpnwri5wa6jrvbwxtbtey6synwxr4@jpeg"
                           << "https://cdn.bsky.app/img/avatar/plain/did:plc:user_11_did/"
                              "bafkreiblrxnoqf2wxdsmwefvgktbfjvtrrasv666veul6raxgpfjj22jay@jpeg");

    QCOMPARE(model.item(3, NotificationListModel::CidRole).toString(),
             "bafyreid45fiup7gptei6wzna534ydnctgl6hahw3gthz4zlurzr6oozrsu");

    QCOMPARE(model.item(4, NotificationListModel::CidRole).toString(),
             "bafyreihh665zos4rof4cefnzd6iiswzswwnotakmxgjbl44vfdt2a7ysxa");

    QCOMPARE(model.item(5, NotificationListModel::CidRole).toString(),
             "bafyreia67z4x3gmhs6afclhdodq2jvdwdimnykjfhpjgmqqtb2hrpce5bm");
}

void hagoromo_test::test_NotificationList_no_collecting()
{
    QString uuid = AccountManager::getInstance()->updateAccount(
            QString(), m_service + "/notifications/collecting/2", "id", "pass",
            "did:plc:ipj5qejfoqu6eukvt72uhyit", "handle", "email", "accessJwt", "refreshJwt", true);

    NotificationListModel model;
    model.setAccount(uuid);
    model.setAggregateReactions(false);
    {
        QSignalSpy spy(&model, SIGNAL(runningChanged()));
        model.getLatest();
        spy.wait(20 * 1000);
        QCOMPARE(spy.count(), 2);
    }
    QCOMPARE(model.rowCount(), 7);
    QStringList liked_avatars;

    QCOMPARE(model.item(0, NotificationListModel::CidRole).toString(),
             "bafyreiebakaokbztlhlbb5sr2ekp3fnzf3apodwnyx3ae4bevz7d4s23e4");
    liked_avatars = model.item(0, NotificationListModel::AggregatedAvatarsRole).toStringList();
    QCOMPARE(liked_avatars.count(), 0);

    QCOMPARE(model.item(1, NotificationListModel::CidRole).toString(),
             "bafyreidofiacyhigmns7av6dvxd27hl27epstaqyp5ml3eg7w4a6lpgapi");
    liked_avatars = model.item(1, NotificationListModel::AggregatedAvatarsRole).toStringList();
    QCOMPARE(liked_avatars.count(), 0);

    QCOMPARE(model.item(2, NotificationListModel::CidRole).toString(),
             "bafyreibx5qd5ggsuxn3cd6mdej3hyvwfvzuephlqpjkfwutz74adozzvey");
    liked_avatars = model.item(2, NotificationListModel::AggregatedAvatarsRole).toStringList();
    QCOMPARE(liked_avatars.count(), 0);

    QCOMPARE(model.item(3, NotificationListModel::CidRole).toString(),
             "bafyreidamian6nt26vfagrxmbzjtbmhdy7dubjp66dd44wygofkijxcv7y");

    QCOMPARE(model.item(4, NotificationListModel::CidRole).toString(),
             "bafyreih7kafwej4ezcr6phztanahjxbegbmix3t3vt5k4nghhqtztnpb6e");

    QCOMPARE(model.item(5, NotificationListModel::CidRole).toString(),
             "bafyreid45fiup7gptei6wzna534ydnctgl6hahw3gthz4zlurzr6oozrsu");

    QCOMPARE(model.item(6, NotificationListModel::CidRole).toString(),
             "bafyreidylqkiau6mt3ps6vjnzi5ibcqrgkroshdjxa4imhevks2ni65cxm");

    {
        QSignalSpy spy(&model, SIGNAL(runningChanged()));
        model.getNext();
        spy.wait(20 * 1000);
        QCOMPARE(spy.count(), 2);
    }
    QCOMPARE(model.rowCount(), 11);

    QCOMPARE(model.item(7, NotificationListModel::CidRole).toString(),
             "bafyreieeelqkoh5qwigmudmyofo3qjsonrpwjbt6enx7xbq3dc55p3grmi");

    QCOMPARE(model.item(8, NotificationListModel::CidRole).toString(),
             "bafyreihh665zos4rof4cefnzd6iiswzswwnotakmxgjbl44vfdt2a7ysxa");

    QCOMPARE(model.item(9, NotificationListModel::CidRole).toString(),
             "bafyreiclmktsadw7kw7upeb6gesi7ebjjdduog6e3c3qru3u6axs2i3p5u");

    QCOMPARE(model.item(10, NotificationListModel::CidRole).toString(),
             "bafyreia67z4x3gmhs6afclhdodq2jvdwdimnykjfhpjgmqqtb2hrpce5bm");

    // /////
    model.setAggregateReactions(true);

    QCOMPARE(model.rowCount(), 6);

    QCOMPARE(model.item(0, NotificationListModel::CidRole).toString(),
             "bafyreiebakaokbztlhlbb5sr2ekp3fnzf3apodwnyx3ae4bevz7d4s23e4");
    liked_avatars = model.item(0, NotificationListModel::AggregatedAvatarsRole).toStringList();
    QCOMPARE(liked_avatars.count(), 4);
    QCOMPARE(liked_avatars,
             QStringList() << "https://cdn.bsky.app/img/avatar/plain/did:plc:user_1_did/"
                              "bafkreid6z7kdocoo6snciauq2fwvzbsr3jjvpgokkekul35hk5jbjhdh5y@jpeg"
                           << "https://cdn.bsky.app/img/avatar/plain/did:plc:user_2_did/"
                              "bafkreibgqvqe5ujw4xxqkwvs7twp2f5vhkm3c4yh6gb5rdruppbhvw4o6u@jpeg"
                           << "https://cdn.bsky.app/img/avatar/plain/did:plc:user_3_did/"
                              "bafkreiah4zikm3wgc73y3efsb4dznet5b6e3zmqyjhqyk44b52jc24j47m@jpeg"
                           << "https://cdn.bsky.app/img/avatar/plain/did:plc:user_18_did/"
                              "bafkreideay2fqegla7bx3vm47mnfbcwhplw6mk6s4dmxo5657ygomlpe7i@jpeg");

    QCOMPARE(model.item(1, NotificationListModel::CidRole).toString(),
             "bafyreidofiacyhigmns7av6dvxd27hl27epstaqyp5ml3eg7w4a6lpgapi");
    liked_avatars = model.item(1, NotificationListModel::AggregatedAvatarsRole).toStringList();
    QCOMPARE(liked_avatars.count(), 2);

    QCOMPARE(model.item(2, NotificationListModel::CidRole).toString(),
             "bafyreih7kafwej4ezcr6phztanahjxbegbmix3t3vt5k4nghhqtztnpb6e");
    liked_avatars = model.item(2, NotificationListModel::AggregatedAvatarsRole).toStringList();
    QCOMPARE(liked_avatars.count(), 2);
    QCOMPARE(liked_avatars,
             QStringList() << "https://cdn.bsky.app/img/avatar/plain/"
                              "did:plc:mqxsuw5b5rhpwo4lw6iwlid5/"
                              "bafkreiaeoiy6fqjypbhbcrb3jdlnjtpnwri5wa6jrvbwxtbtey6synwxr4@jpeg"
                           << "https://cdn.bsky.app/img/avatar/plain/did:plc:user_11_did/"
                              "bafkreiblrxnoqf2wxdsmwefvgktbfjvtrrasv666veul6raxgpfjj22jay@jpeg");

    QCOMPARE(model.item(3, NotificationListModel::CidRole).toString(),
             "bafyreid45fiup7gptei6wzna534ydnctgl6hahw3gthz4zlurzr6oozrsu");

    QCOMPARE(model.item(4, NotificationListModel::CidRole).toString(),
             "bafyreihh665zos4rof4cefnzd6iiswzswwnotakmxgjbl44vfdt2a7ysxa");

    QCOMPARE(model.item(5, NotificationListModel::CidRole).toString(),
             "bafyreia67z4x3gmhs6afclhdodq2jvdwdimnykjfhpjgmqqtb2hrpce5bm");

    // /////
    model.setAggregateReactions(false);
    QCOMPARE(model.rowCount(), 11);

    QCOMPARE(model.item(0, NotificationListModel::CidRole).toString(),
             "bafyreiebakaokbztlhlbb5sr2ekp3fnzf3apodwnyx3ae4bevz7d4s23e4");
    liked_avatars = model.item(0, NotificationListModel::AggregatedAvatarsRole).toStringList();
    QCOMPARE(liked_avatars.count(), 0);

    QCOMPARE(model.item(1, NotificationListModel::CidRole).toString(),
             "bafyreidofiacyhigmns7av6dvxd27hl27epstaqyp5ml3eg7w4a6lpgapi");
    liked_avatars = model.item(1, NotificationListModel::AggregatedAvatarsRole).toStringList();
    QCOMPARE(liked_avatars.count(), 0);

    QCOMPARE(model.item(2, NotificationListModel::CidRole).toString(),
             "bafyreibx5qd5ggsuxn3cd6mdej3hyvwfvzuephlqpjkfwutz74adozzvey");
    liked_avatars = model.item(2, NotificationListModel::AggregatedAvatarsRole).toStringList();
    QCOMPARE(liked_avatars.count(), 0);

    QCOMPARE(model.item(3, NotificationListModel::CidRole).toString(),
             "bafyreidamian6nt26vfagrxmbzjtbmhdy7dubjp66dd44wygofkijxcv7y");
    liked_avatars = model.item(3, NotificationListModel::AggregatedAvatarsRole).toStringList();
    QCOMPARE(liked_avatars.count(), 0);

    QCOMPARE(model.item(4, NotificationListModel::CidRole).toString(),
             "bafyreih7kafwej4ezcr6phztanahjxbegbmix3t3vt5k4nghhqtztnpb6e");
    liked_avatars = model.item(4, NotificationListModel::AggregatedAvatarsRole).toStringList();
    QCOMPARE(liked_avatars.count(), 0);

    QCOMPARE(model.item(5, NotificationListModel::CidRole).toString(),
             "bafyreid45fiup7gptei6wzna534ydnctgl6hahw3gthz4zlurzr6oozrsu");
    liked_avatars = model.item(5, NotificationListModel::AggregatedAvatarsRole).toStringList();
    QCOMPARE(liked_avatars.count(), 0);

    QCOMPARE(model.item(6, NotificationListModel::CidRole).toString(),
             "bafyreidylqkiau6mt3ps6vjnzi5ibcqrgkroshdjxa4imhevks2ni65cxm");
    liked_avatars = model.item(6, NotificationListModel::AggregatedAvatarsRole).toStringList();
    QCOMPARE(liked_avatars.count(), 0);

    QCOMPARE(model.item(7, NotificationListModel::CidRole).toString(),
             "bafyreieeelqkoh5qwigmudmyofo3qjsonrpwjbt6enx7xbq3dc55p3grmi");
    liked_avatars = model.item(7, NotificationListModel::AggregatedAvatarsRole).toStringList();
    QCOMPARE(liked_avatars.count(), 0);

    QCOMPARE(model.item(8, NotificationListModel::CidRole).toString(),
             "bafyreihh665zos4rof4cefnzd6iiswzswwnotakmxgjbl44vfdt2a7ysxa");
    liked_avatars = model.item(8, NotificationListModel::AggregatedAvatarsRole).toStringList();
    QCOMPARE(liked_avatars.count(), 0);

    QCOMPARE(model.item(9, NotificationListModel::CidRole).toString(),
             "bafyreiclmktsadw7kw7upeb6gesi7ebjjdduog6e3c3qru3u6axs2i3p5u");
    liked_avatars = model.item(9, NotificationListModel::AggregatedAvatarsRole).toStringList();
    QCOMPARE(liked_avatars.count(), 0);

    QCOMPARE(model.item(10, NotificationListModel::CidRole).toString(),
             "bafyreia67z4x3gmhs6afclhdodq2jvdwdimnykjfhpjgmqqtb2hrpce5bm");
    liked_avatars = model.item(10, NotificationListModel::AggregatedAvatarsRole).toStringList();
    QCOMPARE(liked_avatars.count(), 0);
}

void hagoromo_test::test_UserProfile()
{
    UserProfile profile;

    profile.setDescription("test \r\ntest\ntest");
    QCOMPARE(profile.formattedDescription(), "test <br/>test<br/>test");

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
    ts.setEncoding(QStringConverter::Utf8);
    QStringEx text = ts.readAll();
    qDebug() << text << text.length();
    for (int i = 0; i < text.size(); i++) {
        qDebug() << i << text.at(i) << text.at(i).isHighSurrogate() << text.at(i).isLowSurrogate()
                 << QString::number(text.at(i).unicode(), 16) << text.at(i).combiningClass()
                 << text.at(i).joiningType();
    }
}

void hagoromo_test::test_TimelineListModel_quote_warn()
{
    QString uuid = AccountManager::getInstance()->updateAccount(
            QString(), m_service + "/timeline/warn", "id", "pass", "did:plc:test", "handle",
            "email", "accessJwt", "refreshJwt", true);
    int row = 0;
    TimelineListModel model;
    model.setAccount(uuid);
    model.setDisplayInterval(0);

    LabelerProvider::getInstance()->clear();

    QSignalSpy spy(&model, SIGNAL(runningChanged()));
    model.getLatest();
    spy.wait(10 * 1000);
    QCOMPARE(spy.count(), 2);

    QCOMPARE(model.rowCount(), 7);

    row = 0;
    QVERIFY(model.item(row, TimelineListModel::RecordTextRole).toString()
            == "quoted mute user's post");
    QCOMPARE(model.item(row, TimelineListModel::QuoteFilterMatchedRole).toBool(), true);
    QCOMPARE(model.item(row, TimelineListModel::QuoteRecordBlockedRole).toBool(), false);

    row = 1;
    QVERIFY(model.item(row, TimelineListModel::RecordTextRole).toString()
            == "quote mute user test with image");
    QCOMPARE(model.item(row, TimelineListModel::QuoteFilterMatchedRole).toBool(), true);
    QCOMPARE(model.item(row, TimelineListModel::QuoteRecordBlockedRole).toBool(), false);

    row = 2;
    QVERIFY(model.item(row, TimelineListModel::RecordTextRole).toString()
            == "quote a post with warn label added");
    QCOMPARE(model.item(row, TimelineListModel::QuoteFilterMatchedRole).toBool(), true);
    QCOMPARE(model.item(row, TimelineListModel::QuoteRecordBlockedRole).toBool(), false);

    row = 3;
    QVERIFY(model.item(row, TimelineListModel::RecordTextRole).toString()
            == "quote a post with labeling image");
    QCOMPARE(model.item(row, TimelineListModel::QuoteFilterMatchedRole).toBool(), true);
    QCOMPARE(model.item(row, TimelineListModel::QuoteRecordBlockedRole).toBool(), false);

    row = 4;
    QVERIFY(model.item(row, TimelineListModel::RecordTextRole).toString()
            == "quote a post with warn label added with image");
    QCOMPARE(model.item(row, TimelineListModel::QuoteFilterMatchedRole).toBool(), true);
    QCOMPARE(model.item(row, TimelineListModel::QuoteRecordBlockedRole).toBool(), false);

    row = 5;
    QVERIFY(model.item(row, TimelineListModel::RecordTextRole).toString()
            == "quote a post with labeling image with image");
    QCOMPARE(model.item(row, TimelineListModel::QuoteFilterMatchedRole).toBool(), true);
    QCOMPARE(model.item(row, TimelineListModel::QuoteRecordBlockedRole).toBool(), false);

    row = 6;
    QVERIFY(model.item(row, TimelineListModel::RecordTextRole).toString()
            == "quote blocked user's post");
    QCOMPARE(model.item(row, TimelineListModel::QuoteFilterMatchedRole).toBool(), false);
    QCOMPARE(model.item(row, TimelineListModel::QuoteRecordBlockedRole).toBool(), true);
}

void hagoromo_test::test_TimelineListModel_quote_hide()
{
    QString uuid = AccountManager::getInstance()->updateAccount(
            QString(), m_service + "/timeline/hide", "id", "pass",
            "did:plc:l4fsx4ujos7uw7n4ijq2ulgs", "ioriayane.bsky.social", "email", "accessJwt",
            "refreshJwt", true);

    int row = 0;
    TimelineListModel model;
    model.setAccount(uuid);
    model.setDisplayInterval(0);

    QSignalSpy spy(&model, SIGNAL(runningChanged()));
    model.getLatest();
    spy.wait(10 * 1000);
    QCOMPARE(spy.count(), 2);

    QCOMPARE(model.rowCount(), 7);

    row = 0;
    QVERIFY(model.item(row, TimelineListModel::RecordTextRole).toString()
            == "quoted mute user's post");
    QCOMPARE(model.item(row, TimelineListModel::QuoteFilterMatchedRole).toBool(), true);
    QCOMPARE(model.item(row, TimelineListModel::QuoteRecordBlockedRole).toBool(), false);

    row = 1;
    QVERIFY(model.item(row, TimelineListModel::RecordTextRole).toString()
            == "quote mute user test with image");
    QCOMPARE(model.item(row, TimelineListModel::QuoteFilterMatchedRole).toBool(), true);
    QCOMPARE(model.item(row, TimelineListModel::QuoteRecordBlockedRole).toBool(), false);

    row = 2;
    QVERIFY(model.item(row, TimelineListModel::RecordTextRole).toString()
            == "quote a post with warn label added");
    QCOMPARE(model.item(row, TimelineListModel::QuoteFilterMatchedRole).toBool(), true);
    QCOMPARE(model.item(row, TimelineListModel::QuoteRecordBlockedRole).toBool(), false);

    row = 3;
    QVERIFY(model.item(row, TimelineListModel::RecordTextRole).toString()
            == "quote a post with labeling image");
    QCOMPARE(model.item(row, TimelineListModel::QuoteFilterMatchedRole).toBool(), true);
    QCOMPARE(model.item(row, TimelineListModel::QuoteRecordBlockedRole).toBool(), false);

    row = 4;
    QVERIFY(model.item(row, TimelineListModel::RecordTextRole).toString()
            == "quote a post with warn label added with image");
    QCOMPARE(model.item(row, TimelineListModel::QuoteFilterMatchedRole).toBool(), true);
    QCOMPARE(model.item(row, TimelineListModel::QuoteRecordBlockedRole).toBool(), false);

    row = 5;
    QVERIFY(model.item(row, TimelineListModel::RecordTextRole).toString()
            == "quote a post with labeling image with image");
    QCOMPARE(model.item(row, TimelineListModel::QuoteFilterMatchedRole).toBool(), true);
    QCOMPARE(model.item(row, TimelineListModel::QuoteRecordBlockedRole).toBool(), false);

    row = 6;
    QVERIFY(model.item(row, TimelineListModel::RecordTextRole).toString()
            == "quote blocked user's post");
    QCOMPARE(model.item(row, TimelineListModel::QuoteFilterMatchedRole).toBool(), false);
    QCOMPARE(model.item(row, TimelineListModel::QuoteRecordBlockedRole).toBool(), true);
}

void hagoromo_test::test_TimelineListModel_quote_hide2()
{
    QString uuid = AccountManager::getInstance()->updateAccount(
            QString(), m_service + "/timeline/hide", "id", "pass",
            "did:plc:mqxsuw5b5rhpwo4lw6iwlid5", "ioriayane2.bsky.social", "email", "accessJwt",
            "refreshJwt", true);

    // 自分のポストが引用されているのを見るイメージ
    // 自分のポストの引用はHide設定でも隠さない
    int row = 0;
    TimelineListModel model;
    model.setAccount(uuid);
    model.setDisplayInterval(0);

    QSignalSpy spy(&model, SIGNAL(runningChanged()));
    model.getLatest();
    spy.wait(10 * 1000);
    QCOMPARE(spy.count(), 2);

    QCOMPARE(model.rowCount(), 7);

    row = 0;
    QVERIFY(model.item(row, TimelineListModel::RecordTextRole).toString()
            == "quoted mute user's post");
    QCOMPARE(model.item(row, TimelineListModel::QuoteFilterMatchedRole).toBool(), true);
    QCOMPARE(model.item(row, TimelineListModel::QuoteRecordBlockedRole).toBool(), false);

    row = 1;
    QVERIFY(model.item(row, TimelineListModel::RecordTextRole).toString()
            == "quote mute user test with image");
    QCOMPARE(model.item(row, TimelineListModel::QuoteFilterMatchedRole).toBool(), true);
    QCOMPARE(model.item(row, TimelineListModel::QuoteRecordBlockedRole).toBool(), false);

    row = 2;
    QVERIFY(model.item(row, TimelineListModel::RecordTextRole).toString()
            == "quote a post with warn label added");
    QCOMPARE(model.item(row, TimelineListModel::QuoteFilterMatchedRole).toBool(), false);
    QCOMPARE(model.item(row, TimelineListModel::QuoteRecordBlockedRole).toBool(), false);

    row = 3;
    QVERIFY(model.item(row, TimelineListModel::RecordTextRole).toString()
            == "quote a post with labeling image");
    QCOMPARE(model.item(row, TimelineListModel::QuoteFilterMatchedRole).toBool(), false);
    QCOMPARE(model.item(row, TimelineListModel::QuoteRecordBlockedRole).toBool(), false);
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
    QCOMPARE(model.item(row, TimelineListModel::QuoteFilterMatchedRole).toBool(), false);
    QCOMPARE(model.item(row, TimelineListModel::QuoteRecordBlockedRole).toBool(), false);

    row = 5;
    QVERIFY(model.item(row, TimelineListModel::RecordTextRole).toString()
            == "quote a post with labeling image with image");
    QCOMPARE(model.item(row, TimelineListModel::QuoteFilterMatchedRole).toBool(), false);
    QCOMPARE(model.item(row, TimelineListModel::QuoteRecordBlockedRole).toBool(), false);

    row = 6;
    QVERIFY(model.item(row, TimelineListModel::RecordTextRole).toString()
            == "quote blocked user's post");
    QCOMPARE(model.item(row, TimelineListModel::QuoteFilterMatchedRole).toBool(), false);
    QCOMPARE(model.item(row, TimelineListModel::QuoteRecordBlockedRole).toBool(), true);
    QCOMPARE(model.item(row, TimelineListModel::LabelsRole).toStringList(), QStringList());
}

void hagoromo_test::test_TimelineListModel_quote_label()
{
    QString uuid = AccountManager::getInstance()->updateAccount(
            QString(), m_service + "/timeline/labels", "id", "pass",
            "did:plc:mqxsuw5b5rhpwo4lw6iwlid5", "ioriayane2.bsky.social", "email", "accessJwt",
            "refreshJwt", true);
    int row = 0;
    TimelineListModel model;
    model.setAccount(uuid);
    model.setDisplayInterval(0);

    QSignalSpy spy(&model, SIGNAL(runningChanged()));
    model.getLatest();
    spy.wait(10 * 1000);
    QCOMPARE(spy.count(), 2);

    QCOMPARE(model.rowCount(), 9);

    row = 0;
    QVERIFY(model.item(row, TimelineListModel::RecordTextRole).toString()
            == "quoted mute user's post");
    QCOMPARE(model.item(row, TimelineListModel::QuoteFilterMatchedRole).toBool(), true);
    QCOMPARE(model.item(row, TimelineListModel::QuoteRecordBlockedRole).toBool(), false);
    QCOMPARE(model.item(row, TimelineListModel::LabelsRole).toStringList(), QStringList());
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
    QCOMPARE(model.item(row, TimelineListModel::RecordTextRole).toString(), "test reply full");
    QVERIFY(model.item(row, TimelineListModel::ReplyRootCidRole).toString()
            == "bafyreievv2yz3obnigwjix5kr2icycfkqdobrfufd3cm4wfavnjfeqhxbe");
    QVERIFY(model.item(row, TimelineListModel::ReplyRootUriRole).toString()
            == "at://did:plc:mqxsuw5b5rhpwo4lw6iwlid5/app.bsky.feed.post/3k7n55v57bn26");

    row = 8;
    QCOMPARE(model.item(row, TimelineListModel::RecordTextRole).toString(), "test reply simple");
    QVERIFY(model.item(row, TimelineListModel::ReplyRootCidRole).toString()
            == "bafyreievv2yz3obnigwjix5kr2icycfkqdobrfufd3cm4wfavnjfeqhxbe");
    QVERIFY(model.item(row, TimelineListModel::ReplyRootUriRole).toString()
            == "at://did:plc:mqxsuw5b5rhpwo4lw6iwlid5/app.bsky.feed.post/3k7n55v57bn26");
}

void hagoromo_test::test_TimelineListModel_animated_image()
{
    QString uuid = AccountManager::getInstance()->updateAccount(
            QString(), m_service + "/timeline/animated", "id", "pass",
            "did:plc:mqxsuw5b5rhpwo4lw6iwlid5", "ioriayane2.bsky.social", "email", "accessJwt",
            "refreshJwt", true);

    int row = 0;
    TimelineListModel model;
    model.setAccount(uuid);
    model.setDisplayInterval(0);

    QSignalSpy spy(&model, SIGNAL(runningChanged()));
    QVERIFY(model.getLatest());
    spy.wait(10 * 1000);
    QCOMPARE(spy.count(), 2);

    QCOMPARE(model.rowCount(), 2);

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
    for (const auto &image_path : std::as_const(image_paths)) {
        QFile::setPermissions(image_path, QFile::WriteOwner | QFile::ReadOwner);
        QFile::remove(image_path);
    }

    row = 1;
    QCOMPARE(model.item(row, TimelineListModel::RecordTextRole).toString(), "animated images");

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
    QCOMPARE(model.item(row, TimelineListModel::EmbedImagesRole).toStringList(), thumb_paths);
    QCOMPARE(model.item(row, TimelineListModel::EmbedImagesFullRole).toStringList(), full_paths);

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
    QCOMPARE(model.item(row, TimelineListModel::EmbedImagesRole).toStringList(), thumb_paths);
    QCOMPARE(model.item(row, TimelineListModel::EmbedImagesFullRole).toStringList(), full_paths);

    for (const auto &image_path : std::as_const(image_paths)) {
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
    QCOMPARE(model.item(row, TimelineListModel::EmbedImagesRole).toStringList(), thumb_paths);
    QCOMPARE(model.item(row, TimelineListModel::EmbedImagesFullRole).toStringList(), full_paths);

    // ////////////////////
    QVERIFY(QFile::copy(":/data/images/sample.jpg", image_paths.at(0)));
    // QVERIFY(QFile::copy(":/data/images/sample.jpg", image_paths.at(1)));
    thumb_paths.clear();
    full_paths.clear();
    thumb_paths << QUrl::fromLocalFile(image_paths.at(0)).toString()
                << QUrl::fromLocalFile(image_paths.at(1)).toString();
    full_paths << QUrl::fromLocalFile(image_paths.at(0)).toString()
               << QUrl::fromLocalFile(image_paths.at(1)).toString();
    QCOMPARE(model.item(row, TimelineListModel::EmbedImagesRole).toStringList(), thumb_paths);
    QCOMPARE(model.item(row, TimelineListModel::EmbedImagesFullRole).toStringList(), full_paths);

    for (const auto &image_path : std::as_const(image_paths)) {
        QFile::setPermissions(image_path, QFile::WriteOwner | QFile::ReadOwner);
        QFile::remove(image_path);
    }
}

void hagoromo_test::test_TimelineListModel_threadgate()
{
    QString uuid = AccountManager::getInstance()->updateAccount(
            QString(), m_service + "/timeline/threadgate", "id", "pass",
            "did:plc:mqxsuw5b5rhpwo4lw6iwlid5", "ioriayane2.bsky.social", "email", "accessJwt",
            "refreshJwt", true);

    int row = 0;
    TimelineListModel model;
    model.setAccount(uuid);
    model.setDisplayInterval(0);

    QSignalSpy spy(&model, SIGNAL(runningChanged()));
    QVERIFY(model.getLatest());
    spy.wait(10 * 1000);
    QCOMPARE(spy.count(), 2);

    row = 0;
    QCOMPARE(model.item(row, TimelineListModel::ThreadGateTypeRole), "choice");
    QCOMPARE(model.item(row, TimelineListModel::ThreadGateRulesRole),
             QStringList() << "followed"
                           << "at://did:plc:mqxsuw5b5rhpwo4lw6iwlid5/"
                              "app.bsky.graph.list/3kflf2r3lwg2x"
                           << "at://did:plc:mqxsuw5b5rhpwo4lw6iwlid5/"
                              "app.bsky.graph.list/3kflbnc4c4o2x");

    row = 1;
    QCOMPARE(model.item(row, TimelineListModel::ThreadGateTypeRole), "everybody");
    QCOMPARE(model.item(row, TimelineListModel::ThreadGateRulesRole), QStringList());

    row = 2;
    QCOMPARE(model.item(row, TimelineListModel::ThreadGateTypeRole), "nobody");
    QCOMPARE(model.item(row, TimelineListModel::ThreadGateRulesRole), QStringList());

    row = 3;
    QCOMPARE(model.item(row, TimelineListModel::ThreadGateTypeRole), "choice");
    QCOMPARE(model.item(row, TimelineListModel::ThreadGateRulesRole),
             QStringList() << "at://did:plc:ipj5qejfoqu6eukvt72uhyit/"
                              "app.bsky.graph.list/3kdoiwxqcjv2v");

    row = 4;
    QCOMPARE(model.item(row, TimelineListModel::ThreadGateTypeRole), "choice");
    QCOMPARE(model.item(row, TimelineListModel::ThreadGateRulesRole), QStringList() << "followed");

    row = 5;
    QCOMPARE(model.item(row, TimelineListModel::ThreadGateTypeRole), "choice");
    QCOMPARE(model.item(row, TimelineListModel::ThreadGateRulesRole), QStringList() << "mentioned");
}

void hagoromo_test::test_TimelineListModel_hide_repost()
{
    QString uuid = AccountManager::getInstance()->updateAccount(
            QString(), m_service + "/timeline/hide2", "id", "pass",
            "did:plc:mqxsuw5b5rhpwo4lw6iwlid5", "ioriayane2.bsky.social", "email", "accessJwt",
            "refreshJwt", true);

    int row = 0;
    TimelineListModel model;
    model.setAccount(uuid);
    model.setDisplayInterval(0);

    QSignalSpy spy(&model, SIGNAL(runningChanged()));
    QVERIFY(model.getLatest());
    spy.wait(10 * 1000);
    QCOMPARE(spy.count(), 2);

    //
    {
        QCOMPARE(model.rowCount(), 7);
        row = 0;
        QCOMPARE(model.item(row, TimelineListModel::CidRole),
                 "bafyreifvecqlsp36t7wzs2owv5r2fdpb2tq3mwmocpwkxopyocdnvhip3u");
        QCOMPARE(model.item(row, TimelineListModel::IsRepostedByRole), true);
        row = 1;
        QCOMPARE(model.item(row, TimelineListModel::CidRole),
                 "bafyreidrtozxt3boj6ftbuoacfpu5wjc3hapx4zum6ijmlhakqyfykugja");
        QCOMPARE(model.item(row, TimelineListModel::IsRepostedByRole), true);
        row = 2;
        QCOMPARE(model.item(row, TimelineListModel::CidRole),
                 "bafyreif5g26rkg42hqigvscvwitm46hwa43v35p5hu7iteycntirquzj7e");
        QCOMPARE(model.item(row, TimelineListModel::IsRepostedByRole), true);
        row = 3;
        QCOMPARE(model.item(row, TimelineListModel::CidRole),
                 "bafyreiasmg6ks4c4voo6tsiqx4klq6mfjjdloijf4ewp35nj7dnpkpvpc4");
        QCOMPARE(model.item(row, TimelineListModel::IsRepostedByRole), true);
        row = 4;
        QCOMPARE(model.item(row, TimelineListModel::CidRole),
                 "bafyreicdfgdzc445denxvuzsklv4q4cw5dlm4kishkrqbv6ssfhbn3lfqy");
        QCOMPARE(model.item(row, TimelineListModel::IsRepostedByRole), true);
        row = 5;
        QCOMPARE(model.item(row, TimelineListModel::CidRole),
                 "bafyreiaaysocrjeueqgaxyhk2r4ov34havbiup4ovbf7p4am7wycv6qvwy");
        QCOMPARE(model.item(row, TimelineListModel::IsRepostedByRole), true);
        row = 6;
        QCOMPARE(model.item(row, TimelineListModel::CidRole),
                 "bafyreih77ppn7ykiharfjkqlyq5q6rqzatq5sxj2koomqv22s2z56dk5fe");
        QCOMPARE(model.item(row, TimelineListModel::IsRepostedByRole), true);
    }
    //
    {
        model.setVisibleRepostOfOwn(false);
        model.setVisibleRepostOfFollowingUsers(true);
        model.setVisibleRepostOfUnfollowingUsers(true);
        model.setVisibleRepostOfMine(true);
        model.setVisibleRepostByMe(true);

        QCOMPARE(model.rowCount(), 5);
        row = 0;
        QCOMPARE(model.item(row, TimelineListModel::CidRole),
                 "bafyreifvecqlsp36t7wzs2owv5r2fdpb2tq3mwmocpwkxopyocdnvhip3u");
        QCOMPARE(model.item(row, TimelineListModel::IsRepostedByRole), true);
        row++;
        QCOMPARE(model.item(row, TimelineListModel::CidRole),
                 "bafyreidrtozxt3boj6ftbuoacfpu5wjc3hapx4zum6ijmlhakqyfykugja");
        QCOMPARE(model.item(row, TimelineListModel::IsRepostedByRole), true);
        //    row++;
        //    QCOMPARE(model.item(row, TimelineListModel::CidRole)
        //, "bafyreif5g26rkg42hqigvscvwitm46hwa43v35p5hu7iteycntirquzj7e");
        //    QCOMPARE(model.item(row, TimelineListModel::IsRepostedByRole), true);
        row++;
        QCOMPARE(model.item(row, TimelineListModel::CidRole),
                 "bafyreiasmg6ks4c4voo6tsiqx4klq6mfjjdloijf4ewp35nj7dnpkpvpc4");
        QCOMPARE(model.item(row, TimelineListModel::IsRepostedByRole), true);
        row++;
        QCOMPARE(model.item(row, TimelineListModel::CidRole),
                 "bafyreicdfgdzc445denxvuzsklv4q4cw5dlm4kishkrqbv6ssfhbn3lfqy");
        QCOMPARE(model.item(row, TimelineListModel::IsRepostedByRole), true);
        row++;
        QCOMPARE(model.item(row, TimelineListModel::CidRole),
                 "bafyreiaaysocrjeueqgaxyhk2r4ov34havbiup4ovbf7p4am7wycv6qvwy");
        QCOMPARE(model.item(row, TimelineListModel::IsRepostedByRole), true);
        //    row++;
        //    QCOMPARE(model.item(row, TimelineListModel::CidRole)
        //, "bafyreih77ppn7ykiharfjkqlyq5q6rqzatq5sxj2koomqv22s2z56dk5fe");
        //    QCOMPARE(model.item(row, TimelineListModel::IsRepostedByRole), true);
    }
    //
    {
        model.setVisibleRepostOfOwn(true);
        model.setVisibleRepostOfFollowingUsers(false);
        model.setVisibleRepostOfUnfollowingUsers(true);
        model.setVisibleRepostOfMine(true);
        model.setVisibleRepostByMe(true);

        QCOMPARE(model.rowCount(), 4);
        row = 0;
        QCOMPARE(model.item(row, TimelineListModel::CidRole),
                 "bafyreifvecqlsp36t7wzs2owv5r2fdpb2tq3mwmocpwkxopyocdnvhip3u");
        QCOMPARE(model.item(row, TimelineListModel::IsRepostedByRole), true);
        //    row++;
        //    QCOMPARE(model.item(row, TimelineListModel::CidRole)
        //, "bafyreidrtozxt3boj6ftbuoacfpu5wjc3hapx4zum6ijmlhakqyfykugja");
        //    QCOMPARE(model.item(row, TimelineListModel::IsRepostedByRole), true);
        //    row++;
        //    QCOMPARE(model.item(row, TimelineListModel::CidRole)
        //, "bafyreif5g26rkg42hqigvscvwitm46hwa43v35p5hu7iteycntirquzj7e");
        //    QCOMPARE(model.item(row, TimelineListModel::IsRepostedByRole), true);
        row++;
        QCOMPARE(model.item(row, TimelineListModel::CidRole),
                 "bafyreiasmg6ks4c4voo6tsiqx4klq6mfjjdloijf4ewp35nj7dnpkpvpc4");
        QCOMPARE(model.item(row, TimelineListModel::IsRepostedByRole), true);
        row++;
        QCOMPARE(model.item(row, TimelineListModel::CidRole),
                 "bafyreicdfgdzc445denxvuzsklv4q4cw5dlm4kishkrqbv6ssfhbn3lfqy");
        QCOMPARE(model.item(row, TimelineListModel::IsRepostedByRole), true);
        //    row++;
        //    QCOMPARE(model.item(row, TimelineListModel::CidRole)
        //, "bafyreiaaysocrjeueqgaxyhk2r4ov34havbiup4ovbf7p4am7wycv6qvwy");
        //    QCOMPARE(model.item(row, TimelineListModel::IsRepostedByRole), true);
        row++;
        QCOMPARE(model.item(row, TimelineListModel::CidRole),
                 "bafyreih77ppn7ykiharfjkqlyq5q6rqzatq5sxj2koomqv22s2z56dk5fe");
        QCOMPARE(model.item(row, TimelineListModel::IsRepostedByRole), true);
    }
    //
    {
        model.setVisibleRepostOfOwn(true);
        model.setVisibleRepostOfFollowingUsers(true);
        model.setVisibleRepostOfUnfollowingUsers(false);
        model.setVisibleRepostOfMine(true);
        model.setVisibleRepostByMe(true);

        QCOMPARE(model.rowCount(), 5);
        row = 0;
        // QCOMPARE(model.item(row, TimelineListModel::CidRole)
        //, "bafyreifvecqlsp36t7wzs2owv5r2fdpb2tq3mwmocpwkxopyocdnvhip3u");
        // QCOMPARE(model.item(row, TimelineListModel::IsRepostedByRole), true);
        //    row++;
        QCOMPARE(model.item(row, TimelineListModel::CidRole),
                 "bafyreidrtozxt3boj6ftbuoacfpu5wjc3hapx4zum6ijmlhakqyfykugja");
        QCOMPARE(model.item(row, TimelineListModel::IsRepostedByRole), true);
        row++;
        QCOMPARE(model.item(row, TimelineListModel::CidRole),
                 "bafyreif5g26rkg42hqigvscvwitm46hwa43v35p5hu7iteycntirquzj7e");
        QCOMPARE(model.item(row, TimelineListModel::IsRepostedByRole), true);
        row++;
        QCOMPARE(model.item(row, TimelineListModel::CidRole),
                 "bafyreiasmg6ks4c4voo6tsiqx4klq6mfjjdloijf4ewp35nj7dnpkpvpc4");
        QCOMPARE(model.item(row, TimelineListModel::IsRepostedByRole), true);
        row++;
        // QCOMPARE(model.item(row, TimelineListModel::CidRole)
        //, "bafyreicdfgdzc445denxvuzsklv4q4cw5dlm4kishkrqbv6ssfhbn3lfqy");
        // QCOMPARE(model.item(row, TimelineListModel::IsRepostedByRole), true);
        // row++;
        QCOMPARE(model.item(row, TimelineListModel::CidRole),
                 "bafyreiaaysocrjeueqgaxyhk2r4ov34havbiup4ovbf7p4am7wycv6qvwy");
        QCOMPARE(model.item(row, TimelineListModel::IsRepostedByRole), true);
        row++;
        QCOMPARE(model.item(row, TimelineListModel::CidRole),
                 "bafyreih77ppn7ykiharfjkqlyq5q6rqzatq5sxj2koomqv22s2z56dk5fe");
        QCOMPARE(model.item(row, TimelineListModel::IsRepostedByRole), true);
    }
    //
    {
        model.setVisibleRepostOfOwn(true);
        model.setVisibleRepostOfFollowingUsers(true);
        model.setVisibleRepostOfUnfollowingUsers(true);
        model.setVisibleRepostOfMine(false);
        model.setVisibleRepostByMe(true);

        QCOMPARE(model.rowCount(), 5);
        row = 0;
        QCOMPARE(model.item(row, TimelineListModel::CidRole),
                 "bafyreifvecqlsp36t7wzs2owv5r2fdpb2tq3mwmocpwkxopyocdnvhip3u");
        QCOMPARE(model.item(row, TimelineListModel::IsRepostedByRole), true);
        row++;
        QCOMPARE(model.item(row, TimelineListModel::CidRole),
                 "bafyreidrtozxt3boj6ftbuoacfpu5wjc3hapx4zum6ijmlhakqyfykugja");
        QCOMPARE(model.item(row, TimelineListModel::IsRepostedByRole), true);
        row++;
        QCOMPARE(model.item(row, TimelineListModel::CidRole),
                 "bafyreif5g26rkg42hqigvscvwitm46hwa43v35p5hu7iteycntirquzj7e");
        QCOMPARE(model.item(row, TimelineListModel::IsRepostedByRole), true);
        row++;
        // QCOMPARE(model.item(row, TimelineListModel::CidRole)
        //, "bafyreiasmg6ks4c4voo6tsiqx4klq6mfjjdloijf4ewp35nj7dnpkpvpc4");
        // QCOMPARE(model.item(row, TimelineListModel::IsRepostedByRole), true);
        // row++;
        QCOMPARE(model.item(row, TimelineListModel::CidRole),
                 "bafyreicdfgdzc445denxvuzsklv4q4cw5dlm4kishkrqbv6ssfhbn3lfqy");
        QCOMPARE(model.item(row, TimelineListModel::IsRepostedByRole), true);
        row++;
        QCOMPARE(model.item(row, TimelineListModel::CidRole),
                 "bafyreiaaysocrjeueqgaxyhk2r4ov34havbiup4ovbf7p4am7wycv6qvwy");
        QCOMPARE(model.item(row, TimelineListModel::IsRepostedByRole), true);
        row++;
        // QCOMPARE(model.item(row, TimelineListModel::CidRole)
        //, "bafyreih77ppn7ykiharfjkqlyq5q6rqzatq5sxj2koomqv22s2z56dk5fe");
        // QCOMPARE(model.item(row, TimelineListModel::IsRepostedByRole), true);
    }
    //
    {
        model.setVisibleRepostOfOwn(true);
        model.setVisibleRepostOfFollowingUsers(true);
        model.setVisibleRepostOfUnfollowingUsers(true);
        model.setVisibleRepostOfMine(true);
        model.setVisibleRepostByMe(false);

        QCOMPARE(model.rowCount(), 4);
        row = 0;
        QCOMPARE(model.item(row, TimelineListModel::CidRole),
                 "bafyreifvecqlsp36t7wzs2owv5r2fdpb2tq3mwmocpwkxopyocdnvhip3u");
        QCOMPARE(model.item(row, TimelineListModel::IsRepostedByRole), true);
        row++;
        QCOMPARE(model.item(row, TimelineListModel::CidRole),
                 "bafyreidrtozxt3boj6ftbuoacfpu5wjc3hapx4zum6ijmlhakqyfykugja");
        QCOMPARE(model.item(row, TimelineListModel::IsRepostedByRole), true);
        row++;
        QCOMPARE(model.item(row, TimelineListModel::CidRole),
                 "bafyreif5g26rkg42hqigvscvwitm46hwa43v35p5hu7iteycntirquzj7e");
        QCOMPARE(model.item(row, TimelineListModel::IsRepostedByRole), true);
        row++;
        QCOMPARE(model.item(row, TimelineListModel::CidRole),
                 "bafyreiasmg6ks4c4voo6tsiqx4klq6mfjjdloijf4ewp35nj7dnpkpvpc4");
        QCOMPARE(model.item(row, TimelineListModel::IsRepostedByRole), true);
        row++;
        // QCOMPARE(model.item(row, TimelineListModel::CidRole)
        //, "bafyreicdfgdzc445denxvuzsklv4q4cw5dlm4kishkrqbv6ssfhbn3lfqy");
        // QCOMPARE(model.item(row, TimelineListModel::IsRepostedByRole), true);
        // row++;
        // QCOMPARE(model.item(row, TimelineListModel::CidRole)
        //, "bafyreiaaysocrjeueqgaxyhk2r4ov34havbiup4ovbf7p4am7wycv6qvwy");
        // QCOMPARE(model.item(row, TimelineListModel::IsRepostedByRole), true);
        // row++;
        // QCOMPARE(model.item(row, TimelineListModel::CidRole)
        //, "bafyreih77ppn7ykiharfjkqlyq5q6rqzatq5sxj2koomqv22s2z56dk5fe");
        // QCOMPARE(model.item(row, TimelineListModel::IsRepostedByRole), true);
    }
}

void hagoromo_test::test_TimelineListModel_labelers()
{
    QString uuid = AccountManager::getInstance()->updateAccount(
            QString(), m_service + "/timeline/labelers/1", "id", "pass",
            "did:plc:l4fsx4ujos7uw7n4ijq2ulgs", "ioriayane.bsky.social", "email", "accessJwt",
            "refreshJwt", true);

    int row = 0;
    TimelineListModel model;
    model.setAccount(uuid);
    model.setDisplayInterval(0);

    LabelerProvider::getInstance()->clear();

    QSignalSpy spy(&model, SIGNAL(runningChanged()));
    QVERIFY(model.getLatest());
    spy.wait(10 * 1000);
    QCOMPARE(spy.count(), 2);

    //
    {
        QCOMPARE(model.rowCount(), 7);
        row = 0;
        QCOMPARE(model.item(row, TimelineListModel::CidRole),
                 "bafyreihhe3chrtye3fnzozazoutwevgjtjats7jjhasgqmyzm5eag3kjcm_1");
        QCOMPARE(model.item(row, TimelineListModel::ContentFilterMatchedRole), true);
        QCOMPARE(model.item(row, TimelineListModel::ContentMediaFilterMatchedRole), false);
        QCOMPARE(model.item(row, TimelineListModel::ContentFilterMessageRole), "Content warning");
        QCOMPARE(model.item(row, TimelineListModel::ContentMediaFilterMessageRole), "");
        row = 1;
        QCOMPARE(model.item(row, TimelineListModel::CidRole),
                 "bafyreihhe3chrtye3fnzozazoutwevgjtjats7jjhasgqmyzm5eag3kjcm_2");
        QCOMPARE(model.item(row, TimelineListModel::ContentFilterMatchedRole), true);
        QCOMPARE(model.item(row, TimelineListModel::ContentMediaFilterMatchedRole), false);
        QCOMPARE(model.item(row, TimelineListModel::ContentFilterMessageRole),
                 "Inauthentic Account");
        QCOMPARE(model.item(row, TimelineListModel::ContentMediaFilterMessageRole), "");
        row = 2;
        QCOMPARE(model.item(row, TimelineListModel::CidRole),
                 "bafyreihhe3chrtye3fnzozazoutwevgjtjats7jjhasgqmyzm5eag3kjcm_3");
        QCOMPARE(model.item(row, TimelineListModel::ContentFilterMatchedRole), false);
        QCOMPARE(model.item(row, TimelineListModel::ContentMediaFilterMatchedRole), true);
        QCOMPARE(model.item(row, TimelineListModel::ContentFilterMessageRole), "");
        QCOMPARE(model.item(row, TimelineListModel::ContentMediaFilterMessageRole),
                 "Sexually Suggestive (Cartoon)");
        row = 3;
        QCOMPARE(model.item(row, TimelineListModel::CidRole),
                 "bafyreihhe3chrtye3fnzozazoutwevgjtjats7jjhasgqmyzm5eag3kjcm_4");
        QCOMPARE(model.item(row, TimelineListModel::ContentFilterMatchedRole), false);
        QCOMPARE(model.item(row, TimelineListModel::ContentMediaFilterMatchedRole), true);
        QCOMPARE(model.item(row, TimelineListModel::ContentFilterMessageRole), "");
        QCOMPARE(model.item(row, TimelineListModel::ContentMediaFilterMessageRole),
                 "Graphic Media");
        row = 4;
        QCOMPARE(model.item(row, TimelineListModel::CidRole),
                 "bafyreialfbse4o4ccllwz42n5xhpautvv46oxwonwfwp5fxiwl3a4cyl4a_5");
        QCOMPARE(model.item(row, TimelineListModel::ContentFilterMatchedRole), false);
        QCOMPARE(model.item(row, TimelineListModel::ContentMediaFilterMatchedRole), false);
        QCOMPARE(model.item(row, TimelineListModel::ContentFilterMessageRole), "");
        QCOMPARE(model.item(row, TimelineListModel::ContentMediaFilterMessageRole), "");
        QCOMPARE(model.item(row, TimelineListModel::QuoteFilterMatchedRole), true);
        row = 5;
        QCOMPARE(model.item(row, TimelineListModel::CidRole),
                 "bafyreialfbse4o4ccllwz42n5xhpautvv46oxwonwfwp5fxiwl3a4cyl4a_6");
        QCOMPARE(model.item(row, TimelineListModel::ContentFilterMatchedRole), false);
        QCOMPARE(model.item(row, TimelineListModel::ContentMediaFilterMatchedRole), false);
        QCOMPARE(model.item(row, TimelineListModel::ContentFilterMessageRole), "");
        QCOMPARE(model.item(row, TimelineListModel::ContentMediaFilterMessageRole), "");
        QCOMPARE(model.item(row, TimelineListModel::QuoteFilterMatchedRole), true);
        row = 6;
        QCOMPARE(model.item(row, TimelineListModel::CidRole),
                 "bafyreialfbse4o4ccllwz42n5xhpautvv46oxwonwfwp5fxiwl3a4cyl4a_7");
        QCOMPARE(model.item(row, TimelineListModel::ContentFilterMatchedRole), false);
        QCOMPARE(model.item(row, TimelineListModel::ContentMediaFilterMatchedRole), false);
        QCOMPARE(model.item(row, TimelineListModel::ContentFilterMessageRole), "");
        QCOMPARE(model.item(row, TimelineListModel::ContentMediaFilterMessageRole), "");
        QCOMPARE(model.item(row, TimelineListModel::QuoteFilterMatchedRole), true);
    }
}

void hagoromo_test::test_TimelineListModel_pinnded()
{
    QString uuid = AccountManager::getInstance()->updateAccount(
            QString(), m_service + "/timeline/pinned/1", "id", "pass",
            "did:plc:l4fsx4ujos7uw7n4ijq2ulgs", "ioriayane.bsky.social", "email", "accessJwt",
            "refreshJwt", true);

    int row = 0;
    TimelineListModel model;
    model.setAccount(uuid);
    model.setDisplayInterval(0);
    model.setDisplayPinnedPost(true);
    model.setPinnedPost("at://did:plc:l4fsx4ujos7uw7n4ijq2ulgs/app.bsky.feed.post/3kgbutwycqd2g");

    {
        QSignalSpy spy(&model, SIGNAL(runningChanged()));
        QVERIFY(model.getLatest());
        spy.wait(10 * 1000);
        QCOMPARE(spy.count(), 2);

        QCOMPARE(model.rowCount(), 5);
        row = 0;
        QCOMPARE(model.item(row, TimelineListModel::UriRole),
                 "at://did:plc:l4fsx4ujos7uw7n4ijq2ulgs/app.bsky.feed.post/3kgbutwycqd2g");
        row = 1;
        QCOMPARE(model.item(row, TimelineListModel::UriRole),
                 "at://did:plc:l4fsx4ujos7uw7n4ijq2ulgs/app.bsky.feed.post/3kgbuys4rx52e");
        row = 2;
        QCOMPARE(model.item(row, TimelineListModel::UriRole),
                 "at://did:plc:l4fsx4ujos7uw7n4ijq2ulgs/app.bsky.feed.post/3kgbuyasqh72k");
    }

    uuid = AccountManager::getInstance()->updateAccount(
            QString(), m_service + "/timeline/pinned/2", "id", "pass",
            "did:plc:l4fsx4ujos7uw7n4ijq2ulgs", "ioriayane.bsky.social", "email", "accessJwt",
            "refreshJwt", true);
    model.setAccount(uuid);
    {
        QSignalSpy spy(&model, SIGNAL(runningChanged()));
        QVERIFY(model.getLatest());
        spy.wait(10 * 1000);
        QCOMPARE(spy.count(), 2);

        QCOMPARE(model.rowCount(), 7);
        row = 0;
        QCOMPARE(model.item(row, TimelineListModel::UriRole),
                 "at://did:plc:l4fsx4ujos7uw7n4ijq2ulgs/app.bsky.feed.post/3kgbutwycqd2g");
        row = 1;
        QCOMPARE(model.item(row, TimelineListModel::UriRole),
                 "at://did:plc:mqxsuw5b5rhpwo4lw6iwlid5/app.bsky.feed.post/3klrvkltf672n");
        row = 2;
        QCOMPARE(model.item(row, TimelineListModel::UriRole),
                 "at://did:plc:mqxsuw5b5rhpwo4lw6iwlid5/app.bsky.feed.post/3klrth7knov2m");
        row = 3;
        QCOMPARE(model.item(row, TimelineListModel::UriRole),
                 "at://did:plc:l4fsx4ujos7uw7n4ijq2ulgs/app.bsky.feed.post/3kgbuys4rx52e");
    }

    // clear
    model.setPinnedPost("");
    {
        QSignalSpy spy(&model, SIGNAL(runningChanged()));
        QVERIFY(model.getLatest());
        spy.wait(10 * 1000);
        QCOMPARE(spy.count(), 2);

        QCOMPARE(model.rowCount(), 6);
        row = 0;
        QCOMPARE(model.item(row, TimelineListModel::UriRole),
                 "at://did:plc:mqxsuw5b5rhpwo4lw6iwlid5/app.bsky.feed.post/3klrvkltf672n");
        row = 1;
        QCOMPARE(model.item(row, TimelineListModel::UriRole),
                 "at://did:plc:mqxsuw5b5rhpwo4lw6iwlid5/app.bsky.feed.post/3klrth7knov2m");
        row = 2;
        QCOMPARE(model.item(row, TimelineListModel::UriRole),
                 "at://did:plc:l4fsx4ujos7uw7n4ijq2ulgs/app.bsky.feed.post/3kgbuys4rx52e");
    }

    // re-set
    uuid = AccountManager::getInstance()->updateAccount(
            QString(), m_service + "/timeline/pinned/3", "id", "pass",
            "did:plc:l4fsx4ujos7uw7n4ijq2ulgs", "ioriayane.bsky.social", "email", "accessJwt",
            "refreshJwt", true);
    model.setAccount(uuid);
    model.setPinnedPost("at://did:plc:mqxsuw5b5rhpwo4lw6iwlid5/app.bsky.feed.post/3klrvkltf672n");
    {
        QSignalSpy spy(&model, SIGNAL(runningChanged()));
        QVERIFY(model.getLatest());
        spy.wait(10 * 1000);
        QCOMPARE(spy.count(), 2);

        QCOMPARE(model.rowCount(), 7);
        row = 0;
        QCOMPARE(model.item(row, TimelineListModel::UriRole),
                 "at://did:plc:mqxsuw5b5rhpwo4lw6iwlid5/app.bsky.feed.post/3klrvkltf672n");
        row = 1;
        QCOMPARE(model.item(row, TimelineListModel::UriRole),
                 "at://did:plc:mqxsuw5b5rhpwo4lw6iwlid5/app.bsky.feed.post/3klrvkltf672n");
        row = 2;
        QCOMPARE(model.item(row, TimelineListModel::UriRole),
                 "at://did:plc:mqxsuw5b5rhpwo4lw6iwlid5/app.bsky.feed.post/3klrth7knov2m");
        row = 3;
        QCOMPARE(model.item(row, TimelineListModel::UriRole),
                 "at://did:plc:l4fsx4ujos7uw7n4ijq2ulgs/app.bsky.feed.post/3kgbuys4rx52e");
    }

    // replace pin
    uuid = AccountManager::getInstance()->updateAccount(
            QString(), m_service + "/timeline/pinned/2", "id", "pass",
            "did:plc:l4fsx4ujos7uw7n4ijq2ulgs", "ioriayane.bsky.social", "email", "accessJwt",
            "refreshJwt", true);
    model.setAccount(uuid);
    model.setPinnedPost("at://did:plc:l4fsx4ujos7uw7n4ijq2ulgs/app.bsky.feed.post/3kgbutwycqd2g");
    {
        QSignalSpy spy(&model, SIGNAL(runningChanged()));
        QVERIFY(model.getLatest());
        spy.wait(10 * 1000);
        QCOMPARE(spy.count(), 2);

        QCOMPARE(model.rowCount(), 7);
        row = 0;
        QCOMPARE(model.item(row, TimelineListModel::UriRole),
                 "at://did:plc:l4fsx4ujos7uw7n4ijq2ulgs/app.bsky.feed.post/3kgbutwycqd2g");
        row = 1;
        QCOMPARE(model.item(row, TimelineListModel::UriRole),
                 "at://did:plc:mqxsuw5b5rhpwo4lw6iwlid5/app.bsky.feed.post/3klrvkltf672n");
        row = 2;
        QCOMPARE(model.item(row, TimelineListModel::UriRole),
                 "at://did:plc:mqxsuw5b5rhpwo4lw6iwlid5/app.bsky.feed.post/3klrth7knov2m");
        row = 3;
        QCOMPARE(model.item(row, TimelineListModel::UriRole),
                 "at://did:plc:l4fsx4ujos7uw7n4ijq2ulgs/app.bsky.feed.post/3kgbuys4rx52e");
    }
}

void hagoromo_test::test_NotificationListModel_warn()
{
    QString uuid = AccountManager::getInstance()->updateAccount(
            QString(), m_service + "/notifications/warn", "id", "pass", "did:plc:test",
            "ioriayane.bsky.social", "email", "accessJwt", "refreshJwt", true);
    int row = 0;
    NotificationListModel model;
    model.setAccount(uuid);
    model.setDisplayInterval(0);

    LabelerProvider::getInstance()->clear();

    QSignalSpy spy(&model, SIGNAL(runningChanged()));
    model.getLatest();
    spy.wait(20 * 1000);
    QCOMPARE(spy.count(), 2);

    QCOMPARE(model.rowCount(), 2);

    row = 0;
    QVERIFY(model.item(row, NotificationListModel::RecordTextPlainRole).toString()
            == "@ioriayane2.bsky.social labeling sexual with image");
    QCOMPARE(model.item(row, NotificationListModel::ContentFilterMatchedRole).toBool(), false);
    QCOMPARE(model.item(row, NotificationListModel::ContentMediaFilterMatchedRole).toBool(), true);
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
    QCOMPARE(model.item(row, NotificationListModel::ContentFilterMatchedRole).toBool(), true);
    QVERIFY(model.item(row, NotificationListModel::ContentMediaFilterMatchedRole).toBool()
            == false);
}

void hagoromo_test::test_TimelineListModel_next()
{
    QString uuid = AccountManager::getInstance()->updateAccount(
            QString(), m_service + "/timeline/next", "id", "pass",
            "did:plc:ipj5qejfoqu6eukvt72uhyit", "ioriayane.bsky.social", "email", "accessJwt",
            "refreshJwt", true);

    int row = 0;
    TimelineListModel model;
    model.setAccount(uuid);
    model.setDisplayInterval(0);
    {
        QSignalSpy spy(&model, SIGNAL(runningChanged()));
        model.getLatest();
        spy.wait(10 * 1000);
        QCOMPARE(spy.count(), 2);
    }
    QCOMPARE(model.rowCount(), 6);

    row = 0;
    QVERIFY(model.item(row, TimelineListModel::CidRole).toString()
            == "bafyreig7i2uyva4rpgxv3slogiwf5fvlwy2wx4bjvwuoywy6e7ojjvcrky_1");
    row = 1;
    QVERIFY(model.item(row, TimelineListModel::CidRole).toString()
            == "bafyreiajxwbfoa5cbnphxcwvunisgjiqkjjqkqxpnr4mgfu3vqqupr6wca_2");
    row = 2;
    QVERIFY(model.item(row, TimelineListModel::CidRole).toString()
            == "bafyreif2ux6qo4b2ez266iewichrjvqgeeehui5c7lo3gcglrzdi54pjja_3");
    QCOMPARE(model.item(row, TimelineListModel::IsRepostedRole).toBool(), true);

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
        QCOMPARE(spy.count(), 2);
    }
    QCOMPARE(model.rowCount(), 13);

    row = 6;
    QVERIFY(model.item(row, TimelineListModel::CidRole).toString()
            == "bafyreiejog3yvjc2tdg4muknodbplaib2yqftukwurd4qjcnal3zdxu4ni_7");
    row = 7;
    QCOMPARE(model.item(row, TimelineListModel::CidRole).toString(),
             "bafyreib67ewj54g6maljtbclhno7mrkquf3w7wbex2woedj5m23mjwyite_8");
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
    QString uuid = AccountManager::getInstance()->updateAccount(
            QString(), m_service + "/anyprofile", "id", "pass", "did:plc:test",
            "ioriayane.bsky.social", "email", "accessJwt", "refreshJwt", true);

    AnyProfileListModel model;
    model.setAccount(uuid);
    model.setTargetUri("at://did:plc:ipj5qejfoqu6eukvt72uhyit/app.bsky.feed.post/3k6tpw4xr4d27");

    model.setType(AnyProfileListModel::AnyProfileListModelType::Like);
    {
        QSignalSpy spy(&model, SIGNAL(runningChanged()));
        model.getLatest();
        spy.wait();
        QCOMPARE(spy.count(), 2);
    }

    QCOMPARE(model.rowCount(), 5);

    model.setType(AnyProfileListModel::AnyProfileListModelType::Repost);
    {
        QSignalSpy spy(&model, SIGNAL(runningChanged()));
        model.getLatest();
        spy.wait();
        QCOMPARE(spy.count(), 2);
    }

    QCOMPARE(model.rowCount(), 2);
}

void hagoromo_test::test_TimelineListModel_text()
{
    QString uuid = AccountManager::getInstance()->updateAccount(
            QString(), m_service + "/timeline/text", "id", "pass", "did:plc:test",
            "ioriayane.bsky.social", "email", "accessJwt", "refreshJwt", true);

    int row = 0;
    TimelineListModel model;
    model.setAccount(uuid);
    model.setDisplayInterval(0);

    QSignalSpy spy(&model, SIGNAL(runningChanged()));
    model.getLatest();
    spy.wait(10 * 1000);
    QCOMPARE(spy.count(), 2);

    QCOMPARE(model.rowCount(), 4);

    qDebug() << QString("hoge\nfuga<>\"foo").toHtmlEscaped();

    row = 0;
    QCOMPARE(model.item(row, TimelineListModel::RecordTextPlainRole).toString(), "test < data");
    QCOMPARE(model.item(row, TimelineListModel::RecordTextRole).toString(), "test &lt; data");

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
    QString uuid = AccountManager::getInstance()->updateAccount(
            QString(), m_service + "/timeline/reply", "id", "pass",
            "did:plc:ipj5qejfoqu6eukvt72uhyit", "ioriayane.bsky.social", "email", "accessJwt",
            "refreshJwt", true);

    TimelineListModel model;
    model.setAccount(uuid);
    model.setDisplayInterval(0);

    model.setVisibleReplyToUnfollowedUsers(true);
    {
        QSignalSpy spy(&model, SIGNAL(runningChanged()));
        model.getLatest();
        spy.wait(10 * 1000);
        QCOMPARE(spy.count(), 2);
    }

    QCOMPARE(model.rowCount(), 5);
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
        QCOMPARE(spy.count(), 2);
    }

    QCOMPARE(model.rowCount(), 3);
    QVERIFY(model.item(0, TimelineListModel::CidRole)
            == "bafyreidsfmfwqaoud3c64lzuxoblcfgcnkmurjcvgx67hmzgp5vhm3tcia_1");
    QVERIFY(model.item(1, TimelineListModel::CidRole)
            == "bafyreienuzckap2uhylwn2sq2pq5k7rj445exfxtzkrk7xby7dvfy4muoa_2");
    QVERIFY(model.item(2, TimelineListModel::CidRole)
            == "bafyreievv2yz3obnigwjix5kr2icycfkqdobrfufd3cm4wfavnjfeqhxbe_4");

    uuid = AccountManager::getInstance()->updateAccount(
            QString(), m_service + "/timeline/reply", "id", "pass",
            "did:plc:mqxsuw5b5rhpwo4lw6iwlid5", "ioriayane.bsky.social", "email", "accessJwt",
            "refreshJwt", true);
    model.setAccount(uuid);
    model.setVisibleReplyToUnfollowedUsers(false);
    {
        QSignalSpy spy(&model, SIGNAL(runningChanged()));
        model.getLatest();
        spy.wait(10 * 1000);
        QCOMPARE(spy.count(), 2);
    }

    QCOMPARE(model.rowCount(), 5);
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
}

void hagoromo_test::test_PostThreadListModel()
{
    QString uuid = AccountManager::getInstance()->updateAccount(
            QString(), m_service + "/postthread/1", "id", "pass",
            "did:plc:mqxsuw5b5rhpwo4lw6iwlid5", "ioriayane.bsky.social", "email", "accessJwt",
            "refreshJwt", true);
    PostThreadListModel model;
    model.setAccount(uuid);
    model.setDisplayInterval(0);
    model.setPostThreadUri("at://uri");

    {
        QSignalSpy spy(&model, SIGNAL(runningChanged()));
        model.getLatest();
        spy.wait(10 * 1000);
        QCOMPARE(spy.count(), 2);
    }

    int row;

    QCOMPARE(model.rowCount(), 4);
    row = 0;
    QCOMPARE(model.item(row, PostThreadListModel::RecordTextPlainRole).toString(), "test");
    QCOMPARE(model.item(row, PostThreadListModel::ThreadConnectorTopRole).toBool(), false);
    QCOMPARE(model.item(row, PostThreadListModel::ThreadConnectorBottomRole).toBool(), true);
    row = 1;
    QCOMPARE(model.item(row, PostThreadListModel::RecordTextPlainRole).toString(), "reply 2");
    QCOMPARE(model.item(row, PostThreadListModel::ThreadConnectorTopRole).toBool(), true);
    QCOMPARE(model.item(row, PostThreadListModel::ThreadConnectorBottomRole).toBool(), true);
    row = 2;
    QCOMPARE(model.item(row, PostThreadListModel::RecordTextPlainRole).toString(), "reply 3");
    QCOMPARE(model.item(row, PostThreadListModel::ThreadConnectorTopRole).toBool(), true);
    QCOMPARE(model.item(row, PostThreadListModel::ThreadConnectorBottomRole).toBool(), true);
    row = 3;
    QCOMPARE(model.item(row, PostThreadListModel::RecordTextPlainRole).toString(), "reply 4");
    QCOMPARE(model.item(row, PostThreadListModel::ThreadConnectorTopRole).toBool(), true);
    QCOMPARE(model.item(row, PostThreadListModel::ThreadConnectorBottomRole).toBool(), false);

    uuid = AccountManager::getInstance()->updateAccount(
            QString(), m_service + "/postthread/2", "id", "pass",
            "did:plc:mqxsuw5b5rhpwo4lw6iwlid5", "ioriayane.bsky.social", "email", "accessJwt",
            "refreshJwt", true);
    model.clear();
    model.setAccount(uuid);
    {
        QSignalSpy spy(&model, SIGNAL(runningChanged()));
        model.getLatest();
        spy.wait(10 * 1000);
        QCOMPARE(spy.count(), 2);
    }

    QCOMPARE(model.rowCount(), 6);
    row = 0;
    QCOMPARE(model.item(row, PostThreadListModel::RecordTextPlainRole).toString(), "test");
    QCOMPARE(model.item(row, PostThreadListModel::ThreadConnectorTopRole).toBool(), false);
    QCOMPARE(model.item(row, PostThreadListModel::ThreadConnectorBottomRole).toBool(), true);
    row = 1;
    QCOMPARE(model.item(row, PostThreadListModel::RecordTextPlainRole).toString(), "reply 2");
    QCOMPARE(model.item(row, PostThreadListModel::ThreadConnectorTopRole).toBool(), true);
    QCOMPARE(model.item(row, PostThreadListModel::ThreadConnectorBottomRole).toBool(), true);
    row = 2;
    QCOMPARE(model.item(row, PostThreadListModel::RecordTextPlainRole).toString(), "reply 3 - 2");
    QCOMPARE(model.item(row, PostThreadListModel::ThreadConnectorTopRole).toBool(), true);
    QCOMPARE(model.item(row, PostThreadListModel::ThreadConnectorBottomRole).toBool(), true);
    row = 3;
    QCOMPARE(model.item(row, PostThreadListModel::RecordTextPlainRole).toString(), "reply 4 - 2");
    QCOMPARE(model.item(row, PostThreadListModel::ThreadConnectorTopRole).toBool(), true);
    QCOMPARE(model.item(row, PostThreadListModel::ThreadConnectorBottomRole).toBool(), false);
    row = 4;
    QCOMPARE(model.item(row, PostThreadListModel::RecordTextPlainRole).toString(), "reply 3");
    QCOMPARE(model.item(row, PostThreadListModel::ThreadConnectorTopRole).toBool(), true);
    QCOMPARE(model.item(row, PostThreadListModel::ThreadConnectorBottomRole).toBool(), true);
    row = 5;
    QCOMPARE(model.item(row, PostThreadListModel::RecordTextPlainRole).toString(), "reply 4");
    QCOMPARE(model.item(row, PostThreadListModel::ThreadConnectorTopRole).toBool(), true);
    QCOMPARE(model.item(row, PostThreadListModel::ThreadConnectorBottomRole).toBool(), false);

    uuid = AccountManager::getInstance()->updateAccount(
            QString(), m_service + "/postthread/3", "id", "pass",
            "did:plc:mqxsuw5b5rhpwo4lw6iwlid5", "ioriayane.bsky.social", "email", "accessJwt",
            "refreshJwt", true);
    model.clear();
    model.setAccount(uuid);
    {
        QSignalSpy spy(&model, SIGNAL(runningChanged()));
        model.getLatest();
        spy.wait(10 * 1000);
        QCOMPARE(spy.count(), 2);
    }

    QCOMPARE(model.rowCount(), 6);
    row = 0;
    QCOMPARE(model.item(row, PostThreadListModel::RecordTextPlainRole).toString(), "test");
    QCOMPARE(model.item(row, PostThreadListModel::ThreadConnectorTopRole).toBool(), false);
    QCOMPARE(model.item(row, PostThreadListModel::ThreadConnectorBottomRole).toBool(), true);
    row = 1;
    QCOMPARE(model.item(row, PostThreadListModel::RecordTextPlainRole).toString(), "reply 2");
    QCOMPARE(model.item(row, PostThreadListModel::ThreadConnectorTopRole).toBool(), true);
    QCOMPARE(model.item(row, PostThreadListModel::ThreadConnectorBottomRole).toBool(), true);
    row = 2;
    QCOMPARE(model.item(row, PostThreadListModel::RecordTextPlainRole).toString(), "reply 3 - 2");
    QCOMPARE(model.item(row, PostThreadListModel::ThreadConnectorTopRole).toBool(), true);
    QCOMPARE(model.item(row, PostThreadListModel::ThreadConnectorBottomRole).toBool(), true);
    row = 3;
    QCOMPARE(model.item(row, PostThreadListModel::RecordTextPlainRole).toString(), "reply 4 - 2");
    QCOMPARE(model.item(row, PostThreadListModel::ThreadConnectorTopRole).toBool(), true);
    QCOMPARE(model.item(row, PostThreadListModel::ThreadConnectorBottomRole).toBool(), false);
    row = 4;
    QCOMPARE(model.item(row, PostThreadListModel::RecordTextPlainRole).toString(), "reply 3");
    QCOMPARE(model.item(row, PostThreadListModel::ThreadConnectorTopRole).toBool(), true);
    QCOMPARE(model.item(row, PostThreadListModel::ThreadConnectorBottomRole).toBool(), true);
    row = 5;
    QCOMPARE(model.item(row, PostThreadListModel::RecordTextPlainRole).toString(), "reply 4");
    QCOMPARE(model.item(row, PostThreadListModel::ThreadConnectorTopRole).toBool(), true);
    QCOMPARE(model.item(row, PostThreadListModel::ThreadConnectorBottomRole).toBool(), false);
}

void hagoromo_test::test_SystemTool_ImageClip()
{
    SystemTool tool;

    qDebug() << tool.clipImage(QUrl::fromLocalFile(":/data/images/sample.jpg"), 1700, 130, 400,
                               250);
}

void hagoromo_test::test_SearchProfileListModel_suggestion()
{
    QString uuid = AccountManager::getInstance()->updateAccount(
            QString(), m_service + "/search_profile", "id", "pass",
            "did:plc:mqxsuw5b5rhpwo4lw6iwlid5", "ioriayane.bsky.social", "email", "accessJwt",
            "refreshJwt", true);
    SearchProfileListModel model;
    model.setAccount(uuid);

    QString actual;

    actual = model.extractHandleBlock("");
    QCOMPARE(actual, "");

    actual = model.extractHandleBlock("\n\n");
    QCOMPARE(actual, "");

    actual = model.extractHandleBlock("hoge fuga @handle.bsky.social ");
    QCOMPARE(actual, "");

    actual = model.extractHandleBlock("hoge fuga @handle.bsky.social\t");
    QCOMPARE(actual, "");

    actual = model.extractHandleBlock("hoge fuga @handle.bsky.social\n");
    QCOMPARE(actual, "");

    actual = model.extractHandleBlock("hoge fuga @handle.bsky.social\n\nhoge");
    QCOMPARE(actual, "");

    actual = model.extractHandleBlock("hoge fuga @handle.bsky.social\n\nhoge@handle2");
    QCOMPARE(actual, "handle2");

    actual = model.extractHandleBlock("hoge fuga @handle.bsky.social\n\nhoge@han.dle2");
    QCOMPARE(actual, "han.dle2");

    actual = model.extractHandleBlock("hoge fuga @handle.bsky.social\n\nhoge@han.dle2[");
    QCOMPARE(actual, "");

    actual = model.extractHandleBlock("hoge fuga @handle.bsky.social hoge");
    QCOMPARE(actual, "");

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
    QCOMPARE(actual, "hoge @handle.bsky.social ");

    // 1. hoge @handl[] fuga
    // 2. handl
    // 3. handle.bsky.social
    // 4. hoge @handle.bsky.social  fuga
    actual = model.replaceText("hoge @handl fuga", 11, "handle.bsky.social");
    QCOMPARE(actual, "hoge @handle.bsky.social fuga");

    // 1. hoge @handl[]  fuga
    // 2. handl
    // 3. handle.bsky.social
    // 4. hoge @handle.bsky.social  fuga
    actual = model.replaceText("hoge @handl  fuga", 11, "handle.bsky.social");
    QCOMPARE(actual, "hoge @handle.bsky.social  fuga");

    // 1. hoge @handl[]\tfuga
    // 2. handl
    // 3. handle.bsky.social
    // 4. hoge @handle.bsky.social\tfuga
    actual = model.replaceText("hoge @handl\tfuga", 11, "handle.bsky.social");
    QCOMPARE(actual, "hoge @handle.bsky.social\tfuga");

    // 1. hoge @handl[]\nfuga
    // 2. handl
    // 3. handle.bsky.social
    // 4. hoge @handle.bsky.social\nfuga
    actual = model.replaceText("hoge @handl\nfuga", 11, "handle.bsky.social");
    QCOMPARE(actual, "hoge @handle.bsky.social\nfuga");

    // 1. hoge @handl[] fuga
    // 2. handl
    // 3. another.handle.bsky.social
    // 4. hoge @another.handle.bsky.social fuga
    actual = model.replaceText("hoge @handl fuga", 11, "another.handle.bsky.social");
    QCOMPARE(actual, "hoge @another.handle.bsky.social fuga");

    // 1. hoge @handl[]fuga
    // 2. handl
    // 3. handle.bsky.social
    // 4. hoge @handle.bsky.socialfuga
    actual = model.replaceText("hoge @handlfuga", 11, "handle.bsky.social");
    QCOMPARE(actual, "hoge @handle.bsky.social fuga");

    // 1.  hoge @handl[] fuga
    // 1'. hoge @hande[] fuga
    // 2. handl
    // 3. handle.bsky.social
    // 4. hoge @handle.bsky.social[] fuga
    actual = model.replaceText("hoge @hande fuga", 11, "handle.bsky.social");
    QCOMPARE(actual, "hoge @handle.bsky.social fuga");

    // 1. ho\nge @handl[] fuga
    // 2. handl
    // 3. handle.bsky.social
    // 4. ho\nge @handle.bsky.social fuga
    actual = model.replaceText("ho\nge @handl fuga", 12, "handle.bsky.social");
    QCOMPARE(actual, "ho\nge @handle.bsky.social fuga");

    // 1. ho\nge @handl[] f\nuga
    // 2. handl
    // 3. handle.bsky.social
    // 4. ho\nge @handle.bsky.social f\nuga
    actual = model.replaceText("ho\nge @handl f\nuga", 12, "handle.bsky.social");
    QCOMPARE(actual, "ho\nge @handle.bsky.social f\nuga");
}

void hagoromo_test::test_SearchPostListModel_text()
{
    QString uuid = AccountManager::getInstance()->updateAccount(
            QString(), m_service + "/search_profile", "id", "pass", "did:plc:hogehoge",
            "hogehoge.bsky.social", "email", "accessJwt", "refreshJwt", true);
    SearchPostListModel model;
    model.setAccount(uuid);

    QCOMPARE(model.replaceSearchCommand("from:me"), "from:hogehoge.bsky.social");

    QCOMPARE(model.replaceSearchCommand("fuga  from:me hoge"),
             "fuga  from:hogehoge.bsky.social hoge");

    QCOMPARE(model.replaceSearchCommand("fuga\tfrom:me\thoge"),
             "fuga from:hogehoge.bsky.social hoge");

    QCOMPARE(model.replaceSearchCommand(QString("fuga%1from:me%1hoge").arg(QChar(0x3000))),
             "fuga from:hogehoge.bsky.social hoge");
}

void hagoromo_test::test_ContentFilterSettingListModel()
{
    int i = 0;
    ContentFilterSettingListModel model;
    model.setService(m_service + "/content_filter/1");
    model.setHandle(QString());
    model.setAccessJwt("access_jwt");

    {
        QSignalSpy spy(&model, SIGNAL(runningChanged()));
        model.load();
        spy.wait(10 * 1000);
        QCOMPARE(spy.count(), 2);
    }

    // -----------------------------------------
    model.setLabelerDid(QString());
    QCOMPARE(model.rowCount(), 11);
    i = 0;
    QCOMPARE(model.item(i, ContentFilterSettingListModel::TitleRole).toString(), "Content hidden");
    QCOMPARE(model.item(i, ContentFilterSettingListModel::DescriptionRole).toString(),
             "Moderator overrides for special cases.");
    QCOMPARE(model.item(i, ContentFilterSettingListModel::StatusRole).toInt(),
             static_cast<int>(ConfigurableLabelStatus::Hide));
    QCOMPARE(model.item(i, ContentFilterSettingListModel::IsAdultImageryRole).toBool(), false);
    QCOMPARE(model.item(i, ContentFilterSettingListModel::ConfigurableRole).toBool(), false);
    //
    i = 10;
    QCOMPARE(model.item(i, ContentFilterSettingListModel::TitleRole).toString(),
             "Impersonation / Scam");
    //
    i = 11;
    QCOMPARE(model.item(i, ContentFilterSettingListModel::TitleRole).toString(), QString());

    model.setLabelerDid("did:plc:ar7c4by46qjdydhdevvrndac");
    QCOMPARE(model.rowCount(), 18);
    i = 0;
    QCOMPARE(model.item(i, ContentFilterSettingListModel::TitleRole).toString(),
             "Sexually Suggestive (Cartoon)");
    QCOMPARE(model.item(i, ContentFilterSettingListModel::DescriptionRole).toString(),
             "Art with explicit or suggestive sexual themes, including provocative "
             "imagery or partial nudity.");
    QCOMPARE(model.item(i, ContentFilterSettingListModel::StatusRole).toInt(),
             static_cast<int>(ConfigurableLabelStatus::Show));
    QCOMPARE(model.item(i, ContentFilterSettingListModel::IsAdultImageryRole).toBool(), true);
    QCOMPARE(model.item(i, ContentFilterSettingListModel::ConfigurableRole).toBool(), true);
    i = 11;
    QCOMPARE(model.item(i, ContentFilterSettingListModel::TitleRole).toString(), "Misinformation");
    QCOMPARE(model.item(i, ContentFilterSettingListModel::DescriptionRole).toString(),
             "Spreading false or misleading info, including unverified claims and "
             "harmful conspiracy theories.");
    QCOMPARE(model.item(i, ContentFilterSettingListModel::StatusRole).toInt(),
             static_cast<int>(ConfigurableLabelStatus::Warning));
    QCOMPARE(model.item(i, ContentFilterSettingListModel::IsAdultImageryRole).toBool(), false);
    QCOMPARE(model.item(i, ContentFilterSettingListModel::ConfigurableRole).toBool(), true);
    i = 17;
    QCOMPARE(model.item(i, ContentFilterSettingListModel::TitleRole).toString(),
             "Inauthentic Account");
    i = 18;
    QCOMPARE(model.item(i, ContentFilterSettingListModel::TitleRole).toString(), QString());

    // -----------------------------------------
    model.setLabelerDid("did:plc:original_labeler_did");
    QCOMPARE(model.rowCount(), 1);
    i = 0;
    QCOMPARE(model.item(i, ContentFilterSettingListModel::TitleRole).toString(), "Original");
    i = 1;
    QCOMPARE(model.item(i, ContentFilterSettingListModel::TitleRole).toString(), QString());

    model.setLabelerDid("did:plc:unknown");
    QCOMPARE(model.rowCount(), 0);
}

struct CalendarTableModelTestData
{
    CalendarTableModelTestData(int p_year, int p_month, int r, int c, int a_year, int a_month,
                               int e_year, int e_month, int e_day)
    {
        this->p_year = p_year;
        this->p_month = p_month;
        this->row = r;
        this->column = c;
        this->a_year = a_year;
        this->a_month = a_month;
        this->e_year = e_year;
        this->e_month = e_month;
        this->e_day = e_day;
    }
    int p_year;
    int p_month;
    int row;
    int column;
    int a_year;
    int a_month;
    int e_year;
    int e_month;
    int e_day;

    QString toString() const
    {
        return QString("%1/%2, r=%3, c=%4, Add(y=%5, m=%6), %7/%8/%9")
                .arg(this->p_year)
                .arg(this->p_month)
                .arg(this->row)
                .arg(this->column)
                .arg(this->a_year)
                .arg(this->a_month)
                .arg(this->e_year)
                .arg(this->e_month)
                .arg(this->e_day);
    }
};

void hagoromo_test::test_CalendarTableModel()
{
    CalendarTableModel model;

    int e_year, e_month, e_day;
    bool ok;
    QList<CalendarTableModelTestData> list;
    list.append(CalendarTableModelTestData(2024, 6, 0, 5, 0, 0, 2024, 5, 31));
    list.append(CalendarTableModelTestData(2024, 6, 0, 6, 0, 0, 2024, 6, 1));
    list.append(CalendarTableModelTestData(2024, 6, 5, 0, 0, 0, 2024, 6, 30));
    list.append(CalendarTableModelTestData(2024, 6, 5, 1, 0, 0, 2024, 7, 1));
    list.append(CalendarTableModelTestData(2024, 7, 0, 1, 0, 0, 2024, 7, 1));
    list.append(CalendarTableModelTestData(2024, 12, 4, 2, 0, 0, 2024, 12, 31));
    list.append(CalendarTableModelTestData(2024, 12, 4, 3, 0, 0, 2025, 1, 1));
    list.append(CalendarTableModelTestData(2025, 1, 0, 2, 0, 0, 2024, 12, 31));
    list.append(CalendarTableModelTestData(2025, 1, 0, 3, 0, 0, 2025, 1, 1));

    list.append(CalendarTableModelTestData(2023, 6, 0, 5, 1, 0, 2024, 5, 31));
    list.append(CalendarTableModelTestData(2023, 6, 0, 6, 1, 0, 2024, 6, 1));
    list.append(CalendarTableModelTestData(2023, 6, 5, 0, 1, 0, 2024, 6, 30));
    list.append(CalendarTableModelTestData(2023, 6, 5, 1, 1, 0, 2024, 7, 1));
    list.append(CalendarTableModelTestData(2023, 7, 0, 1, 1, 0, 2024, 7, 1));
    list.append(CalendarTableModelTestData(2023, 12, 4, 2, 1, 0, 2024, 12, 31));
    list.append(CalendarTableModelTestData(2023, 12, 4, 3, 1, 0, 2025, 1, 1));
    list.append(CalendarTableModelTestData(2024, 1, 0, 2, 1, 0, 2024, 12, 31));
    list.append(CalendarTableModelTestData(2024, 1, 0, 3, 1, 0, 2025, 1, 1));

    list.append(CalendarTableModelTestData(2024, 5, 0, 5, 0, 1, 2024, 5, 31));
    list.append(CalendarTableModelTestData(2024, 5, 0, 6, 0, 1, 2024, 6, 1));
    list.append(CalendarTableModelTestData(2024, 5, 5, 0, 0, 1, 2024, 6, 30));
    list.append(CalendarTableModelTestData(2024, 5, 5, 1, 0, 1, 2024, 7, 1));
    list.append(CalendarTableModelTestData(2024, 6, 0, 1, 0, 1, 2024, 7, 1));
    list.append(CalendarTableModelTestData(2024, 11, 4, 2, 0, 1, 2024, 12, 31));
    list.append(CalendarTableModelTestData(2024, 11, 4, 3, 0, 1, 2025, 1, 1));
    list.append(CalendarTableModelTestData(2024, 12, 0, 2, 0, 1, 2024, 12, 31));
    list.append(CalendarTableModelTestData(2024, 12, 0, 3, 0, 1, 2025, 1, 1));

    list.append(CalendarTableModelTestData(1, 1, 0, 0, 0, 0, -1, -1, -1));
    list.append(CalendarTableModelTestData(1, 1, 0, 1, 0, 0, 1, 1, 1));
    list.append(CalendarTableModelTestData(1, 2, 0, 4, 0, 0, 1, 2, 1));

    list.append(CalendarTableModelTestData(1, 2, 0, 4, -1, 0, 1, 2, 1));
    list.append(CalendarTableModelTestData(1, 2, 0, 4, 0, -2, 1, 1, 4));

    for (const auto &data : list) {
        model.setYear(data.p_year);
        model.setMonth(data.p_month);
        model.addYears(data.a_year);
        model.addMonths(data.a_month);
        e_year = model.item(data.row, data.column, CalendarTableModel::YearRole).toInt(&ok);
        if (!ok)
            e_year = -1;
        e_month = model.item(data.row, data.column, CalendarTableModel::MonthRole).toInt(&ok);
        if (!ok)
            e_month = -1;
        e_day = model.item(data.row, data.column, CalendarTableModel::DayRole).toInt(&ok);
        if (!ok)
            e_day = -1;
        QCOMPARE(e_year, data.e_year);
        QCOMPARE(e_month, data.e_month);
        QCOMPARE(e_day, data.e_day);
    }

    model.setYear(2024);
    model.setMonth(6);

    model.setSinceDate(2024, 6, 10);
    model.setUntilDate(2024, 6, 10);
    model.setEnableSince(true);
    model.setEnableUntil(false);
    QCOMPARE(model.item(2, 0, CalendarTableModel::EnabledRole).toBool(), false); // 6/9
    QCOMPARE(model.item(2, 1, CalendarTableModel::EnabledRole).toBool(), true); // 6/10

    model.setSinceDate(2024, 6, 10);
    model.setUntilDate(2024, 6, 10);
    model.setEnableSince(false);
    model.setEnableUntil(true);
    QCOMPARE(model.item(2, 1, CalendarTableModel::EnabledRole).toBool(), true); // 6/10
    QCOMPARE(model.item(2, 2, CalendarTableModel::EnabledRole).toBool(), false); // 6/11

    model.setEnableSince(true);
    model.setEnableUntil(true);
    model.setSinceDate(2024, 6, 1);
    model.setUntilDate(2024, 6, 10);
    model.setSinceDate(2024, 6, 11);
    QCOMPARE(model.since(), "2024/06/10");
    QCOMPARE(model.until(), "2024/06/10");

    model.setSinceDate(2024, 6, 1);
    model.setUntilDate(2024, 6, 10);
    model.setUntilDate(2024, 5, 31);
    QCOMPARE(model.since(), "2024/06/01");
    QCOMPARE(model.until(), "2024/06/01");

    model.setEnableSince(true);
    model.setEnableUntil(false);
    model.setSinceDate(2024, 6, 1);
    model.setUntilDate(2024, 6, 10);
    model.setSinceDate(2024, 6, 11);
    QCOMPARE(model.since(), "2024/06/11");
    QCOMPARE(model.until(), "2024/06/11");

    model.setEnableSince(false);
    model.setEnableUntil(true);
    model.setSinceDate(2024, 6, 1);
    model.setUntilDate(2024, 6, 10);
    model.setUntilDate(2024, 5, 31);
    QCOMPARE(model.since(), "2024/05/31");
    QCOMPARE(model.until(), "2024/05/31");

    model.setEnableSince(true);
    model.setEnableUntil(true);
    QCOMPARE(model.sinceUtc(), "2024-05-30T15:00:00Z");
    QCOMPARE(model.untilUtc(), "2024-05-31T14:59:59Z");
}

void hagoromo_test::test_TokimekiPollOperator_convertUrlToUri()
{
    TokimekiPollOperator operatorUnderTest;

    const QString validUrl =
            QStringLiteral("https://poll.tokimeki.tech/p/did:plc:mqxsuw5b5rhpwo4lw6iwlid5/"
                           "3mb6j6si7qc2u?options=2");
    const QString expectedUri = QStringLiteral(
            "at://did:plc:mqxsuw5b5rhpwo4lw6iwlid5/tech.tokimeki.poll.poll/3mb6j6si7qc2u");
    QCOMPARE(expectedUri, operatorUnderTest.convertUrlToUri(validUrl));

    const QStringList invalidUrls = {
        QStringLiteral("https://poll.tokimeki.tech/p/did:plc:mqxsuw5b5rhpwo4lw6iwlid5"),
        QStringLiteral("https://poll.tokimeki.tech/p/not-a-did/3mb6j6si7qc2u"),
        QStringLiteral(
                "https://poll.tokimeki.tech/x/did:plc:mqxsuw5b5rhpwo4lw6iwlid5/3mb6j6si7qc2u"),
        QStringLiteral("https://example.com/p/did:plc:mqxsuw5b5rhpwo4lw6iwlid5/3mb6j6si7qc2u"),
        QStringLiteral("not a url")
    };

    for (const QString &invalidUrl : invalidUrls) {
        QCOMPARE(operatorUnderTest.convertUrlToUri(invalidUrl), QString());
    }
}

void hagoromo_test::test_TokimekiPollOperator_getPoll()
{
    TokimekiPollOperator operatorUnderTest;
    operatorUnderTest.setServiceUrl(m_service + "/tokimeki/with_vote");

    QSignalSpy spy(&operatorUnderTest, &TokimekiPollOperator::finished);

    const QString cid = QStringLiteral("test-cid");
    const QString uri = QStringLiteral(
            "at://did:plc:mqxsuw5b5rhpwo4lw6iwlid5/tech.tokimeki.poll.poll/3mb6j6si7qc2u");
    const QString viewer = QStringLiteral("did:plc:viewerexample0000000000000000000");

    operatorUnderTest.getPoll(cid, uri, viewer);

    QVERIFY(spy.wait(10 * 1000));
    QCOMPARE(spy.count(), 1);

    const QList<QVariant> arguments = spy.takeFirst();
    QVERIFY(arguments.at(0).toBool());
    QCOMPARE(arguments.at(1).toString(), cid);

    const QVariant uriVariant = operatorUnderTest.item(uri, TokimekiPollOperator::PollUriRole);
    QVERIFY(uriVariant.isValid());
    QCOMPARE(
            uriVariant.toString(),
            QStringLiteral(
                    "at://did:plc:mqxsuw5b5rhpwo4lw6iwlid5/tech.tokimeki.poll.poll/3mb6j6si7qc2u"));

    const QVariant cidVariant = operatorUnderTest.item(uri, TokimekiPollOperator::PollCidRole);
    QVERIFY(cidVariant.isValid());
    QCOMPARE(cidVariant.toString(),
             QStringLiteral("bafyreibir6pwtrmmj6mczufitlbql7h77not66hacysw2cfr6wlaerltpe"));

    const QVariant optionsVariant =
            operatorUnderTest.item(uri, TokimekiPollOperator::PollOptionsRole);
    QVERIFY(optionsVariant.isValid());
    QCOMPARE(optionsVariant.toStringList(),
             QStringList() << "awake"
                           << "sleeping");
    qDebug() << optionsVariant.toStringList();

    const QVariant countOfOptionsVariant =
            operatorUnderTest.item(uri, TokimekiPollOperator::PollCountOfOptionsRole);
    QVERIFY(countOfOptionsVariant.isValid());
    QCOMPARE(countOfOptionsVariant.toStringList(),
             QStringList() << "1"
                           << "1");
    qDebug() << countOfOptionsVariant.toStringList();

    const QVariant indexOfOptionsVariant =
            operatorUnderTest.item(uri, TokimekiPollOperator::PollIndexOfOptionsRole);
    QVERIFY(indexOfOptionsVariant.isValid());
    QCOMPARE(indexOfOptionsVariant.toStringList(),
             QStringList() << "0"
                           << "1");

    const QVariant myVoteVariant =
            operatorUnderTest.item(uri, TokimekiPollOperator::PollMyVoteRole);
    QVERIFY(myVoteVariant.isValid());
    QCOMPARE(myVoteVariant.toString(), QStringLiteral("1"));

    const QVariant totalVotesVariant =
            operatorUnderTest.item(uri, TokimekiPollOperator::PollTotalVotesRole);
    QVERIFY(totalVotesVariant.isValid());
    QCOMPARE(totalVotesVariant.toInt(), 2);

    const QVariant isEndedVariant =
            operatorUnderTest.item(uri, TokimekiPollOperator::PollIsEndedRole);
    QVERIFY(isEndedVariant.isValid());
    QCOMPARE(isEndedVariant.toBool(), false);

    const QVariant remainTimeVariant =
            operatorUnderTest.item(uri, TokimekiPollOperator::PollRemainTimeRole);
    QVERIFY(remainTimeVariant.isValid());

    const double actualRemainHours = remainTimeVariant.toDouble();
    const QDateTime endsAt =
            QDateTime::fromString(QStringLiteral("2025-12-31T04:28:13.299Z"), Qt::ISODateWithMs);
    QVERIFY(endsAt.isValid());
    const double expectedRemainHours =
            static_cast<double>(QDateTime::currentDateTimeUtc().msecsTo(endsAt))
            / (1000.0 * 60.0 * 60.0);
    const double toleranceHours = 0.01; // 約36秒
    QCOMPARE_LT(qAbs(actualRemainHours - expectedRemainHours), toleranceHours);
}

void hagoromo_test::test_TokimekiPollOperator_getPoll_noVote()
{
    TokimekiPollOperator operatorUnderTest;
    operatorUnderTest.setServiceUrl(m_service + "/tokimeki/no_vote");

    QSignalSpy spy(&operatorUnderTest, &TokimekiPollOperator::finished);

    const QString cid = QStringLiteral("test-cid-novote");
    const QString uri = QStringLiteral(
            "at://did:plc:mqxsuw5b5rhpwo4lw6iwlid5/tech.tokimeki.poll.poll/3mb6j6si7qc2u");
    const QString viewer = QStringLiteral("did:plc:viewerexample-no-vote000000000000");

    operatorUnderTest.getPoll(cid, uri, viewer);

    QVERIFY(spy.wait(10 * 1000));
    QCOMPARE(spy.count(), 1);

    const QList<QVariant> arguments = spy.takeFirst();
    QVERIFY(arguments.at(0).toBool());
    QCOMPARE(arguments.at(1).toString(), cid);

    const QVariant myVoteVariant =
            operatorUnderTest.item(uri, TokimekiPollOperator::PollMyVoteRole);
    QVERIFY(myVoteVariant.isValid());
    QCOMPARE(myVoteVariant.toString(), QStringLiteral("-1"));

    const QVariant optionsVariant =
            operatorUnderTest.item(uri, TokimekiPollOperator::PollOptionsRole);
    QVERIFY(optionsVariant.isValid());
    QCOMPARE(optionsVariant.toStringList(),
             QStringList() << "sleeping"
                           << "awake");

    const QVariant indexOfOptionsVariant =
            operatorUnderTest.item(uri, TokimekiPollOperator::PollIndexOfOptionsRole);
    QVERIFY(indexOfOptionsVariant.isValid());
    QCOMPARE(indexOfOptionsVariant.toStringList(),
             QStringList() << "1"
                           << "0");

    const QVariant totalVotesVariant =
            operatorUnderTest.item(uri, TokimekiPollOperator::PollTotalVotesRole);
    QVERIFY(totalVotesVariant.isValid());
    QCOMPARE(totalVotesVariant.toInt(), 2);
}

void hagoromo_test::test_TokimekiPollOperator_getPoll_noHit()
{
    TokimekiPollOperator operatorUnderTest;

    const QString unknownUri =
            QStringLiteral("at://did:plc:unknown/tech.tokimeki.poll.poll/not-exists");

    const QVariant optionsVariant =
            operatorUnderTest.item(unknownUri, TokimekiPollOperator::PollOptionsRole);
    QVERIFY(optionsVariant.isValid());
    QCOMPARE(optionsVariant.toStringList(), QStringList());

    const QVariant countVariant =
            operatorUnderTest.item(unknownUri, TokimekiPollOperator::PollCountOfOptionsRole);
    QVERIFY(countVariant.isValid());
    QCOMPARE(countVariant.toStringList(), QStringList());

    const QVariant indexVariant =
            operatorUnderTest.item(unknownUri, TokimekiPollOperator::PollIndexOfOptionsRole);
    QVERIFY(indexVariant.isValid());
    QCOMPARE(indexVariant.toStringList(), QStringList());

    const QVariant myVoteVariant =
            operatorUnderTest.item(unknownUri, TokimekiPollOperator::PollMyVoteRole);
    QVERIFY(myVoteVariant.isValid());
    QCOMPARE(myVoteVariant.toString(), QStringLiteral("-1"));

    const QVariant totalVotesVariant =
            operatorUnderTest.item(unknownUri, TokimekiPollOperator::PollTotalVotesRole);
    QVERIFY(totalVotesVariant.isValid());
    QCOMPARE(totalVotesVariant.toInt(), 0);

    const QVariant isEndedVariant =
            operatorUnderTest.item(unknownUri, TokimekiPollOperator::PollIsEndedRole);
    QVERIFY(isEndedVariant.isValid());
    QCOMPARE(isEndedVariant.toBool(), true);

    const QVariant remainTimeVariant =
            operatorUnderTest.item(unknownUri, TokimekiPollOperator::PollRemainTimeRole);
    QVERIFY(remainTimeVariant.isValid());
    QCOMPARE(remainTimeVariant.toDouble(), 0.0);

    // uri が空の場合も同じ固定値を返す
    const QVariant emptyUriMyVote =
            operatorUnderTest.item(QString(), TokimekiPollOperator::PollMyVoteRole);
    QVERIFY(emptyUriMyVote.isValid());
    QCOMPARE(emptyUriMyVote.toString(), QStringLiteral("-1"));
}

void hagoromo_test::test_TokimekiPollOperator_makePollOgpFile()
{

    TokimekiPollOperator ope;

    qDebug() << ope.makePollOgpFile(QStringList() << "item1"
                                                  << "item2"
                                                  << "item3"
                                                  << "item4");
    qDebug() << ope.makePollOgpFile(QStringList() << "item1"
                                                  << "item2"
                                                  << "item3");
    qDebug() << ope.makePollOgpFile(QStringList() << "item1"
                                                  << "item2");
    qDebug() << ope.makePollOgpFile(QStringList() << "item1");
}

void hagoromo_test::test_TokimekiPollOperator_makeAltUrl()
{
    TokimekiPollOperator ope;
    qDebug() << ope.makeAltUrl("did:plc:hogehoge", Tid::next());
}

void hagoromo_test::test_TokimekiPollOperator_getOgp()
{
    // TokimekiPollOperator operatorUnderTest;

    // QSignalSpy spy(&operatorUnderTest, &TokimekiPollOperator::finished);

    // const QStringList options = QStringList() << "item1"
    //                                           << "item2";

    // operatorUnderTest.getOgp(options);

    // QVERIFY(spy.wait(10 * 1000));
    // QCOMPARE(spy.count(), 1);

    // const QList<QVariant> arguments = spy.takeFirst();
    // QVERIFY(arguments.at(0).toBool());

    // qDebug() << arguments.at(1).toString();
}

void hagoromo_test::test_DraftListModel()
{
    QString uuid = AccountManager::getInstance()->updateAccount(
            QString(), m_service + "/draft", "id", "pass", "did:plc:mqxsuw5b5rhpwo4lw6iwlid5",
            "ioriayane.bsky.social", "email", "accessJwt", "refreshJwt", true);

    DraftListModel model;
    model.setAccount(uuid);

    QSignalSpy spy(&model, SIGNAL(runningChanged()));
    model.getLatest();
    spy.wait(10 * 1000);
    QCOMPARE(spy.count(), 2);

    // 5 drafts should be loaded
    QCOMPARE(model.rowCount(), 5);

    // Test first draft (id: 3mf5aryvgm224) - has postgate disable rule
    QCOMPARE(model.item(0, DraftListModel::IdRole).toString(), QStringLiteral("3mf5aryvgm224"));
    QCOMPARE(model.item(0, DraftListModel::PrimaryTextRole).toString(),
             QStringLiteral("quote_disabled"));
    QVERIFY(!model.item(0, DraftListModel::CreatedAtRole).toString().isEmpty());
    QVERIFY(!model.item(0, DraftListModel::UpdatedAtRole).toString().isEmpty());
    QCOMPARE(model.item(0, DraftListModel::PostgateEmbeddingRulesRole).toBool(),
             false); // quote disabled
    QCOMPARE(model.item(0, DraftListModel::ThreadGateTypeRole).toString(),
             QStringLiteral("choice"));
    QStringList rules0 = model.item(0, DraftListModel::ThreadGateRulesRole).toStringList();
    QCOMPARE(rules0.count(), 1);
    QCOMPARE(rules0.at(0), QStringLiteral("follower"));
    QCOMPARE(model.item(0, DraftListModel::PostCountRole).toInt(), 1);
    QCOMPARE(model.item(0, DraftListModel::IsThreadRole).toBool(), false);

    // Test second draft (id: 3mf2uylv5vk2r) - has langs
    QCOMPARE(model.item(1, DraftListModel::IdRole).toString(), QStringLiteral("3mf2uylv5vk2r"));
    QCOMPARE(model.item(1, DraftListModel::PrimaryTextRole).toString(), QStringLiteral("stgk"));
    QStringList langs1 = model.item(1, DraftListModel::LangsRole).toStringList();
    QCOMPARE(langs1.count(), 1);
    QCOMPARE(langs1.at(0), QStringLiteral("ja"));
    QCOMPARE(model.item(1, DraftListModel::PostgateEmbeddingRulesRole).toBool(),
             true); // quote enabled
    QCOMPARE(model.item(1, DraftListModel::ThreadGateTypeRole).toString(),
             QStringLiteral("choice"));
    QCOMPARE(model.item(1, DraftListModel::PostCountRole).toInt(), 1);
    QCOMPARE(model.item(1, DraftListModel::IsThreadRole).toBool(), false);

    // Test third draft (id: 3mf2t7mlnb223) - has images and labels
    QCOMPARE(model.item(2, DraftListModel::IdRole).toString(), QStringLiteral("3mf2t7mlnb223"));
    QCOMPARE(model.item(2, DraftListModel::PrimaryTextRole).toString(),
             QStringLiteral("label_test"));
    QStringList labels2 = model.item(2, DraftListModel::PrimaryLabelsRole).toStringList();
    QCOMPARE(labels2.count(), 1);
    QCOMPARE(labels2.at(0), QStringLiteral("sexual"));
    QStringList imagePaths2 =
            model.item(2, DraftListModel::PrimaryEmbedImagesPathsRole).toStringList();
    QCOMPARE(imagePaths2.count(), 1);
    QCOMPARE(imagePaths2.at(0), QStringLiteral("image:e-jhhgSLUDGgpVuAlenJl"));
    QStringList imageAlts2 =
            model.item(2, DraftListModel::PrimaryEmbedImagesAltsRole).toStringList();
    QCOMPARE(imageAlts2.count(), 1);
    QCOMPARE(imageAlts2.at(0), QString()); // No alt text
    QCOMPARE(model.item(2, DraftListModel::PostCountRole).toInt(), 1);
    QCOMPARE(model.item(2, DraftListModel::IsThreadRole).toBool(), false);

    // Test fourth draft (id: 3mellc6ahps2o) - multiple posts
    QCOMPARE(model.item(3, DraftListModel::IdRole).toString(), QStringLiteral("3mellc6ahps2o"));
    QCOMPARE(model.item(3, DraftListModel::PrimaryTextRole).toString(), QStringLiteral("test1"));
    // Multiple posts - only primary (first) post data is accessible via roles
    QStringList imagePaths3 =
            model.item(3, DraftListModel::PrimaryEmbedImagesPathsRole).toStringList();
    QCOMPARE(imagePaths3.count(), 1);
    QCOMPARE(imagePaths3.at(0), QStringLiteral("image:zQsiEwV7UciTu0fkIL6Po"));
    QCOMPARE(model.item(3, DraftListModel::PostCountRole).toInt(), 3);
    QCOMPARE(model.item(3, DraftListModel::IsThreadRole).toBool(), true);

    // Test fifth draft (id: 3mdl6vx6l2k2z) - has external link and multiple images
    QCOMPARE(model.item(4, DraftListModel::IdRole).toString(), QStringLiteral("3mdl6vx6l2k2z"));
    QCOMPARE(model.item(4, DraftListModel::PrimaryTextRole).toString(),
             QStringLiteral("draft_test"));
    QStringList imagePaths4 =
            model.item(4, DraftListModel::PrimaryEmbedImagesPathsRole).toStringList();
    QCOMPARE(imagePaths4.count(), 2);
    QCOMPARE(imagePaths4.at(0), QStringLiteral("file:///D:/image1.JPG"));
    QCOMPARE(imagePaths4.at(1), QStringLiteral("file:///D:/image2.JPG"));
    QStringList imageAlts4 =
            model.item(4, DraftListModel::PrimaryEmbedImagesAltsRole).toStringList();
    QCOMPARE(imageAlts4.count(), 2);
    QCOMPARE(imageAlts4.at(0), QStringLiteral("alt1"));
    QCOMPARE(imageAlts4.at(1), QStringLiteral("alt2"));
    QStringList externals4 =
            model.item(4, DraftListModel::PrimaryEmbedExternalsRole).toStringList();
    QCOMPARE(externals4.count(), 1);
    QCOMPARE(externals4.at(0), QStringLiteral("https://github.com/ioriayane/Hagoromo"));
    QCOMPARE(model.item(4, DraftListModel::PostCountRole).toInt(), 1);
    QCOMPARE(model.item(4, DraftListModel::IsThreadRole).toBool(), false);

    // Test indexOf
    QCOMPARE(model.indexOf("3mf5aryvgm224"), 0);
    QCOMPARE(model.indexOf("3mellc6ahps2o"), 3);
    QCOMPARE(model.indexOf("nonexistent"), -1);

    // Test getRecordText
    QCOMPARE(model.getRecordText("3mf5aryvgm224"), QStringLiteral("quote_disabled"));
    QCOMPARE(model.getRecordText("3mf2uylv5vk2r"), QStringLiteral("stgk"));
    QCOMPARE(model.getRecordText("nonexistent"), QString());

    // Test ModelData role returns QVariant
    QVariant modelData = model.item(0, DraftListModel::ModelData);
    QVERIFY(!modelData.isValid());
}

QTEST_MAIN(hagoromo_test)

#include "tst_hagoromo_test.moc"
