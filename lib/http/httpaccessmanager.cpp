#include "httpaccessmanager.h"

#include <QUrl>
#include <QDebug>

#include <QDateTime>
#define LOG_DATETIME QDateTime::currentDateTime().toString("yyyy/MM/dd hh:mm:ss.zzz")

class HttpAccessManager::Private
{
public:
    Private(HttpAccessManager *parent);
    ~Private();

    int signalIndex();

private:
    HttpAccessManager *q;

    QList<QPair<HttpAccess *, QThread *>> m_accessThread;
    int m_signalIndex;
};

HttpAccessManager::Private::Private(HttpAccessManager *parent) : q(parent), m_signalIndex(0)
{
    qDebug().noquote() << LOG_DATETIME << "HttpAccessManager::Private(" << this << ") Private()";

    for (int i = 0; i < 5; i++) {
        m_accessThread.append(QPair<HttpAccess *, QThread *>(new HttpAccess(), new QThread()));
        m_accessThread.last().first->moveToThread(m_accessThread.last().second);
        if (i == 0) {
            connect(q, &HttpAccessManager::process0, m_accessThread.last().first,
                    &HttpAccess::process);
        } else if (i == 1) {
            connect(q, &HttpAccessManager::process1, m_accessThread.last().first,
                    &HttpAccess::process);
        } else if (i == 2) {
            connect(q, &HttpAccessManager::process2, m_accessThread.last().first,
                    &HttpAccess::process);
        } else if (i == 3) {
            connect(q, &HttpAccessManager::process3, m_accessThread.last().first,
                    &HttpAccess::process);
        } else {
            connect(q, &HttpAccessManager::process4, m_accessThread.last().first,
                    &HttpAccess::process);
        }
        connect(m_accessThread.last().first, &HttpAccess::finished, q,
                &HttpAccessManager::processReply);
        m_accessThread.last().second->start();
    }
}

HttpAccessManager::Private::~Private()
{
    for (const auto &item : qAsConst(m_accessThread)) {
        item.second->exit();
        item.second->wait();
        delete item.second;
        delete item.first;
    }
    qDebug().noquote() << LOG_DATETIME << "HttpAccessManager::Private(" << this << ") ~Private()";
}

int HttpAccessManager::Private::signalIndex()
{
    if (m_signalIndex >= m_accessThread.length()) {
        m_signalIndex = 0;
    }
    return m_signalIndex++;
}

HttpAccessManager::HttpAccessManager(QObject *parent) : QObject { parent }, d(new Private(this))
{
    qDebug().noquote() << LOG_DATETIME << this << "HttpAccessManager()";
    connect(this, &QObject::destroyed, [this]() { delete d; });
}

HttpAccessManager::~HttpAccessManager()
{
    qDebug().noquote() << LOG_DATETIME << this << "~HttpAccessManager()";
}

HttpReply *HttpAccessManager::get(const QNetworkRequest &request)
{
    HttpReply *reply = new HttpReply(this);

    reply->setOperation(HttpReply::Operation::GetOperation);
    reply->setRequest(request);

    int index = d->signalIndex();
    if (index == 0) {
        emit process0(reply);
    } else if (index == 1) {
        emit process1(reply);
    } else if (index == 2) {
        emit process2(reply);
    } else if (index == 3) {
        emit process3(reply);
    } else {
        emit process4(reply);
    }

    return reply;
}

HttpReply *HttpAccessManager::post(const QNetworkRequest &request, const QByteArray &data)
{
    HttpReply *reply = new HttpReply(this);

    reply->setOperation(HttpReply::Operation::PostOperation);
    reply->setRequest(request);
    reply->setSendData(data);

    int index = d->signalIndex();
    if (index == 0) {
        emit process0(reply);
    } else if (index == 1) {
        emit process1(reply);
    } else if (index == 2) {
        emit process2(reply);
    } else if (index == 3) {
        emit process3(reply);
    } else {
        emit process4(reply);
    }

    return reply;
}

void HttpAccessManager::processReply(HttpReply *reply)
{
    qDebug().noquote() << LOG_DATETIME << this << "processReply() in " << this->thread();
    qDebug().noquote() << LOG_DATETIME << "  reply" << reply;

    if (reply != nullptr)
        emit reply->finished();

    emit finished(reply);
}
