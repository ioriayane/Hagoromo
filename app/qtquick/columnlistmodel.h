#ifndef COLUMNLISTMODEL_H
#define COLUMNLISTMODEL_H

#include <QAbstractListModel>
#include <QObject>

// key: "abcdef"
//      account_uuid: "{a51d2b54-2a28-40e2-b813-cf6a66f5027b}"
//                     account_index: 1
//                     component_type: "timeline"
enum class ColumnComponentType : int {
    Timeline,
    Notification,
};
Q_DECLARE_METATYPE(ColumnComponentType)

struct ColumnItem
{
    QString key;
    QString account_uuid;
    QString account_index;
    ColumnComponentType component_type;
};

class ColumnListModel : public QAbstractListModel
{
    Q_OBJECT
public:
    explicit ColumnListModel(QObject *parent = nullptr);

    // モデルで提供する項目のルールID的な（QML側へ公開するために大文字で始めること）
    enum ColumnListModelRoles {
        ModelData = Qt::UserRole + 1,
        KeyRole,
        AccountUuidRole,
        AccountIndexRole,
        ComponentTypeRole,
    };
    Q_ENUM(ColumnListModelRoles)
    Q_ENUM(ColumnComponentType)

    int rowCount(const QModelIndex &parent = QModelIndex()) const;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;

    Q_INVOKABLE QVariant item(int row, ColumnListModel::ColumnListModelRoles role) const;
    Q_INVOKABLE void update(int row, ColumnListModel::ColumnListModelRoles role,
                            const QVariant &value);

    Q_INVOKABLE void append(const QString &account_uuid, const QString &account_index,
                            ColumnComponentType component_type);
    Q_INVOKABLE void remove(int row);
    Q_INVOKABLE void removeByKey(const QString &key);
    Q_INVOKABLE bool containsKey(const QString &key) const;

    Q_INVOKABLE void save() const;
    Q_INVOKABLE void load();

protected:
    QHash<int, QByteArray> roleNames() const;

    QList<ColumnItem> m_columnList;
};

#endif // COLUMNLISTMODEL_H
