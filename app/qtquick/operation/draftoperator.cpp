#include "operation/draftoperator.h"
#include "atprotocol/app/bsky/draft/appbskydraftcreatedraft.h"
#include "atprotocol/app/bsky/draft/appbskydraftupdatedraft.h"
#include "atprotocol/app/bsky/draft/appbskydraftdeletedraft.h"
#include "atprotocol/app/bsky/draft/appbskydraftgetdrafts.h"
#include "tools/accountmanager.h"
#include "tools/deviceinfo.h"

#include <QJsonArray>

using AtProtocolInterface::AppBskyDraftCreateDraft;
using AtProtocolInterface::AppBskyDraftDeleteDraft;
using AtProtocolInterface::AppBskyDraftGetDrafts;
using AtProtocolInterface::AppBskyDraftUpdateDraft;

DraftOperator::DraftOperator(QObject *parent) : QObject { parent }, m_running(false) { }

AtProtocolInterface::AccountData DraftOperator::account() const
{
    return AccountManager::getInstance()->getAccount(m_account.uuid);
}

QString DraftOperator::accountUuid() const
{
    return m_account.uuid;
}

void DraftOperator::setAccount(const QString &uuid)
{
    m_account.uuid = uuid;
}

void DraftOperator::setText(const QString &text)
{
    m_text = text;
}

void DraftOperator::setImages(const QStringList &images, const QStringList &alts)
{
    m_embedImages.clear();
    for (int i = 0; i < images.length(); i++) {
        DraftEmbedImage e;
        e.path = images.at(i);
        if (i < alts.length()) {
            e.alt = alts.at(i);
        }
        m_embedImages.append(e);
    }
}

void DraftOperator::setExternalLink(const QString &uri)
{
    m_embedExternalUri = uri;
}

void DraftOperator::setQuote(const QString &cid, const QString &uri)
{
    m_embedRecordCid = cid;
    m_embedRecordUri = uri;
}

void DraftOperator::setPostLanguages(const QStringList &langs)
{
    m_postLanguages = langs;
}

void DraftOperator::setSelfLabels(const QStringList &labels)
{
    m_selfLabels = labels;
}

// type = everybody || nobody || choice
// rules = mentioned || followed || follower || at://uri
void DraftOperator::setThreadGate(const QString &type, const QStringList &rules)
{
    m_threadGateType = type;
    m_threadGateRules = rules;
}

// rule = disableRule
// uris = at://uri
void DraftOperator::setPostGate(const bool quote_enabled, const QStringList &uris)
{
    Q_UNUSED(uris); // Draft cannot store detached embedding URIs (determined only when quoted)
    if (quote_enabled) {
        m_postGateEmbeddingRule.clear();
    } else {
        m_postGateEmbeddingRule = "disableRule";
    }
}

void DraftOperator::clear()
{
    m_text.clear();
    m_embedImages.clear();
    m_embedExternalUri.clear();
    m_embedRecordCid.clear();
    m_embedRecordUri.clear();
    m_postLanguages.clear();
    m_selfLabels.clear();
    m_threadGateRules.clear();
    m_postGateEmbeddingRule.clear();
}

void DraftOperator::createDraft()
{
    if (running()) {
        emit errorOccured(QStringLiteral("InProgress"),
                          tr("Another operation is currently in progress"));
        return;
    }

    setRunning(true);
    setProgressMessage(tr("Create draft ..."));

    QJsonObject draft = buildDraftJson();

    AppBskyDraftCreateDraft *create = new AppBskyDraftCreateDraft(this);
    connect(create, &AppBskyDraftCreateDraft::finished, this, [=](bool success) {
        if (success) {
            emit finishedCreateDraft(true, create->id());
        } else {
            emit errorOccured(create->errorCode(), create->errorMessage());
            emit finishedCreateDraft(false, QString());
        }
        setProgressMessage(QString());
        setRunning(false);
        create->deleteLater();
    });
    create->setAccount(account());
    create->createDraft(draft);
}

void DraftOperator::updateDraft(const QString &id)
{
    if (running()) {
        emit errorOccured(QStringLiteral("InProgress"),
                          tr("Another operation is currently in progress"));
        return;
    }

    setRunning(true);
    setProgressMessage(tr("Update draft ..."));

    QJsonObject draft = buildDraftJson();
    QJsonObject draftWithId;
    draftWithId["id"] = id;
    draftWithId["draft"] = draft;

    AppBskyDraftUpdateDraft *update = new AppBskyDraftUpdateDraft(this);
    connect(update, &AppBskyDraftUpdateDraft::finished, this, [=](bool success) {
        if (success) {
            emit finishedUpdateDraft(true);
        } else {
            emit errorOccured(update->errorCode(), update->errorMessage());
            emit finishedUpdateDraft(false);
        }
        setProgressMessage(QString());
        setRunning(false);
        update->deleteLater();
    });
    update->setAccount(account());
    update->updateDraft(draftWithId);
}

void DraftOperator::deleteDraft(const QString &id)
{
    if (running()) {
        emit errorOccured(QStringLiteral("InProgress"),
                          tr("Another operation is currently in progress"));
        return;
    }

    setRunning(true);
    setProgressMessage(tr("Delete draft ..."));

    AppBskyDraftDeleteDraft *del = new AppBskyDraftDeleteDraft(this);
    connect(del, &AppBskyDraftDeleteDraft::finished, this, [=](bool success) {
        if (success) {
            emit finishedDeleteDraft(true);
        } else {
            emit errorOccured(del->errorCode(), del->errorMessage());
            emit finishedDeleteDraft(false);
        }
        setProgressMessage(QString());
        setRunning(false);
        del->deleteLater();
    });
    del->setAccount(account());
    del->deleteDraft(id);
}

