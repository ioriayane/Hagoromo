#ifndef APPBSKYGRAPHGETFOLLOWS_H
#define APPBSKYGRAPHGETFOLLOWS_H

#include "atprotocol/accessatprotocol.h"

namespace AtProtocolInterface {

class AppBskyGraphGetFollows : public AccessAtProtocol
{
public:
    explicit AppBskyGraphGetFollows(QObject *parent = nullptr);

    void getFollows(const QString &actor, const int limit, const QString &cursor);

    const AtProtocolType::AppBskyActorDefs::ProfileView &profileView() const;
    const QList<AtProtocolType::AppBskyActorDefs::ProfileView> &profileViewList() const;

protected:
    QString m_listKey;

private:
    virtual bool parseJson(bool success, const QString reply_json);

    AtProtocolType::AppBskyActorDefs::ProfileView m_profileView;
    QList<AtProtocolType::AppBskyActorDefs::ProfileView> m_profileViewList;
};

}

#endif // APPBSKYGRAPHGETFOLLOWS_H
