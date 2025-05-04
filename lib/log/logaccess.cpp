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
#include <QThread>

#include <QDateTime>
#include <QDir>
#include <QFileInfo>
#define LOG_DATETIME QDateTime::currentDateTime().toString("yyyy/MM/dd hh:mm:ss.zzz")

#define DB_VERSION 1

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

class LogAccess::Private
{
public:
    Private(LogAccess *parent);
    ~Private();

    QString dbPath(QString did) const;
    void dbInit();
    void dbRelease() const;
    bool dbOpen(const QString &did) const;
    void dbClose() const;
    bool dbCreateTable() const;
    void dbDropTable() const;
    bool dbInsertRecord(const QString &uri, const QString &cid, const QString &type,
                        const QJsonObject &json) const;
    bool dbDeleteRecord(const QString &cid) const;
    bool dbSelect(QSqlQuery &query, const QString &sql,
                  const QStringList &bind_values = QStringList()) const;
    QStringList dbGetSavedCids() const;
    QList<TotalItem> dbMakeDailyTotals(int &max) const;
    QList<TotalItem> dbMakeMonthlyTotals(int &max) const;
    QList<TotalItem> dbMakeStatistics() const;
    QString dbSelectRecords(const int kind, const QString &condition, const QString &cursor,
                            const int limit, QStringList &view_posts) const;
    void dbUpdateRecords(const QList<RecordPostItem> &record_post_items) const;
    int dbGetVersion() const;
    void dbSetVersion(const int version) const;

private:
    LogAccess *q;

    QString m_dbConnectionName;
};

LogAccess::Private::Private(LogAccess *parent)
{
    qDebug().noquote() << LOG_DATETIME << "LogAccess::Private(" << this << ") Private()";
}

LogAccess::Private::~Private()
{
    qDebug().noquote() << LOG_DATETIME << "LogAccess::Private(" << this << ") ~Private()";
}

LogAccess::LogAccess(QObject *parent) : QObject { parent }, d(new Private(this))
{
    qDebug().noquote() << LOG_DATETIME << this << "LogAccess()";
    connect(this, &QObject::destroyed, [this]() { delete d; });
}

LogAccess::~LogAccess()
{
    qDebug().noquote() << LOG_DATETIME << this << "~HttpAccess()";
}

void LogAccess::removeDbFile(const QString &did)
{
    qDebug().noquote() << LOG_DATETIME << "Remove database file" << did;
    QString path = d->dbPath(did);
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
    d->dbInit();
    if (d->dbOpen(did)) {
        if (d->dbCreateTable()) {
            int version = d->dbGetVersion();
            qDebug().noquote() << LOG_DATETIME << "Check database version:" << version;
            if (version < DB_VERSION) {
                qDebug().noquote() << LOG_DATETIME << "Database version is out of date";
                qDebug().noquote()
                        << LOG_DATETIME << "Update version from " << version << "to " << DB_VERSION;
                d->dbDropTable();
                ret = d->dbCreateTable();
            } else {
                ret = true;
            }

            if (ret) {
                qDebug().noquote() << LOG_DATETIME << "Decording repository data...";
                emit progressMessage("Decording repository data ...");
                CarDecoder decoder;
                decoder.setContent(data);

                qDebug().noquote() << LOG_DATETIME << "Insert repository data...";
                emit progressMessage("Updating local database ...");
                int i = 0;
                const QStringList saved_cids = d->dbGetSavedCids();
                for (const auto &cid : decoder.cids()) {
                    if (i++ % 100 == 0) {
                        emit progressMessage(
                                QString("Updating local database ... (%1%)")
                                        .arg(static_cast<int>(100 * i / decoder.cids().length())));
                    }
                    if (!saved_cids.contains(cid)) {
                        if (!d->dbInsertRecord(decoder.uri(cid), cid, decoder.type(cid),
                                               decoder.json(cid))) {
                            emit finishedUpdateDb(false);
                            break;
                        }
                    }
                }
                emit progressMessage(QString("Updating local database ... (100%)"));

                // 保存されているけど取得したデータにないものは消す
                qDebug().noquote() << LOG_DATETIME << "Checking deleted data...";
                emit progressMessage("Checking deleted data ...");
                for (const auto &cid : saved_cids) {
                    if (!decoder.cids().contains(cid)) {
                        if (!d->dbDeleteRecord(cid)) {
                            emit finishedUpdateDb(false);
                            break;
                        }
                    }
                }
            }
        }
        d->dbClose();
    }
    d->dbRelease();
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
    d->dbInit();
    if (d->dbOpen(did)) {
        list = d->dbMakeDailyTotals(max);
        d->dbClose();
    }
    d->dbRelease();
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
    d->dbInit();
    if (d->dbOpen(did)) {
        list = d->dbMakeMonthlyTotals(max);
        d->dbClose();
    }
    d->dbRelease();
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
    d->dbInit();
    if (d->dbOpen(did)) {
        list = d->dbMakeStatistics();
        d->dbClose();
    }
    d->dbRelease();
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
    d->dbInit();
    if (d->dbOpen(did)) {
        records = d->dbSelectRecords(kind, condition, cursor, limit, view_posts);
        d->dbClose();
    }
    d->dbRelease();
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
    d->dbInit();
    if (d->dbOpen(did)) {
        d->dbUpdateRecords(record_post_items);
        d->dbClose();
    }
    d->dbRelease();
    emit finishedUpdateRecords();
}

