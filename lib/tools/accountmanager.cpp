#include "accountmanager.h"
#include "encryption.h"
#include "extension/app/bsky/actor/appbskyactorputpreferencesex.h"
#include "extension/com/atproto/server/comatprotoservercreatesessionex.h"
#include "extension/com/atproto/server/comatprotoserverrefreshsessionex.h"
#include "extension/com/atproto/repo/comatprotorepogetrecordex.h"
#include "atprotocol/app/bsky/actor/appbskyactorgetprofile.h"
#include "atprotocol/app/bsky/actor/appbskyactorgetpreferences.h"
#include "atprotocol/app/bsky/actor/appbskyactorputpreferences.h"
#include "atprotocol/com/atproto/repo/comatprotorepodescriberepo.h"
#include "atprotocol/chat/bsky/convo/chatbskyconvolistconvos.h"
#include "extension/directory/plc/directoryplc.h"
#include "atprotocol/lexicons_func_unknown.h"
#include "tools/pinnedpostcache.h"
#include "tools/authorization.h"
#include "tools/dpopsessionstore.h"
#include "common.h"

#include <QDebug>
#include <QJsonArray>
#include <QJsonObject>
#include <QJsonValue>
#include <QTimer>

using AtProtocolInterface::AccountData;
using AtProtocolInterface::AccountStatus;
using AtProtocolInterface::AppBskyActorGetPreferences;
using AtProtocolInterface::AppBskyActorGetProfile;
using AtProtocolInterface::AppBskyActorPutPreferences;
using AtProtocolInterface::AppBskyActorPutPreferencesEx;
using AtProtocolInterface::AuthType;
using AtProtocolInterface::ChatBskyConvoListConvos;
using AtProtocolInterface::ComAtprotoRepoDescribeRepo;
using AtProtocolInterface::ComAtprotoRepoGetRecordEx;
using AtProtocolInterface::ComAtprotoServerCreateSessionEx;
using AtProtocolInterface::ComAtprotoServerRefreshSessionEx;
using AtProtocolInterface::DirectoryPlc;

class AccountManager::Private : public QObject
{
public:
    explicit Private(AccountManager *parent);
    ~Private();

    QJsonObject save() const;
    void load(const QJsonObject &object);

    bool update(AccountManager::AccountManagerRoles role, const QVariant &value);

    AccountData getAccount() const;
    void updateAccount(const QString &uuid, const QString &service, const QString &identifier,
                       const QString &password, const QString &did, const QString &handle,
                       const QString &email, const QString &accessJwt, const QString &refreshJwt,
                       const QString &thread_gate_type, const AccountStatus status);
    void updateRealtimeFeedRule(const QString &name, const QString &condition);
    QList<AtProtocolInterface::RealtimeFeedRule> getRealtimeFeedRules() const;
    void removeRealtimeFeedRule(const QString &name);
    void createSession();
    void refreshSession(bool initial = false);
    void getProfile();
    void getServiceEndpoint(const QString &did, const QString &service,
                            std::function<void(const QString &service_endpoint)> callback);
    void getPostInteractionSettings(std::function<void()> callback);
    void putPostInteractionSettings();
    void getAccountScope();
    void setMain(bool is);

    void removeScope(AtProtocolInterface::AccountScope scope);

    // OAuth
    void setOAuthSession(const QString &uuid, const QString &service, const OAuthSession &session);
    void clearOAuthSession();

private:
    void refreshOAuthSession(bool initial);
    void finishOAuthRefresh(bool success);
    void failSession(const QString &code, const QString &message);
    void scheduleOAuthRefresh(int expires_in);
    Authorization *oauth();
    static bool allowedChatScope(const QString &scope);

    AccountManager *q;

    AccountData m_account;
    Encryption m_encryption;

    // OAuth(DPoPの秘密鍵はAccountDataに入れず、リクエストごとにコピーされないようにする)
    QByteArray m_dPopPrivateKey;
    QString m_oauthIssuer;
    QString m_oauthTokenEndpoint;
    QString m_oauthScope;
    Authorization *m_oauth;
    QTimer m_oauthRefreshTimer;
    QString m_oauthErrorCode;
    QString m_oauthErrorMessage;
};

AccountManager::Private::Private(AccountManager *parent) : q(parent), m_oauth(nullptr)
{
    qDebug().noquote() << this << "AccountManager::Private()";

    m_oauthRefreshTimer.setSingleShot(true);
    connect(&m_oauthRefreshTimer, &QTimer::timeout, this, [=]() { refreshSession(); });
}

AccountManager::Private::~Private()
{
    qDebug().noquote() << this << "AccountManager::~Private()";
}

