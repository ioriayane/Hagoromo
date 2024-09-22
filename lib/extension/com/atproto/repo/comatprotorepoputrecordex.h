#ifndef COMATPROTOREPOPUTRECORDEX_H
#define COMATPROTOREPOPUTRECORDEX_H

#include "atprotocol/com/atproto/repo/comatprotorepoputrecord.h"

namespace AtProtocolInterface {

class ComAtprotoRepoPutRecordEx : public ComAtprotoRepoPutRecord
{
public:
    explicit ComAtprotoRepoPutRecordEx(QObject *parent = nullptr);

    void profile(const AtProtocolType::Blob &avatar, const AtProtocolType::Blob &banner,
                 const QString &description, const QString &display_name,
                 const QString &pinned_post_uri, const QString &cid);
    void list(const AtProtocolType::Blob &avatar, const QString &purpose,
              const QString &description, const QString &name, const QString &rkey);
    void postGate(const QString &uri,
                  const AtProtocolType::AppBskyFeedPostgate::MainEmbeddingRulesType type,
                  const QStringList &detached_uris);

private:
    virtual bool parseJson(bool success, const QString reply_json);
};

}

#endif // COMATPROTOREPOPUTRECORDEX_H
