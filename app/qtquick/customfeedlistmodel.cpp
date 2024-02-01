#include "customfeedlistmodel.h"

#include "atprotocol/app/bsky/feed/appbskyfeedgetfeed.h"
#include "atprotocol/app/bsky/actor/appbskyactorgetpreferences.h"

using AtProtocolInterface::AppBskyActorGetPreferences;
using AtProtocolInterface::AppBskyFeedGetFeed;

CustomFeedListModel::CustomFeedListModel(QObject *parent)
    : TimelineListModel { parent }, m_saving(false)
{
    connect(&m_feedGeneratorListModel, &FeedGeneratorListModel::runningChanged, [=]() {
        qDebug() << "m_feedGeneratorListModel" << m_feedGeneratorListModel.running();
        setRunning(m_feedGeneratorListModel.running());
        if (!m_feedGeneratorListModel.running()) {
            setSaving(m_feedGeneratorListModel.getSaving(uri()));
        }
    });
}

bool CustomFeedListModel::getLatest()
{
    if (running())
        return false;
    setRunning(true);

    if (m_cidList.isEmpty()) {
        updateFeedSaveStatus();
    }

    updateContentFilterLabels([=]() {
        AppBskyFeedGetFeed *feed = new AppBskyFeedGetFeed(this);
        connect(feed, &AppBskyFeedGetFeed::finished, [=](bool success) {
            if (success) {
                copyFrom(feed);
            } else {
                emit errorOccured(feed->errorCode(), feed->errorMessage());
            }
            QTimer::singleShot(100, this, &CustomFeedListModel::displayQueuedPosts);
            feed->deleteLater();
        });
        feed->setAccount(account());
        feed->getFeed(uri(), 50, QString());
    });
    return true;
}

void CustomFeedListModel::updateFeedSaveStatus()
{
    AppBskyActorGetPreferences *pref = new AppBskyActorGetPreferences(this);
    connect(pref, &AppBskyActorGetPreferences::finished, [=](bool success) {
        if (success) {
            bool exist = false;
            for (const auto &feed : *pref->savedFeedsPrefList()) {
                for (const auto &saved : feed.saved) {
                    if (saved == uri()) {
                        exist = true;
                        break;
                    }
                }
            }
            setSaving(exist);
        } else {
            emit errorOccured(pref->errorCode(), pref->errorMessage());
        }
        pref->deleteLater();
    });
    pref->setAccount(account());
    pref->getPreferences();
}

void CustomFeedListModel::saveGenerator()
{
    if (uri().isEmpty())
        return;
    m_feedGeneratorListModel.setAccount(account().service, account().did, account().handle,
                                        account().email, account().accessJwt, account().refreshJwt);
    m_feedGeneratorListModel.saveGenerator(uri());
}

void CustomFeedListModel::removeGenerator()
{
    if (uri().isEmpty())
        return;
    m_feedGeneratorListModel.setAccount(account().service, account().did, account().handle,
                                        account().email, account().accessJwt, account().refreshJwt);
    m_feedGeneratorListModel.removeGenerator(uri());
}

QString CustomFeedListModel::getOfficialUrl() const
{
    return atUriToOfficialUrl(uri(), QStringLiteral("feed"));
}

QString CustomFeedListModel::uri() const
{
    return m_uri;
}

void CustomFeedListModel::setUri(const QString &newUri)
{
    if (m_uri == newUri)
        return;
    m_uri = newUri;
    emit uriChanged();
}

bool CustomFeedListModel::saving() const
{
    return m_saving;
}

void CustomFeedListModel::setSaving(bool newSaving)
{
    if (m_saving == newSaving)
        return;
    m_saving = newSaving;
    emit savingChanged();
}
