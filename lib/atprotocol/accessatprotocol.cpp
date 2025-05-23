#include "accessatprotocol.h"

#include <QCoreApplication>
#include <QJsonDocument>
#include <QJsonObject>
#include <QNetworkReply>
#include <QDebug>
#include <QFileInfo>
#include <QHttpMultiPart>
#include <QDateTime>
#include <QUrlQuery>
#include <QMimeDatabase>
#include <QPointer>
#include <QJsonArray>

#define LOG_DATETIME QDateTime::currentDateTime().toString("yyyy/MM/dd hh:mm:ss.zzz")

using namespace AtProtocolType;

namespace AtProtocolInterface {

HttpAccessManager *AccessAtProtocol::m_manager = nullptr;

AtProtocolAccount::AtProtocolAccount(QObject *parent) : QObject { parent } { }

const AccountData &AtProtocolAccount::account() const
{
    return m_account;
}

void AtProtocolAccount::setAccount(const AccountData &account)
{
    m_account.service = account.service;
    m_account.service_endpoint = account.service_endpoint;
    m_account.identifier.clear();
    m_account.password.clear();

    m_account.did = account.did;
    m_account.handle = account.handle;
    m_account.email = account.email;
    m_account.accessJwt = account.accessJwt;
    m_account.refreshJwt = account.refreshJwt;
    m_account.status = account.status;

    m_account.displayName = account.displayName;
}

void AtProtocolAccount::setSession(const QString &did, const QString &handle, const QString &email,
                                   const QString &accessJwt, const QString &refresh_jwt)
{
    m_account.did = did;
    m_account.handle = handle;
    m_account.email = email;
    m_account.accessJwt = accessJwt;
    m_account.refreshJwt = refresh_jwt;
}

const QStringList &AtProtocolAccount::labelers() const
{
    return m_account.labeler_dids;
}

void AtProtocolAccount::setLabelers(const QStringList &dids)
{
    m_account.labeler_dids.clear();
    for (const auto &did : dids) {
        if (did.startsWith("did:")) {
            m_account.labeler_dids.append(did);
        }
    }
}

QString AtProtocolAccount::service() const
{
    QString s;
    if (!m_account.service_endpoint.isEmpty()) {
        s = m_account.service_endpoint;
    } else {
        s = m_account.service;
    }
    if (s.endsWith("/")) {
        return s.chopped(1);
    } else {
        return s;
    }
}

void AtProtocolAccount::setService(const QString &newService)
{
    m_account.service = newService;
}

QString AtProtocolAccount::did() const
{
    return m_account.did;
}

QString AtProtocolAccount::handle() const
{
    return m_account.handle;
}

QString AtProtocolAccount::email() const
{
    return m_account.email;
}

QString AtProtocolAccount::displayName() const
{
    return m_account.displayName;
}

QString AtProtocolAccount::accessJwt() const
{
    return m_account.accessJwt;
}

QString AtProtocolAccount::refreshJwt() const
{
    return m_account.refreshJwt;
}

bool AtProtocolAccount::allowedScope(AccountScope scope) const
{
    return m_account.scope.contains(scope);
}

AccessAtProtocol::AccessAtProtocol(QObject *parent)
    : AtProtocolAccount { parent }, m_contentType("application/json")
{
    qDebug().noquote() << LOG_DATETIME << "AccessAtProtocol::AccessAtProtocol()" << this;
    if (m_manager == nullptr) {
        qDebug().noquote() << LOG_DATETIME << this << "new HttpAccessManager()"
                           << QCoreApplication::instance();
        m_manager = new HttpAccessManager(QCoreApplication::instance());
    }

    m_atprotoProxyDids["app.bsky."] = QStringLiteral("did:web:api.bsky.app#bsky_appview");
    m_atprotoProxyDids["chat.bsky."] = QStringLiteral("did:web:api.bsky.chat#bsky_chat");
    m_excludedAtprotoProxyEndpoints << QStringLiteral("app.bsky.video.getUploadLimits")
                                    << QStringLiteral("app.bsky.video.uploadVideo")
                                    << QStringLiteral("app.bsky.video.getJobStatus");
}

void AccessAtProtocol::get(const QString &endpoint, const QUrlQuery &query,
                           const bool with_auth_header)
{
    if (accessJwt().isEmpty() && with_auth_header) {
        qCritical().noquote() << LOG_DATETIME << "AccessAtProtocol::get()"
                              << "Empty accessJwt!";
        m_errorCode = QStringLiteral("IncompleteAuthenticationInformation");
        m_errorMessage = "AccessJwt is empty.";
        emit finished(false);
        return;
    }

    qDebug().noquote() << LOG_DATETIME << "AccessAtProtocol::get()" << this;
    qDebug().noquote() << LOG_DATETIME << "   " << handle();
    qDebug().noquote() << LOG_DATETIME << "   " << endpoint;
    qDebug().noquote() << LOG_DATETIME << "   " << query.toString();

    QUrl url;
    if (endpoint.isEmpty()) {
        url = service();
    } else {
        url = QString("%1/%2").arg(service(), endpoint);
    }
    url.setQuery(query);
    QNetworkRequest request(url);
    request.setRawHeader(QByteArray("Cache-Control"), QByteArray("no-cache"));
    if (with_auth_header) {
        request.setRawHeader(QByteArray("Authorization"),
                             QByteArray("Bearer ") + accessJwt().toUtf8());
        if (!labelers().isEmpty()) {
            request.setRawHeader(QByteArray("atproto-accept-labelers"),
                                 labelers().join(",").toUtf8());
        }
    }
    setAdditionalRawHeader(request);
    setAtprotoProxyHeader(request);

    QPointer<AccessAtProtocol> alive = this;
    HttpReply *reply = m_manager->get(request);
    connect(reply, &HttpReply::finished, [=]() {
        qDebug().noquote() << LOG_DATETIME << reply->error() << reply->url().toString();
        if (alive) {
            qDebug().noquote() << LOG_DATETIME << "  " << this->thread();

            bool success = false;
            if (checkReply(reply)) {
                if (reply->contentType().startsWith("image/")
                    || reply->contentType().startsWith("application/vnd.ipld.car")) {
                    success = recvImage(reply->recvData(), reply->contentType());
                } else if (reply->contentType().startsWith("application/json")
                           || reply->contentType().startsWith("application/did+ld+json")) {
                    success = parseJson(true, m_replyJson);
                    if (!success && m_errorCode.isEmpty()) {
                        m_errorCode = QStringLiteral("ContentParseError");
                        m_errorMessage = m_replyJson.left(200);
                        m_errorMessage += "\n---\n" + endpoint;
                    }
                } else {
                    m_errorCode = "InvalidContentType";
                    m_errorMessage = "Invalid content type : " + reply->contentType();
                    m_errorMessage += "\n---\n" + endpoint;
                }
            }
            emit finished(success);
        } else {
            qDebug().noquote() << LOG_DATETIME << "Parent is deleted!!!!!!!!!!";
        }
        reply->deleteLater();
    });
    return;
}

void AccessAtProtocol::post(const QString &endpoint, const QByteArray &json,
                            const bool with_auth_header)
{
    qDebug().noquote() << LOG_DATETIME << "AccessAtProtocol::post()" << this;
    qDebug().noquote() << LOG_DATETIME << "   " << handle();
    qDebug().noquote() << LOG_DATETIME << "   " << endpoint;
    qDebug().noquote() << LOG_DATETIME << "   " << json;

    QUrl url;
    if (endpoint.isEmpty()) {
        url = service();
    } else {
        url = QString("%1/%2").arg(service(), endpoint);
    }
    QNetworkRequest request(url);
    request.setRawHeader(QByteArray("Cache-Control"), QByteArray("no-cache"));
    request.setHeader(QNetworkRequest::ContentTypeHeader, m_contentType);
    if (with_auth_header) {
        if (accessJwt().isEmpty()) {
            qCritical() << LOG_DATETIME << "AccessAtProtocol::post()"
                        << "Empty accessJwt!";
            m_errorCode = QStringLiteral("IncompleteAuthenticationInformation");
            m_errorMessage = "AccessJwt is empty.";
            emit finished(false);
            return;
        }

        request.setRawHeader(QByteArray("Authorization"),
                             QByteArray("Bearer ") + accessJwt().toUtf8());
    }
    setAdditionalRawHeader(request);
    setAtprotoProxyHeader(request);

    QPointer<AccessAtProtocol> alive = this;
    HttpReply *reply = m_manager->post(request, json);
    connect(reply, &HttpReply::finished, [=]() {
        qDebug().noquote() << LOG_DATETIME << reply->error() << reply->url().toString();
        if (alive) {
            bool success = false;
            if (checkReply(reply)) {
                success = parseJson(true, m_replyJson);
            }
            emit finished(success);
        } else {
            qDebug().noquote() << LOG_DATETIME << "Parent is deleted!!!!!!!!!!";
        }
        reply->deleteLater();
    });
    return;
}

void AccessAtProtocol::postWithImage(const QString &endpoint, const QString &path)
{
    if (accessJwt().isEmpty()) {
        qCritical().noquote() << LOG_DATETIME << "AccessAtProtocol::postWithImage()"
                              << "Empty accessJwt!";
        emit finished(false);
        return;
    }
    if (!QFile::exists(path)) {
        qCritical().noquote() << LOG_DATETIME << "AccessAtProtocol::postWithImage()"
                              << "Not found" << path;
        emit finished(false);
        return;
    }
    qDebug().noquote() << LOG_DATETIME << "AccessAtProtocol::postWithImage()" << this << endpoint
                       << path;

    QMimeDatabase mime;
    QFileInfo info(path);
    QNetworkRequest request(QUrl(QString("%1/%2").arg(service(), endpoint)));
    request.setRawHeader(QByteArray("Cache-Control"), QByteArray("no-cache"));
    request.setRawHeader(QByteArray("Authorization"), QByteArray("Bearer ") + accessJwt().toUtf8());
    request.setHeader(QNetworkRequest::ContentTypeHeader, mime.mimeTypeForFile(info).name());
    setAtprotoProxyHeader(request);

    QFile *file = new QFile(path);
    if (!file->open(QIODevice::ReadOnly)) {
        qCritical().noquote() << LOG_DATETIME << LOG_DATETIME << "AccessAtProtocol::postWithImage()"
                              << "Not open" << path;
        delete file;
        emit finished(false);
        return;
    }
    request.setHeader(QNetworkRequest::ContentLengthHeader, file->size());

    QPointer<AccessAtProtocol> alive = this;
    HttpReply *reply = m_manager->post(request, file->readAll());
    file->setParent(reply);
    connect(reply, &HttpReply::finished, [=]() {
        qDebug().noquote() << LOG_DATETIME << reply->error() << reply->url().toString();
        if (alive) {
            bool success = false;
            if (checkReply(reply)) {
                success = parseJson(true, m_replyJson);
            }
            emit finished(success);
        } else {
            qDebug().noquote() << LOG_DATETIME << "Parent is deleted!!!!!!!!!!";
        }
        reply->deleteLater();
    });
    return;
}

bool AccessAtProtocol::recvImage(const QByteArray &data, const QString &content_type)
{
    Q_UNUSED(data)
    Q_UNUSED(content_type)
    return true;
}

bool AccessAtProtocol::checkReply(HttpReply *reply)
{
    bool status = false;
    m_replyJson = QString::fromUtf8(reply->readAll());
    m_errorCode.clear();
    m_errorMessage.clear();

    QByteArray header_key;
    for (const auto &header : reply->rawHeaderPairs()) {
        header_key = header.first.toLower();
        if (header_key.startsWith("ratelimit-")) {
            if (header_key == "ratelimit-reset") {
                qDebug().noquote() << LOG_DATETIME << header.first
                                   << QDateTime::fromSecsSinceEpoch(header.second.toInt())
                                              .toString("yyyy/MM/dd hh:mm:ss");
            } else {
                qDebug().noquote() << LOG_DATETIME << header.first << header.second;
            }
        } else if (header_key == "dpop-nonce") {
            m_dPopNonce = header.second;
        }
    }

    QJsonDocument json_doc = QJsonDocument::fromJson(m_replyJson.toUtf8());
    if (reply->error() != HttpReply::Success) {
        if (json_doc.object().contains("error") && json_doc.object().contains("error")) {
            m_errorCode = json_doc.object().value("error").toString();
            m_errorMessage = json_doc.object().value("message").toString();
        } else if (!m_replyJson.isEmpty()) {
            m_errorCode = QStringLiteral("Other");
            m_errorMessage = m_replyJson;
        } else {
            m_errorCode = QStringLiteral("Unknown");
            if (reply->error() != HttpReply::Error::Read) {
                m_errorMessage =
                        QString("error code : %1\nurl : %2")
                                .arg(QString::number(reply->error()), reply->url().toString());
            }
        }
        if (m_errorCode == "RateLimitExceeded") {
            m_errorMessage += "\n";
            for (const auto &header : reply->rawHeaderPairs()) {
                if (header.first.toLower().startsWith("ratelimit-")) {
                    if (header.first.toLower() == "ratelimit-reset") {
                        m_errorMessage += QString("\n%1:%2").arg(
                                header.first,
                                QDateTime::fromSecsSinceEpoch(header.second.toInt())
                                        .toString("yyyy/MM/dd hh:mm:ss"));
                    } else {
                        m_errorMessage += QString("\n%1:%2").arg(header.first, header.second);
                    }
                }
            }
        }
        qCritical().noquote() << LOG_DATETIME << m_errorCode << m_errorMessage;
        qCritical().noquote() << LOG_DATETIME << m_replyJson;
    } else {
        status = true;
    }
    return status;
}

void AccessAtProtocol::setJsonBlob(const AtProtocolType::Blob &blob, QJsonObject &json_blob)
{
    json_blob.insert("$type", "blob");
    QJsonObject json_link;
    json_link.insert("$link", blob.cid);
    json_blob.insert("ref", json_link);
    json_blob.insert("mimeType", blob.mimeType);
    json_blob.insert("size", blob.size);
}

void AccessAtProtocol::setJsonAspectRatio(const QSize &aspect_ratio, QJsonObject &json_aspect_ratio)
{
    if (!aspect_ratio.isEmpty()) {
        json_aspect_ratio.insert("width", aspect_ratio.width());
        json_aspect_ratio.insert("height", aspect_ratio.height());
    }
}

QJsonObject AccessAtProtocol::makeThreadGateJsonObject(
        const QString &uri, const AtProtocolType::ThreadGateType type,
        const QList<AtProtocolType::ThreadGateAllow> &allow_rules)
{

    QJsonArray json_allow;
    if (type == ThreadGateType::Choice) {
        for (const auto &allow : allow_rules) {
            QJsonObject json_rule;
            if (allow.type == ThreadGateAllowType::Mentioned) {
                json_rule.insert("$type", "app.bsky.feed.threadgate#mentionRule");
            } else if (allow.type == ThreadGateAllowType::Followed) {
                json_rule.insert("$type", "app.bsky.feed.threadgate#followingRule");
            } else if (allow.type == ThreadGateAllowType::Follower) {
                json_rule.insert("$type", "app.bsky.feed.threadgate#followerRule");
            } else if (allow.type == ThreadGateAllowType::List && allow.uri.startsWith("at://")) {
                json_rule.insert("$type", "app.bsky.feed.threadgate#listRule");
                json_rule.insert("list", allow.uri);
            }
            json_allow.append(json_rule);
        }
    }

    QJsonObject json_record;
    json_record.insert("$type", "app.bsky.feed.threadgate");
    json_record.insert("post", uri);
    json_record.insert("createdAt", QDateTime::currentDateTimeUtc().toString(Qt::ISODateWithMs));
    json_record.insert("allow", json_allow);

    return json_record;
}

QJsonObject AccessAtProtocol::makePostGateJsonObject(
        const QString &uri, const AtProtocolType::AppBskyFeedPostgate::MainEmbeddingRulesType type,
        const QStringList &detached_uris)
{
    QJsonArray json_rules;
    if (type == AppBskyFeedPostgate::MainEmbeddingRulesType::embeddingRules_DisableRule) {
        QJsonObject json_rule;
        json_rule.insert("$type", "app.bsky.feed.postgate#disableRule");
        json_rules.append(json_rule);
    }
    QJsonArray json_detached_uris;
    for (const auto &detached_uri : detached_uris) {
        // QJsonObject json_detached_uri;
        // json_detached_uri.insert("", "");
        json_detached_uris.append(QJsonValue::fromVariant(detached_uri));
    }

    QJsonObject json_record;
    json_record.insert("$type", "app.bsky.feed.postgate");
    json_record.insert("post", uri);
    json_record.insert("createdAt", QDateTime::currentDateTimeUtc().toString(Qt::ISODateWithMs));
    json_record.insert("embeddingRules", json_rules);
    json_record.insert("detachedEmbeddingUris", json_detached_uris);

    return json_record;
}

void AccessAtProtocol::setAdditionalRawHeader(QNetworkRequest &request)
{
    QHashIterator<QString, QString> i(m_additionalRawHeaders);
    while (i.hasNext()) {
        i.next();
        request.setRawHeader(i.key().toLocal8Bit(), i.value().toLocal8Bit());
    }
}

void AccessAtProtocol::setAtprotoProxyHeader(QNetworkRequest &request)
{
    if (request.url().host() == QStringLiteral("bsky.social"))
        return;

    QString endpoint = request.url().fileName();

    QHashIterator<QString, QString> i(m_atprotoProxyDids);
    while (i.hasNext()) {
        i.next();
        if (!endpoint.isEmpty() && !m_excludedAtprotoProxyEndpoints.contains(endpoint)
            && endpoint.startsWith(i.key())) {
            request.setRawHeader("atproto-proxy", i.value().toLocal8Bit());
        }
    }
}

QString AccessAtProtocol::dPopNonce() const
{
    return m_dPopNonce;
}

void AccessAtProtocol::setContentType(const QString &newContentType)
{
    m_contentType = newContentType;
}

QString AccessAtProtocol::cursor() const
{
    return m_cursor;
}

void AccessAtProtocol::setCursor(const QString &newCursor)
{
    m_cursor = newCursor;
}

void AccessAtProtocol::appendRawHeader(const QString &name, const QString &value)
{
    m_additionalRawHeaders[name] = value;
}

QString AccessAtProtocol::errorMessage() const
{
    return m_errorMessage;
}

QString AccessAtProtocol::replyJson() const
{
    return m_replyJson;
}

QString AccessAtProtocol::errorCode() const
{
    return m_errorCode;
}

}
