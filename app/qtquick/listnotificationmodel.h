#ifndef LISTNOTIFICATIONMODEL_H
#define LISTNOTIFICATIONMODEL_H

//#include "../atprotocol/lexicons.h"
#include "../atprotocol/accessatprotocol.h"

#include <QAbstractListModel>
#include <QObject>

class ListNotificationModel : public QAbstractListModel
{
    Q_OBJECT
public:
    explicit ListNotificationModel(QObject *parent = nullptr);

    // モデルで提供する項目のルールID的な（QML側へ公開するために大文字で始めること）
    enum ListNotificationModelRoles {
        ModelData = Qt::UserRole + 1,
        DisplayNameRole,
    };
    Q_ENUM(ListNotificationModelRoles)

    int rowCount(const QModelIndex &parent = QModelIndex()) const;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;

    Q_INVOKABLE QVariant item(int row,
                              ListNotificationModel::ListNotificationModelRoles role) const;

    Q_INVOKABLE void setAccount(const QString &service, const QString &did, const QString &handle,
                                const QString &email, const QString &accessJwt,
                                const QString &refreshJwt);

    Q_INVOKABLE void getLatest();

protected:
    QHash<int, QByteArray> roleNames() const;

    AtProtocolInterface::AccountData m_account;
};

#endif // LISTNOTIFICATIONMODEL_H
