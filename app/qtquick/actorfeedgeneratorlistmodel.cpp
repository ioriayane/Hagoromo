#include "actorfeedgeneratorlistmodel.h"

#include "atprotocol/app/bsky/feed/appbskyfeedgetactorfeeds.h"

using AtProtocolInterface::AppBskyFeedGetActorFeeds;

ActorFeedGeneratorListModel::ActorFeedGeneratorListModel(QObject *parent)
    : FeedGeneratorListModel { parent }
{
}

void ActorFeedGeneratorListModel::getLatest()
{
    if (running() || actor().isEmpty())
        return;
    setRunning(true);

    clear();

    AppBskyFeedGetActorFeeds *feeds = new AppBskyFeedGetActorFeeds(this);
    connect(feeds, &AppBskyFeedGetActorFeeds::finished, [=](bool success) {
        if (success) {
            beginInsertRows(QModelIndex(), 0, feeds->generatorViewList()->count() - 1);
            for (const auto &generator : *feeds->generatorViewList()) {
                m_cidList.append(generator.cid);
                m_generatorViewHash[generator.cid] = generator;
            }
            endInsertRows();

            m_cursor = feeds->cursor();
            getSavedGenerators();
        } else {
            emit errorOccured(feeds->errorMessage());
            setRunning(false);
        }
        feeds->deleteLater();
    });
    feeds->setAccount(account());
    feeds->getActorFeeds(actor(), 50, QString());
}

void ActorFeedGeneratorListModel::getNext()
{
    //
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
