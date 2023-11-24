#ifndef APPBSKYGRAPHGETFOLLOWS_H
#define APPBSKYGRAPHGETFOLLOWS_H

#include "atprotocol/accessatprotocol.h"
#include "atprotocol/lexicons.h"

namespace AtProtocolInterface {

class AppBskyGraphGetFollows : public AccessAtProtocol
{
public:
    explicit AppBskyGraphGetFollows(QObject *parent = nullptr);

    bool getFollows(const QString &actor, const int limit, const QString &cursor);

    const QList<AtProtocolType::AppBskyActorDefs::ProfileView> *profileList() const;

protected:
    QString m_listKey;

private:
    virtual bool parseJson(bool success, const QString reply_json);

    QList<AtProtocolType::AppBskyActorDefs::ProfileView> m_profileList;
};

}

#endif // APPBSKYGRAPHGETFOLLOWS_H
