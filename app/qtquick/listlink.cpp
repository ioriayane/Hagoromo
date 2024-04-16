#include "listlink.h"
#include "atprotocol/app/bsky/graph/appbskygraphgetlist.h"

using AtProtocolInterface::AppBskyGraphGetList;

ListLink::ListLink(QObject *parent) : FeedGeneratorLink { parent } { }

void ListLink::getList(const QString &uri)
{
    if (running())
        return;
    setRunning(true);

    clear();

    convertToAtUri("at://%1/app.bsky.graph.list/%2", uri, [=](const QString &at_uri) {
        if (at_uri.isEmpty()) {
            setRunning(false);
            return;
        }
        AppBskyGraphGetList *list = new AppBskyGraphGetList(this);
        connect(list, &AppBskyGraphGetList::finished, [=](bool success) {
            if (success && list->listView().creator) {
                setAvatar(list->listView().avatar);
                setDisplayName(list->listView().name);
                setCreatorHandle(list->listView().creator->handle);
                setLikeCount(0);
                setUri(list->listView().uri);
                setCid(list->listView().cid);
                setDescription(list->listView().description);
                setValid(true);
            }
            setRunning(false);
            list->deleteLater();
        });
        list->setAccount(m_account);
        list->getList(at_uri, 0, QString());
    });
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
