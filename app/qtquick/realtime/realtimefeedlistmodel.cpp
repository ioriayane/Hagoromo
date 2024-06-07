#include "realtimefeedlistmodel.h"

#include "realtime/firehosereceiver.h"
#include "atprotocol/app/bsky/graph/appbskygraphgetfollows.h"
#include "atprotocol/app/bsky/graph/appbskygraphgetfollowers.h"

using namespace RealtimeFeed;
using AtProtocolInterface::AppBskyGraphGetFollowers;
using AtProtocolInterface::AppBskyGraphGetFollows;

RealtimeFeedListModel::RealtimeFeedListModel(QObject *parent) : TimelineListModel { parent } { }

RealtimeFeedListModel::~RealtimeFeedListModel()
{
    FirehoseReceiver::getInstance()->removeSelector(this);
}

bool RealtimeFeedListModel::getLatest()
{
    FirehoseReceiver *receiver = FirehoseReceiver::getInstance();
    if (receiver->containsSelector(this)) {
        setRunning(false);
        return false;
    }
    if (selectorJson().isEmpty()) {
        setRunning(false);
        return false;
    }
    QJsonObject json = QJsonDocument::fromJson(selectorJson().toUtf8()).object();
    if (json.isEmpty()) {
        setRunning(false);
        return false;
    }
    setRunning(true);

    AbstractPostSelector *selector = AbstractPostSelector::create(json, this);
    if (selector == nullptr) {
        setRunning(false);
        return false;
    }
    receiver->appendSelector(selector);

    connect(selector, &AbstractPostSelector::selected, this, [=](const QJsonObject &object) {
        qDebug().noquote() << QJsonDocument(object).toJson();
    });

    if (selector->needFollowing() || selector->needFollowers()) {
        m_cursor.clear();
        getFollowing();
    } else {
        selector->setReady(true);
        receiver->start();
        setRunning(false);
    }

    return true;
}

bool RealtimeFeedListModel::getNext()
{
    return true;
}

QString RealtimeFeedListModel::selectorJson() const
{
    return m_selectorJson;
}

void RealtimeFeedListModel::setSelectorJson(const QString &newSelectorJson)
{
    if (m_selectorJson == newSelectorJson)
        return;
    m_selectorJson = newSelectorJson;
    emit selectorJsonChanged();
}

void RealtimeFeedListModel::getFollowing()
{
    AbstractPostSelector *selector = FirehoseReceiver::getInstance()->getSelector(this);
    if (selector == nullptr) {
        setRunning(false);
        return;
    }

    AppBskyGraphGetFollows *profiles = new AppBskyGraphGetFollows(this);
    connect(profiles, &AppBskyGraphGetFollows::finished, this, [=](bool success) {
        m_cursor.clear();
        if (success) {
            if (profiles->followsList().isEmpty()) {
                // fin
                if (selector->needFollowers()) {
                    QTimer::singleShot(0, this, &RealtimeFeedListModel::getFollowers);
                } else {
                    setRunning(false);
                    selector->setReady(true);
                    FirehoseReceiver::getInstance()->start();
                }
            } else {
                // next
                m_cursor = profiles->cursor();
                QTimer::singleShot(0, this, &RealtimeFeedListModel::getFollowing);
            }
        } else {
            emit errorOccured(profiles->errorCode(), profiles->errorMessage());
            setRunning(false);
        }
        profiles->deleteLater();
    });
    profiles->setAccount(account());
    profiles->getFollows(account().did, 0, m_cursor);
}

void RealtimeFeedListModel::getFollowers()
{
    AbstractPostSelector *selector = FirehoseReceiver::getInstance()->getSelector(this);
    if (selector == nullptr) {
        setRunning(false);
        return;
    }

    AppBskyGraphGetFollowers *profiles = new AppBskyGraphGetFollowers(this);
    connect(profiles, &AppBskyGraphGetFollowers::finished, this, [=](bool success) {
        m_cursor.clear();
        if (success) {
            if (profiles->followsList().isEmpty()) {
                // fin
                setRunning(false);
                selector->setReady(true);
                FirehoseReceiver::getInstance()->start();
            } else {
                // next
                m_cursor = profiles->cursor();
                QTimer::singleShot(0, this, &RealtimeFeedListModel::getFollowers);
            }
        } else {
            emit errorOccured(profiles->errorCode(), profiles->errorMessage());
            setRunning(false);
        }
        profiles->deleteLater();
    });
    profiles->setAccount(account());
    profiles->getFollowers(account().did, 0, m_cursor);
}
