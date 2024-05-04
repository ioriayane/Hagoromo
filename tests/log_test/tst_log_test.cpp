#include <QtTest>
#include <QCoreApplication>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>

#include "webserver.h"
#include <log/logmanager.h>

class log_test : public QObject
{
    Q_OBJECT

public:
    log_test();
    ~log_test();

private slots:
    void initTestCase();
    void cleanupTestCase();
    void test_LogManager();
    void test_LogManager_daily();
    void test_LogManager_monthly();

private:
    WebServer m_mockServer;
    quint16 m_listenPort;
    QString m_service;
};

log_test::log_test()
{
    QCoreApplication::setOrganizationName(QStringLiteral("relog"));
    QCoreApplication::setApplicationName(QStringLiteral("Hagoromo"));

    m_listenPort = m_mockServer.listen(QHostAddress::LocalHost, 0);
    m_service = QString("http://localhost:%1/response").arg(m_listenPort);

    connect(&m_mockServer, &WebServer::receivedPost,
            [=](const QHttpServerRequest &request, bool &result, QString &json) {
                QVERIFY(false);
            });
}

log_test::~log_test() { }

void log_test::initTestCase() { }

void log_test::cleanupTestCase() { }

void log_test::test_LogManager()
{
    LogManager manager;
    QString did("did:plc:log_manager_test");

    manager.clearDb(did);

    {
        QSignalSpy spy(&manager, SIGNAL(finished(bool)));
        manager.update("", "");
        spy.wait();
        QVERIFY2(spy.count() == 1, QString("spy.count()=%1").arg(spy.count()).toUtf8());

        QList<QVariant> arguments = spy.takeFirst();
        QVERIFY(arguments.at(0).toBool() == false);
    }

    {
        QSignalSpy spy(&manager, SIGNAL(finished(bool)));
        manager.update(m_service, did);
        spy.wait();
        QVERIFY2(spy.count() == 1, QString("spy.count()=%1").arg(spy.count()).toUtf8());

        QList<QVariant> arguments = spy.takeFirst();
        QVERIFY(arguments.at(0).toBool() == true);
    }

    {
        QSignalSpy spy(&manager, SIGNAL(finished(bool)));
        manager.update(m_service, did);
        spy.wait();
        QVERIFY2(spy.count() == 1, QString("spy.count()=%1").arg(spy.count()).toUtf8());

        QList<QVariant> arguments = spy.takeFirst();
        QVERIFY(arguments.at(0).toBool() == true);
    }
}

void log_test::test_LogManager_daily()
{
    LogManager manager;
    QString did("did:plc:log_manager_test");

    QFile file(":/data/repo/daily.json");
    QVERIFY(file.open(QFile::ReadOnly));
    QJsonDocument doc = QJsonDocument::fromJson(file.readAll());
    file.close();

    QList<TotalItem> except;
    for (const auto &value : doc.array()) {
        TotalItem item;
        item.name = value.toObject().value("name").toString();
        item.count = value.toObject().value("count").toInt();
        except.append(item);
    }

    {
        QSignalSpy spy(&manager, SIGNAL(finishedTotals(const QList<TotalItem> &)));
        emit manager.dailyTotals(did);
        spy.wait();
        QVERIFY2(spy.count() == 1, QString("spy.count()=%1").arg(spy.count()).toUtf8());

        QList<QVariant> arguments = spy.takeFirst();
        QList<TotalItem> list = qvariant_cast<QList<TotalItem>>(arguments.at(0));

        QVERIFY2(except.length() == list.length(),
                 QString("%1 == %2").arg(except.length()).arg(list.length()).toLocal8Bit());
        for (int i = 0; i < list.length(); i++) {
            QVERIFY2(except.at(i).name == list.at(i).name,
                     QString("%1 == %2").arg(except.at(i).name).arg(list.at(0).name).toLocal8Bit());
            QVERIFY2(except.at(i).count == list.at(i).count,
                     QString("%1 == %2")
                             .arg(except.at(i).count)
                             .arg(list.at(0).count)
                             .toLocal8Bit());
        }
    }
}

void log_test::test_LogManager_monthly()
{
    LogManager manager;
    QString did("did:plc:log_manager_test");

    QFile file(":/data/repo/monthly.json");
    QVERIFY(file.open(QFile::ReadOnly));
    QJsonDocument doc = QJsonDocument::fromJson(file.readAll());
    file.close();

    QList<TotalItem> except;
    for (const auto &value : doc.array()) {
        TotalItem item;
        item.name = value.toObject().value("name").toString();
        item.count = value.toObject().value("count").toInt();
        except.append(item);
    }

    {
        QSignalSpy spy(&manager, SIGNAL(finishedTotals(const QList<TotalItem> &)));
        emit manager.monthlyTotals(did);
        spy.wait();
        QVERIFY2(spy.count() == 1, QString("spy.count()=%1").arg(spy.count()).toUtf8());

        QList<QVariant> arguments = spy.takeFirst();
        QList<TotalItem> list = qvariant_cast<QList<TotalItem>>(arguments.at(0));

        QVERIFY2(except.length() == list.length(),
                 QString("%1 == %2").arg(except.length()).arg(list.length()).toLocal8Bit());
        for (int i = 0; i < list.length(); i++) {
            QVERIFY2(except.at(i).name == list.at(i).name,
                     QString("%1 == %2").arg(except.at(i).name).arg(list.at(0).name).toLocal8Bit());
            QVERIFY2(except.at(i).count == list.at(i).count,
                     QString("%1 == %2")
                             .arg(except.at(i).count)
                             .arg(list.at(0).count)
                             .toLocal8Bit());
        }
    }
}

QTEST_MAIN(log_test)

#include "tst_log_test.moc"
