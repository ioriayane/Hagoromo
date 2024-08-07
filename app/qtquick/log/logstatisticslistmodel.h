#ifndef LOGSTATISTICSLISTMODEL_H
#define LOGSTATISTICSLISTMODEL_H

#include <QAbstractListModel>
#include "log/logaccess.h"

class LogStatisticsListModel : public QAbstractListModel
{
    Q_OBJECT

    Q_PROPERTY(bool running READ running WRITE setRunning NOTIFY runningChanged FINAL)
    Q_PROPERTY(QString did READ did WRITE setDid NOTIFY didChanged FINAL)
    Q_PROPERTY(int totalMax READ totalMax WRITE setTotalMax NOTIFY totalMaxChanged)
public:
    explicit LogStatisticsListModel(QObject *parent = nullptr);

    // モデルで提供する項目のルールID的な（QML側へ公開するために大文字で始めること）
    enum LogStatisticsListModelRoles {
        ModelData = Qt::UserRole + 1,
        GroupRole,
        NameRole,
        CountRole,
        PercentRole,
    };
    Q_ENUM(LogStatisticsListModelRoles)

    int rowCount(const QModelIndex &parent = QModelIndex()) const;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;

    Q_INVOKABLE QVariant item(int row,
                              LogStatisticsListModel::LogStatisticsListModelRoles role) const;

    Q_INVOKABLE void getLatest();
    Q_INVOKABLE void clear();

    bool running() const;
    void setRunning(bool newRunning);
    QString did() const;
    void setDid(const QString &newDid);
    int totalMax() const;
    void setTotalMax(int newTotalMax);

signals:
    void finished();
    void runningChanged();
    void didChanged();
    void totalMaxChanged();

protected:
    QHash<int, QByteArray> roleNames() const;
    QList<TotalItem> m_totalList;

private:
    bool m_running;
    QString m_did;
    int m_totalMax;
};

#endif // LOGSTATISTICSLISTMODEL_H
