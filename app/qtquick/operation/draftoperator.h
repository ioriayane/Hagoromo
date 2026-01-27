#ifndef DRAFTOPERATOR_H
#define DRAFTOPERATOR_H

#include "atprotocol/accessatprotocol.h"
#include <QObject>
#include <QJsonObject>

struct DraftEmbedImage
{
    QString path;
    QString alt;
};

class DraftOperator : public QObject
{
    Q_OBJECT

    Q_PROPERTY(bool running READ running WRITE setRunning NOTIFY runningChanged)
    Q_PROPERTY(QString handle READ handle CONSTANT)

public:
    explicit DraftOperator(QObject *parent = nullptr);

    AtProtocolInterface::AccountData account() const;
    Q_INVOKABLE QString accountUuid() const;
    Q_INVOKABLE void setAccount(const QString &uuid);

    Q_INVOKABLE void setText(const QString &text);
    Q_INVOKABLE void setImages(const QStringList &images, const QStringList &alts);
    Q_INVOKABLE void setExternalLink(const QString &uri);
    Q_INVOKABLE void setQuote(const QString &cid, const QString &uri);
    Q_INVOKABLE void setPostLanguages(const QStringList &langs);
    Q_INVOKABLE void setSelfLabels(const QStringList &labels);
    Q_INVOKABLE void setThreadGateRules(const QStringList &rules);
    Q_INVOKABLE void setPostGateRules(const QStringList &rules);

    Q_INVOKABLE void clear();

    Q_INVOKABLE void createDraft();
    Q_INVOKABLE void updateDraft(const QString &id);
    Q_INVOKABLE void deleteDraft(const QString &id);
    Q_INVOKABLE void getDrafts(int limit, const QString &cursor);

    bool running() const;
    void setRunning(bool newRunning);
    QString handle() const;

signals:
    void errorOccured(const QString &code, const QString &message);
    void finishedCreateDraft(bool success, const QString &id);
    void finishedUpdateDraft(bool success);
    void finishedDeleteDraft(bool success);
    void finishedGetDrafts(bool success,
                           const QList<AtProtocolType::AppBskyDraftDefs::DraftView> &drafts);
    void runningChanged();

private:
    QJsonObject buildDraftJson() const;

    AtProtocolInterface::AccountData m_account;
    bool m_running;

    QString m_text;
    QList<DraftEmbedImage> m_embedImages;
    QString m_embedExternalUri;
    QString m_embedRecordCid;
    QString m_embedRecordUri;
    QStringList m_postLanguages;
    QStringList m_selfLabels;
    QStringList m_threadGateRules;
    QStringList m_postGateRules;
};

#endif // DRAFTOPERATOR_H
