#include <QtTest>
#include <QCoreApplication>
#include <QJsonDocument>
#include <QDir>
#include <QFile>

#include "webserver.h"
#include "unittest_common.h"
#include "timeline/timelinelistmodel.h"
#include "operation/recordoperator.h"
#include "operation/draftoperator.h"
#include "feedgenerator/feedgeneratorlistmodel.h"
#include "link/feedgeneratorlink.h"
#include "account/accountlistmodel.h"
#include "common.h"
#include "list/listslistmodel.h"
#include "list/listitemlistmodel.h"
#include "list/listfeedlistmodel.h"
#include "notification/notificationpreferencelistmodel.h"
#include "tools/accountmanager.h"

class hagoromo_test : public QObject
{
    Q_OBJECT

public:
    hagoromo_test();
    ~hagoromo_test();

private slots:
    void initTestCase();
    void cleanupTestCase();
    void test_RecordOperator();
    void test_RecordOperator_profile();
    void test_FeedGeneratorListModel();
    void test_FeedGeneratorLink();
    void test_AccountListModel();
    void test_AccountManager();
    void test_ListsListModel();
    void test_ListsListModel_search();
    void test_ListsListModel_error();
    void test_ListItemListModel();
    void test_ListItemListModel_error();
    void test_ListFeedListModel();
    void test_NotificationPreferenceListModel();
    void test_NotificationPreferenceListModel_save();

    void test_TokimekiPollOperator_getPoll_vote();

    void test_DraftOperator_createDraft();
    void test_DraftOperator_updateDraft();
    void test_DraftOperator_deleteDraft();
    void test_DraftOperator_getDrafts();

private:
    WebServer m_mockServer;
    quint16 m_listenPort;
    QString m_service;

    void test_RecordOperatorCreateRecord(const QByteArray &body);
    void test_putPreferences(const QString &path, const QByteArray &body);
    void test_putRecord(const QString &path, const QByteArray &body);
    void test_putNotificationPreferences(const QString &path, const QByteArray &body);
    QJsonObject copyObject(const QJsonObject &object, const QStringList &excludes);
};

