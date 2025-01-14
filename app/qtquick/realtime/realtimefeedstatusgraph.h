#ifndef REALTIMEFEEDSTATUSGRAPH_H
#define REALTIMEFEEDSTATUSGRAPH_H

#include <QQuickPaintedItem>

class RealtimeFeedStatusGraph : public QQuickPaintedItem
{
    Q_OBJECT

    Q_PROPERTY(int theme READ theme WRITE setTheme NOTIFY themeChanged FINAL)

public:
    RealtimeFeedStatusGraph(QQuickPaintedItem *parent = nullptr);
    ~RealtimeFeedStatusGraph();

    virtual void paint(QPainter *painter);

    int theme() const;
    void setTheme(int newTheme);

public slots:
    void receiverAnalysisChanged();

signals:
    void themeChanged();

private:
    void appendGraphData(const QString &id, const QColor &color);
    int updateGraphData(const QString &id, const int value);
    void updateColorByTheme();
    QHash<QString, QList<int>> m_graphData; // QHash<nsid, QList<value>>
    QHash<QString, QColor> m_graphColor; // QHash<nsid, color>
    int m_maxValue;
    int m_theme;
};

#endif // REALTIMEFEEDSTATUSGRAPH_H
