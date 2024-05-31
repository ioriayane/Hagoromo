#ifndef APPBSKYFEEDGETFEEDGENERATORS_H
#define APPBSKYFEEDGETFEEDGENERATORS_H

#include "atprotocol/accessatprotocol.h"

namespace AtProtocolInterface {

class AppBskyFeedGetFeedGenerators : public AccessAtProtocol
{
public:
    explicit AppBskyFeedGetFeedGenerators(QObject *parent = nullptr);

    void getFeedGenerators(const QList<QString> &feeds);

    const QList<AtProtocolType::AppBskyFeedDefs::GeneratorView> &feedsList() const;

protected:
    QString m_listKey;

private:
    virtual bool parseJson(bool success, const QString reply_json);

    QList<AtProtocolType::AppBskyFeedDefs::GeneratorView> m_feedsList;
};

}

#endif // APPBSKYFEEDGETFEEDGENERATORS_H
