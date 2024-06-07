#include <QtTest>
#include <QCoreApplication>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonValue>
#include <QJsonDocument>

#include "realtime/abstractpostselector.h"
#include "realtime/firehosereceiver.h"

using namespace RealtimeFeed;

class realtime_test : public QObject
{
    Q_OBJECT

public:
    realtime_test();
    ~realtime_test();

private slots:
    void initTestCase();
    void cleanupTestCase();
    void test_PostSelector();
    void test_FirehoseReceiver();

private:
    QList<UserInfo> extractFromArray(const QJsonArray &array) const;
};

realtime_test::realtime_test() { }

realtime_test::~realtime_test() { }

void realtime_test::initTestCase() { }

void realtime_test::cleanupTestCase() { }

void realtime_test::test_PostSelector()
{
    QFile file(":/data/selector/selector.json");
    QVERIFY(file.open(QFile::ReadOnly));
    QJsonDocument json_doc = QJsonDocument::fromJson(file.readAll());
    QVERIFY(json_doc.isArray());
    for (const auto json_item : json_doc.array()) {
        QVERIFY(json_item.isObject());
        QString description = json_item.toObject().value("description").toString();
        qDebug().noquote() << description;
        QVERIFY(json_item.toObject().contains("following"));
        QVERIFY(json_item.toObject().contains("followers"));
        QVERIFY(json_item.toObject().contains("selector"));
        QVERIFY(json_item.toObject().contains("data"));
        QList<UserInfo> following =
                extractFromArray(json_item.toObject().value("following").toArray());
        QList<UserInfo> followers =
                extractFromArray(json_item.toObject().value("followers").toArray());
        QVERIFY(following.isEmpty() == false);
        QVERIFY(followers.isEmpty() == false);

        AbstractPostSelector *root = AbstractPostSelector::create(
                json_item.toObject().value("selector").toObject(), this);
        QVERIFY(root != nullptr);
        root->setFollowing(following);
        root->setFollowers(followers);
        root->setDid("did:plc:me");
        root->setReady(true);

        QVERIFY(root->needFollowing()
                == json_item.toObject().value("except").toObject().value("needFollowing").toBool());
        QVERIFY(root->needFollowers()
                == json_item.toObject().value("except").toObject().value("needFollowers").toBool());

        bool equal = (QJsonDocument::fromJson(root->toString().toUtf8()).object()
                      == json_item.toObject().value("selector").toObject());
        if (!equal) {
            qDebug().noquote().nospace() << root->toString();
        }
        QVERIFY(equal);

        int i = 0;
        for (const auto data : json_item.toObject().value("data").toArray()) {
            QVERIFY2(root->judge(data.toObject().value("payload").toObject())
                             == data.toObject().value("except").toBool(),
                     QString("%1(%2)").arg(description).arg(i++).toLocal8Bit());
        }

        delete root;
    }
}

void realtime_test::test_FirehoseReceiver()
{
    FirehoseReceiver *recv = FirehoseReceiver::getInstance();

    QObject parent1;
    QObject parent2;

    recv->appendSelector(AbstractPostSelector::create(
            QJsonDocument::fromJson("{\"me\":{}}").object(), &parent1));
    recv->appendSelector(AbstractPostSelector::create(
            QJsonDocument::fromJson("{\"followers\":{}}").object(), &parent2));

    QVERIFY(recv->countSelector() == 2);

    const AbstractPostSelector *s1 = recv->getSelector(&parent1);
    const AbstractPostSelector *s2 = recv->getSelector(&parent2);

    QVERIFY(s1 != nullptr);
    QVERIFY(s2 != nullptr);

    QVERIFY(s1->needFollowers() == false);
    QVERIFY(s1->needFollowing() == false);
    QVERIFY(s2->needFollowers() == true);
    QVERIFY(s2->needFollowing() == false);

    recv->removeSelector(&parent1);
    QVERIFY(recv->countSelector() == 1);

    QVERIFY(recv->getSelector(&parent1) == nullptr);
    QVERIFY(recv->getSelector(&parent2) != nullptr);

    QVERIFY(recv->containsSelector(&parent1) == false);
    QVERIFY(recv->containsSelector(&parent2) == true);
}

QList<UserInfo> realtime_test::extractFromArray(const QJsonArray &array) const
{
    QList<UserInfo> ret;
    for (const auto item : array) {
        UserInfo user;
        user.did = item.toObject().value("did").toString();
        user.rkey = item.toObject().value("rkey").toString();
        ret.append(user);
    }
    return ret;
}

QTEST_MAIN(realtime_test)

#include "tst_realtime_test.moc"
