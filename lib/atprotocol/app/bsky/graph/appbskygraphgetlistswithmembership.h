#ifndef APPBSKYGRAPHGETLISTSWITHMEMBERSHIP_H
#define APPBSKYGRAPHGETLISTSWITHMEMBERSHIP_H

#include "atprotocol/accessatprotocol.h"

namespace AtProtocolInterface {

class AppBskyGraphGetListsWithMembership : public AccessAtProtocol
{
public:
    explicit AppBskyGraphGetListsWithMembership(QObject *parent = nullptr);

    void getListsWithMembership(const QString &actor, const int limit, const QString &cursor,
                                const QList<QString> &purposes);

    const QList<AtProtocolType::AppBskyGraphGetListsWithMembership::ListWithMembership> &
    listsWithMembershipList() const;

private:
    virtual bool parseJson(bool success, const QString reply_json);

    QList<AtProtocolType::AppBskyGraphGetListsWithMembership::ListWithMembership>
            m_listsWithMembershipList;
};

}

#endif // APPBSKYGRAPHGETLISTSWITHMEMBERSHIP_H