QJsonObject AccountManager::Private::save() const
{
    QJsonObject account_item;
    account_item["uuid"] = m_account.uuid;
    account_item["is_main"] = m_account.is_main;
    account_item["service"] = m_account.service;
    account_item["identifier"] = m_account.identifier;
    account_item["password"] = m_encryption.encrypt(m_account.password);
    account_item["refresh_jwt"] = m_encryption.encrypt(m_account.refreshJwt);
    if (m_account.auth_type == AuthType::OAuth) {
        account_item["auth_type"] = "oauth";
        account_item["did"] = m_account.did;
        account_item["handle"] = m_account.handle;
        account_item["service_endpoint"] = m_account.service_endpoint;
        QJsonObject oauth;
        oauth["issuer"] = m_oauthIssuer;
        oauth["token_endpoint"] = m_oauthTokenEndpoint;
        oauth["scope"] = m_oauthScope;
        oauth["dpop_private_key"] = m_encryption.encrypt(QString::fromUtf8(m_dPopPrivateKey));
        account_item["oauth"] = oauth;
    } else {
        account_item["auth_type"] = "password";
    }

    if (!m_account.post_languages.isEmpty()) {
        QJsonArray post_langs;
        for (const auto &lang : m_account.post_languages) {
            post_langs.append(lang);
        }
        account_item["post_languages"] = post_langs;
    }

    // ファイルへの保存はしないでPreferencesに別ルートで保存する（保存は設定ダイアログを開いて操作したときだけなので）
    // （とりあえずは保存しておく）
    if (m_account.thread_gate_type.isEmpty()) {
        account_item["thread_gate_type"] = "everybody";
    } else {
        account_item["thread_gate_type"] = m_account.thread_gate_type;
    }
    if (!m_account.thread_gate_options.isEmpty()) {
        QJsonArray thread_gate_options;
        for (const auto &option : m_account.thread_gate_options) {
            thread_gate_options.append(option);
        }
        account_item["thread_gate_options"] = thread_gate_options;
    }
    account_item["post_gate_quote_enabled"] = m_account.post_gate_quote_enabled;

    if (!m_account.realtime_feed_rules.isEmpty()) {
        QJsonArray realtime_feed_rules;
        for (const auto &rule : m_account.realtime_feed_rules) {
            QJsonObject json_rule;
            json_rule.insert("name", rule.name);
            json_rule.insert("condition", rule.condition);
            realtime_feed_rules.append(json_rule);
        }
        account_item["realtime_feed_rules"] = realtime_feed_rules;
    }

    return account_item;
}

void AccountManager::Private::load(const QJsonObject &object)
{

    QString temp_refresh = object.value("refresh_jwt").toString();
    m_account.uuid = object.value("uuid").toString();
    m_account.is_main = object.value("is_main").toBool();
    m_account.service = object.value("service").toString();
    m_account.service_endpoint = m_account.service;
    m_account.identifier = object.value("identifier").toString();
    m_account.password = m_encryption.decrypt(object.value("password").toString());
    m_account.refreshJwt = m_encryption.decrypt(temp_refresh);
    m_account.handle = m_account.identifier;
    // auth_typeがない(従来の)アカウントはパスワード方式
    if (object.value("auth_type").toString() == "oauth") {
        m_account.auth_type = AuthType::OAuth;
        m_account.did = object.value("did").toString();
        m_account.handle = object.value("handle").toString(m_account.identifier);
        m_account.service_endpoint =
                object.value("service_endpoint").toString(m_account.service_endpoint);
        const QJsonObject oauth = object.value("oauth").toObject();
        m_oauthIssuer = oauth.value("issuer").toString();
        m_oauthTokenEndpoint = oauth.value("token_endpoint").toString();
        m_oauthScope = oauth.value("scope").toString();
        m_dPopPrivateKey =
                m_encryption.decrypt(oauth.value("dpop_private_key").toString()).toUtf8();
        if (!DPopSessionStore::getInstance()->setPrivateKey(m_account.uuid, m_dPopPrivateKey)) {
            qWarning().noquote() << "Failed to load DPoP key :" << m_account.handle;
        }
    } else {
        m_account.auth_type = AuthType::Password;
    }
    for (const auto &value : object.value("post_languages").toArray()) {
        m_account.post_languages.append(value.toString());
    }

    // 互換性のためしばらく読み込みは残す
    m_account.thread_gate_type = object.value("thread_gate_type").toString("everybody");
    if (m_account.thread_gate_type.isEmpty()) {
        m_account.thread_gate_type = "everybody";
    }
    for (const auto &value : object.value("thread_gate_options").toArray()) {
        m_account.thread_gate_options.append(value.toString());
    }
    m_account.post_gate_quote_enabled = object.value("post_gate_quote_enabled").toBool(true);
    // ------

    for (const auto &value : object.value("realtime_feed_rules").toArray()) {
        if (value.toObject().contains("name") && value.toObject().contains("condition")) {
            AtProtocolInterface::RealtimeFeedRule rule;
            rule.name = value.toObject().value("name").toString();
            rule.condition = value.toObject().value("condition").toString();
            m_account.realtime_feed_rules.append(rule);
        }
    }

    if (m_account.auth_type == AuthType::OAuth) {
        // OAuthはパスワードで作り直せないのでrefreshのみ
        refreshSession(true);
    } else if (temp_refresh.isEmpty()) {
        createSession();
    } else {
        refreshSession(true);
    }
}

