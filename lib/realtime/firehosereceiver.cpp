#include "firehosereceiver.h"

#include <QDebug>
// #include <QJsonArray>
// #include <QJsonObject>
#include <QJsonDocument>

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
      m_status(FirehoseReceiverStatus::Disconnected)
{
#ifdef USE_JETSTREAM
    m_serviceEndpoint = "wss://jetstream2.us-west.bsky.network";
#else
    m_serviceEndpoint = "wss://bsky.network";
#endif
    m_wdgTimer.setInterval(60 * 1000);

    connect(&m_client, &ComAtprotoSyncSubscribeReposEx::errorOccured,
            [this](const QString &error, const QString &message) {
                qDebug().noquote() << "Error:" << error << message;
                setStatus(FirehoseReceiverStatus::Error);
                emit errorOccured(error, message);
                emit receivingChanged(false);
            });
    connect(&m_client, &ComAtprotoSyncSubscribeReposEx::received,
            [=](const QString &type, const QJsonObject &json) {
                m_wdgTimer.stop();
                m_wdgTimer.start();
                emit receivingChanged(true);

                if (type != "#commit")
                    return;
                // qDebug().noquote() << "commitDataReceived:" << type << !json.isEmpty();
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
        qDebug().noquote() << "FirehoseTimeout : Nothing was received via Websocket within the "
                              "specified time.";
        setStatus(FirehoseReceiverStatus::Error);
        emit errorOccured("FirehoseTimeout",
                          "Nothing was received via Websocket within the specified time.");
        emit receivingChanged(false);
        stop();
    });
}

FirehoseReceiver::~FirehoseReceiver()
{
    qDebug() << this << "~FirehoseReceiver()";
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
    QUrl url(path
             + "/subscribe?wantedCollections=app.bsky.feed.post&wantedCollections=app.bsky.feed."
               "repost&wantedCollections=app.bsky.graph.follow");
#else
    ComAtprotoSyncSubscribeReposEx::SubScribeMode mode =
            ComAtprotoSyncSubscribeReposEx::SubScribeMode::Firehose;
    QUrl url(path + "/xrpc/com.atproto.sync.subscribeRepos");
#endif
    qDebug().noquote() << "Connect to" << url.toString();
    m_client.open(url, mode);
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
    if (m_selectorHash.contains(selector))
        return;
    m_selectorHash[selector->parent()] = selector;
}

void FirehoseReceiver::removeSelector(QObject *parent)
{
    if (parent == nullptr)
        return;
    if (m_selectorHash.contains(parent)) {
        auto s = m_selectorHash[parent];
        m_selectorHash.remove(parent);
        if (s) {
            qDebug().quote() << "removeSelector" << s << s->type();
            s->deleteLater();
        }
        qDebug().quote() << "remain count" << m_selectorHash.count();
    }
    for (const auto key : m_selectorHash.keys()) {
        if (!m_selectorHash[key]) {
            qDebug() << "already deleted -> clean up";
            m_selectorHash.remove(key);
        }
    }
    if (m_selectorHash.isEmpty()) {
        qDebug().quote() << "stop";
        stop();
    }
}

void FirehoseReceiver::removeAllSelector()
{
    for (auto s : qAsConst(m_selectorHash)) {
        if (s) {
            s->deleteLater();
        }
    }
    m_selectorHash.clear();
    stop();
}

AbstractPostSelector *FirehoseReceiver::getSelector(QObject *parent)
{
    if (parent == nullptr)
        return nullptr;
    if (m_selectorHash.contains(parent)) {
        return m_selectorHash[parent];
    }
    return nullptr;
}

bool FirehoseReceiver::containsSelector(QObject *parent)
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
}
