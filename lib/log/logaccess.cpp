#include "logaccess.h"
#include "tools/cardecoder.h"

#include <QCoreApplication>
#include <QDebug>
#include <QJsonDocument>
#include <QStandardPaths>
#include <QtSql/QSqlDatabase>
#include <QtSql/QSqlQuery>
#include <QtSql/QSqlError>
#include <QFile>

#include <QDateTime>
#include <QThread>
#define LOG_DATETIME QDateTime::currentDateTime().toString("yyyy/MM/dd hh:mm:ss.zzz")

LogAccess::LogAccess(QObject *parent) : QObject { parent } { }

LogAccess::~LogAccess() { }

void LogAccess::removeDbFile(const QString &did)
{
    QString path = dbPath(did);
    if (QFile::exists(path)) {
        QFile::remove(path);
    }
}

void LogAccess::updateDb(const QString &did, const QByteArray &data)
{
    qDebug().noquote() << LOG_DATETIME << "updateDb" << did;
    if (did.isEmpty()) {
        emit finishedUpdateDb(false);
        return;
    }
    bool ret = false;
    dbInit();
    if (dbOpen(did)) {
        if (dbCreateTable()) {

            CarDecoder decoder;
            decoder.setContent(data);

            QStringList saved_cids = dbGetSavedCids();
            for (const auto &cid : decoder.cids()) {
                if (!saved_cids.contains(cid)) {
                    if (!dbInsertRecord(decoder.uri(cid), cid, decoder.type(cid),
                                        decoder.json(cid))) {
                        emit finishedUpdateDb(false);
                        break;
                    }
                }
            }
            ret = true;
        }
        dbClose();
    }
    dbRelease();
    emit finishedUpdateDb(ret);
}

void LogAccess::dailyTotals(const QString &did)
{
    QList<TotalItem> list;
    qDebug().noquote() << LOG_DATETIME << "dailyTotals" << did;
    if (did.isEmpty()) {
        emit finishedTotals(list);
        return;
    }
    dbInit();
    if (dbOpen(did)) {
        list = dbMakeDailyTotals();
        dbClose();
    }
    dbRelease();
    emit finishedTotals(list);
}

void LogAccess::monthlyTotals(const QString &did)
{
    QList<TotalItem> list;
    qDebug().noquote() << LOG_DATETIME << "monthlyTotals" << did;
    if (did.isEmpty()) {
        emit finishedTotals(list);
        return;
    }
    dbInit();
    if (dbOpen(did)) {
        list = dbMakeMonthlyTotals();
        dbClose();
    }
    dbRelease();
    emit finishedTotals(list);
}

void LogAccess::selectRecords(const QString &did, const int type, const QString &condition,
                              const QString &cursor, const int limit)
{
    QString records;
    qDebug().noquote() << LOG_DATETIME << "selectRecords" << did << type << condition;
    if (did.isEmpty()) {
        emit finishedSelection(records);
        return;
    }
    dbInit();
    if (dbOpen(did)) {
        records = dbSelectRecords(type, condition, cursor, limit);
        dbClose();
    }
    dbRelease();
    emit finishedSelection(records);
}

QString LogAccess::dbPath(QString did)
{
    did.replace(":", "_");
    return QString("%1/%2/%3%4/%5.db")
            .arg(QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation))
            .arg(QCoreApplication::organizationName())
            .arg(QCoreApplication::applicationName())
            .arg(
#if defined(HAGOROMO_UNIT_TEST)
                    QStringLiteral("_unittest")
#elif defined(QT_DEBUG)
                    QStringLiteral("_debug")
#else
                    QString()
#endif
                            )
            .arg(did);
}

void LogAccess::dbInit()
{
    m_dbConnectionName = QString("tech.relog.hagoromo.%1")
                                 .arg(QString::number((uintptr_t)(QThread::currentThreadId())));
    qDebug().noquote() << LOG_DATETIME << "dbInit()" << m_dbConnectionName;
    QSqlDatabase::addDatabase("QSQLITE", m_dbConnectionName);
}

void LogAccess::dbRelease()
{
    qDebug().noquote() << LOG_DATETIME << "dbRelease()";
    QSqlDatabase::removeDatabase(m_dbConnectionName);
}

bool LogAccess::dbOpen(const QString &did)
{
    qDebug().noquote() << LOG_DATETIME << "dbOpen()";
    if (did.isEmpty())
        return false;

    QString path = dbPath(did);
    QSqlDatabase db = QSqlDatabase::database(m_dbConnectionName);
    db.setDatabaseName(path);
    if (!db.open()) {
        qWarning() << db.lastError();
        return false;
    }
    qDebug().noquote() << LOG_DATETIME << "open db:" << path;
    return true;
}

void LogAccess::dbClose()
{
    qDebug().noquote() << LOG_DATETIME << "dbClose()";
    QSqlDatabase db = QSqlDatabase::database(m_dbConnectionName);
    db.close();
}

bool LogAccess::dbCreateTable()
{
    qDebug().noquote() << LOG_DATETIME << "dbCreateTable()";
    bool ret = true;
    QStringList sqls;
    sqls.append("CREATE TABLE IF NOT EXISTS record("
                "cid TEXT NOT NULL PRIMARY KEY, "
                "uri TEXT, "
                "day TEXT, "
                "month TEXT, "
                "createdAt DATETIME, "
                "type TEXT NOT NULL, "
                "record TEXT NOT NULL, "
                "view TEXT "
                ")");
    // sqls.append("CREATE TABLE IF NOT EXISTS mention("
    //             "did TEXT NOT NULL, "
    //             "handle TEXT NOT NULL, "
    //             "uri TEXT NOT NULL "
    //             ")");
    // sqls.append("CREATE TABLE IF NOT EXISTS tag("
    //             "tag TEXT NOT NULL, "
    //             "uri TEXT NOT NULL "
    //             ")");

    QSqlQuery query(QSqlDatabase::database(m_dbConnectionName));
    for (const auto &sql : sqls) {
        if (query.prepare(sql)) {
            if (!query.exec()) {
                qWarning().noquote() << LOG_DATETIME << query.lastError();
                qDebug().noquote() << LOG_DATETIME << query.lastQuery() << query.boundValues();
                ret = false;
            }
        } else {
            qWarning().noquote() << LOG_DATETIME << query.lastError();
            ret = false;
        }
    }

    return ret;
}