bool AccountManager::Private::update(AccountManagerRoles role, const QVariant &value)
{
    bool need_save = false;

    if (role == UuidRole)
        m_account.uuid = value.toString();
    else if (role == ServiceRole)
        m_account.service = value.toString();
    else if (role == ServiceEndpointRole)
        m_account.service_endpoint = value.toString();
    else if (role == IdentifierRole)
        m_account.identifier = value.toString();
    else if (role == PasswordRole)
        m_account.password = value.toString();
    else if (role == DidRole)
        m_account.did = value.toString();
    else if (role == HandleRole)
        m_account.handle = value.toString();
    else if (role == EmailRole)
        m_account.email = value.toString();
    else if (role == AccessJwtRole) {
        m_account.accessJwt = value.toString();
        getAccountScope();
    } else if (role == RefreshJwtRole)
        m_account.refreshJwt = value.toString();

    else if (role == DisplayNameRole)
        m_account.displayName = value.toString();
    else if (role == DescriptionRole)
        m_account.description = value.toString();
    else if (role == AvatarRole)
        m_account.avatar = value.toString();

    else if (role == PostLanguagesRole) {
        m_account.post_languages = value.toStringList();
        need_save = true;
    } else if (role == ThreadGateTypeRole) {
        m_account.thread_gate_type = value.toString();
        need_save = true;
    } else if (role == ThreadGateOptionsRole) {
        m_account.thread_gate_options = value.toStringList();
        need_save = true;
    } else if (role == PostGateQuoteEnabledRole) {
        m_account.post_gate_quote_enabled = value.toBool();
    }
    return need_save;
}

AccountData AccountManager::Private::getAccount() const
{
    return m_account;
}

void AccountManager::Private::updateAccount(const QString &uuid, const QString &service,
                                            const QString &identifier, const QString &password,
                                            const QString &did, const QString &handle,
                                            const QString &email, const QString &accessJwt,
                                            const QString &refreshJwt,
                                            const QString &thread_gate_type,
                                            const AccountStatus status)
{
    m_account.uuid = uuid;
    m_account.service = service;
    m_account.identifier = identifier;
    m_account.password = password;
    m_account.did = did;
    m_account.handle = handle;
    m_account.email = email;
    m_account.accessJwt = accessJwt;
    m_account.refreshJwt = refreshJwt;
    m_account.thread_gate_type = thread_gate_type;
    m_account.status = status;
    if (m_account.auth_type == AuthType::OAuth) {
        // パスワード方式に切り替え
        clearOAuthSession();
    }

    getAccountScope();
}

void AccountManager::Private::updateRealtimeFeedRule(const QString &name, const QString &condition)
{
    bool exist = false;
    for (auto &rule : m_account.realtime_feed_rules) {
        if (rule.name == name) {
            rule.condition = condition;
            exist = true;
            break;
        }
    }
    if (!exist) {
        AtProtocolInterface::RealtimeFeedRule rule;
        rule.name = name;
        rule.condition = condition;
        m_account.realtime_feed_rules.append(rule);
    }
}

QList<AtProtocolInterface::RealtimeFeedRule> AccountManager::Private::getRealtimeFeedRules() const
{
    return m_account.realtime_feed_rules;
}

void AccountManager::Private::removeRealtimeFeedRule(const QString &name)
{
    QList<AtProtocolInterface::RealtimeFeedRule> temp;
    for (auto &rule : m_account.realtime_feed_rules) {
        if (rule.name != name) {
            temp.append(rule);
        }
    }
    m_account.realtime_feed_rules = temp;
}

void AccountManager::Private::createSession()
{
    if (m_account.auth_type == AuthType::OAuth) {
        failSession(QStringLiteral("OAuthLoginRequired"),
                    QStringLiteral("Please sign in again with OAuth."));
        return;
    }
    ComAtprotoServerCreateSessionEx *session = new ComAtprotoServerCreateSessionEx(this);
    connect(session, &ComAtprotoServerCreateSessionEx::finished, this, [=](bool success) {
        //        qDebug() << session << session->service() << session->did() <<
        //        session->handle()
        //                 << session->email() << session->accessJwt() << session->refreshJwt();
        //        qDebug() << service << identifier << password;
        if (success) {
            qDebug() << "Create session" << session->did() << session->handle();
            m_account.did = session->did();
            m_account.handle = session->handle();
            m_account.email = session->email();
            m_account.accessJwt = session->accessJwt();
            m_account.refreshJwt = session->refreshJwt();
            getAccountScope();

            // 詳細を取得
            getProfile();
        } else {
            qDebug() << "Fail createSession.";
            m_account.status = AccountStatus::Unauthorized;
            emit q->errorOccurred(session->errorCode(), session->errorMessage());

            q->checkAllAccountsReady();
            if (q->allAccountTried()) {
                emit q->finished();
            }
        }
        session->deleteLater();
    });
    session->setAccount(m_account);
    session->createSession(m_account.identifier, m_account.password, QString(), false);
}

