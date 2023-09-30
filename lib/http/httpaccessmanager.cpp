#include "httpaccessmanager.h"

#include <QUrl>
#include <QDebug>

#include <QDateTime>
#define LOG_DATETIME QDateTime::currentDateTime().toString("yyyy/MM/dd hh:mm:ss.zzz")

HttpAccessManager::HttpAccessManager(QObject *parent) : QObject { parent }
{
    qDebug().noquote() << LOG_DATETIME << this << "HttpAccessManager()";

    m_access.moveToThread(&m_thread);

    connect(this, &HttpAccessManager::process, &m_access, &HttpAccess::process);
    connect(&m_access, &HttpAccess::finished, this, &HttpAccessManager::processReply);

    m_thread.start();
}

HttpAccessManager::~HttpAccessManager()
{
    m_thread.exit();
    m_thread.wait();
    qDebug().noquote() << LOG_DATETIME << this << "~HttpAccessManager()";
}

HttpReply *HttpAccessManager::get(const QNetworkRequest &request)
{
    HttpReply *reply = new HttpReply(this);

    reply->setOperation(HttpReply::Operation::GetOperation);
    reply->setRequest(request);

    emit process(reply);

    return reply;
}

HttpReply *HttpAccessManager::post(const QNetworkRequest &request, const QByteArray &data)
{
    HttpReply *reply = new HttpReply(this);

    reply->setOperation(HttpReply::Operation::PostOperation);
    reply->setRequest(request);
    reply->setSendData(data);

    emit process(reply);

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
