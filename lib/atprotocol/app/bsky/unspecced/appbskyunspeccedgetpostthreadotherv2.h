#ifndef APPBSKYUNSPECCEDGETPOSTTHREADOTHERV2_H
#define APPBSKYUNSPECCEDGETPOSTTHREADOTHERV2_H

#include "atprotocol/accessatprotocol.h"

namespace AtProtocolInterface {

class AppBskyUnspeccedGetPostThreadOtherV2 : public AccessAtProtocol
{
public:
    explicit AppBskyUnspeccedGetPostThreadOtherV2(QObject *parent = nullptr);

    void getPostThreadOtherV2(const QString &anchor);

    const QList<AtProtocolType::AppBskyUnspeccedGetPostThreadOtherV2::ThreadItem> &
    threadList() const;

private:
    virtual bool parseJson(bool success, const QString reply_json);

    QList<AtProtocolType::AppBskyUnspeccedGetPostThreadOtherV2::ThreadItem> m_threadList;
};

}

#endif // APPBSKYUNSPECCEDGETPOSTTHREADOTHERV2_H
