#ifndef TOKIMEKIPOLLOPERATOR_H
#define TOKIMEKIPOLLOPERATOR_H

#include <QObject>
#include <QString>

#include "atprotocol/accessatprotocol.h"
#include <atprotocol/lexicons.h>

class TokimekiPollOperator : public QObject
{
    Q_OBJECT
public:
    explicit TokimekiPollOperator(QObject *parent = nullptr);

    enum Roles {
        HasPollRole,
        PollUriRole,
        PollCidRole,
        PollOptionsRole,
        PollCountOfOptionsRole,
        PollIndexOfOptionsRole,
        PollMyVoteRole,
        PollTotalVotesRole,
        PollIsEndedRole,
        PollRemainTimeRole,
    };

    QString convertUrlToUri(const QString &url) const;

    QVariant item(const QString &uri, TokimekiPollOperator::Roles role) const;

    void setAccount(const QString &uuid);

    void setServiceUrl(const QString &url);
    QString serviceUrl() const;

    void getPoll(const QString &cid, const QString &uri, const QString &viewer);
    void vote(const QString &cid, const QString &uri, const QString &option_index);

signals:
    void finished(bool success, const QString &cid);

private:
    AtProtocolInterface::AccountData account() const;

    QString m_uuid;
    QString m_serviceUrl;

    QHash<QString, AtProtocolType::TechTokimekiPollDefs::PollViewDetailed>
            m_pollViewDetailHash; // QHash<uri, view>
};

#endif // TOKIMEKIPOLLOPERATOR_H
