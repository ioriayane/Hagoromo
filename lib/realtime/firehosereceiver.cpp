#include "firehosereceiver.h"

#include <QDebug>
#include <QJsonArray>
#include <QJsonObject>
#include <QJsonDocument>
#include <QDateTime>

#define USE_JETSTREAM

using AtProtocolInterface::ComAtprotoSyncSubscribeReposEx;

namespace RealtimeFeed {

FirehoseReceiver::FirehoseReceiver(QObject *parent)
    : QObject(parent)
#ifdef QT_DEBUG // HAGOROMO_UNIT_TEST
      ,
      forUnittest(false)
#endif
      ,
      m_wdgCounter(0),
      m_status(FirehoseReceiverStatus::Disconnected)
{
#ifdef USE_JETSTREAM
    m_serviceEndpoint = "wss://jetstream2.us-west.bsky.network";
    // m_serviceEndpoint = "ws://localhost:19283";
#else
    m_serviceEndpoint = "wss://bsky.network";
#endif
    m_wdgTimer.setInterval(10 * 1000);
    m_analysisTimer.start();

    connect(&m_client, &ComAtprotoSyncSubscribeReposEx::errorOccured,
            [this](const QString &error, const QString &message) {
                qDebug().noquote() << "Error:" << error << message;
                setStatus(FirehoseReceiverStatus::Error);
                emit errorOccured(error, message);
                emit receivingChanged(false);
            });
    connect(&m_client, &ComAtprotoSyncSubscribeReposEx::received,
            [=](const QString &type, const QJsonObject &json) {
                m_wdgCounter = 0;
                emit receivingChanged(true);

                if (type != "#commit")
                    return;
                // qDebug().noquote() << "commitDataReceived:" << type << !json.isEmpty();
                analizeReceivingData(json);
                emit judgeSelectionAndReaction(json); // スレッドのselectorへ通知
            });
    connect(&m_client, &ComAtprotoSyncSubscribeReposEx::connectedToService, [this]() {
        setStatus(FirehoseReceiverStatus::Connected);
        emit connectedToService();
    });
    connect(&m_client, &ComAtprotoSyncSubscribeReposEx::disconnectFromService, [this]() {
        setStatus(FirehoseReceiverStatus::Disconnected);
        emit receivingChanged(false);
        emit disconnectFromService();
    });

    connect(&m_wdgTimer, &QTimer::timeout, [this]() {
        if (m_wdgCounter < 12) {
            m_wdgCounter++;
        } else {
            qDebug().noquote() << "FirehoseTimeout : Nothing was received via Websocket within the "
                                  "specified time."
                               << m_wdgCounter;
            setStatus(FirehoseReceiverStatus::Error);
            emit errorOccured("FirehoseTimeout",
                              "Nothing was received via Websocket within the specified time.");
            emit receivingChanged(false);
            stop();
        }
    });

    m_client.moveToThread(&m_clientThread);
    m_clientThread.start();
}

FirehoseReceiver::~FirehoseReceiver()
{
    qDebug() << this << "~FirehoseReceiver()";
    m_clientThread.quit();
    m_clientThread.wait();
    stop();
}

FirehoseReceiver *FirehoseReceiver::getInstance()
{
    static FirehoseReceiver instance;
    return &instance;
}

void FirehoseReceiver::start()
{
    if (m_selectorHash.isEmpty())
        return;

#ifdef QT_DEBUG // HAGOROMO_UNIT_TEST
    if (forUnittest) {
        qDebug().noquote() << "Connect to dummy --- no start on unit test mode";
        emit connectedToService();
        return;
    }
#endif
    QString path = serviceEndpoint();
    if (path.endsWith("/")) {
        path.resize(path.length() - 1);
    }
#ifdef USE_JETSTREAM
    ComAtprotoSyncSubscribeReposEx::SubScribeMode mode =
            ComAtprotoSyncSubscribeReposEx::SubScribeMode::JetStream;
    QUrl url(path + "/subscribe?wantedCollections=app.bsky.feed.post"
             + "&wantedCollections=app.bsky.feed.repost" + "&wantedCollections=app.bsky.feed.like"
             + "&wantedCollections=app.bsky.graph.follow"
             + "&wantedCollections=app.bsky.graph.listitem");
#else
    ComAtprotoSyncSubscribeReposEx::SubScribeMode mode =
            ComAtprotoSyncSubscribeReposEx::SubScribeMode::Firehose;
    QUrl url(path + "/xrpc/com.atproto.sync.subscribeRepos");
#endif
    qDebug().noquote() << "Connect to" << url.toString();
    m_client.open(url, mode);
    m_wdgCounter = 0;
    m_wdgTimer.start();
}

void FirehoseReceiver::stop()
{
    m_wdgTimer.stop();
    if (m_client.state() == QAbstractSocket::SocketState::UnconnectedState
        || m_client.state() == QAbstractSocket::SocketState::ClosingState)
        return;
    m_client.close();
}

void FirehoseReceiver::appendSelector(AbstractPostSelector *selector)
{
    if (selector == nullptr)
        return;
    m_selectorMutex.lock();
    if (!m_selectorHash.contains(selector)) {
        qDebug().quote() << "appendSelector" << selector << selector->type();
        m_selectorHash[selector->key()] = selector;
        appendThreadSelector(selector);
    }
    m_selectorMutex.unlock();
}

void FirehoseReceiver::removeSelector(QObject *parent)
{
    if (parent == nullptr)
        return;
    m_selectorMutex.lock();
    if (m_selectorHash.contains(parent)) {
        auto s = m_selectorHash[parent];
        m_selectorHash.remove(parent);
        if (s) {
            qDebug().quote() << "removeSelector" << s << s->type();
            removeThreadSelector(s->key());
            s->deleteLater();
        }
        qDebug().quote() << "remain count" << m_selectorHash.count();
    }
    for (const auto key : m_selectorHash.keys()) {
        if (!m_selectorHash[key]) {
            qDebug() << "already deleted -> clean up";
            m_selectorHash.remove(key);
            removeThreadSelector(key);
        }
    }
    if (m_selectorHash.isEmpty()) {
        qDebug().quote() << "stop";
        stop();
    }
    m_selectorMutex.unlock();
}

void FirehoseReceiver::removeAllSelector()
{
    m_selectorMutex.lock();
    for (auto s : qAsConst(m_selectorHash)) {
        if (s) {
            removeThreadSelector(s->key());
            s->deleteLater();
        }
    }
    m_selectorHash.clear();
    m_selectorMutex.unlock();
    stop();
}

AbstractPostSelector *FirehoseReceiver::getSelector(QObject *parent) const
{
    if (parent == nullptr)
        return nullptr;
    return m_selectorHash.value(parent, nullptr);
}

bool FirehoseReceiver::containsSelector(QObject *parent) const
{
    return m_selectorHash.contains(parent);
}

int FirehoseReceiver::countSelector() const
{
    return m_selectorHash.count();
}

bool FirehoseReceiver::selectorIsReady(QObject *parent)
{
    AbstractPostSelector *selector = getSelector(parent);
    if (selector == nullptr) {
        return false;
    }
    return selector->ready();
}

#ifdef QT_DEBUG // HAGOROMO_UNIT_TEST
void FirehoseReceiver::testReceived(const QJsonObject &json)
{
    for (auto s : qAsConst(m_selectorHash)) {
        if (!s) {
            // already deleted
        } else if (!s->ready()) {
            // no op
        } else if (s->judge(json)) {
            qDebug().noquote().nospace() << QJsonDocument(json).toJson();
            emit s->selected(json);
        }
    }
}
#endif

QString FirehoseReceiver::serviceEndpoint() const
{
    return m_serviceEndpoint;
}

void FirehoseReceiver::setServiceEndpoint(const QString &newServiceEndpoint)
{
    m_serviceEndpoint = newServiceEndpoint;
}

FirehoseReceiver::FirehoseReceiverStatus FirehoseReceiver::status() const
{
    return m_status;
}

void FirehoseReceiver::setStatus(FirehoseReceiver::FirehoseReceiverStatus newStatus)
{
    if (m_status == newStatus)
        return;
    m_status = newStatus;
    emit statusChanged(m_status);
}

void FirehoseReceiver::analizeReceivingData(const QJsonObject &json)
{
    static qint64 prev_time = 0;
    qint64 cur_time = m_analysisTimer.elapsed();

    const qint64 diff_time = (cur_time - prev_time);
    const bool update = (diff_time > 1000);
    const QStringList nsids = AbstractPostSelector::getOperationNsid(json);
    for (const auto &nsid : nsids) {
        if (!m_nsidsCount.contains(nsid)) {
            m_nsidsCount[nsid] = 1;
        } else {
            m_nsidsCount[nsid]++;
        }
    }
    if (update) {
        int total = 0;
        int value = 0;
        QDateTime date = QDateTime::fromString(json.value("time").toString(), Qt::ISODateWithMs);
        for (const auto &nsid : m_nsidsCount.keys()) {
            value = 1000 * m_nsidsCount[nsid] / diff_time;
            total += value;
            m_nsidsReceivePerSecond[nsid] = QString::number(value);
            m_nsidsCount[nsid] = 0;
        }
        m_nsidsReceivePerSecond["__total"] = QString::number(total);
        m_nsidsReceivePerSecond["__difference"] =
                QString::number(date.msecsTo(QDateTime::currentDateTimeUtc()));
        emit analysisChanged();
        prev_time = cur_time;
    }
}

void FirehoseReceiver::appendThreadSelector(AbstractPostSelector *selector)
{
    auto t = new QThread();
    m_selectorThreadHash[selector->key()] = t;
    selector->moveToThread(t);
    t->start();

    connect(this, &FirehoseReceiver::judgeSelectionAndReaction, selector,
            &AbstractPostSelector::judgeSelectionAndReaction);
}

void FirehoseReceiver::removeThreadSelector(QObject *parent)
{
    if (parent == nullptr)
        return;
    if (m_selectorThreadHash.contains(parent)) {
        auto t = m_selectorThreadHash[parent];
        m_selectorThreadHash.remove(parent);
        if (t) {
            t->quit();
            t->wait();
        }
        t->deleteLater();
    }
}

QHash<QString, QString> FirehoseReceiver::nsidsReceivePerSecond() const
{
    return m_nsidsReceivePerSecond;
}
}
