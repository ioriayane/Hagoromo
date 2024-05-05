#ifndef LOGACCESS_H
#define LOGACCESS_H

#include <QObject>
#include <QSqlQuery>

struct TotalItem
{
    QString name;
    int count = 0;
};
Q_DECLARE_METATYPE(TotalItem)

class LogAccess : public QObject
{
    Q_OBJECT
public:
    explicit LogAccess(QObject *parent = nullptr);
    ~LogAccess();

    void removeDbFile(const QString &did);

public slots:
    void updateDb(const QString &did, const QByteArray &data);
    void dailyTotals(const QString &did);
    void monthlyTotals(const QString &did);
    void statistics(const QString &did);
    void selectRecords(const QString &did, const int kind, const QString &condition,
                       const QString &cursor, const int limit);

signals:
    void finishedUpdateDb(bool success);
    void finishedTotals(const QList<TotalItem> &list);
    void finishedSelection(const QString &records);

private:
    QString dbPath(QString did);
    void dbInit();
    void dbRelease();
    bool dbOpen(const QString &did);
    void dbClose();
    bool dbCreateTable();
    bool dbInsertRecord(const QString &uri, const QString &cid, const QString &type,
                        const QJsonObject &json);
    bool dbSelect(QSqlQuery &query, const QString &sql) const;
    QStringList dbGetSavedCids() const;
    QList<TotalItem> dbMakeDailyTotals() const;
    QList<TotalItem> dbMakeMonthlyTotals() const;
    QList<TotalItem> dbMakeStatistics() const;
    QString dbSelectRecords(const int kind, const QString &condition, const QString &cursor,
                            const int limit) const;

    QString m_dbConnectionName;
};

#endif // LOGACCESS_H
