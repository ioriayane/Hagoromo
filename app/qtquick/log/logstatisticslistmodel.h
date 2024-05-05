#ifndef LOGSTATISTICSLISTMODEL_H
#define LOGSTATISTICSLISTMODEL_H

#include <QAbstractListModel>
#include "log/logaccess.h"

class LogStatisticsListModel : public QAbstractListModel
{
    Q_OBJECT

    Q_PROPERTY(bool running READ running WRITE setRunning NOTIFY runningChanged FINAL)
    Q_PROPERTY(QString did READ did WRITE setDid NOTIFY didChanged FINAL)

public:
    explicit LogStatisticsListModel(QObject *parent = nullptr);

    // モデルで提供する項目のルールID的な（QML側へ公開するために大文字で始めること）
    enum LogStatisticsListModelRoles {
        ModelData = Qt::UserRole + 1,
        NameRole,
        CountRole,
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

signals:
    void finished();
    void runningChanged();
    void didChanged();

protected:
    QHash<int, QByteArray> roleNames() const;

private:
    QList<TotalItem> m_statistics;
    bool m_running;
    QString m_did;
};

#endif // LOGSTATISTICSLISTMODEL_H