void AccountManager::Private::refreshSession(bool initial)
{
    if (m_account.auth_type == AuthType::OAuth) {
        refreshOAuthSession(initial);
        return;
    }

    ComAtprotoServerRefreshSessionEx *session = new ComAtprotoServerRefreshSessionEx(this);
    connect(session, &ComAtprotoServerRefreshSessionEx::finished, this, [=](bool success) {
        if (success) {
            qDebug() << "Refresh session" << session->did() << session->handle()
                     << session->email();
            m_account.did = session->did();
            m_account.handle = session->handle();
            m_account.email = session->email();
            m_account.accessJwt = session->accessJwt();
            m_account.refreshJwt = session->refreshJwt();
            getAccountScope();

            // 詳細を取得
            getProfile();
        } else {
            if (initial) {
                // 初期化時のみ（つまりloadから呼ばれたときだけは失敗したらcreateSessionで再スタート）
                qDebug() << "Initial refresh session fail.";
                m_account.status = AccountStatus::Unknown;
                createSession();
            } else {
                m_account.status = AccountStatus::Unauthorized;
                emit q->errorOccurred(session->errorCode(), session->errorMessage());

                q->checkAllAccountsReady();
                if (q->allAccountTried()) {
                    emit q->finished();
                }
            }
        }
        session->deleteLater();
    });
    session->setAccount(m_account);
    session->refreshSession();
}

void AccountManager::Private::getProfile()
{

    // OAuthはログイン時に検証したPDSを使うので、取得できなければ今のPDSのまま
    const QString fallback_service =
            (m_account.auth_type == AuthType::OAuth && !m_account.service_endpoint.isEmpty())
            ? m_account.service_endpoint
            : m_account.service;
    getServiceEndpoint(m_account.did, fallback_service, [=](const QString &service_endpoint) {
        m_account.service_endpoint = service_endpoint;
        qDebug().noquote() << "Update service endpoint" << m_account.service << "->"
                           << m_account.service_endpoint;

        // ここでPreferencesからThreadGateの設定を取得
        getPostInteractionSettings([=]() {
            AppBskyActorGetProfile *profile = new AppBskyActorGetProfile(this);
            connect(profile, &AppBskyActorGetProfile::finished, this, [=](bool success) {
                if (success) {
                    AtProtocolType::AppBskyActorDefs::ProfileViewDetailed detail =
                            profile->profileViewDetailed();
                    qDebug() << "Update profile detailed" << detail.displayName
                             << detail.description;
                    m_account.displayName = detail.displayName;
                    m_account.description = detail.description;
                    m_account.avatar = detail.avatar;
                    m_account.banner = detail.banner;
                    m_account.status = AccountStatus::Authorized;

                    q->save();

                    emit q->updatedAccount(m_account.uuid);

                    qDebug() << "Update pinned post" << detail.pinnedPost.uri;
                    PinnedPostCache::getInstance()->update(m_account.did, detail.pinnedPost.uri);
                } else {
                    emit q->errorOccurred(profile->errorCode(), profile->errorMessage());
                }

                q->checkAllAccountsReady();
                if (q->allAccountTried()) {
                    emit q->finished();
                }

                profile->deleteLater();
            });
            profile->setAccount(m_account);
            profile->getProfile(m_account.did);
        });
    });
}

void AccountManager::Private::getServiceEndpoint(const QString &did, const QString &service,
                                                 std::function<void(const QString &)> callback)
{
    if (did.isEmpty()) {
        callback(service);
        return;
    }
    // if (!service.startsWith("https://bsky.social")) {
    //     callback(service);
    //     return;
    // }

    ComAtprotoRepoDescribeRepo *repo = new ComAtprotoRepoDescribeRepo(this);
    connect(repo, &ComAtprotoRepoDescribeRepo::finished, this, [=](bool success) {
        if (success) {
            AtProtocolType::DirectoryPlcDefs::DidDoc doc =
                    AtProtocolType::LexiconsTypeUnknown::fromQVariant<
                            AtProtocolType::DirectoryPlcDefs::DidDoc>(repo->didDoc());
            if (!doc.service.isEmpty()) {
                callback(doc.service.first().serviceEndpoint);
            } else {
                callback(service);
            }
        } else {
            callback(service);
        }
        repo->deleteLater();
    });
    repo->setAccount(m_account);
    repo->describeRepo(did);
}