void LogAccess::setVersion(const QString &did, const int version)
{
    qDebug().noquote() << LOG_DATETIME << "setVersion" << version;
    d->dbInit();
    if (d->dbOpen(did)) {
        d->dbSetVersion(version);
        d->dbClose();
    }
    d->dbRelease();
}

int LogAccess::getVersion(const QString &did)
{
    int version = -1;
    qDebug().noquote() << LOG_DATETIME << "getVersion";
    d->dbInit();
    if (d->dbOpen(did)) {
        version = d->dbGetVersion();
        d->dbClose();
    }
    d->dbRelease();
    return version;
}

QString LogAccess::Private::dbPath(QString did) const
{
    did.replace(":", "_");
    QString path =
            QString("%1/%2/%3%4/%5.db")
                    .arg(QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation))
                    .arg(QCoreApplication::organizationName())
                    .arg(QCoreApplication::applicationName())
                    .arg(
// #if defined(HAGOROMO_UNIT_TEST)
//                     QStringLiteral("_unittest")
// #elif defined(QT_DEBUG)
#if defined(QT_DEBUG)
                            QStringLiteral("_debug")
#else
                            QString()
#endif
                                    )
                    .arg(did);
    QFileInfo file(path);
    QDir dir(file.absolutePath());
    if (!dir.exists()) {
        dir.mkpath(file.absolutePath());
    }
    return path;
}

void LogAccess::Private::dbInit()
{
    m_dbConnectionName = QString("tech.relog.hagoromo.%1")
                                 .arg(QString::number((uintptr_t)(QThread::currentThreadId())));
    qDebug().noquote() << LOG_DATETIME << "dbInit()" << m_dbConnectionName;
    QSqlDatabase::addDatabase("QSQLITE", m_dbConnectionName);
}

void LogAccess::Private::dbRelease() const
{
    qDebug().noquote() << LOG_DATETIME << "dbRelease()";
    QSqlDatabase::removeDatabase(m_dbConnectionName);
}

bool LogAccess::Private::dbOpen(const QString &did) const
{
    qDebug().noquote() << LOG_DATETIME << "dbOpen()";
    if (did.isEmpty())
        return false;

    QString path = dbPath(did);
    QSqlDatabase db = QSqlDatabase::database(m_dbConnectionName);
    db.setDatabaseName(path);
    if (!db.open()) {
        qDebug() << db.lastError();
        return false;
    }
    qDebug().noquote() << LOG_DATETIME << "open db:" << path;
    return true;
}

