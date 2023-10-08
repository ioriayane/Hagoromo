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
    void process0(HttpReply *reply);
    void process1(HttpReply *reply);
    void process2(HttpReply *reply);
    void process3(HttpReply *reply);
    void process4(HttpReply *reply);
    void finished(HttpReply *reply);

private:
    class Private;
    Private *d;
    Q_DISABLE_COPY(HttpAccessManager)
};

#endif // HTTPACCESSMANAGER_H
