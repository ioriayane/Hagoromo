#ifndef NOTIFICATIONPREFERENCELISTMODEL_H
#define NOTIFICATIONPREFERENCELISTMODEL_H

#include "atprotocol/lexicons.h"
#include "tools/accountmanager.h"

#include <QAbstractListModel>
#include <QObject>

class NotificationPreferenceListModel : public QAbstractListModel
{
    Q_OBJECT

    Q_PROPERTY(bool running READ running NOTIFY runningChanged)
    Q_PROPERTY(bool modified READ modified NOTIFY modifiedChanged)
    Q_PROPERTY(QString account READ account WRITE setAccount NOTIFY accountChanged)

public:
    explicit NotificationPreferenceListModel(QObject *parent = nullptr);

    enum PreferenceRoles {
        TypeRole = Qt::UserRole + 1,
        DisplayNameRole,
        IncludeRole,
        ListRole,
        PushRole,
        CategoryRole,
        EnabledRole,
        DescriptionRole,
        IncludeTypeRole
    };
    Q_ENUM(PreferenceRoles)

    // リストモデルの基本実装
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole) override;
    Qt::ItemFlags flags(const QModelIndex &index) const override;
    QVariant item(int row, PreferenceRoles role) const;
    QHash<int, QByteArray> roleNames() const override;

    enum PreferenceType {
        FollowType,
        LikeType,
        RepostType,
        LikeViaRepostType,
        RepostViaRepostType,
        ChatType,
        ReplyType,
        MentionType,
        QuoteType,
        StarterpackJoinedType,
        SubscribedPostType,
        UnverifiedType,
        VerifiedType
    };
    Q_ENUM(PreferenceType)

    enum PreferenceCategory {
        SocialCategory,      // フォロー、いいね、リポスト等
        InteractionCategory, // メンション、引用、リプライ
        SystemCategory,      // 認証済み/未認証
        ActivityCategory     // スターターパック、サブスクリプション
    };
    Q_ENUM(PreferenceCategory)

    enum IncludeType {
        NoInclude,       // include設定なし
        FollowsInclude,  // all/followsの選択
        AcceptedInclude  // all/acceptedの選択
    };
    Q_ENUM(IncludeType)

    // プロパティのアクセサ
    bool running() const { return m_running; }
    bool modified() const { return m_modified; }
    QString account() const { return m_account.uuid; }

    AtProtocolInterface::AccountData getAccountData() const;
    void setAccount(const QString &uuid);

    // 設定項目の更新メソッド
    Q_INVOKABLE void update(int row, PreferenceRoles role, const QVariant &value);
    Q_INVOKABLE void updateInclude(int index, const QString &include);
    Q_INVOKABLE void updateList(int index, bool list);
    Q_INVOKABLE void updatePush(int index, bool push);

    // 設定の読み込みと保存
    Q_INVOKABLE void load() { loadPreferences(); }
    Q_INVOKABLE void save() { savePreferences(); }
    Q_INVOKABLE void loadPreferences();
    Q_INVOKABLE void savePreferences();

    // 便利メソッド
    Q_INVOKABLE QString getIncludeDisplayName(const QString &include) const;
    Q_INVOKABLE QStringList getAvailableIncludeOptions(int type) const;

signals:
    void runningChanged();
    void modifiedChanged();
    void accountChanged();
    void errorOccurred(const QString &error);
    void preferencesUpdated();

private:
    struct PreferenceItem {
        PreferenceType type;
        QString displayName;
        QString include;
        bool list;
        bool push;
        PreferenceCategory category;
        IncludeType includeType;  // include設定の種類
        bool enabled;     // 設定項目を表示するかどうか
    };

    void setRunning(bool running);
    void setModified(bool modified);
    void setupPreferenceItems();
    void updateFromAtProtocolPreferences(const AtProtocolType::AppBskyNotificationDefs::Preferences &prefs);
    AtProtocolType::AppBskyNotificationDefs::Preferences createAtProtocolPreferences() const;
    QJsonObject createPreferenceJson(const PreferenceItem &item) const;

    QList<PreferenceItem> m_preferenceItems;
    bool m_running;
    bool m_modified;
    AtProtocolInterface::AccountData m_account;
};

#endif // NOTIFICATIONPREFERENCELISTMODEL_H
