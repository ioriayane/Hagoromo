#include "realtimefeedstatusgraph.h"

#include "realtime/firehosereceiver.h"

#include <QPainter>
#include <QPainterPath>

using namespace RealtimeFeed;

constexpr int GRAPH_POINT_COUNT = 60; // 2以上にすること

RealtimeFeedStatusGraph::RealtimeFeedStatusGraph(QQuickPaintedItem *parent)
    : QQuickPaintedItem(parent), m_maxValue { 0 }, m_theme { 0 }
{
    appendGraphData("app.bsky.feed.post", QColor(255, 255, 0));
    appendGraphData("app.bsky.feed.repost", QColor(255, 0, 255));
    appendGraphData("app.bsky.feed.like", QColor(0, 255, 255));
    appendGraphData("app.bsky.graph.follow", QColor(255, 0, 0));
    appendGraphData("app.bsky.graph.listitem", QColor(0, 255, 0));

    updateColorByTheme();

    FirehoseReceiver *receiver = FirehoseReceiver::getInstance();
    connect(receiver, &FirehoseReceiver::analysisChanged, this,
            &RealtimeFeedStatusGraph::receiverAnalysisChanged);
}

RealtimeFeedStatusGraph::~RealtimeFeedStatusGraph()
{
    FirehoseReceiver *receiver = FirehoseReceiver::getInstance();
    disconnect(receiver, &FirehoseReceiver::analysisChanged, this,
               &RealtimeFeedStatusGraph::receiverAnalysisChanged);
}

void RealtimeFeedStatusGraph::paint(QPainter *painter)
{
    QElapsedTimer timer;
    timer.start();

    qreal d = boundingRect().width() / static_cast<qreal>(GRAPH_POINT_COUNT - 1);

    painter->setPen(QColor(128, 128, 128));
    painter->drawRect(boundingRect());

    QHashIterator<QString, QList<int>> i(m_graphData);
    int max_value = m_maxValue + 1;
    int height = boundingRect().height();
    while (i.hasNext()) {
        i.next();
        painter->setPen(m_graphColor.value(i.key(), QColor(128, 128, 128)));

        QPainterPath path;
        qreal x = 0;
        for (const auto &value : i.value()) {
            if (x == 0) {
                path.moveTo(x, height - height * value / max_value);
            } else {
                path.lineTo(x, height - height * value / max_value);
            }
            x += d;
        }
        painter->drawPath(path);
    }
}

void RealtimeFeedStatusGraph::receiverAnalysisChanged()
{

    FirehoseReceiver *receiver = FirehoseReceiver::getInstance();
    const QHash<QString, QString> nsids_rps = receiver->nsidsReceivePerSecond();

    QHashIterator<QString, QString> i(nsids_rps);
    m_maxValue = 0;
    int max = 0;
    while (i.hasNext()) {
        i.next();
        const int value = i.value().toInt();
        max = updateGraphData(i.key(), value);
        m_maxValue = qMax(max, m_maxValue);
    }

    // 再描画
    update();
}

void RealtimeFeedStatusGraph::appendGraphData(const QString &id, const QColor &color)
{
    m_graphColor[id] = color;
    m_graphData[id].clear();
    for (int i = 0; i < GRAPH_POINT_COUNT; i++) {
        m_graphData[id].append(0);
    }
}

int RealtimeFeedStatusGraph::updateGraphData(const QString &id, const int value)
{
    if (m_graphData[id].isEmpty()) {
        m_graphData[id].append(value);
    } else {
        m_graphData[id].pop_front();
        m_graphData[id].push_back(value);
    }
    int max = 0;
    if (!id.startsWith("_")) {
        for (const auto &v : m_graphData[id]) {
            max = qMax(v, max);
        }
    }
    return max;
}

void RealtimeFeedStatusGraph::updateColorByTheme()
{
    if (theme() == 0) {
        // Light
        m_graphColor["app.bsky.feed.post"] = QColor(0x3F, 0x51, 0xB5); // Indigo
        m_graphColor["app.bsky.feed.repost"] = QColor(0x4C, 0xAF, 0x50); // Green
        m_graphColor["app.bsky.feed.like"] = QColor(0xE9, 0x1E, 0x63); // Pink
        m_graphColor["app.bsky.graph.follow"] = QColor(0x03, 0xA9, 0xF4); // LightBlue
        m_graphColor["app.bsky.graph.listitem"] = QColor(0xFF, 0x98, 0x0); // Orange
    } else {
        // Dark
        m_graphColor["app.bsky.feed.post"] = QColor(0x9F, 0xA8, 0xDA);
        m_graphColor["app.bsky.feed.repost"] = QColor(0xA5, 0xD6, 0xA7);
        m_graphColor["app.bsky.feed.like"] = QColor(0xF4, 0x8F, 0xB1);
        m_graphColor["app.bsky.graph.follow"] = QColor(0x81, 0xD4, 0xFA);
        m_graphColor["app.bsky.graph.listitem"] = QColor(0xFF, 0xCC, 0x80);
    }
}

int RealtimeFeedStatusGraph::theme() const
{
    return m_theme;
}

void RealtimeFeedStatusGraph::setTheme(int newTheme)
{
    if (m_theme == newTheme)
        return;
    m_theme = newTheme;
    emit themeChanged();

    updateColorByTheme();
}
