#ifndef REALTIMEFEEDSTATUSGRAPH_H
#define REALTIMEFEEDSTATUSGRAPH_H

#include <QQuickPaintedItem>

class RealtimeFeedStatusGraph : public QQuickPaintedItem
{
    Q_OBJECT
public:
    RealtimeFeedStatusGraph(QQuickPaintedItem *parent = nullptr);
    ~RealtimeFeedStatusGraph();

    virtual void paint(QPainter *painter);

public slots:
    void receiverAnalysisChanged();

private:
    void appendGraphData(const QString &id, const QColor &color);
    int updateGraphData(const QString &id, const int value);
    QHash<QString, QList<int>> m_graphData; // QHash<nsid, QList<value>>
    QHash<QString, QColor> m_graphColor; // QHash<nsid, color>
    int m_maxValue;
};

#endif // REALTIMEFEEDSTATUSGRAPH_H
