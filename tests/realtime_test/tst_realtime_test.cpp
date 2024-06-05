#include <QtTest>
#include <QCoreApplication>

#include "realtime/followingpostselector.h"
#include "realtime/followerpostselector.h"
#include "realtime/mepostselector.h"
#include "realtime/andpostselector.h"

class realtime_test : public QObject
{
    Q_OBJECT

public:
    realtime_test();
    ~realtime_test();

private slots:
    void initTestCase();
    void cleanupTestCase();
    void test_FollowingPostSelector();
};

realtime_test::realtime_test() { }

realtime_test::~realtime_test() { }

void realtime_test::initTestCase() { }

void realtime_test::cleanupTestCase() { }

void realtime_test::test_FollowingPostSelector()
{
    AndPostSelector *root = new AndPostSelector(this);
    root->setDid("did:plc:mqxsuw5b5rhpwo4lw6iwlid5");
    root->appendChildSelector(new FollowingPostSelector(root));
    root->appendChildSelector(new FollowerPostSelector(root));
    root->appendChildSelector(new MePostSelector(root));

    {
        QJsonObject obj;
        obj.insert("following", 1);
        obj.insert("follower", 1);
        obj.insert("did", "did:plc:mqxsuw5b5rhpwo4lw6iwlid5");
        QVERIFY(root->judge(obj) == true);
    }
    {
        QJsonObject obj;
        obj.insert("following", 1);
        obj.insert("follower", 1);
        obj.insert("did", "did:plc:hoge");
        QVERIFY(root->judge(obj) == false);
    }
    {
        QJsonObject obj;
        obj.insert("following", 0);
        obj.insert("follower", 1);
        obj.insert("did", "did:plc:mqxsuw5b5rhpwo4lw6iwlid5");
        QVERIFY(root->judge(obj) == false);
    }
    {
        QJsonObject obj;
        obj.insert("following", 1);
        obj.insert("follower", 0);
        obj.insert("did", "did:plc:mqxsuw5b5rhpwo4lw6iwlid5");
        QVERIFY(root->judge(obj) == false);
    }
    {
        QJsonObject obj;
        obj.insert("following", 0);
        obj.insert("follower", 0);
        obj.insert("did", "did:plc:hoge");
        QVERIFY(root->judge(obj) == false);
    }
}

QTEST_MAIN(realtime_test)

#include "tst_realtime_test.moc"
