#include <QtTest>
#include <QCoreApplication>
#include <QJsonDocument>
#include <QDir>
#include <QFile>

#include "webserver.h"
#include "unittest_common.h"
#include "timeline/timelinelistmodel.h"
#include "operation/recordoperator.h"
#include "feedgenerator/feedgeneratorlistmodel.h"
#include "link/feedgeneratorlink.h"
#include "account/accountlistmodel.h"
#include "common.h"
#include "list/listslistmodel.h"
#include "list/listitemlistmodel.h"
#include "list/listfeedlistmodel.h"
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

private:
    WebServer m_mockServer;
    quint16 m_listenPort;
    QString m_service;

    void test_RecordOperatorCreateRecord(const QByteArray &body);
    void test_putPreferences(const QString &path, const QByteArray &body);
    void test_putRecord(const QString &path, const QByteArray &body);
    void verifyStr(const QString &expect, const QString &actual);
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
        QVERIFY2(spy.count() == 1, QString("spy.count()=%1").arg(spy.count()).toUtf8());

        QList<QVariant> arguments = spy.takeFirst();
        QVERIFY(arguments.at(0).type() == QVariant::Bool);
        QVERIFY(arguments.at(0).toBool() == true);
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
        ope.updateProfile("", "", "description", "display_name");
        spy.wait();
        QVERIFY2(spy.count() == 1, QString("spy.count()=%1").arg(spy.count()).toUtf8());

        QList<QVariant> arguments = spy.takeFirst();
        QVERIFY(arguments.at(0).type() == QVariant::Bool);
        QVERIFY(arguments.at(0).toBool() == true);
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
        QVERIFY2(spy.count() == 1, QString("spy.count()=%1").arg(spy.count()).toUtf8());

        QList<QVariant> arguments = spy.takeFirst();
        QVERIFY(arguments.at(0).type() == QVariant::Bool);
        QVERIFY(arguments.at(0).toBool() == true);
    }

    uuid = AccountManager::getInstance()->updateAccount(
            QString(), m_service + "/profile/3.2", "id", "pass", "did:plc:ipj5qejfoqu6eukvt72uhyit",
            "hogehoge.bsky.social", "email", "accessJwt", "refreshJwt", true);
    ope.setAccount(uuid);
    {
        QSignalSpy spy(&ope, SIGNAL(finished(bool, const QString &, const QString &)));
        ope.updatePostPinning(QString(), QString());
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
        QString uuid = AccountManager::getInstance()->updateAccount(
                QString(), m_service + "/generator/save", "id", "pass",
                "did:plc:ipj5qejfoqu6eukvt72uhyit", "hogehoge.bsky.social", "email", "accessJwt",
                "refreshJwt", true);
        model.setAccount(uuid);
        QSignalSpy spy(&model, SIGNAL(runningChanged()));
        model.saveGenerator(
                "at://did:plc:z72i7hdynmk6r22z27h6tvur/app.bsky.feed.generator/hot-classic");
        spy.wait(10 * 1000);
        QVERIFY2(spy.count() == 2, QString("spy.count()=%1").arg(spy.count()).toUtf8());
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
        QVERIFY2(spy.count() == 2, QString("spy.count()=%1").arg(spy.count()).toUtf8());
    }
}

