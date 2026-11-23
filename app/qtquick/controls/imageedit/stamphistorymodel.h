#ifndef STAMPHISTORYMODEL_H
#define STAMPHISTORYMODEL_H

#include <QAbstractListModel>
#include <QList>
#include <QString>

struct StampHistoryItem
{
    QString id; // 画像の内容のハッシュ（ファイル名にも使う）
    bool pinned = false;
    qint64 lastUsed = 0;
    qint64 pinnedAt = 0;
};

// 投稿画像に貼り付けた画像（スタンプ）の履歴
// 画像は保存先フォルダにPNGでコピーして管理する（元のファイルが消えても使えるように）
// ピン止めしたものは履歴の最大数を超えても削除しない
class StampHistoryModel : public QAbstractListModel
{
    Q_OBJECT

    Q_PROPERTY(int count READ count NOTIFY countChanged FINAL)
    Q_PROPERTY(int pinnedCount READ pinnedCount NOTIFY countChanged FINAL)
    Q_PROPERTY(int maxHistory READ maxHistory CONSTANT FINAL)
    Q_PROPERTY(int maxPinned READ maxPinned CONSTANT FINAL)
    Q_PROPERTY(QString folder READ folder WRITE setFolder NOTIFY folderChanged FINAL)

public:
    explicit StampHistoryModel(QObject *parent = nullptr);

    enum StampHistoryModelRoles {
        IdRole = Qt::UserRole + 1,
        SourceRole,
        PinnedRole,
        LastUsedRole,
    };
    Q_ENUM(StampHistoryModelRoles)

    int rowCount(const QModelIndex &parent = QModelIndex()) const;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;
    Q_INVOKABLE QVariant item(int row, StampHistoryModel::StampHistoryModelRoles role) const;

    // 保存先フォルダから読み込み直す
    Q_INVOKABLE void load();
    // 画像を履歴に追加（使用）してIDを返す。追加できなかったときは空文字
    Q_INVOKABLE QString add(const QString &url);
    // 使用した（履歴の先頭に移動する）
    Q_INVOKABLE void touch(const QString &id);
    // ピン止めの変更。ピン止めの上限を超える場合はfalse
    Q_INVOKABLE bool setPinned(const QString &id, bool pinned);
    Q_INVOKABLE void remove(const QString &id);
    // 画像のURL（file:）
    Q_INVOKABLE QString source(const QString &id) const;

    int count() const;
    int pinnedCount() const;
    int maxHistory() const;
    int maxPinned() const;
    QString folder() const;
    void setFolder(const QString &newFolder);

    static QString defaultFolder();

signals:
    void countChanged();
    void folderChanged();

protected:
    QHash<int, QByteArray> roleNames() const;

private:
    int indexOf(const QString &id) const;
    QString filePath(const QString &id) const;
    qint64 tick();
    void enforceLimit();
    void sortItems();
    void save() const;
    void update();

    QString m_folder;
    QList<StampHistoryItem> m_items;
    qint64 m_lastTick;
};

#endif // STAMPHISTORYMODEL_H
