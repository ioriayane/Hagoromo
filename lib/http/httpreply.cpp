#include "httpreply.h"

#include <QUrl>
#include <QDebug>

HttpReply::HttpReply(QObject *parent) : QObject { parent }
{
    qDebug().noquote() << this << "HttpReply()";
}

HttpReply::~HttpReply()
{
    qDebug().noquote() << this << "~HttpReply()";
}

HttpReply::Operation HttpReply::operation() const
{
    return m_operation;
}

void HttpReply::setOperation(Operation newOperation)
{
    m_operation = newOperation;
}

const QNetworkRequest *HttpReply::request() const
{
    return &m_request;
}

void HttpReply::setRequest(const QNetworkRequest &newRequest)
{
    m_request = newRequest;
}

QString HttpReply::body() const
{
    return m_body;
}

void HttpReply::setBody(const QString &newBody)
{
    m_body = newBody;
}

QByteArray HttpReply::sendData() const
{
    return m_sendData;
}

void HttpReply::setSendData(const QByteArray &newSendData)
{
    m_sendData = newSendData;
}
