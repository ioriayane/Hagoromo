#include "httpreply.h"

#include <QUrl>
#include <QDebug>

HttpReply::HttpReply(QObject *parent) : QObject { parent }, m_error(HttpReply::Unknown)
{
    qDebug().noquote() << this << "HttpReply()"
                       << ", parent" << parent;
}

HttpReply::~HttpReply()
{
    qDebug().noquote() << this << "~HttpReply()";
}

const QList<QPair<QByteArray, QByteArray>> &HttpReply::rawHeaderPairs() const
{
    return m_rawHeaders;
}

QByteArray HttpReply::rawHeader(const QByteArray &name) const
{
    for (const auto &header : m_rawHeaders) {
        if (header.first.toLower() == name.toLower()) {
            return header.second;
        }
    }
    return QByteArray();
}

void HttpReply::setRawHeader(const QByteArray &name, const QByteArray &value)
{
    for (auto &header : m_rawHeaders) {
        if (header.first.toLower() == name.toLower()) {
            header.first = name;
            header.second = value;
            return;
        }
    }
    m_rawHeaders.append(QPair<QByteArray, QByteArray>(name, value));
}

QByteArray HttpReply::readAll()
{
    return m_recvData;
}

QUrl HttpReply::url() const
{
    return m_request.url();
}

HttpReply::Operation HttpReply::operation() const
{
    return m_operation;
}

void HttpReply::setOperation(Operation newOperation)
{
    m_operation = newOperation;
}

const QNetworkRequest &HttpReply::request() const
{
    return m_request;
}

void HttpReply::setRequest(const QNetworkRequest &newRequest)
{
    m_request = newRequest;
}

QByteArray HttpReply::sendData() const
{
    return m_sendData;
}

void HttpReply::setSendData(const QByteArray &newSendData)
{
    m_sendData = newSendData;
}
QByteArray HttpReply::recvData() const
{
    return m_recvData;
}

void HttpReply::setRecvData(const QByteArray &newRecvData)
{
    m_recvData = newRecvData;
}

HttpReply::Error HttpReply::error() const
{
    return m_error;
}

void HttpReply::setError(Error newError)
{
    m_error = newError;
}
