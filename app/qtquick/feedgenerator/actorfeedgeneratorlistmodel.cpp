#include "actorfeedgeneratorlistmodel.h"

#include "atprotocol/app/bsky/feed/appbskyfeedgetactorfeeds.h"

using AtProtocolInterface::AppBskyFeedGetActorFeeds;

ActorFeedGeneratorListModel::ActorFeedGeneratorListModel(QObject *parent)
    : FeedGeneratorListModel { parent }
{
}

bool ActorFeedGeneratorListModel::getLatest()
{
    if (running() || actor().isEmpty())
        return false;
    setRunning(true);

    clear();

    AppBskyFeedGetActorFeeds *feeds = new AppBskyFeedGetActorFeeds(this);
    connect(feeds, &AppBskyFeedGetActorFeeds::finished, [=](bool success) {
        if (success) {
            m_cursor = feeds->cursor();

            if (!feeds->feedsList().isEmpty()) {
                beginInsertRows(QModelIndex(), 0, feeds->feedsList().count() - 1);
                for (const auto &generator : feeds->feedsList()) {
                    m_cidList.append(generator.cid);
                    m_generatorViewHash[generator.cid] = generator;
                }
                endInsertRows();
                getSavedGenerators();
            } else {
                setRunning(false);
            }
        } else {
            emit errorOccured(feeds->errorCode(), feeds->errorMessage());
            setRunning(false);
        }
        feeds->deleteLater();
    });
    feeds->setAccount(account());
    feeds->getActorFeeds(actor(), 50, QString());

    return true;
}

bool ActorFeedGeneratorListModel::getNext()
{
    return true;
}

QString ActorFeedGeneratorListModel::actor() const
{
    return m_actor;
}

void ActorFeedGeneratorListModel::setActor(const QString &newActor)
{
    if (m_actor == newActor)
        return;
    m_actor = newActor;
    emit actorChanged();
}
