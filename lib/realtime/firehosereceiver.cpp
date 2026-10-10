#include "firehosereceiver.h"

#include <QDebug>
#include <QJsonArray>
#include <QJsonObject>
#include <QJsonDocument>
#include <QDateTime>

#define USE_JETSTREAM

// 受信データの時刻と現在時刻の差がこれ以下になったら追いついたとみなす(ms)
#define CATCHING_UP_LAG_MSECS (5 * 1000)
// 時計のずれなどで追いついたと判定できないときに打ち切る時間(ms)
#define CATCHING_UP_TIMEOUT_MSECS (3 * 60 * 1000)
// 後から開始するセレクター用に保持する受信データの上限(ポストとリポストで約10分)
#define RETAINED_EVENTS_MAX 60000

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
      m_status(FirehoseReceiverStatus::Disconnected),
      m_receivedDataSize(0),
      m_timeOfReceivedData(0),
      m_lastSeq(0),
      m_initialLookbackMinutes(10),
      m_initialCursorRequested(false),
      m_catchingUp(false),
      m_restartAfterDisconnect(false)
{
#ifdef USE_JETSTREAM
    m_serviceEndpoint = "wss://jetstream.us-west.bsky.network";
    // m_serviceEndpoint = "ws://localhost:19283";
#else
    m_serviceEndpoint = "wss://bsky.network";
#endif
    m_wdgTimer.setInterval(10 * 1000);
    m_analysisTimer.start();

    connect(&m_client, &ComAtprotoSyncSubscribeReposEx::errorOccurred, this,
            [this](const QString &error, const QString &message) {
                qDebug().noquote() << "Error:" << error << message;
                setStatus(FirehoseReceiverStatus::Error);
                emit errorOccurred(error, message);
                emit receivingChanged(false);
            });
    connect(&m_client, &ComAtprotoSyncSubscribeReposEx::received, this,
            [=](const QString &type, const QJsonObject &json, const qsizetype size,
                const QByteArray &message) {
                m_wdgCounter = 0;
                updateReceivedCursorState(json);
                updateCatchingUpState(json);
                emit receivingChanged(true);

                if (type != "#commit")
                    return;
                // セレクターへの通知より前に保持して、受け渡しの漏れと重複を防ぐ
                retainReceivedData(json, message);
                // qDebug().noquote() << "commitDataReceived:" << type << !json.isEmpty();
                analizeReceivingData(json, size);
                emit judgeSelectionAndReaction(json); // スレッドのselectorへ通知
            });
    connect(&m_client, &ComAtprotoSyncSubscribeReposEx::connectedToService, this, [this]() {
        setStatus(FirehoseReceiverStatus::Connected);
        emit connectedToService();
    });
    connect(&m_client, &ComAtprotoSyncSubscribeReposEx::disconnectFromService, this, [this]() {
        setStatus(FirehoseReceiverStatus::Disconnected);
        emit receivingChanged(false);
        emit disconnectFromService();
        if (m_restartAfterDisconnect) {
            m_restartAfterDisconnect = false;
            start();
        }
    });
    connect(&m_client, &ComAtprotoSyncSubscribeReposEx::socketStateChanged, this,
            [this](QAbstractSocket::SocketState state) {
                switch (state) {
                case QAbstractSocket::SocketState::ConnectedState:
                    setStatus(FirehoseReceiverStatus::Connected);
                    break;
                case QAbstractSocket::SocketState::UnconnectedState:
                    setStatus(FirehoseReceiverStatus::Disconnected);
                    break;
                case QAbstractSocket::SocketState::ConnectingState:
                    setStatus(FirehoseReceiverStatus::Connecting);
                    break;
                case QAbstractSocket::SocketState::HostLookupState:
                    setStatus(FirehoseReceiverStatus::HostLookup);
                    break;
                case QAbstractSocket::SocketState::BoundState:
                    setStatus(FirehoseReceiverStatus::Bound);
                    break;
                case QAbstractSocket::SocketState::ClosingState:
                    setStatus(FirehoseReceiverStatus::Closing);
                    break;
                default:
                    break;
                }
            });

    connect(&m_wdgTimer, &QTimer::timeout, this, [this]() {
        if (m_wdgCounter < 3) {
            m_wdgCounter++;
        } else {
            qDebug().noquote() << "FirehoseTimeout : Nothing was received via Websocket within the "
                                  "specified time."
                               << m_wdgCounter;
            if (status() == FirehoseReceiver::FirehoseReceiverStatus::Connected
                || status() == FirehoseReceiver::FirehoseReceiverStatus::Connecting) {
                qDebug().noquote() << "Timeout, stop and start : " << status();
                stop();
            } else {
                qDebug().noquote() << "Timeout, start : " << status();
            }
            start();
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
    m_restartAfterDisconnect = false;
    QString cursor = takeCursor();
    if (!cursor.isEmpty()) {
        cursor = "&cursor=" + cursor;
    }
    ComAtprotoSyncSubscribeReposEx::SubScribeMode mode =
            ComAtprotoSyncSubscribeReposEx::SubScribeMode::JetStream;
    QUrl url(path + "/xrpc/network.bsky.jetstream.subscribeEvents?collections="
             + subscribeCollections().join("&collections=") + "&kinds=commit" + cursor);
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
    for (auto s : std::as_const(m_selectorHash)) {
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

void FirehoseReceiver::activateSelector(QObject *parent, int max_backfill)
{
    AbstractPostSelector *selector = getSelector(parent);
    if (selector == nullptr) {
        return;
    }
    // 保持している受信データを流し終えるまで、セレクターにはリアルタイムの受信データを無視させる
    // 保持とセレクターへの通知はどちらもこのスレッドで行うので、
    // ここまでに通知した受信データはすべて保持しているデータに含まれ、
    // 以降に通知する受信データはセレクターのスレッドで保持データの後に処理される
    selector->setBackfillPending(true);
    selector->setReady(true);
    const QList<RetainedEvent> events = m_retainedEvents;
    qDebug().noquote() << "activateSelector" << selector->type() << "retained" << events.count();
    QMetaObject::invokeMethod(
            selector,
            [selector, events, max_backfill]() {
                QList<QJsonObject> objects;
                for (const auto &event : events) {
                    const QJsonObject json =
                            ComAtprotoSyncSubscribeReposEx::convertJetStreamMessage(event.message);
                    if (!json.isEmpty() && selector->judge(json)) {
                        objects.append(json);
                        // 条件が広いと大半が選択されるので新しいものだけ残す
                        if (objects.count() > max_backfill) {
                            objects.removeFirst();
                        }
                    }
                }
                selector->setBackfillPending(false);
                qDebug().noquote() << "backfilled" << selector->type() << objects.count();
                if (!objects.isEmpty()) {
                    emit selector->backfilled(objects);
                }
            },
            Qt::QueuedConnection);
}

#ifdef QT_DEBUG // HAGOROMO_UNIT_TEST
void FirehoseReceiver::testReceived(const QJsonObject &json)
{
    for (auto s : std::as_const(m_selectorHash)) {
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

void FirehoseReceiver::testUpdateReceivedCursorState(const QJsonObject &json)
{
    updateReceivedCursorState(json);
}

QString FirehoseReceiver::testTakeCursor()
{
    return takeCursor();
}

void FirehoseReceiver::testResetCursorState()
{
    m_timeOfReceivedData = 0;
    m_lastSeq = 0;
    m_initialCursorRequested = false;
    setCatchingUp(false);
}

void FirehoseReceiver::testUpdateCatchingUpState(const QJsonObject &json)
{
    updateCatchingUpState(json);
}

void FirehoseReceiver::testSetCatchingUp(bool newCatchingUp)
{
    setCatchingUp(newCatchingUp);
}

QStringList FirehoseReceiver::testSubscribeCollections() const
{
    return subscribeCollections();
}

void FirehoseReceiver::testRetainReceivedData(const QByteArray &message)
{
    retainReceivedData(ComAtprotoSyncSubscribeReposEx::convertJetStreamMessage(message), message);
}

int FirehoseReceiver::testRetainedEventCount() const
{
    return m_retainedEvents.count();
}

void FirehoseReceiver::testClearRetainedEvents()
{
    m_retainedEvents.clear();
}
#endif

QString FirehoseReceiver::serviceEndpoint() const
{
    return m_serviceEndpoint;
}

void FirehoseReceiver::setServiceEndpoint(const QString &newServiceEndpoint)
{
    if (m_serviceEndpoint == newServiceEndpoint)
        return;
    m_serviceEndpoint = newServiceEndpoint;

    emit serviceEndpointChanged(m_serviceEndpoint);
}

void FirehoseReceiver::changeServiceEndpoint(const QString &newServiceEndpoint)
{
    if (m_serviceEndpoint == newServiceEndpoint)
        return;
    m_serviceEndpoint = newServiceEndpoint;

    emit serviceEndpointChanged(m_serviceEndpoint);

    if (status() == FirehoseReceiver::FirehoseReceiverStatus::Connected
        || status() == FirehoseReceiver::FirehoseReceiverStatus::Connecting) {
        // 今現在、接続している場合のみ停止→開始をする
        // そのためstate()の確認のあとにstop()をする必要がある
        qDebug().noquote() << "Change firehose endpoint and restart:" << m_serviceEndpoint;
        stop();
        start();
    } else {
        qDebug().noquote() << "Change firehose endpoint:" << m_serviceEndpoint;
        stop();
    }
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

void FirehoseReceiver::analizeReceivingData(const QJsonObject &json, const qsizetype size)
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
    m_receivedDataSize += size;

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
        m_nsidsReceivePerSecond["__date_time"] = date.toString("MM/dd hh:mm:ss");
        m_nsidsReceivePerSecond["__difference"] =
                QString::number(date.msecsTo(QDateTime::currentDateTimeUtc()));
        m_nsidsReceivePerSecond["__bit_per_sec"] =
                QString::number(m_receivedDataSize * 8 / diff_time / 1000.0, 'f', 1);
        m_receivedDataSize = 0;
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

void FirehoseReceiver::updateReceivedCursorState(const QJsonObject &json)
{
    // さかのぼり受信中はイベントの時刻が過去になるため、再開可否はローカルの受信時刻で判断する
    m_timeOfReceivedData = QDateTime::currentMSecsSinceEpoch();
    if (json.contains("seq")) {
        m_lastSeq = json.value("seq").toVariant().toLongLong();
    }
}

QString FirehoseReceiver::getCursor() const
{
    if (m_timeOfReceivedData == 0 || m_lastSeq <= 0)
        return QString();
    qint64 now = QDateTime::currentMSecsSinceEpoch();
    qDebug().noquote() << "getCursor:"
                       << " now :" << now;
    qDebug().noquote() << "getCursor:"
                       << " time:" << m_timeOfReceivedData;
    qDebug().noquote() << "getCursor:"
                       << " diff:" << (now - m_timeOfReceivedData);
    if ((now < m_timeOfReceivedData) || ((now - m_timeOfReceivedData) > (5 * 60 * 1000)))
        return QString();
    return QString::number(m_lastSeq + 1);
}

QString FirehoseReceiver::getInitialCursor() const
{
    // JetStreamのcursorはunixマイクロ秒のタイムスタンプも指定できる(値の大きさで判別される)
    const qint64 lookback = static_cast<qint64>(m_initialLookbackMinutes) * 60 * 1000;
    const qint64 time =
            (QDateTime::currentMSecsSinceEpoch() - lookback) * static_cast<qint64>(1000);
    qDebug().noquote() << "getInitialCursor:" << time;
    return QString::number(time);
}

QString FirehoseReceiver::takeCursor()
{
    QString cursor = getCursor();
    if (cursor.isEmpty() && m_lastSeq <= 0 && !m_initialCursorRequested
        && m_initialLookbackMinutes > 0) {
        // 起動後最初の接続のみさかのぼって受信する
        // 失敗して再接続するときは通常の処理(リアルタイムから受信)に戻す
        m_initialCursorRequested = true;
        cursor = getInitialCursor();
        m_catchingUpTimer.start();
        setCatchingUp(true);
    } else if (cursor.isEmpty()) {
        // リアルタイムから受信するので追いつく必要がない
        setCatchingUp(false);
    }
    return cursor;
}

void FirehoseReceiver::setCatchingUp(bool newCatchingUp)
{
    if (m_catchingUp == newCatchingUp)
        return;
    m_catchingUp = newCatchingUp;
    qDebug().noquote() << "catchingUp:" << m_catchingUp;
    emit catchingUpChanged(m_catchingUp);
}

void FirehoseReceiver::updateCatchingUpState(const QJsonObject &json)
{
    if (!m_catchingUp)
        return;
    const QDateTime time = QDateTime::fromString(json.value("time").toString(), Qt::ISODateWithMs);
    if (!time.isValid())
        return;
    const qint64 lag = time.msecsTo(QDateTime::currentDateTimeUtc());
    if (lag > CATCHING_UP_LAG_MSECS && m_catchingUpTimer.elapsed() < CATCHING_UP_TIMEOUT_MSECS)
        return;

    qDebug().noquote() << "Caught up : lag" << lag << "ms, elapsed" << m_catchingUpTimer.elapsed()
                       << "ms";
    setCatchingUp(false);

    if (status() == FirehoseReceiver::FirehoseReceiverStatus::Connected
        || status() == FirehoseReceiver::FirehoseReceiverStatus::Connecting) {
        // いいねを含めた購読に切り替えるため、続きから再接続する
        m_restartAfterDisconnect = true;
        stop();
        // 切断が通知されなかったときは監視タイマーで再接続する
        m_wdgCounter = 0;
        m_wdgTimer.start();
    }
}

void FirehoseReceiver::retainReceivedData(const QJsonObject &json, const QByteArray &message)
{
    if (m_initialLookbackMinutes <= 0 || message.isEmpty())
        return;
    const QJsonArray ops = json.value("ops").toArray();
    if (ops.isEmpty())
        return;
    const QJsonObject op = ops.first().toObject();
    if (op.value("action").toString() != "create")
        return;
    const QString path = op.value("path").toString();
    if (!path.startsWith("app.bsky.feed.post/") && !path.startsWith("app.bsky.feed.repost/"))
        return;
    const QDateTime time = QDateTime::fromString(json.value("time").toString(), Qt::ISODateWithMs);
    if (!time.isValid())
        return;

    RetainedEvent event;
    event.time = time.toMSecsSinceEpoch();
    // 受信時のバッファは実際のサイズより大きく確保されていることがあるのでコピーして詰める
    event.message = QByteArray(message.constData(), message.size());
    m_retainedEvents.append(event);

    const qint64 oldest = event.time - static_cast<qint64>(m_initialLookbackMinutes) * 60 * 1000;
    while (!m_retainedEvents.isEmpty()
           && (m_retainedEvents.count() > RETAINED_EVENTS_MAX
               || m_retainedEvents.first().time < oldest)) {
        m_retainedEvents.removeFirst();
    }
}

QStringList FirehoseReceiver::subscribeCollections() const
{
    QStringList collections;
    collections << "app.bsky.feed.post"
                << "app.bsky.feed.repost";
    if (!m_catchingUp) {
        // 追いつくまでのいいねはポスト取得時のカウントに含まれるため使わない
        // (受信量の大半を占めるので除外して負荷を下げる)
        collections << "app.bsky.feed.like";
    }
    collections << "app.bsky.graph.follow"
                << "app.bsky.graph.listitem";
    return collections;
}

QHash<QString, QString> FirehoseReceiver::nsidsReceivePerSecond() const
{
    return m_nsidsReceivePerSecond;
}

bool FirehoseReceiver::catchingUp() const
{
    return m_catchingUp;
}

int FirehoseReceiver::initialLookbackMinutes() const
{
    return m_initialLookbackMinutes;
}

void FirehoseReceiver::setInitialLookbackMinutes(int newInitialLookbackMinutes)
{
    // さかのぼり受信は起動後最初の接続にのみ反映される
    // 後から開始するセレクター用に保持する受信データの期間にも使う
    m_initialLookbackMinutes = newInitialLookbackMinutes < 0 ? 0 : newInitialLookbackMinutes;
    if (m_initialLookbackMinutes == 0) {
        m_retainedEvents.clear();
    }
}
}
