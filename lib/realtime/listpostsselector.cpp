#include "listpostsselector.h"

namespace RealtimeFeed {

ListPostsSelector::ListPostsSelector(QObject *parent) : AbstractPostSelector { parent }
{
    setType("list");
    setDisplayType(tr("List"));
}

bool RealtimeFeed::ListPostsSelector::judge(const QJsonObject &object)
{
    if (!ready())
        return false;

    // 自分のリストの更新データのときに追加・削除する
    if (isMy(object)) {
        QJsonObject op = getOperation(object, "app.bsky.graph.listitem");
        if (!op.isEmpty()) {
            QString action = op.value("action").toString();
            if (action == "create") {
                QString path = op.value("path").toString();
                QJsonObject block = getBlock(object, path);
                if (!block.isEmpty()) {
                    QString list_uri = block.value("value").toObject().value("list").toString();
                    QString subject = block.value("value").toObject().value("subject").toString();
                    if (list_uri == listUri() && !m_members.contains(subject)) {
                        UserInfo user;
                        user.did = subject;
                        user.rkey = extractRkey(path);
                        m_members[user.did] = user;
                    }
                }
            } else if (action == "delete") {
                QString rkey = extractRkey(op.value("path").toString());
                if (!rkey.isEmpty()) {
                    for (const auto &user : qAsConst(m_members)) {
                        if (user.rkey == rkey) {
                            m_members.remove(user.did);
                            break;
                        }
                    }
                }
            }
        }
    }

    return isTarget(object) && m_members.contains(getRepo(object)) && matchImageCondition(object)
            && matchMovieCondition(object) && matchQuoteCondition(object)
            && matchRepostCondition(object);
}

bool RealtimeFeed::ListPostsSelector::validate() const
{
    return listUri().startsWith("at://");
}

QStringList RealtimeFeed::ListPostsSelector::canContain() const
{
    return QStringList();
}

void ListPostsSelector::setListMembers(const QString &list_uri, const QList<UserInfo> &members)
{
    if (list_uri != listUri())
        return;

    for (const auto &user : members) {
        m_members[user.did] = user;
    }
}

UserInfo ListPostsSelector::getUser(const QString &did) const
{
    if (m_members.contains(did)) {
        return m_members.value(did);
    } else {
        return AbstractPostSelector::getUser(did);
    }
}

bool ListPostsSelector::needListMembers() const
{
    return true;
}

QStringList ListPostsSelector::getListUris() const
{
    return QStringList() << listUri();
}
}
