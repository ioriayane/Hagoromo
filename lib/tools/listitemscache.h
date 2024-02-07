#ifndef LISTITEMSCACHE_H
#define LISTITEMSCACHE_H

#include <QHash>
#include <QString>

struct ListInfo
{
    ListInfo() { }
    ListInfo(const QString &name, const QString &cid)
    {
        this->name = name;
        this->cid = cid;
    }
    QString name;
    QString cid;
};

class ListItemsCache
{
private:
    ListItemsCache() = default;
    ~ListItemsCache() = default;

public:
    ListItemsCache(const ListItemsCache &) = delete;
    ListItemsCache &operator=(const ListItemsCache &) = delete;
    ListItemsCache(ListItemsCache &&) = delete;
    ListItemsCache &operator=(ListItemsCache &&) = delete;

    static ListItemsCache *getInstance();

    void addItem(const QString &account_did, const QString &user_did, const QString &list_name,
                 const QString &list_cid);
    void clear();
    QStringList getListNames(const QString &account_did, const QString &user_did);
    QStringList getListCids(const QString &account_did, const QString &user_did);

private:
    // Hash<account_did, Hash<user_did, List<list_info>>>
    QHash<QString, QHash<QString, QList<ListInfo>>> m_listsHash;
};

#endif // LISTITEMSCACHE_H