void AccountManager::Private::getPostInteractionSettings(std::function<void()> callback)
{
    m_account.post_gate_quote_enabled = true;
    m_account.thread_gate_type = "everybody";
    m_account.thread_gate_options.clear();

    AppBskyActorGetPreferences *pref = new AppBskyActorGetPreferences(this);
    connect(pref, &AppBskyActorGetPreferences::finished, this, [=](bool success) {
        if (success && !pref->preferences().postInteractionSettingsPref.isEmpty()) {
            const auto &s = pref->preferences().postInteractionSettingsPref.first();
            if (!s.postgateEmbeddingRules_AppBskyFeedPostgate_DisableRule.isEmpty()) {
                // 引用禁止
                m_account.post_gate_quote_enabled = false;
            } else {
                m_account.post_gate_quote_enabled = true;
            }
            if (s.threadgateAllowRules_type
                == AtProtocolType::AppBskyActorDefs::
                        PostInteractionSettingsPrefThreadgateAllowRulesType::none) {
                // 誰でも everybody
                m_account.thread_gate_type = "everybody";
            } else if (s.threadgateAllowRules_AppBskyFeedThreadgate_MentionRule.isEmpty()
                       && s.threadgateAllowRules_AppBskyFeedThreadgate_FollowingRule.isEmpty()
                       && s.threadgateAllowRules_AppBskyFeedThreadgate_FollowerRule.isEmpty()
                       && s.threadgateAllowRules_AppBskyFeedThreadgate_ListRule.isEmpty()) {
                // 誰も nobody
                m_account.thread_gate_type = "nobody";
            } else {
                // 組み合わせ
                m_account.thread_gate_type = "choice";
                if (!s.threadgateAllowRules_AppBskyFeedThreadgate_MentionRule.isEmpty()) {
                    m_account.thread_gate_options.append("mentioned");
                }
                if (!s.threadgateAllowRules_AppBskyFeedThreadgate_FollowingRule.isEmpty()) {
                    m_account.thread_gate_options.append("followed");
                }
                if (!s.threadgateAllowRules_AppBskyFeedThreadgate_FollowerRule.isEmpty()) {
                    m_account.thread_gate_options.append("follower");
                }
                for (const auto &l : s.threadgateAllowRules_AppBskyFeedThreadgate_ListRule) {
                    m_account.thread_gate_options.append(l.list);
                }
            }
        }
        callback();
        pref->deleteLater();
    });
    pref->setAccount(m_account);
    pref->getPreferences();
}

void AccountManager::Private::putPostInteractionSettings()
{
    AppBskyActorGetPreferences *get_pref = new AppBskyActorGetPreferences(this);
    connect(get_pref, &AppBskyActorGetPreferences::finished, this, [=](bool success) {
        QJsonDocument current_pref = QJsonDocument::fromJson(get_pref->replyJson().toUtf8());
        if (success && current_pref.object().contains("preferences")) {

            AppBskyActorPutPreferencesEx *put_pref = new AppBskyActorPutPreferencesEx(this);
            connect(put_pref, &AppBskyActorPutPreferencesEx::finished, this, [=](bool success) {
                if (success) {
                    qDebug() << "finish put preferences(post interaction settings).";
                }
                emit q->savedPostInteractionSettings(m_account.uuid);
                put_pref->deleteLater();
            });
            put_pref->setAccount(m_account);
            put_pref->putPreferences(put_pref->updatePreferencesJson(
                    get_pref->replyJson(), "app.bsky.actor.defs#postInteractionSettingsPref",
                    put_pref->makePostInteractionSettingsPref(m_account.thread_gate_type,
                                                              m_account.thread_gate_options,
                                                              m_account.post_gate_quote_enabled)));
        } else {
            emit q->savedPostInteractionSettings(m_account.uuid);
        }
        get_pref->deleteLater();
    });
    get_pref->setAccount(m_account);
    get_pref->getPreferences();
}

void AccountManager::Private::getAccountScope()
{
    if (m_account.auth_type == AuthType::OAuth) {
        // アクセストークンの中身は読まず、tokenのレスポンスで許可されたscopeで判断する
        if (allowedChatScope(m_oauthScope)) {
            if (!m_account.scope.contains(AtProtocolInterface::AccountScope::DirectMessage)) {
                m_account.scope.append(AtProtocolInterface::AccountScope::DirectMessage);
            }
        } else {
            m_account.scope.removeAll(AtProtocolInterface::AccountScope::DirectMessage);
        }
        return;
    }
    QStringList items = m_account.accessJwt.split(".");
    if (items.length() != 3)
        return;
    QByteArray json = QByteArray::fromBase64(items.at(1).toUtf8());
    qDebug().noquote() << "Decoded accessJwt:" << json;
    QJsonDocument doc = QJsonDocument::fromJson(json);
    if (!doc.object().contains("scope"))
        return;
    QString scope = doc.object().value("scope").toString();
    if (scope == "com.atproto.appPassPrivileged") {
        qDebug().noquote() << "Direct Message is allowed:" << m_account.handle;
        if (!m_account.scope.contains(AtProtocolInterface::AccountScope::DirectMessage)) {
            m_account.scope.append(AtProtocolInterface::AccountScope::DirectMessage);
        }
    } else {
        qDebug().noquote() << "Direct Message is not allowed:" << m_account.handle;
        if (m_account.scope.contains(AtProtocolInterface::AccountScope::DirectMessage)) {
            m_account.scope.removeAll(AtProtocolInterface::AccountScope::DirectMessage);
        }
    }
}

