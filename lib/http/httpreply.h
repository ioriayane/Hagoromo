#ifndef HTTPREPLY_H
#define HTTPREPLY_H

#include <QObject>
#include <QNetworkRequest>

class HttpReply : public QObject
{
    Q_OBJECT
public:
    explicit HttpReply(QObject *parent = nullptr);
    ~HttpReply();

    enum Operation {
        GetOperation,
        PostOperation,
    };

    enum Error {
        Success = 0,
        Unknown,
        Connection,
        BindIPAddress,
        Read,
        Write,
        ExceedRedirectCount,
        Canceled,
        SSLConnection,
        SSLLoadingCerts,
        SSLServerVerification,
        UnsupportedMultipartBoundaryChars,
        Compression,
        ConnectionTimeout,
        ProxyConnection,
    };

    const QList<QPair<QByteArray, QByteArray>> &rawHeaderPairs() const;
    QByteArray rawHeader(const QByteArray &name) const;
    void setRawHeader(const QByteArray &name, const QByteArray &value);
    QByteArray readAll();
    QUrl url() const;

    Operation operation() const;
    void setOperation(Operation newOperation);
    const QNetworkRequest &request() const;
    void setRequest(const QNetworkRequest &newRequest);
    QByteArray sendData() const;
    void setSendData(const QByteArray &newSendData);
    QByteArray recvData() const;
    void setRecvData(const QByteArray &newRecvData);
    Error error() const;
    void setError(Error newError);
    QString contentType() const;
    void setContentType(const QString &newContentType);
    void setContentType(const QByteArray &name, const QByteArray &value);

    int statusCode() const;
    void setStatusCode(int newStatusCode);

public slots:

signals:
    void finished();
    void downloadProgress(qint64 bytesReceived, qint64 bytesTotal);
    void uploadProgress(qint64 bytesSent, qint64 bytesTotal);

private:
    QList<QPair<QByteArray, QByteArray>> m_rawHeaders;

    Operation m_operation;
    QNetworkRequest m_request;
    QByteArray m_sendData;
    QByteArray m_recvData;
    Error m_error;
    QString m_contentType;
    int m_statusCode;
};

#endif // HTTPREPLY_H