hagoromo_test::hagoromo_test()
{
    QCoreApplication::setOrganizationName(QStringLiteral("relog"));
    QCoreApplication::setApplicationName(QStringLiteral("Hagoromo_unittest"));

    m_listenPort = m_mockServer.listen(QHostAddress::LocalHost, 0);
    m_service = QString("http://localhost:%1/response").arg(m_listenPort);

    connect(&m_mockServer, &WebServer::receivedPost,
            [=](const QHttpServerRequest &request, bool &result, QString &json) {
                if (request.url().path() == "/response/facet/xrpc/com.atproto.repo.createRecord") {
                    test_RecordOperatorCreateRecord(request.body());
                    json = "{\"cid\":\"CID\",\"uri\":\"URI\"}";
                    result = true;
                } else if (request.url().path()
                           == "/response/tokimeki/xrpc/com.atproto.repo.createRecord") {
                    test_RecordOperatorCreateRecord(request.body());
                    json = "{\"cid\":\"CID\",\"uri\":\"at://URI\"}";
                    result = true;
                } else if (request.url().path()
                                   == "/response/generator/save/xrpc/app.bsky.actor.putPreferences"
                           || request.url().path()
                                   == "/response/generator/remove/xrpc/"
                                      "app.bsky.actor.putPreferences") {
                    test_putPreferences(request.url().path(), request.body());
                    json = "{}";
                    result = true;
                } else if (request.url().path().endsWith("/com.atproto.repo.putRecord")) {
                    test_putRecord(request.url().path(), request.body());
                    json = "{}";
                    result = true;
                } else if (request.url().path().endsWith(
                                   "/xrpc/app.bsky.notification.putPreferencesV2")) {
                    test_putNotificationPreferences(request.url().path(), request.body());
                    json = "{}";
                    result = true;
                } else if (request.url().path().endsWith("/xrpc/app.bsky.draft.createDraft")
                           || request.url().path().endsWith("/xrpc/app.bsky.draft.updateDraft")
                           || request.url().path().endsWith("/xrpc/app.bsky.draft.deleteDraft")) {
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
    QCOMPARE_NE(m_listenPort, 0);
}

void hagoromo_test::cleanupTestCase() { }

void hagoromo_test::test_RecordOperator()
{
    QString uuid = AccountManager::getInstance()->updateAccount(
            QString(), m_service + "/facet", "id", "pass", "did:plc:hogehoge",
            "hogehoge.bsky.social", "email", "accessJwt", "refreshJwt", true);

    RecordOperator ope;
    ope.setAccount(uuid);
    QHash<QString, QString> hash =
            UnitTestCommon::loadPostHash(":/data/com.atproto.repo.createRecord_post.expect");

    QHashIterator<QString, QString> i(hash);
    while (i.hasNext()) {
        i.next();

        QString uuid = AccountManager::getInstance()->updateAccount(
                QString(), m_service + "/facet", "id", "pass", i.key(), "handle", "email",
                "accessJwt", "refreshJwt", true);

        ope.clear();
        ope.setAccount(uuid);
        ope.setText(i.value());

        QSignalSpy spy(&ope, SIGNAL(finished(bool, const QString &, const QString &)));
        ope.post();
        spy.wait();
        QCOMPARE(spy.count(), 1);

        QList<QVariant> arguments = spy.takeFirst();
        QCOMPARE(arguments.at(0).typeId(), QMetaType::Bool);
        QCOMPARE(arguments.at(0).toBool(), true);
    }
}

void hagoromo_test::test_RecordOperator_profile()
{
    QString uuid = AccountManager::getInstance()->updateAccount(
            QString(), m_service + "/profile/1", "id", "pass", "did:plc:ipj5qejfoqu6eukvt72uhyit",
            "hogehoge.bsky.social", "email", "accessJwt", "refreshJwt", true);

    RecordOperator ope;
    ope.setAccount(uuid);
    {
        QSignalSpy spy(&ope, SIGNAL(finished(bool, const QString &, const QString &)));
        ope.updateProfile("", "", "description", "display_name", "", "");
        spy.wait();
        QCOMPARE(spy.count(), 1);

        QList<QVariant> arguments = spy.takeFirst();
        QCOMPARE(arguments.at(0).typeId(), QMetaType::Bool);
        QCOMPARE(arguments.at(0).toBool(), true);
    }

    uuid = AccountManager::getInstance()->updateAccount(
            QString(), m_service + "/profile/3.1", "id", "pass", "did:plc:ipj5qejfoqu6eukvt72uhyit",
            "hogehoge.bsky.social", "email", "accessJwt", "refreshJwt", true);
    ope.setAccount(uuid);
    {
        QSignalSpy spy(&ope, SIGNAL(finished(bool, const QString &, const QString &)));
        ope.updatePostPinning(
                "at://did:plc:ipj5qejfoqu6eukvt72uhyit/app.bsky.feed.post/3k5ml2mujje2n",
                "cid_after");
        spy.wait();
        QCOMPARE(spy.count(), 1);

        QList<QVariant> arguments = spy.takeFirst();
        QCOMPARE(arguments.at(0).typeId(), QMetaType::Bool);
        QCOMPARE(arguments.at(0).toBool(), true);
    }

    uuid = AccountManager::getInstance()->updateAccount(
            QString(), m_service + "/profile/3.2", "id", "pass", "did:plc:ipj5qejfoqu6eukvt72uhyit",
            "hogehoge.bsky.social", "email", "accessJwt", "refreshJwt", true);
    ope.setAccount(uuid);
    {
        QSignalSpy spy(&ope, SIGNAL(finished(bool, const QString &, const QString &)));
        ope.updatePostPinning(QString(), QString());
        spy.wait();
        QCOMPARE(spy.count(), 1);

        QList<QVariant> arguments = spy.takeFirst();
        QCOMPARE(arguments.at(0).typeId(), QMetaType::Bool);
        QCOMPARE(arguments.at(0).toBool(), true);
    }
}

void hagoromo_test::test_FeedGeneratorListModel()
{
    FeedGeneratorListModel model;
    model.setDisplayInterval(0);
    {
        QString uuid = AccountManager::getInstance()->updateAccount(
                QString(), m_service + "/generator", "id", "pass",
                "did:plc:ipj5qejfoqu6eukvt72uhyit", "hogehoge.bsky.social", "email", "accessJwt",
                "refreshJwt", true);
        model.setAccount(uuid);
        QSignalSpy spy(&model, SIGNAL(runningChanged()));
        model.getLatest();
        spy.wait();
        QCOMPARE(spy.count(), 2);

        QCOMPARE(model.rowCount(), 8);
        QCOMPARE(model.item(0, FeedGeneratorListModel::SavingRole), true);
        QVERIFY(model.item(1, FeedGeneratorListModel::DisplayNameRole)
                == QStringLiteral("What's Hot"));
        QCOMPARE(model.item(1, FeedGeneratorListModel::SavingRole), true);
        QCOMPARE(model.item(2, FeedGeneratorListModel::SavingRole), false);
        QCOMPARE(model.item(3, FeedGeneratorListModel::SavingRole), true);
        QCOMPARE(model.item(4, FeedGeneratorListModel::SavingRole), false);
        QCOMPARE(model.item(5, FeedGeneratorListModel::SavingRole), false);
        QCOMPARE(model.item(6, FeedGeneratorListModel::SavingRole), false);
        QVERIFY(model.item(7, FeedGeneratorListModel::DisplayNameRole)
                == QStringLiteral("mostpop"));
        QCOMPARE(model.item(7, FeedGeneratorListModel::SavingRole), false);
    }
    {
        // save
        QString uuid = AccountManager::getInstance()->updateAccount(
                QString(), m_service + "/generator/save", "id", "pass",
                "did:plc:ipj5qejfoqu6eukvt72uhyit", "hogehoge.bsky.social", "email", "accessJwt",
                "refreshJwt", true);
        model.setAccount(uuid);
        QSignalSpy spy(&model, SIGNAL(runningChanged()));
        model.saveGenerator(
                "at://did:plc:z72i7hdynmk6r22z27h6tvur/app.bsky.feed.generator/hot-classic");
        spy.wait(10 * 1000);
        QCOMPARE(spy.count(), 2);
    }
    {
        // remove
        QString uuid = AccountManager::getInstance()->updateAccount(
                QString(), m_service + "/generator/remove", "id", "pass",
                "did:plc:ipj5qejfoqu6eukvt72uhyit", "hogehoge.bsky.social", "email", "accessJwt",
                "refreshJwt", true);
        model.setAccount(uuid);
        QSignalSpy spy(&model, SIGNAL(runningChanged()));
        model.removeGenerator(
                "at://did:plc:z72i7hdynmk6r22z27h6tvur/app.bsky.feed.generator/whats-hot");
        spy.wait(10 * 1000);
        QCOMPARE(spy.count(), 2);
    }
}

void hagoromo_test::test_FeedGeneratorLink()
{
    QString uuid = AccountManager::getInstance()->updateAccount(
            QString(), m_service + "/generator", "id", "pass", "did:plc:ipj5qejfoqu6eukvt72uhyit",
            "hogehoge.bsky.social", "email", "accessJwt", "refreshJwt", true);

    FeedGeneratorLink link;

    QCOMPARE(link.checkUri("https://bsky.app/profile/did:plc:hoge/feed/aaaaaaaa", "feed"), true);
    QVERIFY(link.checkUri("https://staging.bsky.app/profile/did:plc:hoge/feed/aaaaaaaa", "feed")
            == false);
    QCOMPARE(link.checkUri("https://bsky.app/feeds/did:plc:hoge/feed/aaaaaaaa", "feed"), false);
    QCOMPARE(link.checkUri("https://bsky.app/profile/did:plc:hoge/feeds/aaaaaaaa", "feed"), false);
    QCOMPARE(link.checkUri("https://bsky.app/profile/did:plc:hoge/feed/", "feed"), false);
    QCOMPARE(link.checkUri("https://bsky.app/profile/handle/feed/aaaaaaaa", "feed"), false);
    QCOMPARE(link.checkUri("https://bsky.app/profile/handle.com/feed/aaaaaaaa", "feed"), true);
    QCOMPARE(link.checkUri("https://bsky.app/profile/@handle.com/feed/aaaaaaaa", "feed"), false);

    link.setAccount(uuid);
    {
        QSignalSpy spy(&link, SIGNAL(runningChanged()));
        link.getFeedGenerator("https://bsky.app/profile/did:plc:hoge/feed/aaaaaaaa");
        spy.wait();
        QCOMPARE(spy.count(), 2);
    }

    QCOMPARE(link.avatar(), "https://cdn.bsky.social/view_avator.jpeg");
    QCOMPARE(link.displayName(), "view:displayName");
    QCOMPARE(link.creatorHandle(), "creator.bsky.social");
    QCOMPARE(link.likeCount(), 9);
}

void hagoromo_test::test_AccountListModel()
{
    AccountManager::getInstance()->clear();

    QString temp_path = Common::appDataFolder() + "/account.json";
    if (QFile::exists(temp_path)) {
        QFile::remove(temp_path);
    }

    AccountListModel model;
    QString uuid1, uuid2;
    QString s_endpoint1, s_endpoint2;

    uuid1 = model.updateAccount(m_service + "/account/account1", "id1", "password1",
                                "did:plc:account1", "account1.relog.tech", "account1@relog.tech",
                                "accessJwt_account1", "refreshJwt_account1", true);
    uuid2 = model.updateAccount(m_service + "/account/account2", "id2", "password2",
                                "did:plc:account2", "account2.relog.tech", "account2@relog.tech",
                                "accessJwt_account2", "refreshJwt_account2", true);

    AccountListModel model2;
    {
        QSignalSpy spy(&model2, SIGNAL(finished()));
        model2.load();
        spy.wait(15 * 1000);
        QCOMPARE(spy.count(), 1);
    }
    QCOMPARE(model2.rowCount(), 2);
    int row = 0;
    QCOMPARE(model2.item(row, AccountListModel::DidRole).toString(), "did:plc:account1_refresh");
    QCOMPARE(model2.item(row, AccountListModel::RefreshJwtRole).toString(),
             "refreshJwt_account1_refresh");
    row = 1;
    QCOMPARE(model2.item(row, AccountListModel::DidRole).toString(), "did:plc:account2_refresh");
    QCOMPARE(model2.item(row, AccountListModel::RefreshJwtRole).toString(),
             "refreshJwt_account2_refresh");

    // model.update(0, AccountListModel::UuidRole, "UuidRole");
    // model.update(0, AccountListModel::IsMainRole, "IsMainRole");
    model.update(0, AccountListModel::ServiceRole, "ServiceRole");
    model.update(0, AccountListModel::ServiceEndpointRole, "ServiceEndpointRole");
    model.update(0, AccountListModel::IdentifierRole, "IdentifierRole");
    model.update(0, AccountListModel::PasswordRole, "PasswordRole");
    model.update(0, AccountListModel::DidRole, "DidRole");
    model.update(0, AccountListModel::HandleRole, "HandleRole");
    model.update(0, AccountListModel::EmailRole, "EmailRole");
    model.update(0, AccountListModel::AccessJwtRole, "AccessJwtRole");
    model.update(0, AccountListModel::RefreshJwtRole, "RefreshJwtRole");
    model.update(0, AccountListModel::DisplayNameRole, "DisplayNameRole");
    model.update(0, AccountListModel::DescriptionRole, "DescriptionRole");
    model.update(0, AccountListModel::AvatarRole, "AvatarRole");
    model.update(0, AccountListModel::PostLanguagesRole,
                 QStringList() << "PostLanguagesRole1"
                               << "PostLanguagesRole2");
    model.update(0, AccountListModel::ThreadGateTypeRole, "ThreadGateTypeRole");
    model.update(0, AccountListModel::ThreadGateOptionsRole,
                 QStringList() << "ThreadGateOptionsRole1"
                               << "ThreadGateOptionsRole2");
    model.update(0, AccountListModel::PostGateQuoteEnabledRole, true);

    // QCOMPARE(model.item(0, AccountListModel::UuidRole).toString(), "UuidRole");
    // QCOMPARE(model.item(0, AccountListModel::IsMainRole).toString(), "IsMainRole");
    QCOMPARE(model.item(0, AccountListModel::ServiceRole).toString(), "ServiceRole");
    QCOMPARE(model.item(0, AccountListModel::ServiceEndpointRole).toString(),
             "ServiceEndpointRole");
    QCOMPARE(model.item(0, AccountListModel::IdentifierRole).toString(), "IdentifierRole");
    QCOMPARE(model.item(0, AccountListModel::PasswordRole).toString(), "PasswordRole");
    QCOMPARE(model.item(0, AccountListModel::DidRole).toString(), "DidRole");
    QCOMPARE(model.item(0, AccountListModel::HandleRole).toString(), "HandleRole");
    QCOMPARE(model.item(0, AccountListModel::EmailRole).toString(), "EmailRole");
    QCOMPARE(model.item(0, AccountListModel::AccessJwtRole).toString(), "AccessJwtRole");
    QCOMPARE(model.item(0, AccountListModel::RefreshJwtRole).toString(), "RefreshJwtRole");
    QCOMPARE(model.item(0, AccountListModel::DisplayNameRole).toString(), "DisplayNameRole");
    QCOMPARE(model.item(0, AccountListModel::DescriptionRole).toString(), "DescriptionRole");
    QCOMPARE(model.item(0, AccountListModel::AvatarRole).toString(), "AvatarRole");
    QCOMPARE(model.item(0, AccountListModel::PostLanguagesRole).toStringList(),
             QStringList() << "PostLanguagesRole1"
                           << "PostLanguagesRole2");
    QCOMPARE(model.item(0, AccountListModel::ThreadGateTypeRole).toString(), "ThreadGateTypeRole");
    QCOMPARE(model.item(0, AccountListModel::ThreadGateOptionsRole).toStringList(),
             QStringList() << "ThreadGateOptionsRole1"
                           << "ThreadGateOptionsRole2");
    QCOMPARE(model.item(0, AccountListModel::PostGateQuoteEnabledRole).toBool(), true);
}

void hagoromo_test::test_AccountManager()
{

    QString temp_path = Common::appDataFolder() + "/account.json";
    if (QFile::exists(temp_path)) {
        QFile::remove(temp_path);
    }

    AccountManager *manager = AccountManager::getInstance();
    AtProtocolInterface::AccountData account;

    manager->clear();
    manager->updateAccount(QString(), m_service + "/account/account1", "id1", "password1",
                           "did:plc:account1", "account1.relog.tech", "account1@relog.tech",
                           "accessJwt_account1", "refreshJwt_account1", false);
    manager->updateAccount(QString(), m_service + "/account/account2", "id2", "password2",
                           "did:plc:account2", "account2.relog.tech", "account2@relog.tech",
                           "accessJwt_account2", "refreshJwt_account2", false);

    QStringList uuids = manager->getUuids();

    QCOMPARE(uuids.count(), 2);

    account = manager->getAccount(uuids.at(0));
    QCOMPARE(account.service, m_service + "/account/account1");
    QCOMPARE(account.password, "password1");
    QCOMPARE(account.did, "did:plc:account1");
    QCOMPARE(account.handle, "account1.relog.tech");
    QCOMPARE(account.email, "account1@relog.tech");
    QCOMPARE(account.accessJwt, "accessJwt_account1");
    QCOMPARE(account.refreshJwt, "refreshJwt_account1");

    account = manager->getAccount(uuids.at(1));
    QCOMPARE(account.service, m_service + "/account/account2");
    QCOMPARE(account.password, "password2");
    QCOMPARE(account.did, "did:plc:account2");
    QCOMPARE(account.handle, "account2.relog.tech");
    QCOMPARE(account.email, "account2@relog.tech");
    QCOMPARE(account.accessJwt, "accessJwt_account2");
    QCOMPARE(account.refreshJwt, "refreshJwt_account2");

    QCOMPARE(manager->checkAllAccountsReady(), false);

    manager->save();

    manager->removeAccount(manager->getUuids().at(0));
    QCOMPARE(manager->getUuids().count(), 1);

    account = manager->getAccount(manager->getUuids().at(0));
    QCOMPARE(account.service, m_service + "/account/account2");
    QCOMPARE(account.password, "password2");
    QCOMPARE(account.did, "did:plc:account2");
    QCOMPARE(account.handle, "account2.relog.tech");
    QCOMPARE(account.email, "account2@relog.tech");
    QCOMPARE(account.accessJwt, "accessJwt_account2");
    QCOMPARE(account.refreshJwt, "refreshJwt_account2");

    manager->clear();
    QVERIFY(manager->getUuids().isEmpty());

    {
        QSignalSpy spy(manager, SIGNAL(finished()));
        manager->load();
        spy.wait(10 * 1000);
        QCOMPARE(spy.count(), 1);
    }

    uuids = manager->getUuids();
    QCOMPARE(manager->count(), 1);

    account = manager->getAccount(uuids.at(0));
    QCOMPARE(account.service, m_service + "/account/account2");
    QVERIFY(account.service_endpoint
            == QString("http://localhost:%1/response/account/account2")
                       .arg(QString::number(m_listenPort)));
    QCOMPARE(account.did, "did:plc:account2_refresh");
}

void hagoromo_test::test_ListsListModel()
{
    QString uuid = AccountManager::getInstance()->updateAccount(
            QString(), m_service + "/lists/lists", "id", "pass", "did:plc:mqxsuw5b5rhpwo4lw6iwlid5",
            "hogehoge.bsky.social", "email", "accessJwt", "refreshJwt", true);

    ListsListModel model;
    model.setAccount(uuid);

    model.setVisibilityType(ListsListModel::VisibilityTypeAll);
    {
        QSignalSpy spy(&model, SIGNAL(runningChanged()));
        model.getLatest();
        spy.wait();
        QCOMPARE(spy.count(), 2);
    }
    QCOMPARE(model.rowCount(), 3);
    QVERIFY(model.item(0, ListsListModel::CidRole).toString()
            == "bafyreihnheyk74x4jts23gl23icubyarggmo37xn55pop2lpystq426bqu");
    QCOMPARE(model.item(0, ListsListModel::MutedRole).toBool(), false);
    QCOMPARE(model.item(0, ListsListModel::BlockedRole).toBool(), false);
    QVERIFY(model.item(1, ListsListModel::CidRole).toString()
            == "bafyreieyd765syuilkovwe3ms3cpegt7wo3xksistzy2v4xmazrwbzlwtm");
    QCOMPARE(model.item(1, ListsListModel::MutedRole).toBool(), true);
    QCOMPARE(model.item(1, ListsListModel::BlockedRole).toBool(), false);
    QVERIFY(model.item(2, ListsListModel::CidRole).toString()
            == "bafyreifeiua5ltajiaad76rdfuc6c63g5xd45ysro6cjptm5enwzqpcxdy");
    QCOMPARE(model.item(2, ListsListModel::MutedRole).toBool(), false);
    QCOMPARE(model.item(2, ListsListModel::BlockedRole).toBool(), true);

    model.clear();
    model.setVisibilityType(ListsListModel::VisibilityTypeCuration);
    {
        QSignalSpy spy(&model, SIGNAL(runningChanged()));
        model.getLatest();
        spy.wait();
        QCOMPARE(spy.count(), 2);
    }
    QCOMPARE(model.rowCount(), 2);
    QVERIFY(model.item(0, ListsListModel::CidRole).toString()
            == "bafyreihnheyk74x4jts23gl23icubyarggmo37xn55pop2lpystq426bqu");
    QCOMPARE(model.item(0, ListsListModel::MutedRole).toBool(), false);
    QCOMPARE(model.item(0, ListsListModel::BlockedRole).toBool(), false);
    QVERIFY(model.item(1, ListsListModel::CidRole).toString()
            == "bafyreifeiua5ltajiaad76rdfuc6c63g5xd45ysro6cjptm5enwzqpcxdy");
    QCOMPARE(model.item(1, ListsListModel::MutedRole).toBool(), false);
    QCOMPARE(model.item(1, ListsListModel::BlockedRole).toBool(), true);

    model.clear();
    model.setVisibilityType(ListsListModel::VisibilityTypeModeration);
    {
        QSignalSpy spy(&model, SIGNAL(runningChanged()));
        model.getLatest();
        spy.wait();
        QCOMPARE(spy.count(), 2);
    }
    QCOMPARE(model.rowCount(), 1);
    QVERIFY(model.item(0, ListsListModel::CidRole).toString()
            == "bafyreieyd765syuilkovwe3ms3cpegt7wo3xksistzy2v4xmazrwbzlwtm");
    QCOMPARE(model.item(0, ListsListModel::MutedRole).toBool(), true);
    QCOMPARE(model.item(0, ListsListModel::BlockedRole).toBool(), false);

    model.clear();
    model.setVisibilityType(ListsListModel::VisibilityTypeAll);
    {
        QSignalSpy spy(&model, SIGNAL(runningChanged()));
        model.getLatest();
        spy.wait();
        QCOMPARE(spy.count(), 2);
    }
    {
        QSignalSpy spy(&model, SIGNAL(runningChanged()));
        model.getNext();
        spy.wait();
        QCOMPARE(spy.count(), 2);
    }
    QCOMPARE(model.rowCount(), 6);
    QVERIFY(model.item(0, ListsListModel::CidRole).toString()
            == "bafyreihnheyk74x4jts23gl23icubyarggmo37xn55pop2lpystq426bqu");
    QVERIFY(model.item(1, ListsListModel::CidRole).toString()
            == "bafyreieyd765syuilkovwe3ms3cpegt7wo3xksistzy2v4xmazrwbzlwtm");
    QVERIFY(model.item(2, ListsListModel::CidRole).toString()
            == "bafyreifeiua5ltajiaad76rdfuc6c63g5xd45ysro6cjptm5enwzqpcxdy");
    QVERIFY(model.item(3, ListsListModel::CidRole).toString()
            == "bafyreihnheyk74x4jts23gl23icubyarggmo37xn55pop2lpystq426bqu_next");
    QVERIFY(model.item(4, ListsListModel::CidRole).toString()
            == "bafyreieyd765syuilkovwe3ms3cpegt7wo3xksistzy2v4xmazrwbzlwtm_next");
    QVERIFY(model.item(5, ListsListModel::CidRole).toString()
            == "bafyreifeiua5ltajiaad76rdfuc6c63g5xd45ysro6cjptm5enwzqpcxdy_next");
}

void hagoromo_test::test_ListsListModel_search()
{
    QString uuid = AccountManager::getInstance()->updateAccount(
            QString(), m_service + "/lists/search", "id", "pass",
            "did:plc:mqxsuw5b5rhpwo4lw6iwlid5", "hogehoge.bsky.social", "email", "accessJwt",
            "refreshJwt", true);

    ListsListModel model;
    model.setAccount(uuid);
    model.setSearchTarget("did:plc:user_42");
    {
        QSignalSpy spy(&model, SIGNAL(runningChanged()));
        model.getLatest();
        spy.wait();
        spy.wait();
        spy.wait();
        spy.wait();
        spy.wait();
        spy.wait(10 * 1000);
        QCOMPARE(spy.count(), 6);
    }

    QCOMPARE(model.rowCount(), 6);
    QVERIFY(model.item(0, ListsListModel::SearchStatusRole).toInt()
            == ListsListModel::SearchStatusTypeNotContains);
    QVERIFY(model.item(1, ListsListModel::SearchStatusRole).toInt()
            == ListsListModel::SearchStatusTypeNotContains);
    QVERIFY(model.item(2, ListsListModel::SearchStatusRole).toInt()
            == ListsListModel::SearchStatusTypeContains);
    QVERIFY(model.item(3, ListsListModel::SearchStatusRole).toInt()
            == ListsListModel::SearchStatusTypeNotContains);
    QVERIFY(model.item(4, ListsListModel::SearchStatusRole).toInt()
            == ListsListModel::SearchStatusTypeNotContains);
    QVERIFY(model.item(5, ListsListModel::SearchStatusRole).toInt()
            == ListsListModel::SearchStatusTypeNotContains);

    QCOMPARE(model.item(0, ListsListModel::ListItemUriRole).toString(), "");
    QCOMPARE(model.item(1, ListsListModel::ListItemUriRole).toString(), "");
    QCOMPARE(model.item(2, ListsListModel::ListItemUriRole).toString(),
             "at://did:plc:ipj5qejfoqu6eukvt72uhyit/app.bsky.graph.listitem/101");
    QCOMPARE(model.item(3, ListsListModel::ListItemUriRole).toString(), "");
    QCOMPARE(model.item(4, ListsListModel::ListItemUriRole).toString(), "");
    QCOMPARE(model.item(5, ListsListModel::ListItemUriRole).toString(), "");
}

void hagoromo_test::test_ListsListModel_error()
{
    ListsListModel model;

    model.setRunning(true);
    QCOMPARE(model.getLatest(), false);
    QCOMPARE(model.getNext(), false);
}

void hagoromo_test::test_ListItemListModel()
{
    QString uuid = AccountManager::getInstance()->updateAccount(
            QString(), m_service + "/lists/list", "id", "pass", "did:plc:ipj5qejfoqu6eukvt72uhyit",
            "hogehoge.bsky.social", "email", "accessJwt", "refreshJwt", true);

    ListItemListModel model;
    model.setAccount(uuid);

    model.setUri("at://did:plc:ipj5qejfoqu6eukvt72uhyit/app.bsky.graph.list/3k7igyxfizg27");
    {
        QSignalSpy spy(&model, SIGNAL(runningChanged()));
        model.getLatest();
        spy.wait();
        QCOMPARE(spy.count(), 2);
    }
    QCOMPARE(model.rowCount(), 3);
    QVERIFY(model.item(0, ListItemListModel::DidRole).toString()
            == "did:plc:ipj5qejfoqu6eukvt72uhyit");
    QVERIFY(model.item(1, ListItemListModel::DidRole).toString()
            == "did:plc:l4fsx4ujos7uw7n4ijq2ulgs");
    QVERIFY(model.item(2, ListItemListModel::DidRole).toString()
            == "did:plc:mqxsuw5b5rhpwo4lw6iwlid5");
    QVERIFY(model.uri()
            == "at://did:plc:ipj5qejfoqu6eukvt72uhyit/app.bsky.graph.list/3k7igyxfizg27");
    QCOMPARE(model.cid(), "bafyreifeiua5ltajiaad76rdfuc6c63g5xd45ysro6cjptm5enwzqpcxdy");
    QCOMPARE(model.name(), "my accounts");
    QCOMPARE(model.avatar(), "");
    QCOMPARE(model.description(), "my accounts list");
    QCOMPARE(model.muted(), false);
    QCOMPARE(model.blocked(), false);

    model.setUri("at://did:plc:ipj5qejfoqu6eukvt72uhyit/app.bsky.graph.list/3k7igyxfizg27");
    {
        QSignalSpy spy(&model, SIGNAL(runningChanged()));
        model.getNext();
        spy.wait();
        QCOMPARE(spy.count(), 2);
    }
    QCOMPARE(model.rowCount(), 6);
    QVERIFY(model.item(0, ListItemListModel::DidRole).toString()
            == "did:plc:ipj5qejfoqu6eukvt72uhyit");
    QVERIFY(model.item(1, ListItemListModel::DidRole).toString()
            == "did:plc:l4fsx4ujos7uw7n4ijq2ulgs");
    QVERIFY(model.item(2, ListItemListModel::DidRole).toString()
            == "did:plc:mqxsuw5b5rhpwo4lw6iwlid5");
    QVERIFY(model.item(3, ListItemListModel::DidRole).toString()
            == "did:plc:ipj5qejfoqu6eukvt72uhyit_next");
    QVERIFY(model.item(4, ListItemListModel::DidRole).toString()
            == "did:plc:l4fsx4ujos7uw7n4ijq2ulgs_next");
    QVERIFY(model.item(5, ListItemListModel::DidRole).toString()
            == "did:plc:mqxsuw5b5rhpwo4lw6iwlid5_next");
    QVERIFY(model.uri()
            == "at://did:plc:ipj5qejfoqu6eukvt72uhyit/app.bsky.graph.list/3k7igyxfizg27");
    QCOMPARE(model.cid(), "bafyreifeiua5ltajiaad76rdfuc6c63g5xd45ysro6cjptm5enwzqpcxdy");
    QCOMPARE(model.name(), "my accounts");
    QCOMPARE(model.avatar(), "");
    QCOMPARE(model.description(), "");
    QCOMPARE(model.muted(), false);
    QCOMPARE(model.blocked(), false);
}

void hagoromo_test::test_ListItemListModel_error()
{
    ListItemListModel model;

    model.setRunning(true);
    QCOMPARE(model.getLatest(), false);
    QCOMPARE(model.getNext(), false);
}

void hagoromo_test::test_ListFeedListModel()
{
    QString uuid = AccountManager::getInstance()->updateAccount(
            QString(), m_service + "/lists/feed/0", "id", "pass",
            "did:plc:l4fsx4ujos7uw7n4ijq2ulgs", "hogehoge.bsky.social", "email", "accessJwt",
            "refreshJwt", true);

    ListFeedListModel model;
    model.setAccount(uuid);
    model.setUri("at://did:plc:ipj5qejfoqu6eukvt72uhyit/app.bsky.graph.list/3k7igyxfizg27");
    {
        QSignalSpy spy(&model, SIGNAL(runningChanged()));
        model.getLatest();
        spy.wait(10 * 1000);
        QCOMPARE(spy.count(), 2);
    }

    QCOMPARE(model.rowCount(), 6);
}

void hagoromo_test::test_NotificationPreferenceListModel()
{
    // テストリソースファイルの存在確認
    QFile resourceFile(
            ":/response/notification/preference/0/xrpc/app.bsky.notification.getPreferences");
    QCOMPARE(resourceFile.exists(), true);

    QString uuid = AccountManager::getInstance()->updateAccount(
            QString(), m_service + "/notification/preference/0", "id", "pass",
            "did:plc:mqxsuw5b5rhpwo4lw6iwlid5", "hogehoge.bsky.social", "email", "accessJwt",
            "refreshJwt", true);

    NotificationPreferenceListModel model;
    model.setAccount(uuid);

    // 初期状態では13の設定項目があることを確認
    QCOMPARE(model.rowCount(), 13);

    {
        QSignalSpy spy(&model, SIGNAL(preferencesUpdated()));
        model.loadPreferences();
        spy.wait();
        QCOMPARE(spy.count(), 1);
    }

    // 設定読み込み後も13の設定項目があることを確認
    QCOMPARE(model.rowCount(), 13);

    // リソースファイルから読み込まれた実際の設定値の詳細検証

    // Follow (Social category) - リソース: include="all", list=true, push=false
    QVERIFY(model.item(0, NotificationPreferenceListModel::TypeRole).toInt()
            == NotificationPreferenceListModel::FollowType);
    QCOMPARE(model.item(0, NotificationPreferenceListModel::CategoryRole).toString(), "Social");
    QCOMPARE(model.item(0, NotificationPreferenceListModel::IncludeRole).toString(), "all");
    QCOMPARE(model.item(0, NotificationPreferenceListModel::ListRole).toBool(), true);
    QCOMPARE(model.item(0, NotificationPreferenceListModel::PushRole).toBool(), false);

    // Like (Social category) - リソース: include="all", list=true, push=true
    QVERIFY(model.item(1, NotificationPreferenceListModel::TypeRole).toInt()
            == NotificationPreferenceListModel::LikeType);
    QCOMPARE(model.item(1, NotificationPreferenceListModel::CategoryRole).toString(), "Social");
    QCOMPARE(model.item(1, NotificationPreferenceListModel::IncludeRole).toString(), "all");
    QCOMPARE(model.item(1, NotificationPreferenceListModel::ListRole).toBool(), true);
    QCOMPARE(model.item(1, NotificationPreferenceListModel::PushRole).toBool(), true);

    // Repost (Social category) - リソース: include="all", list=true, push=true
    QVERIFY(model.item(2, NotificationPreferenceListModel::TypeRole).toInt()
            == NotificationPreferenceListModel::RepostType);
    QCOMPARE(model.item(2, NotificationPreferenceListModel::IncludeRole).toString(), "all");
    QCOMPARE(model.item(2, NotificationPreferenceListModel::ListRole).toBool(), true);
    QCOMPARE(model.item(2, NotificationPreferenceListModel::PushRole).toBool(), true);

    // LikeViaRepost - リソース: include="all", list=true, push=true
    QVERIFY(model.item(3, NotificationPreferenceListModel::TypeRole).toInt()
            == NotificationPreferenceListModel::LikeViaRepostType);
    QCOMPARE(model.item(3, NotificationPreferenceListModel::IncludeRole).toString(), "all");
    QCOMPARE(model.item(3, NotificationPreferenceListModel::ListRole).toBool(), true);
    QCOMPARE(model.item(3, NotificationPreferenceListModel::PushRole).toBool(), true);

    // RepostViaRepost - リソース: include="all", list=true, push=true
    QVERIFY(model.item(4, NotificationPreferenceListModel::TypeRole).toInt()
            == NotificationPreferenceListModel::RepostViaRepostType);
    QCOMPARE(model.item(4, NotificationPreferenceListModel::IncludeRole).toString(), "all");
    QCOMPARE(model.item(4, NotificationPreferenceListModel::ListRole).toBool(), true);
    QCOMPARE(model.item(4, NotificationPreferenceListModel::PushRole).toBool(), true);

    // Chat (System category) - リソース: include="all", push=true
    QVERIFY(model.item(5, NotificationPreferenceListModel::TypeRole).toInt()
            == NotificationPreferenceListModel::ChatType);
    QVERIFY(model.item(5, NotificationPreferenceListModel::CategoryRole).toString()
            == "Interaction");
    QCOMPARE(model.item(5, NotificationPreferenceListModel::IncludeRole).toString(), "all");
    QCOMPARE(model.item(5, NotificationPreferenceListModel::PushRole).toBool(), true);

    // Reply (Interaction category) - リソース: include="all", list=true, push=true
    QVERIFY(model.item(6, NotificationPreferenceListModel::TypeRole).toInt()
            == NotificationPreferenceListModel::ReplyType);
    QVERIFY(model.item(6, NotificationPreferenceListModel::CategoryRole).toString()
            == "Interaction");
    QCOMPARE(model.item(6, NotificationPreferenceListModel::IncludeRole).toString(), "all");
    QCOMPARE(model.item(6, NotificationPreferenceListModel::ListRole).toBool(), true);
    QCOMPARE(model.item(6, NotificationPreferenceListModel::PushRole).toBool(), true);

    // Mention (Interaction category) - リソース: include="all", list=true, push=true
    QVERIFY(model.item(7, NotificationPreferenceListModel::TypeRole).toInt()
            == NotificationPreferenceListModel::MentionType);
    QVERIFY(model.item(7, NotificationPreferenceListModel::CategoryRole).toString()
            == "Interaction");
    QCOMPARE(model.item(7, NotificationPreferenceListModel::IncludeRole).toString(), "all");
    QCOMPARE(model.item(7, NotificationPreferenceListModel::ListRole).toBool(), true);
    QCOMPARE(model.item(7, NotificationPreferenceListModel::PushRole).toBool(), true);

    // Quote (Interaction category) - リソース: include="all", list=true, push=true
    QVERIFY(model.item(8, NotificationPreferenceListModel::TypeRole).toInt()
            == NotificationPreferenceListModel::QuoteType);
    QCOMPARE(model.item(8, NotificationPreferenceListModel::IncludeRole).toString(), "all");
    QCOMPARE(model.item(8, NotificationPreferenceListModel::ListRole).toBool(), true);
    QCOMPARE(model.item(8, NotificationPreferenceListModel::PushRole).toBool(), true);

    // StarterpackJoined (Activity category) - リソース: list=true, push=true (include設定なし)
    QVERIFY(model.item(9, NotificationPreferenceListModel::TypeRole).toInt()
            == NotificationPreferenceListModel::StarterpackJoinedType);
    QCOMPARE(model.item(9, NotificationPreferenceListModel::CategoryRole).toString(), "Activity");
    QCOMPARE(model.item(9, NotificationPreferenceListModel::ListRole).toBool(), true);
    QCOMPARE(model.item(9, NotificationPreferenceListModel::PushRole).toBool(), true);

    // SubscribedPost (Activity category) - リソース: list=true, push=true (include設定なし)
    QVERIFY(model.item(10, NotificationPreferenceListModel::TypeRole).toInt()
            == NotificationPreferenceListModel::SubscribedPostType);
    QCOMPARE(model.item(10, NotificationPreferenceListModel::ListRole).toBool(), true);
    QCOMPARE(model.item(10, NotificationPreferenceListModel::PushRole).toBool(), true);

    // Unverified (System category) - リソース: list=true, push=true (include設定なし)
    QVERIFY(model.item(11, NotificationPreferenceListModel::TypeRole).toInt()
            == NotificationPreferenceListModel::UnverifiedType);
    QCOMPARE(model.item(11, NotificationPreferenceListModel::CategoryRole).toString(), "System");
    QCOMPARE(model.item(11, NotificationPreferenceListModel::ListRole).toBool(), true);
    QCOMPARE(model.item(11, NotificationPreferenceListModel::PushRole).toBool(), true);

    // Verified (System category) - リソース: list=true, push=true (include設定なし)
    QVERIFY(model.item(12, NotificationPreferenceListModel::TypeRole).toInt()
            == NotificationPreferenceListModel::VerifiedType);
    QCOMPARE(model.item(12, NotificationPreferenceListModel::CategoryRole).toString(), "System");
    QCOMPARE(model.item(12, NotificationPreferenceListModel::ListRole).toBool(), true);
    QCOMPARE(model.item(12, NotificationPreferenceListModel::PushRole).toBool(), true);

    // 設定変更のテスト
    {
        // Include設定の変更
        model.updateInclude(2, "follows"); // Likeの設定を"follows"に変更
        QCOMPARE(model.item(2, NotificationPreferenceListModel::IncludeRole).toString(), "follows");

        // List設定の変更
        model.updateList(2, false); // Likeのlist設定をfalseに変更
        QCOMPARE(model.item(2, NotificationPreferenceListModel::ListRole).toBool(), false);

        // Push設定の変更
        model.updatePush(2, false); // Likeのpush設定をfalseに変更
        QCOMPARE(model.item(2, NotificationPreferenceListModel::PushRole).toBool(), false);
    }

    // include選択肢のテスト
    QStringList chatOptions =
            model.getAvailableIncludeOptions(NotificationPreferenceListModel::ChatType);
    QCOMPARE(chatOptions.contains("all"), true);
    QCOMPARE(chatOptions.contains("accepted"), true);
    QCOMPARE(chatOptions.size(), 2);

    QStringList followOptions =
            model.getAvailableIncludeOptions(NotificationPreferenceListModel::FollowType);
    QCOMPARE(followOptions.contains("all"), true);
    QCOMPARE(followOptions.contains("follows"), true);
    QCOMPARE(followOptions.size(), 2);

    // 表示名のテスト
    QCOMPARE(model.getIncludeDisplayName("all"), "All");
    QCOMPARE(model.getIncludeDisplayName("follows"), "Follows only");
    QCOMPARE(model.getIncludeDisplayName("accepted"), "Accepted only");

    // ShowListRoleのテスト
    // ChatTypeのみshowListがfalseであることを確認
    for (int i = 0; i < model.rowCount(); ++i) {
        int type = model.item(i, NotificationPreferenceListModel::TypeRole).toInt();
        bool showList = model.item(i, NotificationPreferenceListModel::ShowListRole).toBool();

        if (type == NotificationPreferenceListModel::ChatType) {
            QCOMPARE(showList, false);
        } else {
            QCOMPARE(showList, true);
        }
    }

    // データ整合性の確認
    qDebug() << "=== Loaded notification preferences from resource ===";
    for (int i = 0; i < model.rowCount(); ++i) {
        qDebug() << QString("Row %1: Type=%2, Include=%3, List=%4, Push=%5")
                            .arg(i)
                            .arg(model.item(i, NotificationPreferenceListModel::TypeRole).toInt())
                            .arg(model.item(i, NotificationPreferenceListModel::IncludeRole)
                                         .toString())
                            .arg(model.item(i, NotificationPreferenceListModel::ListRole).toBool())
                            .arg(model.item(i, NotificationPreferenceListModel::PushRole).toBool());
    }
}

void hagoromo_test::test_NotificationPreferenceListModel_save()
{
    // テストリソースファイルの存在確認
    QFile resourceFile(":/data/notification/preference/save/app.bsky.notification.putPreferences");
    QCOMPARE(resourceFile.exists(), true);

    QString uuid = AccountManager::getInstance()->updateAccount(
            QString(), m_service + "/notification/preference/save", "id", "pass",
            "did:plc:mqxsuw5b5rhpwo4lw6iwlid5", "hogehoge.bsky.social", "email", "accessJwt",
            "refreshJwt", true);

    NotificationPreferenceListModel model;
    model.setAccount(uuid);

    // 初期状態のデータロード
    {
        QSignalSpy spy(&model, SIGNAL(preferencesUpdated()));
        model.loadPreferences();
        spy.wait();
        QCOMPARE(spy.count(), 1);
    }

    // 保存処理のテスト - savePreferences()が期待通りのJSONを送信するかを検証
    {
        QSignalSpy spy(&model, SIGNAL(preferencesUpdated()));
        model.savePreferences();
        spy.wait();
        QCOMPARE(spy.count(), 1);
    }

    qDebug() << "=== savePreferences() test completed successfully ===";
}

void hagoromo_test::test_TokimekiPollOperator_getPoll_vote()
{
    QString uuid = AccountManager::getInstance()->updateAccount(
            QString(), m_service + "/tokimeki", "id", "pass", "did:plc:hogehoge",
            "hogehoge.bsky.social", "email", "accessJwt", "refreshJwt", true);

    TokimekiPollOperator ope;
    ope.setAccount(uuid);
    ope.setServiceUrl(m_service + "/tokimeki/no_vote");

    {
        QSignalSpy spy(&ope, &TokimekiPollOperator::finished);

        const QString cid = QStringLiteral("test-cid-novote");
        const QString uri = QStringLiteral(
                "at://did:plc:mqxsuw5b5rhpwo4lw6iwlid5/tech.tokimeki.poll.poll/3mb6j6si7qc2u");
        const QString viewer = QStringLiteral("did:plc:viewerexample-no-vote000000000000");

        ope.getPoll(cid, uri, viewer);

        QVERIFY(spy.wait(10 * 1000));
        QCOMPARE(spy.count(), 1);

        const QList<QVariant> arguments = spy.takeFirst();
        QVERIFY(arguments.at(0).toBool());
        QCOMPARE(arguments.at(1).toString(), cid);
    }
    {
        QHash<QString, QJsonObject> hash =
                UnitTestCommon::loadVoteHash(":/data/com.atproto.repo.createRecord_vote.expect");

        QHashIterator<QString, QJsonObject> i(hash);
        while (i.hasNext()) {
            i.next();
            const QString uri = i.value().value("uri").toString();
            const QString cid = i.value().value("cid").toString();
            const QString option_index = QString::number(i.value().value("optionIndex").toInt());
            const QString uuid = AccountManager::getInstance()->updateAccount(
                    QString(), m_service + "/tokimeki", "id", "pass", i.key(), "handle", "email",
                    "accessJwt", "refreshJwt", true);
            ope.setAccount(uuid);

            QSignalSpy spy(&ope, &TokimekiPollOperator::finished);
            ope.vote(cid, uri, option_index);
            spy.wait();
            QCOMPARE(spy.count(), 1);

            QList<QVariant> arguments = spy.takeFirst();
            QVERIFY(arguments.at(0).toBool());
            QCOMPARE(arguments.at(1).toString(),
                     "bafyreibir6pwtrmmj6mczufitlbql7h77not66hacysw2cfr6wlaerltpe");

            QVariant v;
            v = ope.item(uri, TokimekiPollOperator::PollMyVoteRole);
            QCOMPARE(v.isValid(), true);
            QCOMPARE(v.toString(), option_index);

            v = ope.item(uri, TokimekiPollOperator::PollTotalVotesRole);
            QCOMPARE(v.isValid(), true);
            QCOMPARE(v.toInt(), 3);

            v = ope.item(uri, TokimekiPollOperator::PollCountOfOptionsRole);
            QCOMPARE(v.isValid(), true);
            QCOMPARE(v.toStringList(),
                     QStringList() << "2"
                                   << "1");
        }
    }
}

void hagoromo_test::test_DraftOperator_createDraft()
{
    QString uuid = AccountManager::getInstance()->updateAccount(
            QString(), m_service + "/draft", "id", "pass", "did:plc:hogehoge",
            "hogehoge.bsky.social", "email", "accessJwt", "refreshJwt", true);

    DraftOperator ope;
    ope.setAccount(uuid);
    ope.setText("Test draft text");
    ope.setPostLanguages(QStringList() << "ja"
                                       << "en");
    ope.setSelfLabels(QStringList() << "sexual");
    ope.setThreadGate("choice", QStringList() << "follower");
    ope.setPostGate(false, QStringList());
    ope.setImages(QStringList() << "/path/to/file1"
                                << "/path/to/file2",
                  QStringList() << "alt1"
                                << "alt2");

    QSignalSpy spyFinished(&ope, SIGNAL(finishedCreateDraft(bool, const QString &)));
    ope.createDraft();
    spyFinished.wait();
    QCOMPARE(spyFinished.count(), 1);

    QList<QVariant> arguments = spyFinished.takeFirst();
    QCOMPARE(arguments.at(0).typeId(), QMetaType::Bool);
    QCOMPARE(arguments.at(0).toBool(), true);
    QCOMPARE(arguments.at(1).typeId(), QMetaType::QString);
    QCOMPARE(arguments.at(1).toString(), QString("draft-001"));
}

void hagoromo_test::test_DraftOperator_updateDraft()
{
    QString uuid = AccountManager::getInstance()->updateAccount(
            QString(), m_service + "/draft", "id", "pass", "did:plc:hogehoge",
            "hogehoge.bsky.social", "email", "accessJwt", "refreshJwt", true);

    DraftOperator ope;
    ope.setAccount(uuid);
    ope.setText("Updated draft text");

    QSignalSpy spyFinished(&ope, SIGNAL(finishedUpdateDraft(bool)));
    ope.updateDraft("draft-001");
    spyFinished.wait();
    QCOMPARE(spyFinished.count(), 1);

    QList<QVariant> arguments = spyFinished.takeFirst();
    QCOMPARE(arguments.at(0).typeId(), QMetaType::Bool);
    QCOMPARE(arguments.at(0).toBool(), true);
}

void hagoromo_test::test_DraftOperator_deleteDraft()
{
    QString uuid = AccountManager::getInstance()->updateAccount(
            QString(), m_service + "/draft", "id", "pass", "did:plc:hogehoge",
            "hogehoge.bsky.social", "email", "accessJwt", "refreshJwt", true);

    DraftOperator ope;
    ope.setAccount(uuid);

    QSignalSpy spyFinished(&ope, SIGNAL(finishedDeleteDraft(bool)));
    ope.deleteDraft("draft-001");
    spyFinished.wait();
    QCOMPARE(spyFinished.count(), 1);

    QList<QVariant> arguments = spyFinished.takeFirst();
    QCOMPARE(arguments.at(0).typeId(), QMetaType::Bool);
    QCOMPARE(arguments.at(0).toBool(), true);
}

void hagoromo_test::test_DraftOperator_getDrafts()
{
    QString uuid = AccountManager::getInstance()->updateAccount(
            QString(), m_service + "/draft", "id", "pass", "did:plc:hogehoge",
            "hogehoge.bsky.social", "email", "accessJwt", "refreshJwt", true);

    DraftOperator ope;
    ope.setAccount(uuid);

    QSignalSpy spyFinished(
            &ope,
            SIGNAL(finishedGetDrafts(bool,
                                     const QList<AtProtocolType::AppBskyDraftDefs::DraftView> &)));
    ope.getDrafts(10, QString());
    spyFinished.wait();
    QCOMPARE(spyFinished.count(), 1);

    QList<QVariant> arguments = spyFinished.takeFirst();
    QCOMPARE(arguments.at(0).typeId(), QMetaType::Bool);
    QCOMPARE(arguments.at(0).toBool(), true);
}

void hagoromo_test::test_RecordOperatorCreateRecord(const QByteArray &body)
{
    QJsonDocument json_doc = QJsonDocument::fromJson(body);
    QString collection = json_doc.object().value("collection").toString();

    if (collection == "app.bsky.feed.post") {
        QString did = json_doc.object().value("repo").toString();
        QString record = json_doc.object().value("record").toObject().value("text").toString();

        QHash<QString, QJsonObject> hash = UnitTestCommon::loadPostExpectHash(
                ":/data/com.atproto.repo.createRecord_post.expect");

        QCOMPARE(hash.contains(did), true);
        QCOMPARE(hash[did].value("record").toObject().value("text").toString(), record);
        if (!hash[did].value("record").toObject().value("facets").isNull()) {
            QCOMPARE(json_doc.object().value("record").toObject().value("facets"),
                     hash[did].value("record").toObject().value("facets"));
        }
    } else if (collection == "tech.tokimeki.poll.vote") {
        QString did = json_doc.object().value("repo").toString();
        QJsonObject expect_obj = json_doc.object();

        QHash<QString, QJsonObject> hash = UnitTestCommon::loadPostExpectHash(
                ":/data/com.atproto.repo.createRecord_vote.expect");
        QCOMPARE(hash.contains(did), true);
        {
            QJsonObject tmp = hash[did].value("record").toObject();
            tmp.remove("createdAt");
            hash[did]["record"] = tmp;
        }
        {
            QJsonObject tmp = expect_obj.value("record").toObject();
            tmp.remove("createdAt");
            expect_obj["record"] = tmp;
        }
        QCOMPARE(hash[did], expect_obj);
    }
}

void hagoromo_test::test_putPreferences(const QString &path, const QByteArray &body)
{
    QJsonDocument json_doc_expect;
    if (path.contains("/save/")) {
        json_doc_expect =
                UnitTestCommon::loadJson(":/data/generator/save/app.bsky.actor.putPreferences");
    } else {
        json_doc_expect =
                UnitTestCommon::loadJson(":/data/generator/remove/app.bsky.actor.putPreferences");
    }
    QJsonDocument json_doc_expect2 =
            QJsonDocument(copyObject(json_doc_expect.object(), QStringList() << "id"));

    QJsonDocument json_doc = QJsonDocument::fromJson(body);
    QJsonDocument json_doc2 = QJsonDocument(copyObject(json_doc.object(), QStringList() << "id"));

    if (json_doc_expect2.object() != json_doc2.object()) {
        qDebug().noquote().nospace() << QString("\nexpect:%1\nactual:%2\n")
                                                .arg(json_doc_expect2.toJson(), json_doc2.toJson());
    }
    QCOMPARE(json_doc_expect2.object(), json_doc2.object());
}

void hagoromo_test::test_putRecord(const QString &path, const QByteArray &body)
{
    QJsonDocument json_doc_expect;
    if (path.contains("/profile/1/")) {
        json_doc_expect = UnitTestCommon::loadJson(":/data/profile/1/com.atproto.repo.putRecord");
    } else if (path.contains("/profile/3.1/")) {
        json_doc_expect = UnitTestCommon::loadJson(":/data/profile/3.1/com.atproto.repo.putRecord");
    } else if (path.contains("/profile/3.2/")) {
        json_doc_expect = UnitTestCommon::loadJson(":/data/profile/3.2/com.atproto.repo.putRecord");
    } else {
        QVERIFY(false);
    }

    QJsonDocument json_doc = QJsonDocument::fromJson(body);

    if (json_doc_expect.object() != json_doc.object()) {
        qDebug().noquote().nospace() << QString("\nexpect:%1\nactual:%2\n")
                                                .arg(json_doc_expect.toJson(), json_doc.toJson());
    }
    QCOMPARE(json_doc_expect.object(), json_doc.object());
}

void hagoromo_test::test_putNotificationPreferences(const QString &path, const QByteArray &body)
{
    QJsonDocument json_doc_expect = UnitTestCommon::loadJson(
            ":/data/notification/preference/save/app.bsky.notification.putPreferences");
    QJsonDocument json_doc = QJsonDocument::fromJson(body);

    if (json_doc_expect.object() != json_doc.object()) {
        qDebug().noquote().nospace() << QString("\nexpect:%1\nactual:%2\n")
                                                .arg(json_doc_expect.toJson(), json_doc.toJson());
    }
    QCOMPARE(json_doc_expect.object(), json_doc.object());
}

QJsonObject hagoromo_test::copyObject(const QJsonObject &src, const QStringList &excludes)
{

    QJsonObject dest;
    for (const auto &key : src.keys()) {
        if (excludes.contains(key))
            continue;
        if (src.value(key).isObject()) {
            dest.insert(key, copyObject(src.value(key).toObject(), excludes));
        } else if (src.value(key).isArray()) {
            QJsonArray dest_array;
            for (const auto value : src.value(key).toArray()) {
                if (value.isObject()) {
                    dest_array.append(copyObject(value.toObject(), excludes));
                } else {
                    dest_array.append(value);
                }
            }
            dest.insert(key, dest_array);
        } else {
            dest.insert(key, src.value(key));
        }
    }
    return dest;
}

QTEST_MAIN(hagoromo_test)

#include "tst_hagoromo_test2.moc"
