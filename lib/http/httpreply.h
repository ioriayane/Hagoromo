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

    Operation operation() const;
    void setOperation(Operation newOperation);
    const QNetworkRequest *request() const;
    void setRequest(const QNetworkRequest &newRequest);
    QByteArray sendData() const;
    void setSendData(const QByteArray &newSendData);
    QString body() const;
    void setBody(const QString &newBody);

public slots:

signals:
    void finished(bool success);

private:
    Operation m_operation;
    QNetworkRequest m_request;
    QByteArray m_sendData;
    QString m_body;
};

#endif // HTTPREPLY_H
