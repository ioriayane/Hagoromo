#include "firehosereceiver.h"

#include <QDebug>
// #include <QJsonArray>
// #include <QJsonObject>
#include <QJsonDocument>

namespace RealtimeFeed {

FirehoseReceiver::FirehoseReceiver(QObject *parent) : QObject { parent }
{
    m_serviceEndpoint = "wss://bsky.network";

    connect(&m_client, &ComAtprotoSyncSubscribeReposEx::errorOccured,
            [](const QString &error, const QString &message) {
                qDebug().noquote() << "Error:" << error << message;
            });
    connect(&m_client, &ComAtprotoSyncSubscribeReposEx::received,
            [=](const QString &type, const QJsonObject &json) {
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
    connect(&m_client, &ComAtprotoSyncSubscribeReposEx::disconnectFromService,
            [this]() { emit disconnectFromService(); });
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

    QString path = serviceEndpoint();
    if (path.endsWith("/")) {
        path.resize(path.length() - 1);
    }
    QUrl url(path + "/xrpc/com.atproto.sync.subscribeRepos");
    qDebug().noquote() << "Connect to" << url.toString();
    m_client.open(url);
}

void FirehoseReceiver::stop()
{
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

QString FirehoseReceiver::serviceEndpoint() const
{
    return m_serviceEndpoint;
}

void FirehoseReceiver::setServiceEndpoint(const QString &newServiceEndpoint)
{
    m_serviceEndpoint = newServiceEndpoint;
}
}
