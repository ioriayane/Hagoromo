#ifndef HTTPACCESSMANAGER_H
#define HTTPACCESSMANAGER_H

#include "httpaccess.h"
#include "httpreply.h"

#include <QByteArray>
#include <QObject>
#include <QThread>

class HttpAccessManager : public QObject
{
    Q_OBJECT
public:
    explicit HttpAccessManager(QObject *parent = nullptr);
    ~HttpAccessManager();

    HttpReply *get(const QNetworkRequest &request);
    HttpReply *post(const QNetworkRequest &request, const QByteArray &data);

public slots:
    void processReply(HttpReply *reply);

signals:
    void process(HttpReply *reply);
    void getAccess(const QString &url);
    void finished(HttpReply *reply);

private:
    HttpAccess m_access;
    QThread m_thread;
};

#endif // HTTPACCESSMANAGER_H
