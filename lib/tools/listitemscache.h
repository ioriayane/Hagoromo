#ifndef LISTITEMSCACHE_H
#define LISTITEMSCACHE_H

#include <QHash>
#include <QString>

struct ListInfo
{
    ListInfo() { }
    ListInfo(const QString &name, const QString &uri, const QString &item_uri)
    {
        this->name = name;
        this->uri = uri;
        this->item_uri = item_uri;
    }
    QString name;
    QString uri;
    QString item_uri;
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
                 const QString &list_uri, const QString &item_uri);
    void removeItem(const QString &account_did, const QString &user_did, const QString &list_uri);
    void clear();
    void clear(const QString &account_did);
    QStringList getListNames(const QString &account_did, const QString &user_did) const;
    QStringList getListUris(const QString &account_did, const QString &user_did) const;
    ListInfo getListInfo(const QString &account_did, const QString &user_did,
                         const QString &list_uri) const;

    bool has(const QString &account_did) const;

private:
    // Hash<account_did, Hash<user_did, List<list_info>>>
    QHash<QString, QHash<QString, QList<ListInfo>>> m_listsHash;
};

#endif // LISTITEMSCACHE_H
