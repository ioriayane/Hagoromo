#include "tokimekipolloperator.h"
#include "atprotocol/tech/tokimeki/poll/techtokimekipollgetpoll.h"

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

    connect(poll, &TechTokimekiPollGetPoll::finished, this, [this, poll, cid](bool success) {
        if (success) {
            //
        }
        emit finished(success, cid);
        poll->deleteLater();
    });
    poll->setService(m_serviceUrl);
    poll->getPoll(uri, viewer);
}