void AccountManager::Private::setOAuthSession(const QString &uuid, const QString &service,
                                              const OAuthSession &session)
{
    m_account.uuid = uuid;
    m_account.auth_type = AuthType::OAuth;
    m_account.service = service;
    m_account.service_endpoint = session.service_endpoint;
    m_account.identifier = session.handle.isEmpty() ? session.token.sub : session.handle;
    m_account.password.clear();
    m_account.did = session.token.sub;
    m_account.handle = session.handle;
    m_account.accessJwt = session.token.access_token;
    m_account.refreshJwt = session.token.refresh_token;
    m_account.status = AccountStatus::Authorized;
    if (m_account.thread_gate_type.isEmpty()) {
        m_account.thread_gate_type = "everybody";
    }

    m_oauthIssuer = session.issuer;
    m_oauthTokenEndpoint = session.token_endpoint;
    m_oauthScope = session.token.scope;
    m_dPopPrivateKey = session.dpop_private_key;
    if (!DPopSessionStore::getInstance()->setPrivateKey(uuid, m_dPopPrivateKey)) {
        qWarning().noquote() << "Failed to set DPoP key :" << m_account.handle;
        m_account.status = AccountStatus::Unauthorized;
    }

    getAccountScope();
    scheduleOAuthRefresh(session.token.expires_in);
}

void AccountManager::Private::clearOAuthSession()
{
    m_oauthRefreshTimer.stop();
    DPopSessionStore::getInstance()->removeSession(m_account.uuid);
    m_account.auth_type = AuthType::Password;
    m_dPopPrivateKey.clear();
    m_oauthIssuer.clear();
    m_oauthTokenEndpoint.clear();
    m_oauthScope.clear();
    if (m_oauth != nullptr) {
        m_oauth->deleteLater();
        m_oauth = nullptr;
    }
}

void AccountManager::Private::refreshOAuthSession(bool initial)
{
    Q_UNUSED(initial)
    m_oauthRefreshTimer.stop();

    if (m_account.refreshJwt.isEmpty() || m_oauthTokenEndpoint.isEmpty()
        || !DPopSessionStore::getInstance()->hasSession(m_account.uuid)) {
        failSession(QStringLiteral("OAuthLoginRequired"),
                    QStringLiteral("Please sign in again with OAuth."));
        return;
    }

    // refresh tokenは使い捨てなので、アカウントごとに1つのAuthorizationで排他する
    Authorization *authorization = oauth();
    AtProtocolType::OauthDefs::TokenResponse token = authorization->token();
    token.refresh_token = m_account.refreshJwt;
    token.sub = m_account.did;
    authorization->setToken(token);
    authorization->setTokenEndopoint(m_oauthTokenEndpoint);
    if (authorization->dPopPrivateKey() != m_dPopPrivateKey) {
        authorization->setDPopPrivateKey(m_dPopPrivateKey);
    }
    authorization->makeClientId();
    m_oauthErrorCode.clear();
    m_oauthErrorMessage.clear();
    authorization->requestToken(true);
}

void AccountManager::Private::finishOAuthRefresh(bool success)
{
    if (!success) {
        failSession(m_oauthErrorCode.isEmpty() ? QStringLiteral("OAuthLoginRequired")
                                               : m_oauthErrorCode,
                    m_oauthErrorMessage.isEmpty()
                            ? QStringLiteral("Please sign in again with OAuth.")
                            : m_oauthErrorMessage);
        return;
    }
    const AtProtocolType::OauthDefs::TokenResponse token = m_oauth->token();
    qDebug() << "Refresh OAuth session" << m_account.did << m_account.handle;
    m_account.accessJwt = token.access_token;
    m_account.refreshJwt = token.refresh_token;
    if (!token.scope.isEmpty()) {
        m_oauthScope = token.scope;
    }
    getAccountScope();
    scheduleOAuthRefresh(token.expires_in);
    // refresh tokenは更新されて古いものは使えなくなるので、すぐに保存する
    q->save();

    // 詳細を取得
    getProfile();
}

void AccountManager::Private::failSession(const QString &code, const QString &message)
{
    qDebug().noquote() << "Fail session :" << m_account.handle << code;
    m_account.status = AccountStatus::Unauthorized;
    emit q->errorOccurred(code, message);

    q->checkAllAccountsReady();
    if (q->allAccountTried()) {
        emit q->finished();
    }
}

void AccountManager::Private::scheduleOAuthRefresh(int expires_in)
{
    // アクセストークンの期限の少し前にrefreshする
    const int margin = 5 * 60;
    int sec = (expires_in > 0) ? expires_in - margin : 15 * 60;
    if (sec < 60) {
        sec = 60;
    }
    m_oauthRefreshTimer.start(sec * 1000);
}

Authorization *AccountManager::Private::oauth()
{
    if (m_oauth == nullptr) {
        m_oauth = new Authorization(this);
        connect(m_oauth, &Authorization::errorOccurred, this,
                [=](const QString &code, const QString &message) {
                    m_oauthErrorCode = code;
                    m_oauthErrorMessage = message;
                });
        connect(m_oauth, &Authorization::finished, this,
                [=](bool success) { finishOAuthRefresh(success); });
    }
    return m_oauth;
}

bool AccountManager::Private::allowedChatScope(const QString &scope)
{
    for (const auto &item : scope.split(' ', Qt::SkipEmptyParts)) {
        if (item == "transition:chat.bsky" || item.startsWith("include:chat.bsky.")) {
            return true;
        }
        // includeはtokenのレスポンスでrpcに展開される
        if (item.startsWith("rpc") && item.contains("chat.bsky.convo.")) {
            return true;
        }
    }
    return false;
}

void AccountManager::Private::setMain(bool is)
{
    m_account.is_main = is;
}

