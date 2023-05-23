#include "comatprotorepolistrecords.h"
#include "atprotocol/lexicons_func.h"

#include <QJsonDocument>
#include <QJsonObject>
#include <QUrlQuery>

namespace AtProtocolInterface {

ComAtprotoRepoListRecords::ComAtprotoRepoListRecords(QObject *parent) : AccessAtProtocol { parent }
{
}

void ComAtprotoRepoListRecords::listRecords(const QString &repo, const QString &collection,
                                            const int limit, const QString &cursor,
                                            const QString &rkeyStart, const QString &rkeyEnd)
{
    Q_UNUSED(rkeyStart)
    Q_UNUSED(rkeyEnd)

    QUrlQuery query;
    query.addQueryItem(QStringLiteral("repo"), repo);
    query.addQueryItem(QStringLiteral("collection"), collection);

    get(QStringLiteral("xrpc/com.atproto.repo.listRecords"), query);
}

void ComAtprotoRepoListRecords::listLikes(const QString &repo)
{
    listRecords(repo, "app.bsky.feed.like", 50, QString(), QString(), QString());
}

void ComAtprotoRepoListRecords::listReposts(const QString &repo)
{
    listRecords(repo, "app.bsky.feed.repost", 50, QString(), QString(), QString());
}

const QList<AtProtocolType::ComAtprotoRepoListRecords::Record> *
ComAtprotoRepoListRecords::recordList() const
{
    return &m_recordList;
}

void ComAtprotoRepoListRecords::parseJson(const QString reply_json)
{
    bool success = false;

    QJsonDocument json_doc = QJsonDocument::fromJson(reply_json.toUtf8());
    if (json_doc.isEmpty()) {
    } else {
        for (const auto &obj : json_doc.object().value("records").toArray()) {
            AtProtocolType::ComAtprotoRepoListRecords::Record record;

            AtProtocolType::ComAtprotoRepoListRecords::copyRecord(obj.toObject(), record);

            m_recordList.append(record);
        }

        success = true;
    }

    emit finished(success);
}

}