void DraftOperator::getDrafts(int limit, const QString &cursor)
{
    if (running()) {
        emit errorOccured(QStringLiteral("InProgress"),
                          tr("Another operation is currently in progress"));
        return;
    }

    setRunning(true);
    setProgressMessage(tr("Get drafts ..."));

    AppBskyDraftGetDrafts *get = new AppBskyDraftGetDrafts(this);
    connect(get, &AppBskyDraftGetDrafts::finished, this, [=](bool success) {
        if (success) {
            emit finishedGetDrafts(true, get->draftsList());
        } else {
            emit errorOccured(get->errorCode(), get->errorMessage());
            emit finishedGetDrafts(false, QList<AtProtocolType::AppBskyDraftDefs::DraftView>());
        }
        setProgressMessage(QString());
        setRunning(false);
        get->deleteLater();
    });
    get->setAccount(account());
    get->getDrafts(limit, cursor);
}

bool DraftOperator::running() const
{
    return m_running;
}

void DraftOperator::setRunning(bool newRunning)
{
    if (m_running == newRunning)
        return;
    m_running = newRunning;
    emit runningChanged();
}

QString DraftOperator::progressMessage() const
{
    return m_progressMessage;
}

void DraftOperator::setProgressMessage(const QString &newProgressMessage)
{
    if (m_progressMessage == newProgressMessage)
        return;
    m_progressMessage = newProgressMessage;
    emit progressMessageChanged();
}

QString DraftOperator::handle() const
{
    return account().handle;
}

QJsonObject DraftOperator::buildDraftJson() const
{
    QJsonObject draftPost;
    draftPost["$type"] = QStringLiteral("app.bsky.draft.defs#draftPost");
    draftPost["text"] = m_text;

    // Self labels
    if (!m_selfLabels.isEmpty()) {
        QJsonArray values;
        for (const QString &label : m_selfLabels) {
            QJsonObject labelObj;
            labelObj["val"] = label;
            values.append(labelObj);
        }
        QJsonObject labelsObj;
        labelsObj["$type"] = QStringLiteral("com.atproto.label.defs#selfLabels");
        labelsObj["values"] = values;
        draftPost["labels"] = labelsObj;
    }

    // Embed images
    if (!m_embedImages.isEmpty()) {
        QJsonArray images;
        for (const DraftEmbedImage &img : m_embedImages) {
            QJsonObject localRef;
            localRef["path"] = img.path;

            QJsonObject imageObj;
            imageObj["localRef"] = localRef;
            if (!img.alt.isEmpty()) {
                imageObj["alt"] = img.alt;
            }
            images.append(imageObj);
        }
        draftPost["embedImages"] = images;
    }

    // Embed external
    if (!m_embedExternalUri.isEmpty()) {
        QJsonArray externals;
        QJsonObject externalObj;
        externalObj["uri"] = m_embedExternalUri;
        externals.append(externalObj);
        draftPost["embedExternals"] = externals;
    }

    // Embed record (quote)
    if (!m_embedRecordUri.isEmpty()) {
        QJsonArray records;
        QJsonObject strongRef;
        strongRef["cid"] = m_embedRecordCid;
        strongRef["uri"] = m_embedRecordUri;

        QJsonObject recordObj;
        recordObj["record"] = strongRef;
        records.append(recordObj);
        draftPost["embedRecords"] = records;
    }

    // Build draft object
    QJsonObject draft;
    QJsonArray posts;
    posts.append(draftPost);
    draft["$type"] = QStringLiteral("app.bsky.draft.defs#draft");
    draft["posts"] = posts;

    // Device ID (hashed MAC address)
    QString deviceId = DeviceInfo::getDeviceId();
    if (!deviceId.isEmpty()) {
        draft["deviceId"] = deviceId;
    }

    // Device Name (computer name)
    QString deviceName = DeviceInfo::getDeviceName();
    if (!deviceName.isEmpty()) {
        draft["deviceName"] = deviceName;
    }

    // Languages
    if (!m_postLanguages.isEmpty()) {
        QJsonArray langs;
        for (const QString &lang : m_postLanguages) {
            langs.append(lang);
        }
        draft["langs"] = langs;
    }

    // Postgate embedding rules
    if (m_postGateEmbeddingRule == QStringLiteral("disableRule")) {
        QJsonArray rules;
        QJsonObject ruleObj;
        ruleObj["$type"] = QStringLiteral("app.bsky.feed.postgate#disableRule");
        rules.append(ruleObj);
        draft["postgateEmbeddingRules"] = rules;
    }

    // Threadgate allow rules
    if (m_threadGateType == QStringLiteral("nobody")) {
        draft["threadgateAllow"] = QJsonArray();
    } else if (m_threadGateType == QStringLiteral("choice")) {
        QJsonArray allows;
        for (const QString &rule : m_threadGateRules) {
            QJsonObject ruleObj;
            if (rule == QStringLiteral("mentioned")) {
                ruleObj["$type"] = QStringLiteral("app.bsky.feed.threadgate#mentionRule");
            } else if (rule == QStringLiteral("followed")) {
                ruleObj["$type"] = QStringLiteral("app.bsky.feed.threadgate#followingRule");
            } else if (rule == QStringLiteral("follower")) {
                ruleObj["$type"] = QStringLiteral("app.bsky.feed.threadgate#followerRule");
            } else if (rule.startsWith(QStringLiteral("at://"))) {
                ruleObj["$type"] = QStringLiteral("app.bsky.feed.threadgate#listRule");
                ruleObj["list"] = rule;
            }
            allows.append(ruleObj);
        }
        draft["threadgateAllow"] = allows;
    }

    return draft;
}
