#ifndef REALTIMEFEEDSTATUSLISTMODEL_H
#define REALTIMEFEEDSTATUSLISTMODEL_H

#include <QAbstractListModel>

#include "realtime/firehosereceiver.h"

struct FeedStatusData
{
    QString id;
    QString name;
    QString value;
    QString unit;
};

class RealtimeFeedStatusListModel : public QAbstractListModel
{
    Q_OBJECT
public:
    explicit RealtimeFeedStatusListModel(QObject *parent = nullptr);
    ~RealtimeFeedStatusListModel();

    // モデルで提供する項目のルールID的な（QML側へ公開するために大文字で始めること）
    enum RealtimeFeedStatusListModelRoles {
        ModelData = Qt::UserRole + 1,
        NameRole,
        ValueRole,
        UnitRole,
    };
    Q_ENUM(RealtimeFeedStatusListModelRoles)

    int rowCount(const QModelIndex &parent = QModelIndex()) const;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;
    Q_INVOKABLE QVariant
    item(int row, RealtimeFeedStatusListModel::RealtimeFeedStatusListModelRoles role) const;
    Q_INVOKABLE void update(int row,
                            RealtimeFeedStatusListModel::RealtimeFeedStatusListModelRoles role,
                            const QVariant &value);

public slots:
    void receiverStatusChanged(RealtimeFeed::FirehoseReceiver::FirehoseReceiverStatus newStatus);
    void receiverAnalysisChanged();

protected:
    QHash<int, QByteArray> roleNames() const;

private:
    void appendStatusData(const QString &id, const QString &name, const QString &value,
                          const QString &unit);
    QStringList m_feedStatusIds;
    QHash<QString, FeedStatusData> m_feedStatusData;
};

#endif // REALTIMEFEEDSTATUSLISTMODEL_H
