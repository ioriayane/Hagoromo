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

    Operation operation() const;
    void setOperation(Operation newOperation);
    const QNetworkRequest *request() const;
    void setRequest(const QNetworkRequest &newRequest);
    QByteArray sendData() const;
    void setSendData(const QByteArray &newSendData);
    QString body() const;
    void setBody(const QString &newBody);
    Error error() const;
    void setError(Error newError);

public slots:

signals:
    void finished(bool success);

private:
    Operation m_operation;
    QNetworkRequest m_request;
    QByteArray m_sendData;
    QString m_body;
    Error m_error;
};

#endif // HTTPREPLY_H
