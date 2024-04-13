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
    const bool &isOnline() const;
    const bool &isValid() const;

private:
    virtual bool parseJson(bool success, const QString reply_json);

    AtProtocolType::AppBskyFeedDefs::GeneratorView m_generatorView;
    bool m_isOnline;
    bool m_isValid;
};

}

#endif // APPBSKYFEEDGETFEEDGENERATOR_H
