#ifndef LOGACCESS_H
#define LOGACCESS_H

#include <QObject>

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

signals:
    void finishedUpdateDb(bool success);
    void finishedTotals(const QList<TotalItem> &list);

private:
    QString dbPath(QString did);
    void dbInit();
    void dbRelease();
    bool dbOpen(const QString &did);
    void dbClose();
    bool dbCreateTable();
    bool dbInsertRecord(const QString &uri, const QString &cid, const QString &type,
                        const QJsonObject &json);
    bool dbInsertHandle(const QString &uri, const QString &cid, const QString &type,
                        const QJsonObject &json);
    QStringList dbGetSavedCids() const;
    QList<TotalItem> dbMakeDailyTotals() const;
    QList<TotalItem> dbMakeMonthlyTotals() const;

    QString m_dbConnectionName;
};

#endif // LOGACCESS_H
