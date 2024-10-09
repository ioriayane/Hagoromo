#include "comatprotorepodescriberepo.h"
#include "atprotocol/lexicons_func.h"
#include "atprotocol/lexicons_func_unknown.h"

#include <QJsonDocument>
#include <QJsonObject>
#include <QUrlQuery>

namespace AtProtocolInterface {

ComAtprotoRepoDescribeRepo::ComAtprotoRepoDescribeRepo(QObject *parent)
    : AccessAtProtocol { parent }
{
}

void ComAtprotoRepoDescribeRepo::describeRepo(const QString &repo)
{
    QUrlQuery url_query;
    if (!repo.isEmpty()) {
        url_query.addQueryItem(QStringLiteral("repo"), repo);
    }

    get(QStringLiteral("xrpc/com.atproto.repo.describeRepo"), url_query, false);
}

const QString &ComAtprotoRepoDescribeRepo::handle() const
{
    return m_handle;
}

const QString &ComAtprotoRepoDescribeRepo::did() const
{
    return m_did;
}

const QVariant &ComAtprotoRepoDescribeRepo::didDoc() const
{
    return m_didDoc;
}

const QStringList &ComAtprotoRepoDescribeRepo::collectionsList() const
{
    return m_collectionsList;
}

const bool &ComAtprotoRepoDescribeRepo::handleIsCorrect() const
{
    return m_handleIsCorrect;
}

bool ComAtprotoRepoDescribeRepo::parseJson(bool success, const QString reply_json)
{
    QJsonDocument json_doc = QJsonDocument::fromJson(reply_json.toUtf8());
    if (json_doc.isEmpty()) {
        success = false;
    } else {
        AtProtocolType::LexiconsTypeUnknown::copyString(json_doc.object().value("handle"),
                                                        m_handle);
        AtProtocolType::LexiconsTypeUnknown::copyString(json_doc.object().value("did"), m_did);
        AtProtocolType::LexiconsTypeUnknown::copyUnknown(
                json_doc.object().value("didDoc").toObject(), m_didDoc);
        AtProtocolType::LexiconsTypeUnknown::copyStringList(
                json_doc.object().value("collections").toArray(), m_collectionsList);
        AtProtocolType::LexiconsTypeUnknown::copyBool(json_doc.object().value("handleIsCorrect"),
                                                      m_handleIsCorrect);
    }

    return success;
}

}
