#include "listlink.h"
#include "atprotocol/app/bsky/graph/appbskygraphgetlist.h"

using AtProtocolInterface::AppBskyGraphGetList;

ListLink::ListLink(QObject *parent) : FeedGeneratorLink { parent } { }

bool ListLink::checkUri(const QString &uri) const
{
    // https://bsky.app/profile/did:hogehoge/lists/rkey
    if (uri.isEmpty())
        return false;
    if (!uri.startsWith("https://bsky.app/profile/"))
        return false;
    QStringList items = uri.split("/");
    if (items.length() != 7)
        return false;
    if (!items.at(4).startsWith("did:plc:"))
        return false;
    if (items.at(5) != "lists")
        return false;
    if (items.at(6).isEmpty())
        return false;

    return true;
}

QString ListLink::convertToAtUri(const QString &uri)
{
    if (!checkUri(uri))
        return QString();

    QStringList items = uri.split("/");

    return QString("at://%1/app.bsky.graph.list/%2").arg(items.at(4), items.at(6));
}

void ListLink::getList(const QString &uri)
{
    if (running())
        return;
    setRunning(true);

    clear();

    AppBskyGraphGetList *list = new AppBskyGraphGetList(this);
    connect(list, &AppBskyGraphGetList::finished, [=](bool success) {
        if (success && list->listView() && list->listView()->creator) {
            setAvatar(list->listView()->avatar);
            setDisplayName(list->listView()->name);
            setCreatorHandle(list->listView()->creator->handle);
            setLikeCount(0);
            setUri(list->listView()->uri);
            setCid(list->listView()->cid);
            setDescription(list->listView()->description);
            setValid(true);
        }
        setRunning(false);
        list->deleteLater();
    });
    list->setAccount(m_account);
    list->getList(uri, 0, QString());
}

void ListLink::clear()
{
    FeedGeneratorLink::clear();
    setDescription(QString());
}

QString ListLink::description() const
{
    return m_description;
}

void ListLink::setDescription(const QString &newDescription)
{
    if (m_description == newDescription)
        return;
    m_description = newDescription;
    emit descriptionChanged();
}
