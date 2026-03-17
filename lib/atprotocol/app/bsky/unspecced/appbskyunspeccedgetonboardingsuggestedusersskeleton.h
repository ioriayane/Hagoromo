#ifndef APPBSKYUNSPECCEDGETONBOARDINGSUGGESTEDUSERSSKELETON_H
#define APPBSKYUNSPECCEDGETONBOARDINGSUGGESTEDUSERSSKELETON_H

#include "atprotocol/accessatprotocol.h"

namespace AtProtocolInterface {

class AppBskyUnspeccedGetOnboardingSuggestedUsersSkeleton : public AccessAtProtocol
{
public:
    explicit AppBskyUnspeccedGetOnboardingSuggestedUsersSkeleton(QObject *parent = nullptr);

    void getOnboardingSuggestedUsersSkeleton(const QString &viewer, const QString &category,
                                             const int limit);

    const QStringList &didsList() const;
    const QString &recId() const;

private:
    virtual bool parseJson(bool success, const QString reply_json);

    QStringList m_didsList;
    QString m_recId;
};

}

#endif // APPBSKYUNSPECCEDGETONBOARDINGSUGGESTEDUSERSSKELETON_H
