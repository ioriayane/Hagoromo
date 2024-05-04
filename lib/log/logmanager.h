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
    void updateDb(const QString &did, const QByteArray &data);
    void finished(bool success);
    void dailyTotals(const QString &did);
    void monthlyTotals(const QString &did);
    void finishedTotals(const QList<TotalItem> &list);

private:
    QThread m_thread;
    LogAccess m_logAccess;
};

#endif // LOGMANAGER_H
