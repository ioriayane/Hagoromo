#ifndef CREATERECORD_H
#define CREATERECORD_H

#include "atprotocol/lexicons.h"
#include "atprotocol/lexicons_func_unknown.h"
#include "atprotocol/accessatprotocol.h"
#include <QObject>

class CreateRecord : public QObject
{
    Q_OBJECT

public:
    explicit CreateRecord(QObject *parent = nullptr);

    Q_INVOKABLE void setAccount(const QString &service, const QString &did, const QString &handle,
                                const QString &email, const QString &accessJwt,
                                const QString &refreshJwt);
    Q_INVOKABLE void setText(const QString &text);
    Q_INVOKABLE void setReply(const QString &parent_cid, const QString &parent_uri,
                              const QString &root_cid, const QString &root_uri);
    Q_INVOKABLE void setQuote(const QString &cid, const QString &uri);
    Q_INVOKABLE void setImages(const QStringList &images);

    Q_INVOKABLE void clear();

    Q_INVOKABLE void post();
    Q_INVOKABLE void postWithImages();
    Q_INVOKABLE void repost(const QString &cid, const QString &uri);
    Q_INVOKABLE void like(const QString &cid, const QString &uri);

signals:
    void finished(bool success);

private:
    AtProtocolInterface::AccountData m_account;

    QString m_text;
    AtProtocolType::ComAtprotoRepoStrongRef::Main m_replyParent;
    AtProtocolType::ComAtprotoRepoStrongRef::Main m_replyRoot;
    AtProtocolType::ComAtprotoRepoStrongRef::Main m_embedQuote;
    QStringList m_embedImages;
    QList<AtProtocolType::LexiconsTypeUnknown::Blob> m_embedImageBlogs;
};

#endif // CREATERECORD_H