void AccountManager::Private::removeScope(AtProtocolInterface::AccountScope scope)
{
    m_account.scope.removeAll(scope);
}

AccountManager::AccountManager(QObject *parent) : QObject { parent }, m_allAccountsReady(false)
{
    qDebug().noquote() << this << "AccountManager()";
}

AccountManager::~AccountManager()
{
    qDebug().noquote() << this << "~AccountManager()";
    clear();
}

AccountManager *AccountManager::getInstance()
{
    static AccountManager instance;
    return &instance;
}

void AccountManager::clear()
{
    for (const auto d : dList) {
        delete d;
    }
    dList.clear();
    dIndex.clear();
    emit countChanged();
}

void AccountManager::save() const
{
    QJsonArray account_array;

    for (const auto d : dList) {
        account_array.append(d->save());
    }

    Common::saveJsonDocument(QJsonDocument(account_array), QStringLiteral("account.json"));
}

void AccountManager::load()
{
    QJsonDocument doc = Common::loadJsonDocument(QStringLiteral("account.json"));

    if (doc.isArray()) {
        bool has_main = false;
        for (const auto &item : doc.array()) {
            QString uuid = item.toObject().value("uuid").toString();
            if (!dIndex.contains(uuid)) {
                dList.append(new AccountManager::Private(this));
                dIndex[uuid] = dList.count() - 1;

                emit countChanged();
            }
            dList.at(dIndex[uuid])->load(item.toObject());
            if (dList.at(dIndex[uuid])->getAccount().is_main) {
                has_main = true;
            }
        }
        if (!has_main && !dList.isEmpty()) {
            // mainになっているものがない
            dList.at(0)->setMain(true);
        }
    }
    if (dList.isEmpty()) {
        emit finished();
    }
}

void AccountManager::update(int row, AccountManagerRoles role, const QVariant &value)
{
    if (row < 0 || row >= count())
        return;

    if (dList.at(row)->update(role, value)) {
        save();
    }
}

AccountData AccountManager::getAccount(const QString &uuid) const
{
    if (!dIndex.contains(uuid)) {
        return AccountData();
    }
    return dList.at(dIndex.value(uuid))->getAccount();
}

QString AccountManager::updateAccount(const QString &uuid, const QString &service,
                                      const QString &identifier, const QString &password,
                                      const QString &did, const QString &handle,
                                      const QString &email, const QString &accessJwt,
                                      const QString &refreshJwt, const bool authorized)
{
    QString ret;
    if (!uuid.isEmpty() && dIndex.contains(uuid)) {
        AccountData account = dList.at(dIndex[uuid])->getAccount();
        dList.at(dIndex[uuid])
                ->updateAccount(account.uuid, service, identifier, password, did, handle, email,
                                accessJwt, refreshJwt, account.thread_gate_type,
                                authorized ? AccountStatus::Authorized
                                           : AccountStatus::Unauthorized);
        ret = uuid;
    } else {
        // append
        QString new_uuid = QUuid::createUuid().toString(QUuid::WithoutBraces);
        dList.append(new AccountManager::Private(this));
        dIndex[new_uuid] = dList.count() - 1;
        dList.last()->updateAccount(
                new_uuid, service, identifier, password, did, handle, email, accessJwt, refreshJwt,
                "everybody", authorized ? AccountStatus::Authorized : AccountStatus::Unauthorized);

        ret = new_uuid;
        emit countChanged();
    }
    save();
    checkAllAccountsReady();
    return ret;
}

QString AccountManager::updateOAuthAccount(const QString &uuid, const QString &service,
                                           const OAuthSession &session)
{
    if (!session.token.sub.startsWith("did:")) {
        return QString();
    }
    QString target_uuid;
    if (!uuid.isEmpty() && dIndex.contains(uuid)) {
        target_uuid = uuid;
    } else {
        // 同じアカウントを追加し直した場合(パスワード方式からの切り替えを含む)はuuidを引き継ぐ
        for (const auto d : std::as_const(dList)) {
            if (d->getAccount().did == session.token.sub) {
                target_uuid = d->getAccount().uuid;
                break;
            }
        }
    }
    if (target_uuid.isEmpty()) {
        target_uuid = QUuid::createUuid().toString(QUuid::WithoutBraces);
        dList.append(new AccountManager::Private(this));
        dIndex[target_uuid] = dList.count() - 1;
        dList.last()->setOAuthSession(target_uuid, service, session);
        emit countChanged();
    } else {
        dList.at(dIndex[target_uuid])->setOAuthSession(target_uuid, service, session);
    }
    save();
    checkAllAccountsReady();
    return target_uuid;
}

void AccountManager::removeAccount(const QString &uuid)
{
    if (!dIndex.contains(uuid)) {
        return;
    }
    DPopSessionStore::getInstance()->removeSession(uuid);
    int i = dIndex.value(uuid);
    delete dList.at(i);
    dList.removeAt(i);

    // refresh index
    dIndex.clear();
    for (int i = 0; i < dList.count(); i++) {
        dIndex[dList.at(i)->getAccount().uuid] = i;
    }

    save();
    emit countChanged();
    checkAllAccountsReady();
}

