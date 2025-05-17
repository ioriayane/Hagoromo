#ifndef APPBSKYUNSPECCEDGETCONFIG_H
#define APPBSKYUNSPECCEDGETCONFIG_H

#include "atprotocol/accessatprotocol.h"

namespace AtProtocolInterface {

class AppBskyUnspeccedGetConfig : public AccessAtProtocol
{
public:
    explicit AppBskyUnspeccedGetConfig(QObject *parent = nullptr);

    void getConfig();

    const bool &checkEmailConfirmed() const;
    const QList<AtProtocolType::AppBskyUnspeccedGetConfig::LiveNowConfig> &liveNowList() const;

private:
    virtual bool parseJson(bool success, const QString reply_json);

    bool m_checkEmailConfirmed;
    QList<AtProtocolType::AppBskyUnspeccedGetConfig::LiveNowConfig> m_liveNowList;
};

}

#endif // APPBSKYUNSPECCEDGETCONFIG_H
