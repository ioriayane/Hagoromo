#ifndef TOKIMEKIPOLLOPERATOR_H
#define TOKIMEKIPOLLOPERATOR_H

#include <QObject>
#include <QString>
#include "atprotocol/lexicons.h"

class TokimekiPollOperator : public QObject
{
    Q_OBJECT
public:
    explicit TokimekiPollOperator(QObject *parent = nullptr);

    QString convertUrlToUri(const QString &url) const;

    void setServiceUrl(const QString &url);
    QString serviceUrl() const;

    void getPoll(const QString &cid, const QString &uri, const QString &viewer);

signals:
    void finished(bool success, const QString &cid);

private:
    QString m_serviceUrl;
};

#endif // TOKIMEKIPOLLOPERATOR_H
