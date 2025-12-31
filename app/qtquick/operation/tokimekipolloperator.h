#ifndef TOKIMEKIPOLLOPERATOR_H
#define TOKIMEKIPOLLOPERATOR_H

#include <QObject>
#include <QString>

#include <atprotocol/lexicons.h>

class TokimekiPollOperator : public QObject
{
    Q_OBJECT
public:
    explicit TokimekiPollOperator(QObject *parent = nullptr);

    enum Roles {
        PollOptionsRole,
        PollCountOfOptionsRole,
        PollMyVoteRole,
        PollTotalVotesRole,
        PollIsEndedRole,
        PollRemainTimeRole,
    };

    QString convertUrlToUri(const QString &url) const;

    QVariant item(const QString &uri, TokimekiPollOperator::Roles role) const;

    void setServiceUrl(const QString &url);
    QString serviceUrl() const;

    void getPoll(const QString &cid, const QString &uri, const QString &viewer);

signals:
    void finished(bool success, const QString &cid);

private:
    QString m_serviceUrl;

    QHash<QString, AtProtocolType::TechTokimekiPollDefs::PollViewDetailed>
            m_pollViewDetailHash; // QHash<uri, view>
};

#endif // TOKIMEKIPOLLOPERATOR_H
