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

#define REG_EXP_WHITE_SPACE                                                                        \
    QStringLiteral("[ \\f\\n\\r\\t\\v%1%2%3-%4%5%6%7%8%9%10]")                                     \
            .arg(QChar(0x00a0))                                                                    \
            .arg(QChar(0x1680))                                                                    \
            .arg(QChar(0x2000))                                                                    \
            .arg(QChar(0x200a))                                                                    \
            .arg(QChar(0x2028))                                                                    \
            .arg(QChar(0x2029))                                                                    \
            .arg(QChar(0x202f))                                                                    \
            .arg(QChar(0x205f))                                                                    \
            .arg(QChar(0x3000))                                                                    \
            .arg(QChar(0xfeff))

LogAccess::LogAccess(QObject *parent) : QObject { parent } { }

LogAccess::~LogAccess() { }

void LogAccess::removeDbFile(const QString &did)
{
    qDebug().noquote() << LOG_DATETIME << "Remove database file" << did;
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

            qDebug().noquote() << LOG_DATETIME << "Decording repository data...";
            emit progressMessage("Decording repository data ...");
            CarDecoder decoder;
            decoder.setContent(data);

            qDebug().noquote() << LOG_DATETIME << "Insert repository data...";
            emit progressMessage("Updating local databse ...");
            int i = 0;
            QStringList saved_cids = dbGetSavedCids();
            for (const auto &cid : decoder.cids()) {
                if (i++ % 100 == 0) {
                    emit progressMessage(
                            QString("Updating local databse ... (%1%)")
                                    .arg(static_cast<int>(100 * i / decoder.cids().length())));
                }
                if (!saved_cids.contains(cid)) {
                    if (!dbInsertRecord(decoder.uri(cid), cid, decoder.type(cid),
                                        decoder.json(cid))) {
                        emit finishedUpdateDb(false);
                        break;
                    }
                }
            }
            emit progressMessage(QString("Updating local databse ... (100%)"));

            // 保存されているけど取得したデータにないものは消す
            qDebug().noquote() << LOG_DATETIME << "Checking deleted data...";
            emit progressMessage("Checking deleted data ...");
            for (const auto &cid : saved_cids) {
                if (!decoder.cids().contains(cid)) {
                    if (!dbDeleteRecord(cid)) {
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
    int max = 0;
    QList<TotalItem> list;
    qDebug().noquote() << LOG_DATETIME << "dailyTotals" << did;
    if (did.isEmpty()) {
        emit finishedTotals(list, max);
        return;
    }
    dbInit();
    if (dbOpen(did)) {
        list = dbMakeDailyTotals(max);
        dbClose();
    }
    dbRelease();
    emit finishedTotals(list, max);
}

void LogAccess::monthlyTotals(const QString &did)
{
    int max = 0;
    QList<TotalItem> list;
    qDebug().noquote() << LOG_DATETIME << "monthlyTotals" << did;
    if (did.isEmpty()) {
        emit finishedTotals(list, max);
        return;
    }
    dbInit();
    if (dbOpen(did)) {
        list = dbMakeMonthlyTotals(max);
        dbClose();
    }
    dbRelease();
    emit finishedTotals(list, max);
}

void LogAccess::statistics(const QString &did)
{
    QList<TotalItem> list;
    qDebug().noquote() << LOG_DATETIME << "statistics" << did;
    if (did.isEmpty()) {
        emit finishedTotals(list, 0);
        return;
    }
    dbInit();
    if (dbOpen(did)) {
        list = dbMakeStatistics();
        dbClose();
    }
    dbRelease();
    emit finishedTotals(list, 0);
}

// kind, 0:daily, 1:montyly, 2:words
void LogAccess::selectRecords(const QString &did, const int kind, const QString &condition,
                              const QString &cursor, const int limit)
{
    QString records;
    QStringList view_posts;
    qDebug().noquote() << LOG_DATETIME << "selectRecords" << did << kind << condition;
    if (did.isEmpty()) {
        emit finishedSelection(records, view_posts);
        return;
    }
    dbInit();
    if (dbOpen(did)) {
        records = dbSelectRecords(kind, condition, cursor, limit, view_posts);
        dbClose();
    }
    dbRelease();
    emit finishedSelection(records, view_posts);
}

void LogAccess::updateRecords(const QString &did, const QList<RecordPostItem> &record_post_items)
{
    qDebug().noquote() << LOG_DATETIME << "updateRecords" << did
                       << "count:" << record_post_items.length();
    if (did.isEmpty() || record_post_items.isEmpty()) {
        emit finishedUpdateRecords();
        return;
    }
    dbInit();
    if (dbOpen(did)) {
        dbUpdateRecords(record_post_items);
        dbClose();
    }
    dbRelease();
    emit finishedUpdateRecords();
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
                "parent_uri TEXT, "
                "name TEXT, "
                "day TEXT, "
                "month TEXT, "
                "createdAt DATETIME, "
                "text TEXT, "
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
    QString parent_uri = json.value("list").toString();
    QString name = json.value("name").toString();
    QString text = json.value("text").toString();
    QString created_at_str = json.value("createdAt").toString();
    QDateTime created_at = QDateTime::fromString(created_at_str, Qt::ISODateWithMs).toLocalTime();

    QSqlQuery query(QSqlDatabase::database(m_dbConnectionName));
    if (query.prepare("INSERT INTO record(cid, uri, parent_uri, name,"
                      " day, month, createdAt, text, type,"
                      " record, view)"
                      " VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?)")) {
        query.addBindValue(cid);
        query.addBindValue(uri);
        query.addBindValue(parent_uri);
        query.addBindValue(name);
        if (!created_at_str.isEmpty() && created_at.isValid()) {
            // 集計結果の見た目や検索条件に使うのでローカルタイム
            query.addBindValue(created_at.toString("yyyy/MM/dd"));
            query.addBindValue(created_at.toString("yyyy/MM"));
            // カーソルで使うのでrecordの値を同じ
            query.addBindValue(created_at_str);
        } else {
            query.addBindValue(QVariant());
            query.addBindValue(QVariant());
            query.addBindValue(QVariant());
        }
        query.addBindValue(text);
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

bool LogAccess::dbDeleteRecord(const QString &cid)
{
    bool ret = true;
    QSqlQuery query(QSqlDatabase::database(m_dbConnectionName));
    if (query.prepare("DELETE FROM record WHERE cid = ?")) {
        query.addBindValue(cid);
        if (!query.exec()) {
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

QList<TotalItem> LogAccess::dbMakeDailyTotals(int &max) const
{
    max = 0;
    QList<TotalItem> list;
    QSqlQuery query(QSqlDatabase::database(m_dbConnectionName));
    if (dbSelect(query,
                 "SELECT day, count(day) FROM record"
                 " WHERE day NOTNULL AND type = 'app.bsky.feed.post'"
                 " GROUP BY day ORDER BY day DESC")) {
        while (query.next()) {
            TotalItem item;
            item.name = query.value(0).toString();
            item.count = query.value(1).toInt();
            max = (item.count > max) ? item.count : max;
            list.append(item);
        }
    }
    return list;
}

QList<TotalItem> LogAccess::dbMakeMonthlyTotals(int &max) const
{
    max = 0;
    QList<TotalItem> list;
    QSqlQuery query(QSqlDatabase::database(m_dbConnectionName));
    if (dbSelect(query,
                 "SELECT month, count(month) FROM record"
                 " WHERE month NOTNULL AND type = 'app.bsky.feed.post'"
                 " GROUP BY month ORDER BY month DESC")) {
        while (query.next()) {
            TotalItem item;
            item.name = query.value(0).toString();
            item.count = query.value(1).toInt();
            max = (item.count > max) ? item.count : max;
            list.append(item);
        }
    }
    return list;
}

QList<TotalItem> LogAccess::dbMakeStatistics() const
{
    QStringList types;
    types << "app.bsky.feed.post"
          << "number.of.days.posts"
          << "average.number.of.daily.posts"
          << "app.bsky.feed.like"
          << "number.of.days.like"
          << "average.number.of.daily.like"
          << "app.bsky.feed.repost"
          << "number.of.days.repost"
          << "average.number.of.daily.repost"
          << "app.bsky.graph.list"
          << "app.bsky.graph.listitem"
          << "app.bsky.graph.follow"
          << "app.bsky.feed.threadgate"
          << "app.bsky.graph.block";
    QHash<QString, TotalItem> type2name;
    type2name["app.bsky.feed.post"] = TotalItem(tr("Post"), tr("Total number of posts"));
    type2name["number.of.days.posts"] = TotalItem(tr("Post"), tr("Number of days posts"));
    type2name["average.number.of.daily.posts"] =
            TotalItem(tr("Post"), tr("Average number of daily posts"));
    type2name["app.bsky.feed.like"] = TotalItem(tr("Like"), tr("Total number of likes"));
    type2name["number.of.days.like"] = TotalItem(tr("Like"), tr("Number of days likes"));
    type2name["average.number.of.daily.like"] =
            TotalItem(tr("Like"), tr("Average number of daily likes"));
    type2name["app.bsky.feed.repost"] = TotalItem(tr("Repost"), tr("Total number of reposts"));
    type2name["number.of.days.repost"] = TotalItem(tr("Repost"), tr("Number of days reposts"));
    type2name["average.number.of.daily.repost"] =
            TotalItem(tr("Repost"), tr("Average number of daily reposts"));
    type2name["app.bsky.graph.list"] = TotalItem(tr("List"), tr("Total number of lists"));
    type2name["app.bsky.graph.listitem"] = TotalItem(tr("List"), tr("Total number of list items"));
    type2name["app.bsky.graph.follow"] = TotalItem(tr("Other"), tr("Total number of follows"));
    type2name["app.bsky.feed.threadgate"] =
            TotalItem(tr("Other"), tr("Total number of who can reply"));
    type2name["app.bsky.graph.block"] = TotalItem(tr("Other"), tr("Total number of blocks"));

    QList<TotalItem> list;
    for (int i = 0; i < types.length(); i++) {
        list.append(type2name.value(types.at(i)));
    }

    {
        QSqlQuery query(QSqlDatabase::database(m_dbConnectionName));
        if (dbSelect(query,
                     "SELECT type, count(type) FROM record"
                     " WHERE type != '$car_address'"
                     " GROUP BY type")) {
            while (query.next()) {
                int i = types.indexOf(query.value(0).toString());
                if (i >= 0) {
                    list[i].count = query.value(1).toInt();
                }
            }
        }
    }

    QMap<QString, QString> type2sql;
    type2sql["number.of.days.posts"] =
            "SELECT count(type) FROM("
            "SELECT day, type FROM record WHERE type = 'app.bsky.feed.post' GROUP BY day"
            ") GROUP BY type";
    type2sql["average.number.of.daily.posts"] =
            "SELECT avg(day_count) FROM("
            "SELECT type, day, count(day) AS day_count "
            " FROM record WHERE type = 'app.bsky.feed.post' GROUP BY day"
            ") GROUP BY type";
    type2sql["number.of.days.like"] =
            "SELECT count(type) FROM("
            "SELECT day, type FROM record WHERE type = 'app.bsky.feed.like' GROUP BY day"
            ") GROUP BY type";
    type2sql["average.number.of.daily.like"] =
            "SELECT avg(day_count) FROM("
            "SELECT type, day, count(day) AS day_count "
            " FROM record WHERE type = 'app.bsky.feed.like' GROUP BY day"
            ") GROUP BY type";
    type2sql["number.of.days.repost"] =
            "SELECT count(type) FROM("
            "SELECT day, type FROM record WHERE type = 'app.bsky.feed.repost' GROUP BY day"
            ") GROUP BY type";
    type2sql["average.number.of.daily.repost"] =
            "SELECT avg(day_count) FROM("
            "SELECT type, day, count(day) AS day_count "
            " FROM record WHERE type = 'app.bsky.feed.repost' GROUP BY day"
            ") GROUP BY type";

    for (QMap<QString, QString>::const_iterator it = type2sql.cbegin(); it != type2sql.cend();
         it++) {
        QSqlQuery query(QSqlDatabase::database(m_dbConnectionName));
        if (dbSelect(query, it.value())) {
            while (query.next()) {
                int i = types.indexOf(it.key());
                if (i >= 0) {
                    list[i].count = query.value(0).toInt();
                }
                break;
            }
        }
    }

    {
        QString sql = "SELECT parent.cid, parent.name, count(parent.cid) FROM record AS parent "
                      " INNER JOIN record AS child ON parent.uri = child.parent_uri"
                      " WHERE parent.type = 'app.bsky.graph.list'"
                      " AND child.type = 'app.bsky.graph.listitem'"
                      " GROUP BY parent.cid ORDER BY parent.name";
        int offset = 1;
        int i = types.indexOf("app.bsky.graph.listitem");
        if (i >= 0) {
            QSqlQuery query(QSqlDatabase::database(m_dbConnectionName));
            if (dbSelect(query, sql)) {
                while (query.next()) {
                    TotalItem item;
                    item.group = tr("Number of registrations for list");
                    item.name = query.value(1).toString();
                    item.count = query.value(2).toInt();
                    list.insert(i + offset, item);
                    offset++;
                }
            }
        }
    }
    return list;
}

QString LogAccess::dbSelectRecords(const int kind, const QString &condition, const QString &cursor,
                                   const int limit, QStringList &view_posts) const
{
    QString sql("SELECT uri, cid, record, createdAt, view FROM record"
                " WHERE type = 'app.bsky.feed.post'");
    if (!condition.isEmpty()) {
        if (kind == 0) {
            sql.append(" AND day == '").append(condition).append("'");
        } else if (kind == 1) {
            sql.append(" AND month == '").append(condition).append("'");
        } else if (kind == 2) {
            const QStringList words =
                    condition.trimmed().split(QRegularExpression(REG_EXP_WHITE_SPACE));
            for (const auto &word : words) {
                sql.append(" AND text like '%").append(word).append("%'");
            }
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
        QString view;
        QString uri;
        while (query.next()) {
            uri = query.value(0).toString();
            records.append(record_base.arg(uri)
                                   .arg(query.value(1).toString())
                                   .arg(query.value(2).toString()));
            last_created_at = query.value(3).toString();
            view = query.value(4).toString();
            if (view.isEmpty()) {
                view_posts.append(uri);
            } else {
                view_posts.append(view);
            }
        }
        return QString("{\"records\": [%1], \"cursor\":\"%2\"}")
                .arg(records.join(","))
                .arg(last_created_at);
    }
    return QString("{\"records\": []}");
}

void LogAccess::dbUpdateRecords(const QList<RecordPostItem> &record_post_items)
{
    QString sql;
    for (const auto &item : record_post_items) {
        QSqlQuery query(QSqlDatabase::database(m_dbConnectionName));
        if (query.prepare("UPDATE record"
                          " SET view = ?"
                          " WHERE uri = ?")) {
            query.addBindValue(item.json);
            query.addBindValue(item.uri);
            if (!query.exec()) {
                qWarning().noquote() << LOG_DATETIME << query.lastError();
                qInfo().noquote() << LOG_DATETIME << query.lastQuery() << query.boundValues();
            }
        } else {
            qWarning().noquote() << LOG_DATETIME << query.lastError();
        }
    }
}
