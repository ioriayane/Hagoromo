#include <QtTest>
#include <QCoreApplication>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>

#include "webserver.h"
#include "log/logmanager.h"

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
    void test_LogManager_select();
    void test_LogManager_statistics();

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

void log_test::test_LogManager_select()
{
    LogManager manager;
    QString did("did:plc:log_manager_test");

    {
        QSignalSpy spy(&manager, SIGNAL(finishedSelection(const QString &)));
        emit manager.selectRecords(did, 0, "2024/04/18", QString(), 5);
        spy.wait();
        QVERIFY2(spy.count() == 1, QString("spy.count()=%1").arg(spy.count()).toUtf8());

        QList<QVariant> arguments = spy.takeFirst();

        QFile file(":/data/repo/select_1.json");
        QVERIFY(file.open(QFile::ReadOnly));
        QJsonDocument except = QJsonDocument::fromJson(file.readAll());
        file.close();

        bool check = (except.object()
                      == QJsonDocument::fromJson(arguments.at(0).toString().toUtf8()).object());
        if (!check) {
            qDebug().noquote().nospace()
                    << "except:" << except.toJson(QJsonDocument::JsonFormat::Compact);
            qDebug().noquote().nospace() << "actual:" << arguments.at(0).toString();
        }
        QVERIFY(check);
    }

    {
        QSignalSpy spy(&manager, SIGNAL(finishedSelection(const QString &)));
        emit manager.selectRecords(did, 0, "2024/04/18", "2024-04-18T17:17:47.225Z", 5);
        spy.wait();
        QVERIFY2(spy.count() == 1, QString("spy.count()=%1").arg(spy.count()).toUtf8());

        QList<QVariant> arguments = spy.takeFirst();

        QFile file(":/data/repo/select_2.json");
        QVERIFY(file.open(QFile::ReadOnly));
        QJsonDocument except = QJsonDocument::fromJson(file.readAll());
        file.close();

        bool check = (except.object()
                      == QJsonDocument::fromJson(arguments.at(0).toString().toUtf8()).object());
        if (!check) {
            qDebug().noquote().nospace()
                    << "except:" << except.toJson(QJsonDocument::JsonFormat::Compact);
            qDebug().noquote().nospace() << "actual:" << arguments.at(0).toString();
        }
        QVERIFY(check);
    }

    {
        QSignalSpy spy(&manager, SIGNAL(finishedSelection(const QString &)));
        emit manager.selectRecords(did, 1, "2023/07", QString(), 5);
        spy.wait();
        QVERIFY2(spy.count() == 1, QString("spy.count()=%1").arg(spy.count()).toUtf8());

        QList<QVariant> arguments = spy.takeFirst();

        QFile file(":/data/repo/select_3.json");
        QVERIFY(file.open(QFile::ReadOnly));
        QJsonDocument except = QJsonDocument::fromJson(file.readAll());
        file.close();

        bool check = (except.object()
                      == QJsonDocument::fromJson(arguments.at(0).toString().toUtf8()).object());
        if (!check) {
            qDebug().noquote().nospace()
                    << "except:" << except.toJson(QJsonDocument::JsonFormat::Compact);
            qDebug().noquote().nospace() << "actual:" << arguments.at(0).toString();
        }
        QVERIFY(check);
    }
}

void log_test::test_LogManager_statistics()
{
    LogManager manager;
    QString did("did:plc:log_manager_test");

    QFile file(":/data/repo/statistics.json");
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
        emit manager.statistics(did);
        spy.wait();
        QVERIFY2(spy.count() == 1, QString("spy.count()=%1").arg(spy.count()).toUtf8());

        QList<QVariant> arguments = spy.takeFirst();
        QList<TotalItem> list = qvariant_cast<QList<TotalItem>>(arguments.at(0));

        QVERIFY2(except.length() == list.length(),
                 QString("%1 == %2").arg(except.length()).arg(list.length()).toLocal8Bit());
        for (int i = 0; i < list.length(); i++) {
            QVERIFY2(except.at(i).name == list.at(i).name,
                     QString("%1, %2 == %3")
                             .arg(i)
                             .arg(except.at(i).name)
                             .arg(list.at(0).name)
                             .toLocal8Bit());
            QVERIFY2(except.at(i).count == list.at(i).count,
                     QString("%1, %2 == %3")
                             .arg(i)
                             .arg(except.at(i).count)
                             .arg(list.at(0).count)
                             .toLocal8Bit());
        }
    }
}

QTEST_MAIN(log_test)

#include "tst_log_test.moc"
