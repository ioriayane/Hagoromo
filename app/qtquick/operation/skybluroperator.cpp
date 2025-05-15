#include "skybluroperator.h"
#include "atprotocol/lexicons_func_unknown.h"
#include "atprotocol/com/atproto/repo/comatprotorepodescriberepo.h"
#include "extension/com/atproto/repo/comatprotorepogetrecordex.h"
#include "tools/accountmanager.h"

using AtProtocolInterface::ComAtprotoRepoDescribeRepo;
using AtProtocolInterface::ComAtprotoRepoGetRecordEx;

SkyblurOperator::SkyblurOperator(QObject *parent) : QObject { parent } { }

SkyblurOperator *SkyblurOperator::getInstance()
{
    static SkyblurOperator instance;
    return &instance;
}

void SkyblurOperator::restoreBluredText(const QString &cid, const QString &at_uri)
{
    const auto uri_items = at_uri.split("/");
    if (uri_items.count() != 5) {
        emit finished(cid, "Error : " + tr("The AT URI is invalid."));
        return;
    }
    const auto did = uri_items.at(2);
    const auto rkey = uri_items.last();

    getServiceEndpoint(did, [=](const QString &target_endpoint) {
        if (target_endpoint.isEmpty()) {
            const auto text =
                    "Error : " + tr("The PDS for the target account could not be obtained.");
            m_unbluredText[cid] = text;
            emit finished(cid, text);
        } else {
            ComAtprotoRepoGetRecordEx *record = new ComAtprotoRepoGetRecordEx(this);
            connect(record, &ComAtprotoRepoGetRecordEx::finished, this, [=](bool success) {
                QString text;
                if (success) {
                    const auto value = AtProtocolType::LexiconsTypeUnknown::fromQVariant<
                            AtProtocolType::UkSkyblurPost::Main>(record->value());
                    text = value.text;
                } else {
                    text = record->errorMessage();
                }
                m_unbluredText[cid] = text;
                emit finished(cid, text);
                record->deleteLater();
            });
            auto a = account();
            a.service_endpoint = target_endpoint;
            record->setAccount(a);
            record->skyBlurPost(did, rkey);
        }
    });
}

QString SkyblurOperator::getUnbluredText(const QString &cid) const
{
    return m_unbluredText.value(cid, QString());
}

AtProtocolInterface::AccountData SkyblurOperator::account() const
{
    return AccountManager::getInstance()->getAccount(m_uuid);
}

void SkyblurOperator::getServiceEndpoint(const QString &did,
                                         std::function<void(const QString &)> callback)
{
    ComAtprotoRepoDescribeRepo *repo = new ComAtprotoRepoDescribeRepo(this);
    connect(repo, &ComAtprotoRepoDescribeRepo::finished, this, [=](bool success) {
        if (success) {
            AtProtocolType::DirectoryPlcDefs::DidDoc doc =
                    AtProtocolType::LexiconsTypeUnknown::fromQVariant<
                            AtProtocolType::DirectoryPlcDefs::DidDoc>(repo->didDoc());
            if (!doc.service.isEmpty()) {
                callback(doc.service.first().serviceEndpoint);
            } else {
                callback(QString());
            }
        } else {
            callback(QString());
        }
        repo->deleteLater();
    });
    auto a = account();
    a.service_endpoint = QStringLiteral("https://bsky.social");
    repo->setAccount(a);
    repo->describeRepo(did);
}

void SkyblurOperator::setAccount(const QString &uuid)
{
    m_uuid = uuid;
}
