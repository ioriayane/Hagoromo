#ifndef LOGMANAGER_H
#define LOGMANAGER_H

#include <QObject>
#include <QThread>
#include "logaccess.h"

class LogManager : public QObject
{
    Q_OBJECT
public:
    explicit LogManager(QObject *parent = nullptr);
    ~LogManager();

    void update(const QString &service, const QString &did);
    void clearDb(const QString &did);

public slots:

signals:
    void errorOccured(const QString &code, const QString &message);
    void progressMessage(const QString &message);
    void updateDb(const QString &did, const QByteArray &data);
    void finished(bool success);
    void dailyTotals(const QString &did);
    void monthlyTotals(const QString &did);
    void statistics(const QString &did);
    void finishedTotals(const QList<TotalItem> &list);
    void selectRecords(const QString &did, const int kind, const QString &condition,
                       const QString &cursor, const int limit);
    void finishedSelection(const QString &records);

private:
    QThread m_thread;
    LogAccess m_logAccess;
};

#endif // LOGMANAGER_H
