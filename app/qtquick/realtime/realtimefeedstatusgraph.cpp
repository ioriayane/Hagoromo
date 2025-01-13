#include "realtimefeedstatusgraph.h"

#include "realtime/firehosereceiver.h"

#include <QPainter>
#include <QPainterPath>

using namespace RealtimeFeed;

constexpr int GRAPH_POINT_COUNT = 20; // 2以上にすること

RealtimeFeedStatusGraph::RealtimeFeedStatusGraph(QQuickPaintedItem *parent)
    : QQuickPaintedItem(parent), m_maxValue { 0 }
{
    appendGraphData("app.bsky.feed.post", QColor(255, 255, 0));
    appendGraphData("app.bsky.feed.repost", QColor(255, 0, 255));
    appendGraphData("app.bsky.feed.like", QColor(0, 255, 255));
    appendGraphData("app.bsky.graph.follow", QColor(255, 0, 0));
    appendGraphData("app.bsky.graph.listitem", QColor(0, 255, 0));

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

    int d = boundingRect().width() / (GRAPH_POINT_COUNT - 1);

    QHashIterator<QString, QList<int>> i(m_graphData);
    int max_value = m_maxValue + 1;
    int height = boundingRect().height();
    while (i.hasNext()) {
        i.next();
        painter->setPen(m_graphColor.value(i.key(), QColor(128, 128, 128)));

        QPainterPath path;
        int x = 0;
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

    qDebug() << __FUNCTION__ << timer.elapsed();
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