void hagoromo_test::test_FeedGeneratorLink()
{
    QString uuid = AccountManager::getInstance()->updateAccount(
            QString(), m_service + "/generator", "id", "pass", "did:plc:ipj5qejfoqu6eukvt72uhyit",
            "hogehoge.bsky.social", "email", "accessJwt", "refreshJwt", true);

    FeedGeneratorLink link;

    QVERIFY(link.checkUri("https://bsky.app/profile/did:plc:hoge/feed/aaaaaaaa", "feed") == true);
    QVERIFY(link.checkUri("https://staging.bsky.app/profile/did:plc:hoge/feed/aaaaaaaa", "feed")
            == false);
    QVERIFY(link.checkUri("https://bsky.app/feeds/did:plc:hoge/feed/aaaaaaaa", "feed") == false);
    QVERIFY(link.checkUri("https://bsky.app/profile/did:plc:hoge/feeds/aaaaaaaa", "feed") == false);
    QVERIFY(link.checkUri("https://bsky.app/profile/did:plc:hoge/feed/", "feed") == false);
    QVERIFY(link.checkUri("https://bsky.app/profile/handle/feed/aaaaaaaa", "feed") == false);
    QVERIFY(link.checkUri("https://bsky.app/profile/handle.com/feed/aaaaaaaa", "feed") == true);
    QVERIFY(link.checkUri("https://bsky.app/profile/@handle.com/feed/aaaaaaaa", "feed") == false);

    link.setAccount(uuid);
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
        QVERIFY2(spy.count() == 1, QString("spy.count()=%1").arg(spy.count()).toUtf8());
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

    // QVERIFY2(model.item(0, AccountListModel::UuidRole).toString() == "UuidRole",
    //          model.item(0, AccountListModel::UuidRole).toString().toLocal8Bit());
    // QVERIFY2(model.item(0, AccountListModel::IsMainRole).toString() == "IsMainRole",
    //          model.item(0, AccountListModel::IsMainRole).toString().toLocal8Bit());
    QVERIFY2(model.item(0, AccountListModel::ServiceRole).toString() == "ServiceRole",
             model.item(0, AccountListModel::ServiceRole).toString().toLocal8Bit());
    QVERIFY2(model.item(0, AccountListModel::ServiceEndpointRole).toString()
                     == "ServiceEndpointRole",
             model.item(0, AccountListModel::ServiceEndpointRole).toString().toLocal8Bit());
    QVERIFY2(model.item(0, AccountListModel::IdentifierRole).toString() == "IdentifierRole",
             model.item(0, AccountListModel::IdentifierRole).toString().toLocal8Bit());
    QVERIFY2(model.item(0, AccountListModel::PasswordRole).toString() == "PasswordRole",
             model.item(0, AccountListModel::PasswordRole).toString().toLocal8Bit());
    QVERIFY2(model.item(0, AccountListModel::DidRole).toString() == "DidRole",
             model.item(0, AccountListModel::DidRole).toString().toLocal8Bit());
    QVERIFY2(model.item(0, AccountListModel::HandleRole).toString() == "HandleRole",
             model.item(0, AccountListModel::HandleRole).toString().toLocal8Bit());
    QVERIFY2(model.item(0, AccountListModel::EmailRole).toString() == "EmailRole",
             model.item(0, AccountListModel::EmailRole).toString().toLocal8Bit());
    QVERIFY2(model.item(0, AccountListModel::AccessJwtRole).toString() == "AccessJwtRole",
             model.item(0, AccountListModel::AccessJwtRole).toString().toLocal8Bit());
    QVERIFY2(model.item(0, AccountListModel::RefreshJwtRole).toString() == "RefreshJwtRole",
             model.item(0, AccountListModel::RefreshJwtRole).toString().toLocal8Bit());
    QVERIFY2(model.item(0, AccountListModel::DisplayNameRole).toString() == "DisplayNameRole",
             model.item(0, AccountListModel::DisplayNameRole).toString().toLocal8Bit());
    QVERIFY2(model.item(0, AccountListModel::DescriptionRole).toString() == "DescriptionRole",
             model.item(0, AccountListModel::DescriptionRole).toString().toLocal8Bit());
    QVERIFY2(model.item(0, AccountListModel::AvatarRole).toString() == "AvatarRole",
             model.item(0, AccountListModel::AvatarRole).toString().toLocal8Bit());
    QVERIFY2(model.item(0, AccountListModel::PostLanguagesRole).toStringList()
                     == QStringList() << "PostLanguagesRole1"
                                      << "PostLanguagesRole2",
             model.item(0, AccountListModel::PostLanguagesRole)
                     .toStringList()
                     .join(",")
                     .toLocal8Bit());
    QVERIFY2(model.item(0, AccountListModel::ThreadGateTypeRole).toString() == "ThreadGateTypeRole",
             model.item(0, AccountListModel::ThreadGateTypeRole).toString().toLocal8Bit());
    QVERIFY2(model.item(0, AccountListModel::ThreadGateOptionsRole).toStringList()
                     == QStringList() << "ThreadGateOptionsRole1"
                                      << "ThreadGateOptionsRole2",
             model.item(0, AccountListModel::ThreadGateOptionsRole)
                     .toStringList()
                     .join(",")
                     .toLocal8Bit());
    QVERIFY2(model.item(0, AccountListModel::PostGateQuoteEnabledRole).toBool() == true,
             QString::number(model.item(0, AccountListModel::PostGateQuoteEnabledRole).toBool())
                     .toLocal8Bit());
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

    QVERIFY(uuids.count() == 2);

    account = manager->getAccount(uuids.at(0));
    QVERIFY(account.service == m_service + "/account/account1");
    QVERIFY(account.password == "password1");
    QVERIFY(account.did == "did:plc:account1");
    QVERIFY(account.handle == "account1.relog.tech");
    QVERIFY(account.email == "account1@relog.tech");
    QVERIFY(account.accessJwt == "accessJwt_account1");
    QVERIFY(account.refreshJwt == "refreshJwt_account1");

    account = manager->getAccount(uuids.at(1));
    QVERIFY(account.service == m_service + "/account/account2");
    QVERIFY(account.password == "password2");
    QVERIFY(account.did == "did:plc:account2");
    QVERIFY(account.handle == "account2.relog.tech");
    QVERIFY(account.email == "account2@relog.tech");
    QVERIFY(account.accessJwt == "accessJwt_account2");
    QVERIFY(account.refreshJwt == "refreshJwt_account2");

    QVERIFY(manager->checkAllAccountsReady() == false);

    manager->save();

    manager->removeAccount(manager->getUuids().at(0));
    QVERIFY(manager->getUuids().count() == 1);

    account = manager->getAccount(manager->getUuids().at(0));
    QVERIFY(account.service == m_service + "/account/account2");
    QVERIFY(account.password == "password2");
    QVERIFY(account.did == "did:plc:account2");
    QVERIFY(account.handle == "account2.relog.tech");
    QVERIFY(account.email == "account2@relog.tech");
    QVERIFY(account.accessJwt == "accessJwt_account2");
    QVERIFY(account.refreshJwt == "refreshJwt_account2");

    manager->clear();
    QVERIFY(manager->getUuids().isEmpty());

    {
        QSignalSpy spy(manager, SIGNAL(finished()));
        manager->load();
        spy.wait(10 * 1000);
        QVERIFY2(spy.count() == 1, QString("spy.count()=%1").arg(spy.count()).toUtf8());
    }

    uuids = manager->getUuids();
    QVERIFY(manager->count() == 1);

    account = manager->getAccount(uuids.at(0));
    QVERIFY2(account.service == m_service + "/account/account2", account.service.toLocal8Bit());
    QVERIFY(account.service_endpoint
            == QString("http://localhost:%1/response/account/account2")
                       .arg(QString::number(m_listenPort)));
    QVERIFY(account.did == "did:plc:account2_refresh");
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
        QVERIFY2(spy.count() == 2, QString("spy.count()=%1").arg(spy.count()).toUtf8());
    }
    QVERIFY2(model.rowCount() == 3,
             QString("model.rowCount()=%1").arg(model.rowCount()).toLocal8Bit());
    QVERIFY(model.item(0, ListsListModel::CidRole).toString()
            == "bafyreihnheyk74x4jts23gl23icubyarggmo37xn55pop2lpystq426bqu");
    QVERIFY(model.item(0, ListsListModel::MutedRole).toBool() == false);
    QVERIFY(model.item(0, ListsListModel::BlockedRole).toBool() == false);
    QVERIFY(model.item(1, ListsListModel::CidRole).toString()
            == "bafyreieyd765syuilkovwe3ms3cpegt7wo3xksistzy2v4xmazrwbzlwtm");
    QVERIFY(model.item(1, ListsListModel::MutedRole).toBool() == true);
    QVERIFY(model.item(1, ListsListModel::BlockedRole).toBool() == false);
    QVERIFY(model.item(2, ListsListModel::CidRole).toString()
            == "bafyreifeiua5ltajiaad76rdfuc6c63g5xd45ysro6cjptm5enwzqpcxdy");
    QVERIFY(model.item(2, ListsListModel::MutedRole).toBool() == false);
    QVERIFY(model.item(2, ListsListModel::BlockedRole).toBool() == true);

    model.clear();
    model.setVisibilityType(ListsListModel::VisibilityTypeCuration);
    {
        QSignalSpy spy(&model, SIGNAL(runningChanged()));
        model.getLatest();
        spy.wait();
        QVERIFY2(spy.count() == 2, QString("spy.count()=%1").arg(spy.count()).toUtf8());
    }
    QVERIFY2(model.rowCount() == 2,
             QString("model.rowCount()=%1").arg(model.rowCount()).toLocal8Bit());
    QVERIFY(model.item(0, ListsListModel::CidRole).toString()
            == "bafyreihnheyk74x4jts23gl23icubyarggmo37xn55pop2lpystq426bqu");
    QVERIFY(model.item(0, ListsListModel::MutedRole).toBool() == false);
    QVERIFY(model.item(0, ListsListModel::BlockedRole).toBool() == false);
    QVERIFY(model.item(1, ListsListModel::CidRole).toString()
            == "bafyreifeiua5ltajiaad76rdfuc6c63g5xd45ysro6cjptm5enwzqpcxdy");
    QVERIFY(model.item(1, ListsListModel::MutedRole).toBool() == false);
    QVERIFY(model.item(1, ListsListModel::BlockedRole).toBool() == true);

    model.clear();
    model.setVisibilityType(ListsListModel::VisibilityTypeModeration);
    {
        QSignalSpy spy(&model, SIGNAL(runningChanged()));
        model.getLatest();
        spy.wait();
        QVERIFY2(spy.count() == 2, QString("spy.count()=%1").arg(spy.count()).toUtf8());
    }
    QVERIFY2(model.rowCount() == 1,
             QString("model.rowCount()=%1").arg(model.rowCount()).toLocal8Bit());
    QVERIFY(model.item(0, ListsListModel::CidRole).toString()
            == "bafyreieyd765syuilkovwe3ms3cpegt7wo3xksistzy2v4xmazrwbzlwtm");
    QVERIFY(model.item(0, ListsListModel::MutedRole).toBool() == true);
    QVERIFY(model.item(0, ListsListModel::BlockedRole).toBool() == false);

    model.clear();
    model.setVisibilityType(ListsListModel::VisibilityTypeAll);
    {
        QSignalSpy spy(&model, SIGNAL(runningChanged()));
        model.getLatest();
        spy.wait();
        QVERIFY2(spy.count() == 2, QString("spy.count()=%1").arg(spy.count()).toUtf8());
    }
    {
        QSignalSpy spy(&model, SIGNAL(runningChanged()));
        model.getNext();
        spy.wait();
        QVERIFY2(spy.count() == 2, QString("spy.count()=%1").arg(spy.count()).toUtf8());
    }
    QVERIFY2(model.rowCount() == 6,
             QString("model.rowCount()=%1").arg(model.rowCount()).toLocal8Bit());
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
        QVERIFY2(spy.count() == 6, QString("spy.count()=%1").arg(spy.count()).toUtf8());
    }

    QVERIFY2(model.rowCount() == 6,
             QString("model.rowCount()=%1").arg(model.rowCount()).toLocal8Bit());
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

    QVERIFY2(model.item(0, ListsListModel::ListItemUriRole).toString() == "",
             model.item(0, ListsListModel::ListItemUriRole).toString().toLocal8Bit());
    QVERIFY2(model.item(1, ListsListModel::ListItemUriRole).toString() == "",
             model.item(1, ListsListModel::ListItemUriRole).toString().toLocal8Bit());
    QVERIFY2(model.item(2, ListsListModel::ListItemUriRole).toString()
                     == "at://did:plc:ipj5qejfoqu6eukvt72uhyit/app.bsky.graph.listitem/101",
             model.item(2, ListsListModel::ListItemUriRole).toString().toLocal8Bit());
    QVERIFY2(model.item(3, ListsListModel::ListItemUriRole).toString() == "",
             model.item(3, ListsListModel::ListItemUriRole).toString().toLocal8Bit());
    QVERIFY2(model.item(4, ListsListModel::ListItemUriRole).toString() == "",
             model.item(4, ListsListModel::ListItemUriRole).toString().toLocal8Bit());
    QVERIFY2(model.item(5, ListsListModel::ListItemUriRole).toString() == "",
             model.item(5, ListsListModel::ListItemUriRole).toString().toLocal8Bit());
}

