#include "firehosereceiver.h"

#include <QDebug>
// #include <QJsonArray>
// #include <QJsonObject>
#include <QJsonDocument>

using AtProtocolInterface::ComAtprotoSyncSubscribeReposEx;

namespace RealtimeFeed {

FirehoseReceiver::FirehoseReceiver(QObject *parent) : QObject { parent }
{
    m_serviceEndpoint = "wss://bsky.network";
    m_wdgTimer.setInterval(60 * 1000);

    connect(&m_client, &ComAtprotoSyncSubscribeReposEx::errorOccured,
            [this](const QString &error, const QString &message) {
                qDebug().noquote() << "Error:" << error << message;
                emit errorOccured(error, message);
                emit receiving(false);
            });
    connect(&m_client, &ComAtprotoSyncSubscribeReposEx::received,
            [=](const QString &type, const QJsonObject &json) {
                m_wdgTimer.stop();
                m_wdgTimer.start();
                emit receiving(true);

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
    connect(&m_client, &ComAtprotoSyncSubscribeReposEx::connectedToService,
            [this]() { emit connectedToService(); });
    connect(&m_client, &ComAtprotoSyncSubscribeReposEx::disconnectFromService, [this]() {
        emit receiving(false);
        emit disconnectFromService();
    });

    connect(&m_wdgTimer, &QTimer::timeout, [this]() {
        qDebug().noquote() << "FirehoseTimeout : Nothing was received via Websocket within the "
                              "specified time.";
        emit errorOccured("FirehoseTimeout",
                          "Nothing was received via Websocket within the specified time.");
        emit receiving(false);
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

#ifndef HAGOROMO_UNIT_TEST
    QString path = serviceEndpoint();
    if (path.endsWith("/")) {
        path.resize(path.length() - 1);
    }
    QUrl url(path + "/xrpc/com.atproto.sync.subscribeRepos");
    qDebug().noquote() << "Connect to" << url.toString();
    m_client.open(url);
    m_wdgTimer.start();
#else
    qDebug().noquote() << "Connect to dummy --- no start on unit test mode";
    emit connectedToService();
#endif
}

void FirehoseReceiver::stop()
{
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
            qDebug().quote() << "removeSelector" << s << s->name();
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

#ifdef HAGOROMO_UNIT_TEST
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
}