void LogAccess::Private::dbClose() const
{
    qDebug().noquote() << LOG_DATETIME << "dbClose()";
    QSqlDatabase db = QSqlDatabase::database(m_dbConnectionName);
    db.close();
}

bool LogAccess::Private::dbCreateTable() const
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
    sqls.append("CREATE TABLE IF NOT EXISTS inform("
                "version INTEGER NOT NULL "
                ")");
    QSqlDatabase db = QSqlDatabase::database(m_dbConnectionName);
    bool isempty = db.tables().isEmpty();
    QSqlQuery query(db);
    for (const auto &sql : sqls) {
        if (query.prepare(sql)) {
            if (!query.exec()) {
                qDebug().noquote() << LOG_DATETIME << query.lastError();
                qDebug().noquote() << LOG_DATETIME << query.lastQuery() << query.boundValues();
                ret = false;
            }
        } else {
            qDebug().noquote() << LOG_DATETIME << query.lastError();
            ret = false;
        }
    }

    if (isempty) {
        qDebug().noquote() << LOG_DATETIME << "Set version" << DB_VERSION << "(new tables)";
        dbSetVersion(DB_VERSION);
    }

    return ret;
}

void LogAccess::Private::dbDropTable() const
{
    QSqlDatabase db = QSqlDatabase::database(m_dbConnectionName);
    for (const auto &name : db.tables()) {
        QSqlQuery query(db);
        if (query.prepare("DROP TABLE IF EXISTS " + name)) {
            if (!query.exec()) {
                qDebug().noquote() << LOG_DATETIME << query.lastError();
                qDebug().noquote() << LOG_DATETIME << query.lastQuery() << query.boundValues();
            }
        } else {
            qDebug().noquote() << LOG_DATETIME << query.lastError();
        }
    }
}

bool LogAccess::Private::dbInsertRecord(const QString &uri, const QString &cid, const QString &type,
                                        const QJsonObject &json) const
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
            // qDebug() << query.lastInsertId().toLongLong() << "added";
        } else {
            qDebug().noquote() << LOG_DATETIME << query.lastError();
            qDebug().noquote() << LOG_DATETIME << query.lastQuery() << query.boundValues();
            ret = false;
        }
    } else {
        qDebug().noquote() << LOG_DATETIME << query.lastError();
        ret = false;
    }
    return ret;
}

bool LogAccess::Private::dbDeleteRecord(const QString &cid) const
{
    bool ret = true;
    QSqlQuery query(QSqlDatabase::database(m_dbConnectionName));
    if (query.prepare("DELETE FROM record WHERE cid = ?")) {
        query.addBindValue(cid);
        if (!query.exec()) {
            qDebug().noquote() << LOG_DATETIME << query.lastError();
            qDebug().noquote() << LOG_DATETIME << query.lastQuery() << query.boundValues();
            ret = false;
        }
    } else {
        qDebug().noquote() << LOG_DATETIME << query.lastError();
        ret = false;
    }
    return ret;
}

bool LogAccess::Private::dbSelect(QSqlQuery &query, const QString &sql,
                                  const QStringList &bind_values) const
{
    bool ret = false;
    if (query.prepare(sql)) {
        for (const auto &bind_value : bind_values) {
            query.addBindValue(bind_value);
        }
        if (query.exec()) {
            ret = true;
        } else {
            qDebug().noquote() << LOG_DATETIME << query.lastError();
            qDebug().noquote() << LOG_DATETIME << query.lastQuery() << query.boundValues();
        }
    } else {
        qDebug().noquote() << LOG_DATETIME << query.lastError();
    }
    return ret;
}

QStringList LogAccess::Private::dbGetSavedCids() const
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

