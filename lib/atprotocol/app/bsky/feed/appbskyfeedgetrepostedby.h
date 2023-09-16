#ifndef APPBSKYFEEDGETREPOSTEDBY_H
#define APPBSKYFEEDGETREPOSTEDBY_H

#include "atprotocol/accessatprotocol.h"
#include "atprotocol/lexicons.h"

namespace AtProtocolInterface {

class AppBskyFeedGetRepostedBy : public AccessAtProtocol
{
public:
    explicit AppBskyFeedGetRepostedBy(QObject *parent = nullptr);

    const QList<AtProtocolType::AppBskyActorDefs::ProfileView> *profileViewList() const;

    void getRepostedBy(const QString &uri, const QString &cid, const int limit,
                       const QString &cursor);

private:
    virtual bool parseJson(bool success, const QString reply_json);

    QList<AtProtocolType::AppBskyActorDefs::ProfileView> m_profileViewList;
};

}

#endif // APPBSKYFEEDGETREPOSTEDBY_H