bool LogAccess::dbInsertRecord(const QString &uri, const QString &cid, const QString &type,
                               const QJsonObject &json)
{
    bool ret = true;
    QString created_at_str = json.value("createdAt").toString();
    QDateTime created_at = QDateTime::fromString(created_at_str, Qt::ISODateWithMs);

    QSqlQuery query(QSqlDatabase::database(m_dbConnectionName));
    if (query.prepare("INSERT INTO record(cid, uri, day, month, createdAt, type, record, view)"
                      " VALUES (?, ?, ?, ?, ?, ?, ?, ?)")) {
        query.addBindValue(cid);
        query.addBindValue(uri);
        if (!created_at_str.isEmpty() && created_at.isValid()) {
            query.addBindValue(created_at.toUTC().toString("yyyy/MM/dd"));
            query.addBindValue(created_at.toUTC().toString("yyyy/MM"));
            query.addBindValue(created_at_str);
        } else {
            query.addBindValue(QVariant());
            query.addBindValue(QVariant());
            query.addBindValue(QVariant());
        }
        query.addBindValue(type);
        query.addBindValue(QJsonDocument(json).toJson(QJsonDocument::Compact));
        query.addBindValue(QVariant());
        if (query.exec()) {
            // qInfo() << query.lastInsertId().toLongLong() << "added";
        } else {
            qWarning().noquote() << LOG_DATETIME << query.lastError();
            qInfo().noquote() << LOG_DATETIME << query.lastQuery() << query.boundValues();
            ret = false;
        }
    } else {
        qWarning().noquote() << LOG_DATETIME << query.lastError();
        ret = false;
    }
    return ret;
}

bool LogAccess::dbSelect(QSqlQuery &query, const QString &sql) const
{
    bool ret = false;
    if (query.prepare(sql)) {
        if (query.exec()) {
            ret = true;
        } else {
            qWarning().noquote() << LOG_DATETIME << query.lastError();
            qInfo().noquote() << LOG_DATETIME << query.lastQuery() << query.boundValues();
        }
    } else {
        qWarning().noquote() << LOG_DATETIME << query.lastError();
    }
    return ret;
}

QStringList LogAccess::dbGetSavedCids() const
{
    QStringList cids;
    QSqlQuery query(QSqlDatabase::database(m_dbConnectionName));
    if (dbSelect(query, "SELECT cid FROM record")) {
        while (query.next()) {
            cids.append(query.value(0).toString());
        }
    }
    return cids;
}

QList<TotalItem> LogAccess::dbMakeDailyTotals() const
{
    QList<TotalItem> list;
    QSqlQuery query(QSqlDatabase::database(m_dbConnectionName));
    if (dbSelect(query,
                 "SELECT day, count(day) FROM record"
                 " WHERE day NOTNULL AND type = 'app.bsky.feed.post' "
                 " GROUP BY day ORDER BY day DESC")) {
        while (query.next()) {
            TotalItem item;
            item.name = query.value(0).toString();
            item.count = query.value(1).toInt();
            list.append(item);
        }
    }
    return list;
}

QList<TotalItem> LogAccess::dbMakeMonthlyTotals() const
{
    QList<TotalItem> list;
    QSqlQuery query(QSqlDatabase::database(m_dbConnectionName));
    if (dbSelect(query,
                 "SELECT month, count(month) FROM record"
                 " WHERE month NOTNULL AND type = 'app.bsky.feed.post' "
                 " GROUP BY month ORDER BY month DESC")) {
        while (query.next()) {
            TotalItem item;
            item.name = query.value(0).toString();
            item.count = query.value(1).toInt();
            list.append(item);
        }
    }
    return list;
}

QString LogAccess::dbSelectRecords(const int type, const QString &condition, const QString &cursor,
                                   const int limit) const
{
    QString sql("SELECT uri, cid, record, createdAt FROM record"
                " WHERE type = 'app.bsky.feed.post'");
    if (!condition.isEmpty()) {
        if (type == 0) {
            sql.append(" AND day == '").append(condition).append("'");
        } else if (type == 1) {
            sql.append(" AND month == '").append(condition).append("'");
        }
    }
    if (!cursor.isEmpty()) {
        sql.append(" AND createdAt < '").append(cursor).append("'");
    }
    sql.append(" ORDER BY createdAt DESC");
    if (limit > 0) {
        sql.append(" LIMIT ").append(QString::number(limit));
    } else {
        sql.append(" LIMIT 50");
    }

    QSqlQuery query(QSqlDatabase::database(m_dbConnectionName));
    if (dbSelect(query, sql)) {
        QStringList records;
        QString record_base("{\"uri\": \"%1\", \"cid\": \"%2\", \"value\": %3}");
        QString last_created_at;
        while (query.next()) {
            records.append(record_base.arg(query.value(0).toString())
                                   .arg(query.value(1).toString())
                                   .arg(query.value(2).toString()));
            last_created_at = query.value(3).toString();
        }
        return QString("{\"records\": [%1], \"cursor\":\"%2\"}")
                .arg(records.join(","))
                .arg(last_created_at);
    }
    return QString("{\"records\": []}");
}
