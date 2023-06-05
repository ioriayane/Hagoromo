#ifndef COMATPROTOREPOCREATERECORD_H
#define COMATPROTOREPOCREATERECORD_H

#include "atprotocol/accessatprotocol.h"
#include "atprotocol/lexicons.h"
#include "atprotocol/lexicons_func_unknown.h"

namespace AtProtocolInterface {

class ComAtprotoRepoCreateRecord : public AccessAtProtocol
{
public:
    explicit ComAtprotoRepoCreateRecord(QObject *parent = nullptr);

    void post(const QString &text);
    void repost(const QString &cid, const QString &uri);
    void like(const QString &cid, const QString &uri);
    void follow(const QString &did);

    void setReply(const QString &parent_cid, const QString &parent_uri, const QString &root_cid,
                  const QString &root_uri);
    void setQuote(const QString &cid, const QString &uri);
    void setImageBlobs(const QList<AtProtocolType::LexiconsTypeUnknown::Blob> &blobs);

    QString replyCid() const;

    QString replyUri() const;

private:
    virtual void parseJson(const QString reply_json);

    template<typename F>
    void makeFacets(const QString &text, F callback);

    QRegularExpression m_rxUri;

    AtProtocolType::ComAtprotoRepoStrongRef::Main m_replyParent;
    AtProtocolType::ComAtprotoRepoStrongRef::Main m_replyRoot;
    AtProtocolType::ComAtprotoRepoStrongRef::Main m_embedQuote;
    QList<AtProtocolType::LexiconsTypeUnknown::Blob> m_embedImageBlobs;

    QString m_replyCid;
    QString m_replyUri;
};

}

#endif // COMATPROTOREPOCREATERECORD_H
