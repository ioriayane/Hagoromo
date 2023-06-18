#ifndef APPBSKYUNSPECCEDGETPOPULARFEEDGENERATORS_H
#define APPBSKYUNSPECCEDGETPOPULARFEEDGENERATORS_H

#include "atprotocol/accessatprotocol.h"
#include "atprotocol/lexicons.h"

namespace AtProtocolInterface {

class AppBskyUnspeccedGetPopularFeedGenerators : public AccessAtProtocol
{
public:
    explicit AppBskyUnspeccedGetPopularFeedGenerators(QObject *parent = nullptr);

    void getPopularFeedGenerators();

    const QList<AtProtocolType::AppBskyFeedDefs::GeneratorView> *generatorViewList() const;

private:
    virtual void parseJson(const QString reply_json);

    QList<AtProtocolType::AppBskyFeedDefs::GeneratorView> m_generatorViewList;
};

}

#endif // APPBSKYUNSPECCEDGETPOPULARFEEDGENERATORS_H
