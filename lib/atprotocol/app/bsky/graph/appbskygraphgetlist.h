#ifndef APPBSKYGRAPHGETLIST_H
#define APPBSKYGRAPHGETLIST_H

#include "atprotocol/accessatprotocol.h"

namespace AtProtocolInterface {

class AppBskyGraphGetList : public AccessAtProtocol
{
public:
    explicit AppBskyGraphGetList(QObject *parent = nullptr);

    void getList(const QString &list, const int limit, const QString &cursor);

    const AtProtocolType::AppBskyGraphDefs::ListView &listView() const;
    const QList<AtProtocolType::AppBskyGraphDefs::ListItemView> &listItemViewList() const;

private:
    virtual bool parseJson(bool success, const QString reply_json);

    AtProtocolType::AppBskyGraphDefs::ListView m_listView;
    QList<AtProtocolType::AppBskyGraphDefs::ListItemView> m_listItemViewList;
};

}

#endif // APPBSKYGRAPHGETLIST_H
