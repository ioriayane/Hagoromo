#ifndef APPBSKYUNSPECCEDGETPOSTTHREADHIDDENV2_H
#define APPBSKYUNSPECCEDGETPOSTTHREADHIDDENV2_H

#include "atprotocol/accessatprotocol.h"

namespace AtProtocolInterface {

class AppBskyUnspeccedGetPostThreadHiddenV2 : public AccessAtProtocol
{
public:
    explicit AppBskyUnspeccedGetPostThreadHiddenV2(QObject *parent = nullptr);

    void getPostThreadHiddenV2(const QString &anchor);

    const QList<AtProtocolType::AppBskyUnspeccedGetPostThreadHiddenV2::ThreadHiddenItem> &
    threadList() const;

private:
    virtual bool parseJson(bool success, const QString reply_json);

    QList<AtProtocolType::AppBskyUnspeccedGetPostThreadHiddenV2::ThreadHiddenItem> m_threadList;
};

}

#endif // APPBSKYUNSPECCEDGETPOSTTHREADHIDDENV2_H
