#include "accessatprotocol.h"
#include "account/dpopsessionstore.h"
#include "account/sessionstatestore.h"
#include "atprotocol/lexicons_func_unknown.h"

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
    m_account.uuid = account.uuid;
    m_account.auth_type = account.auth_type;
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
    // 接続先はservice_endpointが優先されるので、指定した接続先が使われるように両方置き換える
    m_account.service = newService;
    m_account.service_endpoint = newService;
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

void AtProtocolAccount::setAccessToken(AuthType auth_type, const QString &access_jwt)
{
    m_account.auth_type = auth_type;
    m_account.accessJwt = access_jwt;
}

bool AtProtocolAccount::allowedScope(AccountScope scope) const
{
    return m_account.scope.contains(scope);
}

AccessAtProtocol::AccessAtProtocol(QObject *parent)
    : AtProtocolAccount { parent },
      m_httpStatusCode(0),
      m_contentType("application/json"),
      m_dPopNonceRequired(false)
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
    if (with_auth_header) {
        applyLatestAccessToken(endpoint);
    }
    if (accessJwt().isEmpty() && with_auth_header) {
        qCritical().noquote() << LOG_DATETIME << "AccessAtProtocol::get()"
                              << "Empty accessJwt!";
        m_errorCode = QStringLiteral("IncompleteAuthenticationInformation");
        m_errorMessage = "AccessJwt is empty.";
        emit finished(false);
        return;
    }
    if (with_auth_header && !isSessionAvailable()) {
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
        if (!setAuthorizationHeader(request, QByteArrayLiteral("GET"))) {
            emit finished(false);
            return;
        }
        if (!labelers().isEmpty()) {
            request.setRawHeader(QByteArray("atproto-accept-labelers"),
                                 labelers().join(",").toUtf8());
        }
    }
    setAdditionalRawHeader(request);
    setAtprotoProxyHeader(request);

    sendGet(request, endpoint, with_auth_header, false);
}

void AccessAtProtocol::sendGet(QNetworkRequest request, const QString &endpoint,
                               const bool with_auth_header, const bool retried)
{
    QPointer<AccessAtProtocol> alive = this;
    HttpReply *reply = m_manager->get(request);
    connect(reply, &HttpReply::finished, this, [=]() mutable {
        qDebug().noquote() << LOG_DATETIME << reply->error() << reply->url().toString();
        if (alive) {
            qDebug().noquote() << LOG_DATETIME << "  " << this->thread();

            bool success = false;
            if (checkReply(reply)) {
                if (isRawContentType(reply->contentType())) {
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
            } else if (canRetryWithDPopNonce(with_auth_header, retried)
                       && setAuthorizationHeader(request, QByteArrayLiteral("GET"))) {
                // 新しいnonceで1回だけ再送する
                qDebug().noquote() << LOG_DATETIME << "Retry with new DPoP nonce";
                sendGet(request, endpoint, with_auth_header, true);
                reply->deleteLater();
                return;
            }
            emit finished(success);
        } else {
            qDebug().noquote() << LOG_DATETIME << "Parent is deleted!!!!!!!!!!";
        }
        reply->deleteLater();
    });
}

void AccessAtProtocol::post(const QString &endpoint, const QByteArray &json,
                            const bool with_auth_header)
{
    qDebug().noquote() << LOG_DATETIME << "AccessAtProtocol::post()" << this;
    qDebug().noquote() << LOG_DATETIME << "   " << handle();
    qDebug().noquote() << LOG_DATETIME << "   " << endpoint;
    if (m_contentType.startsWith("application/x-www-form-urlencoded")
        || endpoint == QStringLiteral("xrpc/com.atproto.server.createSession")) {
        // OAuthのPAR/tokenリクエストはcode_verifierやrefresh tokenを、
        // createSessionはパスワードを含むので出力しない
        qDebug().noquote() << LOG_DATETIME << "   "
                           << "(form data:" << json.size() << "bytes)";
    } else {
        qDebug().noquote() << LOG_DATETIME << "   " << json;
    }

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
        applyLatestAccessToken(endpoint);
        if (accessJwt().isEmpty()) {
            qCritical() << LOG_DATETIME << "AccessAtProtocol::post()"
                        << "Empty accessJwt!";
            m_errorCode = QStringLiteral("IncompleteAuthenticationInformation");
            m_errorMessage = "AccessJwt is empty.";
            emit finished(false);
            return;
        }
        if (!isSessionAvailable()) {
            emit finished(false);
            return;
        }
        if (!setAuthorizationHeader(request, QByteArrayLiteral("POST"))) {
            emit finished(false);
            return;
        }
    }
    setAdditionalRawHeader(request);
    setAtprotoProxyHeader(request);

    sendPost(request, json, with_auth_header, false);
}

