#include "firehosereceiver.h"

#include <QDebug>

namespace RealtimeFeed {

FirehoseReceiver::FirehoseReceiver(QObject *parent) : QObject { parent }
{
    m_serviceEndpoint = "wss://bsky.network";

    QObject::connect(&m_client, &ComAtprotoSyncSubscribeReposEx::errorOccured,
                     [](const QString &error, const QString &message) {
                         qDebug().noquote() << "Error:" << error << message;
                     });
    QObject::connect(&m_client, &ComAtprotoSyncSubscribeReposEx::received,
                     [=](const QString &type, const QJsonObject &json) {
                         qDebug().noquote() << "commitDataReceived:" << type << !json.isEmpty();
                         for (auto s : qAsConst(m_selectorHash)) {
                             if (s->judge(json)) {
                                 emit s->selected(json);
                             }
                         }
                     });
}

FirehoseReceiver::~FirehoseReceiver()
{
    qDebug() << this << "~FirehoseReceiver()";
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
    m_client.open(QUrl(path + "/xrpc/com.atproto.sync.subscribeRepos"));
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
        m_selectorHash.remove(parent);
    }
    if (m_selectorHash.isEmpty()) {
        stop();
    }
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
