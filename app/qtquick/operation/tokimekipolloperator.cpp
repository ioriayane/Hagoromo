#include "tokimekipolloperator.h"
#include "atprotocol/tech/tokimeki/poll/techtokimekipollgetpoll.h"
#include "atprotocol/com/atproto/repo/comatprotorepocreaterecord.h"
#include "atprotocol/com/atproto/repo/comatprotorepodeleterecord.h"
#include "atprotocol/lexicons_func_unknown.h"
#include "tools/accountmanager.h"

#include <QDateTime>
#include <QUrl>
#include <QStringList>
#include <QJsonObject>

using AtProtocolInterface::ComAtprotoRepoCreateRecord;
using AtProtocolInterface::ComAtprotoRepoDeleteRecord;
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
    if (uri.isEmpty() || !m_pollViewDetailHash.contains(uri)) {
        if (role == HasPollRole) {
            return HasPollRole;
        } else if (role == PollUriRole) {
            return QString();
        } else if (role == PollCidRole) {
            return QString();
        } else if (role == PollOptionsRole) {
            return QStringList();
        } else if (role == PollCountOfOptionsRole) {
            return QStringList();
        } else if (role == PollIndexOfOptionsRole) {
            return QStringList();
        } else if (role == PollMyVoteRole) {
            return QStringLiteral("-1");
        } else if (role == PollTotalVotesRole) {
            return 0;
        } else if (role == PollIsEndedRole) {
            return 1;
        } else if (role == PollRemainTimeRole) {
            return 0;
        }
    }

    auto view = m_pollViewDetailHash.value(uri);

    if (role == HasPollRole) {
        return true;
    } else if (role == PollUriRole) {
        return view.poll.uri;
    } else if (role == PollCidRole) {
        return view.poll.cid;
    } else if (role == PollOptionsRole) {
        QStringList options;
        for (const auto option : view.options) {
            options.append(option.text);
        }
        return options;
    } else if (role == PollCountOfOptionsRole) {
        QStringList count;
        for (const auto option : view.options) {
            count.append(QString::number(option.count));
        }
        return count;
    } else if (role == PollIndexOfOptionsRole) {
        QStringList indexes;
        for (const auto option : view.options) {
            indexes.append(QString::number(option.index));
        }
        return indexes;
    } else if (role == PollMyVoteRole) {
        // QMLに対してindexOfOptionsをQStringListでしか渡せないのでこちらも文字列にする
        return QString::number(view.myVote);
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

void TokimekiPollOperator::setAccount(const QString &uuid)
{
    m_uuid = uuid;
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

void TokimekiPollOperator::vote(const QString &cid, const QString &uri, const QString &option_index)
{
    if (uri.isEmpty() || !m_pollViewDetailHash.contains(uri)) {
        emit finished(false, cid);
        return;
    }
    int option_index_num = option_index.toInt();

    auto view = m_pollViewDetailHash.value(uri);

    if (view.myVote == -1) {
        QJsonObject json_poll;
        json_poll.insert("cid", view.poll.cid);
        json_poll.insert("uri", view.poll.uri);
        QJsonObject json_record;
        json_record.insert("poll", json_poll);
        json_record.insert("optionIndex", option_index_num);
        json_record.insert("createdAt",
                           QDateTime::currentDateTimeUtc().toString(Qt::ISODateWithMs));
        json_record.insert("$type", "tech.tokimeki.poll.vote");

        auto record = new ComAtprotoRepoCreateRecord(this);
        connect(record, &ComAtprotoRepoCreateRecord::finished, this, [=](bool success) {
            if (success) {
                for (int i = 0; i < m_pollViewDetailHash[uri].options.length(); i++) {
                    if (m_pollViewDetailHash[uri].options[i].index == option_index_num) {
                        m_pollViewDetailHash[uri].options[i].count++;
                        break;
                    }
                }
                m_pollViewDetailHash[uri].totalVotes++;
                m_pollViewDetailHash[uri].myVote = option_index_num;
                m_pollViewDetailHash[uri].myVoteUri = record->uri();
            } else {
            }
            emit finished(success, cid);
            record->deleteLater();
        });
        record->setAccount(account());
        record->createRecord(account().did, QStringLiteral("tech.tokimeki.poll.vote"), QString(),
                             false, json_record, QString());
    } else if (!view.myVoteUri.isEmpty()) {
        qDebug() << "remove vote";
        QString rkey = AtProtocolType::LexiconsTypeUnknown::extractRkey(view.myVoteUri);
        auto record = new ComAtprotoRepoDeleteRecord(this);
        connect(record, &ComAtprotoRepoDeleteRecord::finished, this, [=](bool success) {
            if (success) {
                for (int i = 0; i < m_pollViewDetailHash[uri].options.length(); i++) {
                    if (m_pollViewDetailHash[uri].options[i].index
                        == m_pollViewDetailHash[uri].myVote) {
                        m_pollViewDetailHash[uri].options[i].count--;
                        break;
                    }
                }
                m_pollViewDetailHash[uri].totalVotes--;
                m_pollViewDetailHash[uri].myVote = -1;
                m_pollViewDetailHash[uri].myVoteUri.clear();
            } else {
            }
            emit finished(success, cid);
            record->deleteLater();
        });
        record->setAccount(account());
        record->deleteRecord(account().did, QStringLiteral("tech.tokimeki.poll.vote"), rkey,
                             QString(), QString());
    } else {
        qDebug() << "ERROR: myVoteUri is empty";
        emit finished(false, cid);
    }
}

AtProtocolInterface::AccountData TokimekiPollOperator::account() const
{
    return AccountManager::getInstance()->getAccount(m_uuid);
}
