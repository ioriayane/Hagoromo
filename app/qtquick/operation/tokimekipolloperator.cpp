#include "tokimekipolloperator.h"
#include "atprotocol/tech/tokimeki/poll/techtokimekipollgetpoll.h"
#include "atprotocol/com/atproto/repo/comatprotorepocreaterecord.h"
#include "atprotocol/com/atproto/repo/comatprotorepodeleterecord.h"
#include "atprotocol/lexicons_func_unknown.h"
#include "tools/accountmanager.h"
#include "common.h"

#include <QDateTime>
#include <QUrl>
#include <QStringList>
#include <QJsonObject>
#include <QColor>
#include <QFontMetrics>
#include <QImage>
#include <QLinearGradient>
#include <QPainter>
#include <QtGlobal>
#include <QUuid>

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
        emit finished(success, cid, FunctionType::GetPoll);
        poll->deleteLater();
    });
    poll->setService(m_serviceUrl);
    poll->getPoll(uri, viewer);
}

void TokimekiPollOperator::vote(const QString &cid, const QString &uri, const QString &option_index)
{
    if (uri.isEmpty() || !m_pollViewDetailHash.contains(uri)) {
        emit finished(false, cid, FunctionType::Vote);
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
            emit finished(success, cid, FunctionType::Vote);
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
            emit finished(success, cid, FunctionType::Vote);
            record->deleteLater();
        });
        record->setAccount(account());
        record->deleteRecord(account().did, QStringLiteral("tech.tokimeki.poll.vote"), rkey,
                             QString(), QString());
    } else {
        qDebug() << "ERROR: myVoteUri is empty";
        emit finished(false, cid, FunctionType::Vote);
    }
}

QString TokimekiPollOperator::makePollOgpFile(const QStringList &options) const
{
    QStringList normalizedOptions;
    normalizedOptions.reserve(options.size());
    for (const QString &option : options) {
        QString text = option;
        text.replace(QLatin1Char('\r'), QLatin1Char(' '));
        text.replace(QLatin1Char('\n'), QLatin1Char(' '));
        text = text.simplified();
        normalizedOptions.append(text.isEmpty() ? tr("(No option)") : text);
    }

    if (normalizedOptions.isEmpty()) {
        return QString();
    }

    constexpr int imageWidth = 1000;
    constexpr int imageHeight = 525;
    constexpr int maxDisplayableOptions = 6;

    if (normalizedOptions.size() > maxDisplayableOptions) {
        const int remaining = normalizedOptions.size() - maxDisplayableOptions;
        normalizedOptions = normalizedOptions.mid(0, maxDisplayableOptions);
        normalizedOptions.append(tr("+%1 more choices").arg(remaining));
    }

    QImage image(imageWidth, imageHeight, QImage::Format_ARGB32_Premultiplied);
    image.fill(Qt::transparent);

    QPainter painter(&image);
    painter.setRenderHint(QPainter::Antialiasing, true);
    painter.setRenderHint(QPainter::TextAntialiasing, true);

    QLinearGradient gradient(0, 0, 0, imageHeight);
    gradient.setColorAt(0.0, QColor(12, 12, 20));
    gradient.setColorAt(0.5, QColor(6, 6, 12));
    gradient.setColorAt(1.0, QColor(2, 2, 4));
    painter.fillRect(image.rect(), gradient);

    const int boxWidth = 920;
    const int boxHeight = 80;
    const int horizontalMargin = (imageWidth - boxWidth) / 2;
    const int titleTop = 40;
    const int titleHeight = 60;
    const int optionAreaSpacing = 14;
    const int optionAreaBottomMargin = 60;

    QFont titleFont = painter.font();
    titleFont.setBold(true);
    titleFont.setPointSize(32);
    painter.setFont(titleFont);
    painter.setPen(QColor(0xF5, 0xF5, 0xF7));
    painter.drawText(
            QRect(horizontalMargin, titleTop, imageWidth - horizontalMargin * 2, titleHeight),
            Qt::AlignLeft | Qt::AlignVCenter, tr("TOKIMEKI Poll"));

    const int optionAreaTop = titleTop + titleHeight + 24;
    int boxTop = optionAreaTop;

    QFont optionFont = painter.font();
    optionFont.setBold(false);
    optionFont.setPointSize(28);
    painter.setFont(optionFont);
    QFontMetrics optionMetrics(optionFont);

    for (const QString &optionText : normalizedOptions) {
        QRect boxRect(horizontalMargin, boxTop, boxWidth, boxHeight);
        painter.setPen(QPen(QColor(70, 70, 90, 230), 2));
        painter.setBrush(QColor(24, 24, 32, 235));
        painter.drawRoundedRect(boxRect, 20, 20);

        painter.setPen(QColor(0xEF, 0xEF, 0xF2));
        const QString elidedText =
                optionMetrics.elidedText(optionText, Qt::ElideRight, boxRect.width() - 64);
        painter.drawText(boxRect.adjusted(32, 0, -32, 0), Qt::AlignVCenter | Qt::AlignLeft,
                         elidedText);

        boxTop += boxHeight + optionAreaSpacing;
    }

    painter.end();

    const QString folder = Common::appTempFolder(QStringLiteral("tokimeki_poll"));
    const QString filePath = QStringLiteral("%1/poll_ogp_%2.jpg")
                                     .arg(folder, QUuid::createUuid().toString(QUuid::Id128));
    if (!image.save(filePath, "JPG", 92)) {
        return QString();
    }

    return filePath;
}

QString TokimekiPollOperator::makeAltUrl(const QString &did, const QString &rkey) const
{
    return QString("https://poll.tokimeki.tech/p/%1/%2").arg(did).arg(rkey);
}

AtProtocolInterface::AccountData TokimekiPollOperator::account() const
{
    return AccountManager::getInstance()->getAccount(m_uuid);
}
