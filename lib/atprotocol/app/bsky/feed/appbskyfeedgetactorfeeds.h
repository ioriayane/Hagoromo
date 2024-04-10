#ifndef APPBSKYFEEDGETACTORFEEDS_H
#define APPBSKYFEEDGETACTORFEEDS_H

#include "atprotocol/accessatprotocol.h"

namespace AtProtocolInterface {

class AppBskyFeedGetActorFeeds : public AccessAtProtocol
{
public:
    explicit AppBskyFeedGetActorFeeds(QObject *parent = nullptr);

    void getActorFeeds(const QString &actor, const int limit, const QString &cursor);

    const QList<AtProtocolType::AppBskyFeedDefs::GeneratorView> &generatorViewList() const;

private:
    virtual bool parseJson(bool success, const QString reply_json);

    QList<AtProtocolType::AppBskyFeedDefs::GeneratorView> m_generatorViewList;
};

}

#endif // APPBSKYFEEDGETACTORFEEDS_H
