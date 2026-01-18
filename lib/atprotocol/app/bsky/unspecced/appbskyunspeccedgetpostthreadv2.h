#ifndef APPBSKYUNSPECCEDGETPOSTTHREADV2_H
#define APPBSKYUNSPECCEDGETPOSTTHREADV2_H

#include "atprotocol/accessatprotocol.h"

namespace AtProtocolInterface {

class AppBskyUnspeccedGetPostThreadV2 : public AccessAtProtocol
{
public:
    explicit AppBskyUnspeccedGetPostThreadV2(QObject *parent = nullptr);

    void getPostThreadV2(const QString &anchor, const bool above, const int below,
                         const int branchingFactor, const QString &sort);

    const QList<AtProtocolType::AppBskyUnspeccedGetPostThreadV2::ThreadItem> &threadList() const;
    const AtProtocolType::AppBskyFeedDefs::ThreadgateView &threadgate() const;
    const bool &hasOtherReplies() const;

private:
    virtual bool parseJson(bool success, const QString reply_json);

    QList<AtProtocolType::AppBskyUnspeccedGetPostThreadV2::ThreadItem> m_threadList;
    AtProtocolType::AppBskyFeedDefs::ThreadgateView m_threadgate;
    bool m_hasOtherReplies;
};

}

#endif // APPBSKYUNSPECCEDGETPOSTTHREADV2_H
