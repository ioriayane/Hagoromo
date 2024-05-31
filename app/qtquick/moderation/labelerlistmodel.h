#ifndef LABELERLISTMODEL_H
#define LABELERLISTMODEL_H

#include "atprotocol/lexicons.h"

#include <QAbstractListModel>

class LabelerListModel : public QAbstractListModel
{
    Q_OBJECT

    Q_PROPERTY(bool running READ running WRITE setRunning NOTIFY runningChanged)
    Q_PROPERTY(QString service READ service WRITE setService NOTIFY serviceChanged)
    Q_PROPERTY(QString handle READ handle WRITE setHandle NOTIFY handleChanged)
    Q_PROPERTY(QString accessJwt READ accessJwt WRITE setAccessJwt NOTIFY accessJwtChanged)

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
    QString service() const;
    void setService(const QString &newService);
    QString handle() const;
    void setHandle(const QString &newHandle);
    QString accessJwt() const;
    void setAccessJwt(const QString &newAccessJwt);

signals:
    void finished();
    void errorOccured(const QString &code, const QString &message);
    void runningChanged();
    void serviceChanged();
    void handleChanged();
    void accessJwtChanged();

protected:
    QHash<int, QByteArray> roleNames() const;

private:
    QList<AtProtocolType::AppBskyLabelerDefs::LabelerView> m_labelerList;
    bool m_running;
    QString m_service;
    QString m_handle;
    QString m_accessJwt;
};

#endif // LABELERLISTMODEL_H
