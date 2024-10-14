#ifndef BLOGENTRYLISTMODEL_H
#define BLOGENTRYLISTMODEL_H

#include "atprotocol/accessatprotocol.h"

#include <QAbstractListModel>

class BlogEntryListModel : public QAbstractListModel
{
    Q_OBJECT
    Q_PROPERTY(bool running READ running WRITE setRunning NOTIFY runningChanged)
    Q_PROPERTY(QString targetHandle READ targetHandle WRITE setTargetHandle NOTIFY
                       targetHandleChanged FINAL)
    Q_PROPERTY(QString targetDid READ targetDid WRITE setTargetDid NOTIFY targetDidChanged)
    Q_PROPERTY(QString targetServiceEndpoint READ targetServiceEndpoint WRITE
                       setTargetServiceEndpoint NOTIFY targetServiceEndpointChanged FINAL)

public:
    explicit BlogEntryListModel(QObject *parent = nullptr);

    enum BlogEntryListModelRoles {
        ModelData = Qt::UserRole + 1,

        CidRole,
        UriRole,

        ServiceNameRole,
        TitleRole,
        ContentRole,
        CreatedAtRole,
        VisibilityRole,
        PermalinkRole,
    };
    Q_ENUM(BlogEntryListModelRoles)

    int rowCount(const QModelIndex &parent = QModelIndex()) const;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;

    Q_INVOKABLE QVariant item(int row, BlogEntryListModel::BlogEntryListModelRoles role) const;

    AtProtocolInterface::AccountData account() const;
    Q_INVOKABLE void setAccount(const QString &service, const QString &did, const QString &handle,
                                const QString &email, const QString &accessJwt,
                                const QString &refreshJwt);

    Q_INVOKABLE bool getLatest();

    bool running() const;
    void setRunning(bool newRunning);
    QString targetDid() const;
    void setTargetDid(const QString &newTargetDid);
    QString targetServiceEndpoint() const;
    void setTargetServiceEndpoint(const QString &newTargetServiceEndpoint);

    QString targetHandle() const;
    void setTargetHandle(const QString &newTargetHandle);

signals:
    void errorOccured(const QString &code, const QString &message);
    void runningChanged();
    void targetDidChanged();
    void countChanged();
    void targetServiceEndpointChanged();

    void targetHandleChanged();

protected:
    QHash<int, QByteArray> roleNames() const;

private:
    QVariant itemFromWhiteWind(int row, BlogEntryListModel::BlogEntryListModelRoles role) const;
    QVariant itemFromLinkat(int row, BlogEntryListModel::BlogEntryListModelRoles role) const;
    void getLatestFromWhiteWind(std::function<void(const bool &)> callback);
    void getLatestFromLinkat(std::function<void(const bool &)> callback);

    QList<AtProtocolType::ComAtprotoRepoListRecords::Record> m_blogEntryRecordList;
    AtProtocolInterface::AccountData m_account;
    bool m_running;
    QString m_targetDid;
    QString m_targetServiceEndpoint;
    QString m_targetHandle;
};

#endif // BLOGENTRYLISTMODEL_H
