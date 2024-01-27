#ifndef APPBSKYGRAPHGETLISTS_H
#define APPBSKYGRAPHGETLISTS_H

#include "atprotocol/accessatprotocol.h"
#include "atprotocol/lexicons.h"

namespace AtProtocolInterface {

class AppBskyGraphGetLists : public AccessAtProtocol
{
public:
    explicit AppBskyGraphGetLists(QObject *parent = nullptr);

    void getLists(const QString &actor, const int limit, const QString &cursor);

    const QList<AtProtocolType::AppBskyGraphDefs::ListView> *listViewList() const;

private:
    virtual bool parseJson(bool success, const QString reply_json);

    QList<AtProtocolType::AppBskyGraphDefs::ListView> m_listViewList;
};

}

#endif // APPBSKYGRAPHGETLISTS_H
