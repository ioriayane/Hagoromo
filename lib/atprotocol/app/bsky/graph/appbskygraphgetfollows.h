#ifndef APPBSKYGRAPHGETFOLLOWS_H
#define APPBSKYGRAPHGETFOLLOWS_H

#include "atprotocol/accessatprotocol.h"

namespace AtProtocolInterface {

class AppBskyGraphGetFollows : public AccessAtProtocol
{
public:
    explicit AppBskyGraphGetFollows(QObject *parent = nullptr);

    void getFollows(const QString &actor, const int limit, const QString &cursor);

    const AtProtocolType::AppBskyActorDefs::ProfileView &subject() const;
    const QList<AtProtocolType::AppBskyActorDefs::ProfileView> &followsList() const;

protected:
    QString m_listKey;

private:
    virtual bool parseJson(bool success, const QString reply_json);

    AtProtocolType::AppBskyActorDefs::ProfileView m_subject;
    QList<AtProtocolType::AppBskyActorDefs::ProfileView> m_followsList;
};

}

#endif // APPBSKYGRAPHGETFOLLOWS_H
