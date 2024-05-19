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
            if (success && list->list().creator) {
                setAvatar(list->list().avatar);
                setDisplayName(list->list().name);
                setCreatorHandle(list->list().creator->handle);
                setLikeCount(0);
                setUri(list->list().uri);
                setCid(list->list().cid);
                setDescription(list->list().description);
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