QList<TotalItem> LogAccess::Private::dbMakeDailyTotals(int &max) const
{
    max = 0;
    QList<TotalItem> list;
    QSqlQuery query(QSqlDatabase::database(m_dbConnectionName));
    if (dbSelect(query,
                 "SELECT day, count(day) FROM record"
                 " WHERE day NOTNULL "
                 " AND (type = 'app.bsky.feed.post' OR type = 'app.bsky.feed.repost')"
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

QList<TotalItem> LogAccess::Private::dbMakeMonthlyTotals(int &max) const
{
    max = 0;
    QList<TotalItem> list;
    QSqlQuery query(QSqlDatabase::database(m_dbConnectionName));
    if (dbSelect(query,
                 "SELECT month, count(month) FROM record"
                 " WHERE month NOTNULL"
                 " AND (type = 'app.bsky.feed.post' OR type = 'app.bsky.feed.repost')"
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

QList<TotalItem> LogAccess::Private::dbMakeStatistics() const
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

QString LogAccess::Private::dbSelectRecords(const int kind, const QString &condition,
                                            const QString &cursor, const int limit,
                                            QStringList &view_posts) const
{
    QStringList bind_values;
    QString sql("SELECT uri, cid, record, createdAt, view FROM record"
                " WHERE (type = 'app.bsky.feed.post' OR type = 'app.bsky.feed.repost')");
    if (!condition.isEmpty()) {
        if (kind == 0) {
            sql.append(" AND day == ?");
            bind_values.append(condition.trimmed());
        } else if (kind == 1) {
            sql.append(" AND month == ?");
            bind_values.append(condition.trimmed());
        } else if (kind == 2) {
            const QStringList words =
                    condition.trimmed().split(QRegularExpression(REG_EXP_WHITE_SPACE));
            for (const auto &word : words) {
                sql.append(" AND text like ?");
                bind_values.append("%" + word.trimmed() + "%");
            }
        }
    }
    if (!cursor.isEmpty()) {
        sql.append(" AND createdAt < ?");
        bind_values.append(cursor.trimmed());
    }
    sql.append(" ORDER BY createdAt DESC");
    if (limit > 0) {
        sql.append(" LIMIT ?");
        bind_values.append(QString::number(limit));
    } else {
        sql.append(" LIMIT 50");
    }

    QSqlQuery query(QSqlDatabase::database(m_dbConnectionName));
    if (dbSelect(query, sql, bind_values)) {
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

void LogAccess::Private::dbUpdateRecords(const QList<RecordPostItem> &record_post_items) const
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
                qDebug().noquote() << LOG_DATETIME << query.lastError();
                qDebug().noquote() << LOG_DATETIME << query.lastQuery() << query.boundValues();
            }
        } else {
            qDebug().noquote() << LOG_DATETIME << query.lastError();
        }
    }
}

int LogAccess::Private::dbGetVersion() const
{
    int version = -1;
    QSqlQuery query(QSqlDatabase::database(m_dbConnectionName));
    if (dbSelect(query, "SELECT version FROM inform")) {
        while (query.next()) {
            version = query.value(0).toInt();
            break;
        }
    }
    return version;
}

void LogAccess::Private::dbSetVersion(const int version) const
{
    int old = dbGetVersion();
    QString sql;
    if (old < 0) {
        sql = "INSERT INTO inform(version) VALUES(?)";
    } else {
        sql = "UPDATE inform SET version = ?";
    }
    QSqlQuery query(QSqlDatabase::database(m_dbConnectionName));
    if (query.prepare(sql)) {
        query.addBindValue(version);
        if (!query.exec()) {
            qDebug().noquote() << LOG_DATETIME << query.lastError();
            qDebug().noquote() << LOG_DATETIME << query.lastQuery() << query.boundValues();
        }
    } else {
        qDebug().noquote() << LOG_DATETIME << query.lastError();
    }
}