void hagoromo_test::test_ListsListModel_error()
{
    ListsListModel model;

    model.setRunning(true);
    QVERIFY(model.getLatest() == false);
    QVERIFY(model.getNext() == false);
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
        QVERIFY2(spy.count() == 2, QString("spy.count()=%1").arg(spy.count()).toUtf8());
    }
    QVERIFY2(model.rowCount() == 3,
             QString("model.rowCount()=%1").arg(model.rowCount()).toLocal8Bit());
    QVERIFY(model.item(0, ListItemListModel::DidRole).toString()
            == "did:plc:ipj5qejfoqu6eukvt72uhyit");
    QVERIFY(model.item(1, ListItemListModel::DidRole).toString()
            == "did:plc:l4fsx4ujos7uw7n4ijq2ulgs");
    QVERIFY(model.item(2, ListItemListModel::DidRole).toString()
            == "did:plc:mqxsuw5b5rhpwo4lw6iwlid5");
    QVERIFY(model.uri()
            == "at://did:plc:ipj5qejfoqu6eukvt72uhyit/app.bsky.graph.list/3k7igyxfizg27");
    QVERIFY(model.cid() == "bafyreifeiua5ltajiaad76rdfuc6c63g5xd45ysro6cjptm5enwzqpcxdy");
    QVERIFY(model.name() == "my accounts");
    QVERIFY(model.avatar() == "");
    QVERIFY(model.description() == "my accounts list");
    QVERIFY(model.muted() == false);
    QVERIFY(model.blocked() == false);

    model.setUri("at://did:plc:ipj5qejfoqu6eukvt72uhyit/app.bsky.graph.list/3k7igyxfizg27");
    {
        QSignalSpy spy(&model, SIGNAL(runningChanged()));
        model.getNext();
        spy.wait();
        QVERIFY2(spy.count() == 2, QString("spy.count()=%1").arg(spy.count()).toUtf8());
    }
    QVERIFY2(model.rowCount() == 6,
             QString("model.rowCount()=%1").arg(model.rowCount()).toLocal8Bit());
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
    QVERIFY(model.cid() == "bafyreifeiua5ltajiaad76rdfuc6c63g5xd45ysro6cjptm5enwzqpcxdy");
    QVERIFY(model.name() == "my accounts");
    QVERIFY(model.avatar() == "");
    QVERIFY(model.description() == "");
    QVERIFY(model.muted() == false);
    QVERIFY(model.blocked() == false);
}

void hagoromo_test::test_ListItemListModel_error()
{
    ListItemListModel model;

    model.setRunning(true);
    QVERIFY(model.getLatest() == false);
    QVERIFY(model.getNext() == false);
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
        QVERIFY2(spy.count() == 2, QString("spy.count()=%1").arg(spy.count()).toUtf8());
    }

    QVERIFY2(model.rowCount() == 6,
             QString("model.rowCount()=%1").arg(model.rowCount()).toLocal8Bit());
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
    QVERIFY(json_doc_expect2.object() == json_doc2.object());
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
    QVERIFY(json_doc_expect.object() == json_doc.object());
}

void hagoromo_test::verifyStr(const QString &expect, const QString &actual)
{
    QVERIFY2(expect == actual,
             QString("\nexpect:%1\nactual:%2\n").arg(expect, actual).toLocal8Bit());
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
