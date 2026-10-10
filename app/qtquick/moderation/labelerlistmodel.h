#ifndef LABELERLISTMODEL_H
#define LABELERLISTMODEL_H

#include "atprotocol/lexicons.h"

#include <QAbstractListModel>

class LabelerListModel : public QAbstractListModel
{
    Q_OBJECT

    Q_PROPERTY(bool running READ running WRITE setRunning NOTIFY runningChanged)
    // アカウントのuuid
    Q_PROPERTY(QString account READ account WRITE setAccount NOTIFY accountChanged)

public:
    explicit LabelerListModel(QObject *parent = nullptr);

    // モデルで提供する項目のルールID的な（QML側へ公開するために大文字で始めること）
    enum LabelerListModelRoles {
        ModelData = Qt::UserRole + 1,
        DidRole,
        TitleRole,
        DescriptionRole,
        StatusRole,
        LevelRole,
        IsAdultImageryRole,
        ConfigurableRole,
    };
    Q_ENUM(LabelerListModelRoles)

    int rowCount(const QModelIndex &parent = QModelIndex()) const;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;

    Q_INVOKABLE QVariant item(int row, LabelerListModel::LabelerListModelRoles role) const;

    Q_INVOKABLE void load();

    bool running() const;
    void setRunning(bool newRunning);
    QString account() const;
    void setAccount(const QString &uuid);

signals:
    void finished();
    void errorOccurred(const QString &code, const QString &message);
    void runningChanged();
    void accountChanged();

protected:
    QHash<int, QByteArray> roleNames() const;

private:
    QList<AtProtocolType::AppBskyLabelerDefs::LabelerView> m_labelerList;
    bool m_running;
    QString m_account;
};

#endif // LABELERLISTMODEL_H
