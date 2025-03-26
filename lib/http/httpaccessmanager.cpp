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

    HttpReply *process(HttpReply::Operation operation, const QNetworkRequest &request,
                       const QByteArray &data = QByteArray());
    void processReply(HttpReply *reply);

private:
    HttpAccessManager *q;

    int signalIndex();

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
    for (const auto &item : std::as_const(m_accessThread)) {
        item.second->exit();
        item.second->wait();
        delete item.second;
        delete item.first;
    }
    qDebug().noquote() << LOG_DATETIME << "HttpAccessManager::Private(" << this << ") ~Private()";
}

HttpReply *HttpAccessManager::Private::process(HttpReply::Operation operation,
                                               const QNetworkRequest &request,
                                               const QByteArray &data)
{
    HttpReply *reply = new HttpReply(q);

    reply->setOperation(operation);
    reply->setRequest(request);
    reply->setSendData(data);

    int index = signalIndex();
    if (index == 0) {
        emit q->process0(reply);
    } else if (index == 1) {
        emit q->process1(reply);
    } else if (index == 2) {
        emit q->process2(reply);
    } else if (index == 3) {
        emit q->process3(reply);
    } else {
        emit q->process4(reply);
    }

    return reply;
}

void HttpAccessManager::Private::processReply(HttpReply *reply)
{
    qDebug().noquote() << LOG_DATETIME << this << "processReply() in " << q->thread();
    qDebug().noquote() << LOG_DATETIME << "  reply" << reply;

    if (reply != nullptr)
        emit reply->finished();

    emit q->finished(reply);
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
    return d->process(HttpReply::GetOperation, request);
}

HttpReply *HttpAccessManager::post(const QNetworkRequest &request, const QByteArray &data)
{
    return d->process(HttpReply::PostOperation, request, data);
}

void HttpAccessManager::processReply(HttpReply *reply)
{
    d->processReply(reply);
}
