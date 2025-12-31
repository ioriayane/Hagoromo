#include "tokimekipolloperator.h"
#include "atprotocol/tech/tokimeki/poll/techtokimekipollgetpoll.h"

#include <QDateTime>
#include <QUrl>
#include <QStringList>

using AtProtocolInterface::TechTokimekiPollGetPoll;

TokimekiPollOperator::TokimekiPollOperator(QObject *parent)
    : QObject { parent }, m_serviceUrl(QStringLiteral("https://poll.tokimeki.tech"))
{
}

QString TokimekiPollOperator::convertUrlToUri(const QString &url) const
{
    const QUrl qurl(url);
    if (!qurl.isValid()) {
        return QString();
    }

    const QString host = qurl.host().toLower();
    if (host != QLatin1String("poll.tokimeki.tech")) {
        return QString();
    }

    const QStringList segments = qurl.path().split('/', Qt::SkipEmptyParts);
    if (segments.size() < 3 || segments[0] != QLatin1String("p")) {
        return QString();
    }

    const QString did = segments[1];
    const QString rkey = segments[2];
    if (!did.startsWith(QLatin1String("did:")) || rkey.isEmpty()) {
        return QString();
    }

    return QStringLiteral("at://%1/tech.tokimeki.poll.poll/%2").arg(did, rkey);
}

QVariant TokimekiPollOperator::item(const QString &uri, Roles role) const
{
    if (uri.isEmpty() || !m_pollViewDetailHash.contains(uri))
        return QVariant();

    auto view = m_pollViewDetailHash.value(uri);

    if (role == PollOptionsRole) {
        QStringList options;
        for (const auto option : view.options) {
            options.append(option.text);
        }
    } else if (role == PollCountOfOptionsRole) {
        QStringList options;
        for (const auto option : view.options) {
            options.append(QString::number(option.count));
        }
    } else if (role == PollMyVoteRole) {
        return view.myVote;
    } else if (role == PollTotalVotesRole) {
        return view.totalVotes;
    } else if (role == PollIsEndedRole) {
        return view.isEnded;
    } else if (role == PollRemainTimeRole) {
        const QDateTime endsAt = QDateTime::fromString(view.poll.endsAt, Qt::ISODateWithMs);
        if (!endsAt.isValid()) {
            return QVariant();
        }
        const QDateTime now = QDateTime::currentDateTimeUtc();
        const qint64 msecsDiff = now.msecsTo(endsAt);
        const double hoursDiff = static_cast<double>(msecsDiff) / (1000.0 * 60.0 * 60.0);
        return hoursDiff;
    }
    return QVariant();
}

void TokimekiPollOperator::setServiceUrl(const QString &url)
{
    if (url.isEmpty())
        return;
    m_serviceUrl = url;
}

QString TokimekiPollOperator::serviceUrl() const
{
    return m_serviceUrl;
}

void TokimekiPollOperator::getPoll(const QString &cid, const QString &uri, const QString &viewer)
{
    auto poll = new TechTokimekiPollGetPoll();

    connect(poll, &TechTokimekiPollGetPoll::finished, this, [this, poll, cid, uri](bool success) {
        if (success) {
            m_pollViewDetailHash[uri] = poll->pollViewDetailed();
        }
        emit finished(success, cid);
        poll->deleteLater();
    });
    poll->setService(m_serviceUrl);
    poll->getPoll(uri, viewer);
}
