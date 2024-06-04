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
                                            const bool reverse)
{
    QUrlQuery url_query;
    if (!repo.isEmpty()) {
        url_query.addQueryItem(QStringLiteral("repo"), repo);
    }
    if (!collection.isEmpty()) {
        url_query.addQueryItem(QStringLiteral("collection"), collection);
    }
    if (limit > 0) {
        url_query.addQueryItem(QStringLiteral("limit"), QString::number(limit));
    }
    if (!cursor.isEmpty()) {
        url_query.addQueryItem(QStringLiteral("cursor"), cursor);
    }
    if (reverse) {
        url_query.addQueryItem(QStringLiteral("reverse"), "true");
    }

    get(QStringLiteral("xrpc/com.atproto.repo.listRecords"), url_query, false);
}

const QList<AtProtocolType::ComAtprotoRepoListRecords::Record> &
ComAtprotoRepoListRecords::recordsList() const
{
    return m_recordsList;
}

bool ComAtprotoRepoListRecords::parseJson(bool success, const QString reply_json)
{
    QJsonDocument json_doc = QJsonDocument::fromJson(reply_json.toUtf8());
    if (json_doc.isEmpty() || !json_doc.object().contains("records")) {
        success = false;
    } else {
        setCursor(json_doc.object().value("cursor").toString());
        for (const auto &value : json_doc.object().value("records").toArray()) {
            AtProtocolType::ComAtprotoRepoListRecords::Record data;
            AtProtocolType::ComAtprotoRepoListRecords::copyRecord(value.toObject(), data);
            m_recordsList.append(data);
        }
    }

    return success;
}

}
