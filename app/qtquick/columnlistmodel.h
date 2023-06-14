#ifndef COLUMNLISTMODEL_H
#define COLUMNLISTMODEL_H

#include <QAbstractListModel>
#include <QObject>

// QMLではマジックナンバーになるので変更しないこと
enum class FeedComponentType : int {
    Timeline = 0,
    Notification = 1,
    SearchPosts = 2,
    SearchProfiles = 3,
    CustomFeed = 4,

    PostThread = 100,
};

struct ColumnItem
{
    QString key;
    QString account_uuid;
    FeedComponentType component_type;
    bool auto_loading = false;
    int loading_interval = 300000; // msec
    int width = 400;
    QString name; // カラム名
    // component_type
    //  == SearchPosts or SearchProfiles : 検索文字列
    //  == CustomFeed : カスタムフィードのat-uri
    QString value;
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
        ComponentTypeRole,
        AutoLoadingRole,
        LoadingIntervalRole,
        WidthRole,
        NameRole,
        ValueRole,
    };
    Q_ENUM(ColumnListModelRoles)
    Q_ENUM(FeedComponentType)

    int rowCount(const QModelIndex &parent = QModelIndex()) const;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;

    Q_INVOKABLE QVariant item(int row, ColumnListModel::ColumnListModelRoles role) const;
    Q_INVOKABLE void update(int row, ColumnListModel::ColumnListModelRoles role,
                            const QVariant &value);

    Q_INVOKABLE void append(const QString &account_uuid, int component_type, bool auto_loading,
                            int interval, int width, const QString &name, const QString &value);
    Q_INVOKABLE void insert(int row, const QString &account_uuid, int component_type,
                            bool auto_loading, int interval, int width, const QString &name,
                            const QString &value);
    Q_INVOKABLE void remove(int row);
    Q_INVOKABLE void removeByKey(const QString &key);
    Q_INVOKABLE bool containsKey(const QString &key) const;
    Q_INVOKABLE int indexOf(const QString &key) const;

    Q_INVOKABLE void save() const;
    Q_INVOKABLE void load();

protected:
    QHash<int, QByteArray> roleNames() const;

    QList<ColumnItem> m_columnList;
};

#endif // COLUMNLISTMODEL_H