void AccessAtProtocol::postWithImage(const QString &endpoint, const QString &path)
{
    applyLatestAccessToken(endpoint);
    if (accessJwt().isEmpty()) {
        qCritical().noquote() << LOG_DATETIME << "AccessAtProtocol::postWithImage()"
                              << "Empty accessJwt!";
        emit finished(false);
        return;
    }
    if (!isSessionAvailable()) {
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
    if (!setAuthorizationHeader(request, QByteArrayLiteral("POST"))) {
        emit finished(false);
        return;
    }
    request.setHeader(QNetworkRequest::ContentTypeHeader, mime.mimeTypeForFile(info).name());
    setAtprotoProxyHeader(request);

    QFile file(path);
    if (!file.open(QIODevice::ReadOnly)) {
        qCritical().noquote() << LOG_DATETIME << LOG_DATETIME << "AccessAtProtocol::postWithImage()"
                              << "Not open" << path;
        emit finished(false);
        return;
    }
    request.setHeader(QNetworkRequest::ContentLengthHeader, file.size());
    const QByteArray data = file.readAll();
    file.close();

    sendPost(request, data, true, false);
}

void AccessAtProtocol::sendPost(QNetworkRequest request, const QByteArray &data,
                                const bool with_auth_header, const bool retried)
{
    QPointer<AccessAtProtocol> alive = this;
    HttpReply *reply = m_manager->post(request, data);
    connect(reply, &HttpReply::finished, this, [=]() mutable {
        qDebug().noquote() << LOG_DATETIME << reply->error() << reply->url().toString();
        if (alive) {
            bool success = false;
            if (checkReply(reply)) {
                success = parseJson(true, m_replyJson);
            } else if (canRetryWithDPopNonce(with_auth_header, retried)
                       && setAuthorizationHeader(request, QByteArrayLiteral("POST"))) {
                // 新しいnonceで1回だけ再送する
                qDebug().noquote() << LOG_DATETIME << "Retry with new DPoP nonce";
                sendPost(request, data, with_auth_header, true);
                reply->deleteLater();
                return;
            }
            emit finished(success);
        } else {
            qDebug().noquote() << LOG_DATETIME << "Parent is deleted!!!!!!!!!!";
        }
        reply->deleteLater();
    });
}

bool AccessAtProtocol::setAuthorizationHeader(QNetworkRequest &request, const QByteArray &method)
{
    if (account().auth_type != AuthType::OAuth) {
        request.setRawHeader(QByteArray("Authorization"),
                             QByteArray("Bearer ") + accessJwt().toUtf8());
        return true;
    }

    // OAuth: アクセストークンはDPoPの鍵に紐づいているので、リクエストごとにproofを付ける
    const QByteArray proof = DPopSessionStore::getInstance()->generateProof(
            account().uuid, QString::fromUtf8(method), request.url(), accessJwt());
    if (proof.isEmpty()) {
        qCritical().noquote() << LOG_DATETIME << "AccessAtProtocol"
                              << "DPoP key is not set.";
        m_errorCode = QStringLiteral("IncompleteAuthenticationInformation");
        m_errorMessage = "DPoP key is not set.";
        return false;
    }
    request.setRawHeader(QByteArray("Authorization"), QByteArray("DPoP ") + accessJwt().toUtf8());
    request.setRawHeader(QByteArray("DPoP"), proof);
    return true;
}

void AccessAtProtocol::applyLatestAccessToken(const QString &endpoint)
{
    // refresh tokenで認証するAPIはaccessJwtにrefresh tokenを入れて呼ばれるので置き換えない
    if (endpoint == QStringLiteral("xrpc/com.atproto.server.refreshSession")
        || endpoint == QStringLiteral("xrpc/com.atproto.server.deleteSession")) {
        return;
    }
    AuthType auth_type = AuthType::Password;
    QString access_token;
    if (!SessionStateStore::getInstance()->accessToken(account().uuid, auth_type, access_token)) {
        return;
    }
    if (auth_type == account().auth_type && access_token == accessJwt()) {
        return;
    }
    // トークンは出力しない
    qDebug().noquote() << LOG_DATETIME << "AccessAtProtocol"
                       << "Use the latest access token :" << handle();
    setAccessToken(auth_type, access_token);
}

bool AccessAtProtocol::isSessionAvailable()
{
    SessionStateStore *store = SessionStateStore::getInstance();
    if (store->isReauthRequired(account().uuid)) {
        // 再ログインするまで失敗が確定しているので送信しない
        // 通知は状態が変わったときにAccountManagerから1回だけ行うので、メッセージは空にする
        qDebug().noquote() << LOG_DATETIME << "AccessAtProtocol"
                           << "Session is unavailable until re-login :" << handle();
        m_errorCode = QStringLiteral("SessionUnavailable");
        m_errorMessage.clear();
        return false;
    }
    if (store->isAccessTokenExpired(account().uuid)) {
        // アクセストークンの期限が切れているので送信しても失敗する
        // リフレッシュの結果(失敗時)はAccountManagerから通知するので、メッセージは空にする
        qDebug().noquote() << LOG_DATETIME << "AccessAtProtocol"
                           << "Access token is expired :" << handle();
        store->requestRefresh(account().uuid);
        m_errorCode = QStringLiteral("SessionRefreshing");
        m_errorMessage.clear();
        return false;
    }
    return true;
}

bool AccessAtProtocol::canRetryWithDPopNonce(const bool with_auth_header, const bool retried) const
{
    return with_auth_header && !retried && account().auth_type == AuthType::OAuth
            && m_dPopNonceRequired;
}

bool AccessAtProtocol::recvImage(const QByteArray &data, const QString &content_type)
{
    Q_UNUSED(data)
    Q_UNUSED(content_type)
    return true;
}

bool AccessAtProtocol::isRawContentType(const QString &content_type) const
{
    return content_type.startsWith("image/") || content_type.startsWith("application/vnd.ipld.car");
}

bool AccessAtProtocol::checkReply(HttpReply *reply)
{
    bool status = false;
    m_replyJson = QString::fromUtf8(reply->readAll());
    m_errorCode.clear();
    m_errorMessage.clear();
    m_httpStatusCode = reply->statusCode();
    m_dPopNonceRequired = false;

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
            if (account().auth_type == AuthType::OAuth) {
                DPopSessionStore::getInstance()->setNonce(account().uuid, reply->url(),
                                                          QString::fromUtf8(header.second));
            }
        } else if (header_key == "www-authenticate") {
            // Resource Server(PDS)は401とWWW-Authenticateでnonceの更新を求める
            if (header.second.startsWith("DPoP")
                && header.second.contains("error=\"use_dpop_nonce\"")) {
                m_dPopNonceRequired = true;
            }
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
        if (m_errorCode == QStringLiteral("use_dpop_nonce")) {
            m_dPopNonceRequired = true;
            // DPoPのnonceの更新要求は通常の流れで、呼び出し側で再送する
            qDebug().noquote() << LOG_DATETIME << m_errorCode << m_errorMessage;
        } else {
            qCritical().noquote() << LOG_DATETIME << m_errorCode << m_errorMessage;
            qCritical().noquote() << LOG_DATETIME << m_replyJson;
        }
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

QJsonObject AccessAtProtocol::makePostJsonObject(const QString &text, const PostRecordData &data)
{
    QJsonObject json_record;
    json_record.insert("text", text);
    json_record.insert("createdAt", QDateTime::currentDateTimeUtc().toString(Qt::ISODateWithMs));
    json_record.insert("space.aoisora.post.via", "Hagoromo");
    if (!data.post_languages.isEmpty()) {
        QJsonArray json_langs;
        for (const auto &lang : std::as_const(data.post_languages)) {
            json_langs.append(lang);
        }
        json_record.insert("langs", json_langs);
    }

    if (!data.reply_parent.cid.isEmpty() && !data.reply_parent.uri.isEmpty()) {
        QJsonObject json_root;
        if (!data.reply_root.cid.isEmpty() && !data.reply_root.uri.isEmpty()) {
            json_root.insert("cid", data.reply_root.cid);
            json_root.insert("uri", data.reply_root.uri);
        } else {
            json_root.insert("cid", data.reply_parent.cid);
            json_root.insert("uri", data.reply_parent.uri);
        }
        QJsonObject json_parent;
        json_parent.insert("cid", data.reply_parent.cid);
        json_parent.insert("uri", data.reply_parent.uri);
        QJsonObject json_reply;
        json_reply.insert("root", json_root);
        json_reply.insert("parent", json_parent);
        json_record.insert("reply", json_reply);
    }

    QJsonObject json_embed_images;

    if (!data.external_link_uri.isEmpty()) {
        // リンクカードと画像添付は排他
        QJsonObject json_external;
        json_external.insert("uri", data.external_link_uri);
        json_external.insert("title", data.external_link_title.left(300));
        json_external.insert("description", data.external_link_description.left(1000));
        if (!data.embed_image_blobs.isEmpty()) {
            QJsonObject json_external_thumb;
            setJsonBlob(data.embed_image_blobs.at(0), json_external_thumb);
            json_external.insert("thumb", json_external_thumb);
        }
        json_embed_images.insert("$type", "app.bsky.embed.external");
        json_embed_images.insert("external", json_external);

    } else if (!data.feed_generator_link_uri.isEmpty()) {
        // カスタムフィードカードとリンクカード
        QJsonObject json_generator;
        json_generator.insert("uri", data.feed_generator_link_uri);
        json_generator.insert("cid", data.feed_generator_link_cid);
        json_embed_images.insert("$type", "app.bsky.embed.record");
        json_embed_images.insert("record", json_generator);

    } else if (!data.embed_image_blobs.isEmpty()) {
        QJsonArray json_blobs;
        for (const auto &blob : std::as_const(data.embed_image_blobs)) {
            QJsonObject json_blob;
            QJsonObject json_image;
            QJsonObject json_aspect_ratio;
            setJsonAspectRatio(blob.aspect_ratio, json_aspect_ratio);
            if (!json_aspect_ratio.isEmpty()) {
                json_blob.insert("aspectRatio", json_aspect_ratio);
            }
            setJsonBlob(blob, json_image);
            json_blob.insert("image", json_image);
            json_blob.insert("alt", blob.alt);
            if (data.embed_image_blobs.length() > 4) {
                json_blob.insert("$type", "app.bsky.embed.gallery#image");
            }

            json_blobs.append(json_blob);
        }

        if (data.embed_image_blobs.length() > 4) {
            json_embed_images.insert("$type", "app.bsky.embed.gallery");
            json_embed_images.insert("items", json_blobs);
        } else {
            json_embed_images.insert("$type", "app.bsky.embed.images");
            json_embed_images.insert("images", json_blobs);
        }
    }

    if (!data.embed_quote.cid.isEmpty() && !data.embed_quote.uri.isEmpty()) {
        QJsonObject json_quote;
        json_quote.insert("cid", data.embed_quote.cid);
        json_quote.insert("uri", data.embed_quote.uri);

        QJsonObject json_embed;
        if (json_embed_images.isEmpty()) {
            json_embed.insert("$type", "app.bsky.embed.record");
            json_embed.insert("record", json_quote);
        } else {
            json_embed.insert("$type", "app.bsky.embed.recordWithMedia");
            json_embed.insert("media", json_embed_images);
            QJsonObject json_record2;
            json_record2.insert("$type", "app.bsky.embed.record");
            json_record2.insert("record", json_quote);
            json_embed.insert("record", json_record2);
        }

        json_record.insert("embed", json_embed);

    } else if (!json_embed_images.isEmpty()) {
        json_record.insert("embed", json_embed_images);
    }

    if (!data.facets.isEmpty()) {
        LexiconsTypeUnknown::insertFacetsJson(json_record, data.facets);
    }

    if (!data.self_labels.isEmpty()) {
        QJsonObject json_labels;
        QJsonArray json_label_vals;
        for (const auto &label : std::as_const(data.self_labels)) {
            QJsonObject json_val;
            json_val.insert("val", label);
            json_label_vals.append(json_val);
        }
        json_labels.insert("$type", "com.atproto.label.defs#selfLabels");
        json_labels.insert("values", json_label_vals);
        json_record.insert("labels", json_labels);
    }

    if (!data.long_post.uri.isEmpty() && !data.long_post.cid.isEmpty()) {
        QJsonObject json_long_post;
        json_long_post.insert("uri", data.long_post.uri);
        json_long_post.insert("cid", data.long_post.cid);
        json_record.insert("site.mochott.longPost", json_long_post);
    }

    return json_record;
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

QJsonObject AccessAtProtocol::makeTokimekiPollJsonObject(const QString &post_uri,
                                                         const QString &post_cid,
                                                         const QStringList &options,
                                                         const int duration)
{
    QJsonObject json_subject;
    json_subject.insert("uri", post_uri);
    json_subject.insert("cid", post_cid);

    QJsonArray json_options;
    for (const auto option : options) {
        json_options.append(QJsonValue(option));
    }

    const auto created_at = QDateTime::currentDateTimeUtc();

    QJsonObject json_record;
    json_record.insert("$type", "tech.tokimeki.poll.poll");
    json_record.insert("subject", json_subject);
    json_record.insert("options", json_options);
    json_record.insert("createdAt", created_at.toString(Qt::ISODateWithMs));
    json_record.insert("endsAt", created_at.addSecs(duration).toString(Qt::ISODateWithMs));

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

bool AccessAtProtocol::dPopNonceRequired() const
{
    return m_dPopNonceRequired;
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

int AccessAtProtocol::httpStatusCode() const
{
    return m_httpStatusCode;
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
