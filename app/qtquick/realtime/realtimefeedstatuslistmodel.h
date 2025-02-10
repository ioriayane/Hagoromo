#ifndef REALTIMEFEEDSTATUSLISTMODEL_H
#define REALTIMEFEEDSTATUSLISTMODEL_H

#include <QAbstractListModel>
#include <QColor>

#include "realtime/firehosereceiver.h"

struct FeedStatusData
{
    QString id;
    QString name;
    QString value;
    QString unit;
    QColor color;
};

class RealtimeFeedStatusListModel : public QAbstractListModel
{
    Q_OBJECT

    Q_PROPERTY(int theme READ theme WRITE setTheme NOTIFY themeChanged FINAL)
    Q_PROPERTY(QString serviceEndpoint READ serviceEndpoint WRITE setServiceEndpoint NOTIFY
                       serviceEndpointChanged FINAL)

public:
    explicit RealtimeFeedStatusListModel(QObject *parent = nullptr);
    ~RealtimeFeedStatusListModel();

    // モデルで提供する項目のルールID的な（QML側へ公開するために大文字で始めること）
    enum RealtimeFeedStatusListModelRoles {
        ModelData = Qt::UserRole + 1,
        NameRole,
        ValueRole,
        UnitRole,
        UseColorRole,
        ColorRole,
    };
    Q_ENUM(RealtimeFeedStatusListModelRoles)

    int rowCount(const QModelIndex &parent = QModelIndex()) const;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;
    Q_INVOKABLE QVariant
    item(int row, RealtimeFeedStatusListModel::RealtimeFeedStatusListModelRoles role) const;
    Q_INVOKABLE void update(int row,
                            RealtimeFeedStatusListModel::RealtimeFeedStatusListModelRoles role,
                            const QVariant &value);

    int theme() const;
    void setTheme(int newTheme);

    QString serviceEndpoint() const;
    void setServiceEndpoint(const QString &newServiceEndpoint);

public slots:
    void receiverStatusChanged(RealtimeFeed::FirehoseReceiver::FirehoseReceiverStatus newStatus);
    void receiverAnalysisChanged();
    void serviceEndpointChangedInFirehose(const QString &endpoint);

signals:
    void themeChanged();
    void serviceEndpointChanged();

protected:
    QHash<int, QByteArray> roleNames() const;

private:
    void appendStatusData(const QString &id, const QString &name, const QString &value,
                          const QString &unit, const QColor &color);
    void updateColorByTheme();
    QStringList m_feedStatusIds;
    QHash<QString, FeedStatusData> m_feedStatusData;
    int m_theme;
    QString m_serviceEndpoint;
};

#endif // REALTIMEFEEDSTATUSLISTMODEL_H
