#include "listitemscache.h"

ListItemsCache *ListItemsCache::getInstance()
{
    static ListItemsCache instance;
    return &instance;
}

void ListItemsCache::addItem(const QString &account_did, const QString &user_did,
                             const QString &list_name, const QString &list_cid)
{
    if (m_listsHash.contains(account_did)) {
        if (m_listsHash[account_did].contains(user_did)) {
            bool exist = false;
            foreach (const auto &info, m_listsHash[account_did][user_did]) {
                if (info.cid == list_cid) {
                    exist = true;
                    break;
                }
            }
            if (!exist) {
                m_listsHash[account_did][user_did].append(ListInfo(list_name, list_cid));
            }
        } else {
            QList<ListInfo> info_list;
            info_list.append(ListInfo(list_name, list_cid));
            m_listsHash[account_did][user_did] = info_list;
        }
    } else {
        QHash<QString, QList<ListInfo>> list_hash;
        QList<ListInfo> info_list;
        info_list.append(ListInfo(list_name, list_cid));
        list_hash[user_did] = info_list;
        m_listsHash[account_did] = list_hash;
    }
}

void ListItemsCache::clear()
{
    m_listsHash.clear();
}

QStringList ListItemsCache::getListNames(const QString &account_did, const QString &user_did)
{
    QStringList ret;
    if (m_listsHash.contains(account_did) && m_listsHash[account_did].contains(user_did)) {
        foreach (const auto &info, m_listsHash[account_did][user_did]) {
            ret.append(info.name);
        }
    }
    return ret;
}

QStringList ListItemsCache::getListCids(const QString &account_did, const QString &user_did)
{
    QStringList ret;
    if (m_listsHash.contains(account_did) && m_listsHash[account_did].contains(user_did)) {
        foreach (const auto &info, m_listsHash[account_did][user_did]) {
            ret.append(info.cid);
        }
    }
    return ret;
}
