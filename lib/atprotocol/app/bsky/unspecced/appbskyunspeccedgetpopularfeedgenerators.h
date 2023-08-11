#ifndef APPBSKYUNSPECCEDGETPOPULARFEEDGENERATORS_H
#define APPBSKYUNSPECCEDGETPOPULARFEEDGENERATORS_H

#include "atprotocol/accessatprotocol.h"
#include "atprotocol/lexicons.h"

namespace AtProtocolInterface {

class AppBskyUnspeccedGetPopularFeedGenerators : public AccessAtProtocol
{
public:
    explicit AppBskyUnspeccedGetPopularFeedGenerators(QObject *parent = nullptr);

    void getPopularFeedGenerators(const int limit, const QString &cursor, const QString &query);

    const QList<AtProtocolType::AppBskyFeedDefs::GeneratorView> *generatorViewList() const;

private:
    virtual void parseJson(bool success, const QString reply_json);

    QList<AtProtocolType::AppBskyFeedDefs::GeneratorView> m_generatorViewList;
};

}

#endif // APPBSKYUNSPECCEDGETPOPULARFEEDGENERATORS_H
