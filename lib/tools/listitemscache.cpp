#include "listitemscache.h"
#include <QDebug>

ListItemsCache::ListItemsCache(QObject *parent) : QObject { parent }
{
    qDebug() << "ListItemsCache()" << this << parent;
}

ListItemsCache::~ListItemsCache()
{
    qDebug() << "~ListItemsCache()" << this;
}

ListItemsCache *ListItemsCache::getInstance()
{
    static ListItemsCache instance;
    return &instance;
}

void ListItemsCache::addItem(const QString &account_did, const QString &user_did,
                             const QString &list_name, const QString &list_uri,
                             const QString &item_uri)
{
    if (m_listsHash.contains(account_did)) {
        if (m_listsHash[account_did].contains(user_did)) {
            bool exist = false;
            foreach (const ListInfo &info, m_listsHash[account_did][user_did]) {
                if (info.uri == list_uri) {
                    exist = true;
                    break;
                }
            }
            if (!exist) {
                m_listsHash[account_did][user_did].append(ListInfo(list_name, list_uri, item_uri));
                emit updated(account_did, user_did);
            }
        } else {
            QList<ListInfo> info_list;
            info_list.append(ListInfo(list_name, list_uri, item_uri));
            m_listsHash[account_did][user_did] = info_list;
            emit updated(account_did, user_did);
        }
    } else {
        QHash<QString, QList<ListInfo>> list_hash;
        QList<ListInfo> info_list;
        info_list.append(ListInfo(list_name, list_uri, item_uri));
        list_hash[user_did] = info_list;
        m_listsHash[account_did] = list_hash;
        emit updated(account_did, user_did);
    }
}

void ListItemsCache::removeItem(const QString &account_did, const QString &user_did,
                                const QString &list_uri)
{
    if (m_listsHash.contains(account_did)) {
        if (m_listsHash[account_did].contains(user_did)) {
            for (int i = 0; i < m_listsHash[account_did][user_did].count(); i++) {
                if (m_listsHash[account_did][user_did].at(i).uri == list_uri) {
                    m_listsHash[account_did][user_did].removeAt(i);
                    emit updated(account_did, user_did);
                    break;
                }
            }
        }
    }
}

void ListItemsCache::clear()
{
    m_listsHash.clear();
    emit updated(QString(), QString());
}

void ListItemsCache::clear(const QString &account_did)
{
    if (m_listsHash.contains(account_did)) {
        m_listsHash[account_did] = QHash<QString, QList<ListInfo>>();
        emit updated(account_did, QString());
    }
}

QStringList ListItemsCache::getListNames(const QString &account_did, const QString &user_did) const
{
    QStringList ret;
    if (m_listsHash.contains(account_did) && m_listsHash[account_did].contains(user_did)) {
        foreach (const auto &info, m_listsHash[account_did][user_did]) {
            ret.append(info.name);
        }
    }
    return ret;
}

QStringList ListItemsCache::getListUris(const QString &account_did, const QString &user_did) const
{
    QStringList ret;
    if (m_listsHash.contains(account_did) && m_listsHash[account_did].contains(user_did)) {
        foreach (const auto &info, m_listsHash[account_did][user_did]) {
            ret.append(info.uri);
        }
    }
    return ret;
}

ListInfo ListItemsCache::getListInfo(const QString &account_did, const QString &user_did,
                                     const QString &list_uri) const
{
    if (m_listsHash.contains(account_did)) {
        if (m_listsHash[account_did].contains(user_did)) {
            for (int i = 0; i < m_listsHash[account_did][user_did].count(); i++) {
                if (m_listsHash[account_did][user_did].at(i).uri == list_uri) {
                    return m_listsHash[account_did][user_did].at(i);
                }
            }
        }
    }
    return ListInfo();
}

bool ListItemsCache::has(const QString &account_did) const
{
    // 該当アカウントのデータが1つでもあるか
    return m_listsHash.contains(account_did) && !m_listsHash[account_did].empty();
}
