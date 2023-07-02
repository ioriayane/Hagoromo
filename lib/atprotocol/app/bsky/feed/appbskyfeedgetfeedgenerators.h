#ifndef APPBSKYFEEDGETFEEDGENERATORS_H
#define APPBSKYFEEDGETFEEDGENERATORS_H

#include "atprotocol/accessatprotocol.h"
#include "atprotocol/lexicons.h"

namespace AtProtocolInterface {

class AppBskyFeedGetFeedGenerators : public AccessAtProtocol
{
public:
    explicit AppBskyFeedGetFeedGenerators(QObject *parent = nullptr);

    void getFeedGenerators(const QList<QString> &feeds);

    const QList<AtProtocolType::AppBskyFeedDefs::GeneratorView> *generatorViewList() const;

private:
    virtual void parseJson(bool success, const QString reply_json);

    QList<AtProtocolType::AppBskyFeedDefs::GeneratorView> m_generatorViewList;
};

}

#endif // APPBSKYFEEDGETFEEDGENERATORS_H