void AccountManager::updateAccountProfile(const QString &uuid)
{
    if (!dIndex.contains(uuid)) {
        return;
    }
    dList.at(dIndex.value(uuid))->getProfile();
}

void AccountManager::updateServiceEndpoint(const QString &uuid, const QString &service_endpoint)
{
    update(indexAt(uuid), AccountManager::AccountManagerRoles::ServiceEndpointRole,
           service_endpoint);
}

void AccountManager::updateRealtimeFeedRule(const QString &uuid, const QString &name,
                                            const QString &condition)
{
    int row = indexAt(uuid);
    if (row < 0 || row >= count() || name.isEmpty() || condition.isEmpty())
        return;

    dList.at(row)->updateRealtimeFeedRule(name, condition);
    save();
}

QList<AtProtocolInterface::RealtimeFeedRule>
AccountManager::getRealtimeFeedRules(const QString &uuid)
{
    int row = indexAt(uuid);
    if (row < 0 || row >= count())
        return QList<AtProtocolInterface::RealtimeFeedRule>();

    return dList.at(row)->getRealtimeFeedRules();
}

void AccountManager::removeRealtimeFeedRule(const QString &uuid, const QString &name)
{
    int row = indexAt(uuid);
    if (row < 0 || row >= count())
        return;

    dList.at(row)->removeRealtimeFeedRule(name);
    save();
}

void AccountManager::loadPostInteractionSettings(const QString &uuid)
{
    int row = indexAt(uuid);
    if (row < 0 || row >= count())
        return;

    dList.at(row)->getPostInteractionSettings(
            [=]() { emit loadedPostInteractionSettings(dList.at(row)->getAccount().uuid); });
}

void AccountManager::savePostInteractionSettings(const QString &uuid,
                                                 const QString &thread_gate_type,
                                                 const QStringList &thread_gate_options,
                                                 const bool post_gate_quote_enabled)
{
    int row = indexAt(uuid);
    if (row < 0 || row >= count())
        return;

    dList.at(row)->update(AccountManagerRoles::ThreadGateTypeRole, thread_gate_type);
    dList.at(row)->update(AccountManagerRoles::ThreadGateOptionsRole, thread_gate_options);
    dList.at(row)->update(AccountManagerRoles::PostGateQuoteEnabledRole, post_gate_quote_enabled);

    dList.at(row)->putPostInteractionSettings();
}

int AccountManager::getMainAccountIndex() const
{
    if (dList.isEmpty())
        return -1;

    for (int i = 0; i < dList.count(); i++) {
        if (dList.at(i)->getAccount().is_main) {
            return i;
        }
    }
    return 0;
}

void AccountManager::setMainAccount(int row)
{
    if (row < 0 || row >= count())
        return;

    for (int i = 0; i < dList.count(); i++) {
        bool new_val = (row == i);
        if (dList.at(i)->getAccount().is_main != new_val) {
            dList.at(i)->setMain(new_val);
        }
    }

    save();
}

bool AccountManager::checkAllAccountsReady()
{
    int ready_count = 0;
    for (const auto d : std::as_const(dList)) {
        if (d->getAccount().status == AccountStatus::Authorized) {
            ready_count++;
        }
    }
    setAllAccountsReady(!dList.isEmpty() && dList.count() == ready_count);
    return allAccountsReady();
}

int AccountManager::indexAt(const QString &uuid)
{
    return dIndex.value(uuid, -1);
}

void AccountManager::removeScope(const QString &uuid, AtProtocolInterface::AccountScope scope)
{
    int row = indexAt(uuid);
    if (row < 0 || row >= count())
        return;

    dList.at(row)->removeScope(scope);
}

QStringList AccountManager::getUuids() const
{
    QStringList uuids;
    for (const auto d : dList) {
        uuids.append(d->getAccount().uuid);
    }
    return uuids;
}

QString AccountManager::getUuid(int row) const
{
    if (row < 0 || row >= count())
        return QString();
    return dList.at(row)->getAccount().uuid;
}

bool AccountManager::allAccountTried() const
{
    int count = 0;
    for (const auto d : dList) {
        if (d->getAccount().status != AccountStatus::Unknown) {
            count++;
        }
    }
    return (dList.count() == count);
}

bool AccountManager::allAccountsReady() const
{
    return m_allAccountsReady;
}

void AccountManager::setAllAccountsReady(bool newAllAccountsReady)
{
    if (m_allAccountsReady == newAllAccountsReady)
        return;
    m_allAccountsReady = newAllAccountsReady;
    emit allAccountsReadyChanged();
}

int AccountManager::count() const
{
    return dList.count();
}

void AccountManager::createSession(int row)
{
    if (row < 0 || row >= count())
        return;
    dList.at(row)->createSession();
}

void AccountManager::refreshSession(int row, bool initial)
{
    if (row < 0 || row >= count())
        return;
    dList.at(row)->refreshSession(initial);
}

void AccountManager::getProfile(int row)
{
    if (row < 0 || row >= count())
        return;
    dList.at(row)->getProfile();
}
