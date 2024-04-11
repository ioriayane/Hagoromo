#ifndef APPBSKYFEEDGETFEEDGENERATOR_H
#define APPBSKYFEEDGETFEEDGENERATOR_H

#include "atprotocol/accessatprotocol.h"

namespace AtProtocolInterface {

class AppBskyFeedGetFeedGenerator : public AccessAtProtocol
{
public:
    explicit AppBskyFeedGetFeedGenerator(QObject *parent = nullptr);

    void getFeedGenerator(const QString &feed);

    const AtProtocolType::AppBskyFeedDefs::GeneratorView &generatorView() const;

private:
    virtual bool parseJson(bool success, const QString reply_json);

    AtProtocolType::AppBskyFeedDefs::GeneratorView m_generatorView;
};

}

#endif // APPBSKYFEEDGETFEEDGENERATOR_H
