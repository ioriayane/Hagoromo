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
    if (!cursor.isEmpty()) {
        query.addQueryItem(QStringLiteral("cursor"), cursor);
    }

    get(QStringLiteral("xrpc/com.atproto.repo.listRecords"), query);
}

void ComAtprotoRepoListRecords::listLikes(const QString &repo, const QString &cursor)
{
    listRecords(repo, "app.bsky.feed.like", 50, cursor, QString(), QString());
}

void ComAtprotoRepoListRecords::listReposts(const QString &repo, const QString &cursor)
{
    listRecords(repo, "app.bsky.feed.repost", 50, cursor, QString(), QString());
}

const QList<AtProtocolType::ComAtprotoRepoListRecords::Record> *
ComAtprotoRepoListRecords::recordList() const
{
    return &m_recordList;
}

void ComAtprotoRepoListRecords::parseJson(bool success, const QString reply_json)
{
    QJsonDocument json_doc = QJsonDocument::fromJson(reply_json.toUtf8());
    if (json_doc.isEmpty()) {
        success = false;
    } else {
        setCursor(json_doc.object().value("cursor").toString());
        for (const auto &obj : json_doc.object().value("records").toArray()) {
            AtProtocolType::ComAtprotoRepoListRecords::Record record;

            AtProtocolType::ComAtprotoRepoListRecords::copyRecord(obj.toObject(), record);

            m_recordList.append(record);
        }
    }

    emit finished(success);
}

}
