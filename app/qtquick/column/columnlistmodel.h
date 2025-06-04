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
    AuthorFeed = 5,
    ListFeed = 6,
    ChatList = 7,
    ChatMessageList = 8,
    RealtimeFeed = 9,

    PostThread = 100,
    DiscoverFeeds = 101,
    EditRealtimeFeed = 102,
};

// QMLではマジックナンバーになるので変更しないこと
enum class ImageLayoutType : int {
    Compact = 0,
    Normal = 1,
    WhenOneIsWhole = 2,
    AllWhole = 3,
};

struct ColumnPostType
{
    bool like = true;
    bool repost = true;
    bool follow = true;
    bool mention = true;
    bool reply = true;
    bool quote = true;
    bool like_via_repost = true;
    bool repost_via_repost = true;
    bool reply_to_unfollowed_users = true;
    bool repost_of_own = true;
    bool repost_of_following_users = true;
    bool repost_of_unfollowing_users = true;
    bool repost_of_mine = true;
    bool repost_by_me = true;
};

struct ColumnItem
{
    int position = -1; // カラムの表示位置
    QString key;
    QString account_uuid;
    FeedComponentType component_type;
    bool auto_loading = false;
    int loading_interval = 300000; // msec
    int width = 400;
    ImageLayoutType image_layout_type = ImageLayoutType::Normal;

    QString name; // カラム名
    // component_type
    //  == SearchPosts or SearchProfiles : 検索文字列
    //  == CustomFeed : カスタムフィードのat-uri
    //  == AuthorFeed : ユーザーのDID
    QString value;
    // component_type
    //  == ChatMessageList : チャット相手のdidリスト
    QStringList value_list;

    ColumnPostType type_visibility;
    bool aggregate_reactions = true;
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
        ImageLayoutTypeRole,
        NameRole,
        ValueRole,
        ValueListRole,

        SelectedRole,

        VisibleLikeRole,
        VisibleRepostRole,
        VisibleFollowRole,
        VisibleMentionRole,
        VisibleReplyRole,
        VisibleQuoteRole,
        VisibleLikeViaRepostRole,
        VisibleRepostViaRepostRole,
        VisibleReplyToUnfollowedUsersRole,
        VisibleRepostOfOwnRole,
        VisibleRepostOfFollowingUsersRole,
        VisibleRepostOfUnfollowingUsersRole,
        VisibleRepostOfMineRole,
        VisibleRepostByMeRole,

        AggregateReactionsRole,
    };
    Q_ENUM(ColumnListModelRoles)
    Q_ENUM(FeedComponentType)
    enum MoveDirection {
        MoveLeft,
        MoveRight,
    };
    Q_ENUM(MoveDirection)

    int rowCount(const QModelIndex &parent = QModelIndex()) const;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;

    Q_INVOKABLE QVariant item(int row, ColumnListModel::ColumnListModelRoles role) const;
    Q_INVOKABLE void update(int row, ColumnListModel::ColumnListModelRoles role,
                            const QVariant &value);

    Q_INVOKABLE void append(const QString &account_uuid, int component_type, bool auto_loading,
                            int interval, int width, int image_layout_type, const QString &name,
                            const QString &value, const QStringList &value_list);
    Q_INVOKABLE void insert(int row, const QString &account_uuid, int component_type,
                            bool auto_loading, int interval, int width, int image_layout_type,
                            const QString &name, const QString &value,
                            const QStringList &value_list);
    Q_INVOKABLE int insertNext(const QString &key, const QString &account_uuid, int component_type,
                               bool auto_loading, int interval, int width, int image_layout_type,
                               const QString &name, const QString &value,
                               const QStringList &value_list);
    Q_INVOKABLE void move(const QString &key, const ColumnListModel::MoveDirection direction);
    Q_INVOKABLE int moveSelectionToLeft();
    Q_INVOKABLE int moveSelectionToRight();
    Q_INVOKABLE void moveSelection(int position);
    Q_INVOKABLE void remove(int row);
    Q_INVOKABLE void removeByKey(const QString &key);
    Q_INVOKABLE bool containsKey(const QString &key) const;
    Q_INVOKABLE int indexOf(const QString &key) const;
    Q_INVOKABLE int indexOf(const QString &account_uuid, int component_type, int start = 0) const;
    Q_INVOKABLE bool contains(const QString &account_uuid, int component_type) const;

    Q_INVOKABLE int getPreviousRow(const int row);
    Q_INVOKABLE int getPosition(const int row) const;
    Q_INVOKABLE QList<int> getRowListInOrderOfPosition() const;

    Q_INVOKABLE void save() const;
    Q_INVOKABLE void load();

signals:
    void positionChanged();

protected:
    QHash<int, QByteArray> roleNames() const;
    bool validateIndex();

    QList<ColumnItem> m_columnList;
    int m_selectedPosition;
};

#endif // COLUMNLISTMODEL_H
