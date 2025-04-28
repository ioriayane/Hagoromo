#ifndef EMOJILISTMODEL_H
#define EMOJILISTMODEL_H

#include <QAbstractListModel>

struct EmojiData
{
    QString emoji;
    QString description;
};

struct EmojiRowData
{
    QString group_name;
    QList<EmojiData> emojis;
};

class EmojiListModel : public QAbstractListModel
{
    Q_OBJECT

    Q_PROPERTY(
            int columnCount READ columnCount WRITE setColumnCount NOTIFY columnCountChanged FINAL)
    Q_PROPERTY(QStringList selectedEmojis READ selectedEmojis WRITE setSelectedEmojis NOTIFY
                       selectedEmojisChanged FINAL)

public:
    explicit EmojiListModel(QObject *parent = nullptr);

    enum EmojiListModelRoles {
        GroupNameRole = Qt::UserRole + 1,
        EmojisRole,
        EmojisSelectedRole,
        EmojisEnabledRole,
    };
    Q_ENUM(EmojiListModelRoles)

    int rowCount(const QModelIndex &row = QModelIndex()) const;
    QVariant data(const QModelIndex &index, int role) const;
    QVariant item(int row, int column, EmojiListModel::EmojiListModelRoles role) const;
    QHash<int, QByteArray> roleNames() const;

    Q_INVOKABLE void loadFrequentlyUsed();
    Q_INVOKABLE QString getGroupName(int index) const;
    Q_INVOKABLE int getGroupTopRow(int group_index) const;
    Q_INVOKABLE int getGroupIndex(const QString &group_name) const;
    Q_INVOKABLE void setFrequentlyUsed(const QString &emoji);

    int columnCount() const;
    void setColumnCount(int newColumnCount);
    QStringList selectedEmojis() const;
    void setSelectedEmojis(const QStringList &newSelectedEmojis);

signals:
    void columnCountChanged();
    void selectedEmojisChanged();

private:
    void load();
    void saveFrequentlyUsed() const;

    QList<EmojiRowData> m_emojiRowDataList;
    QStringList m_groupList;
    int m_columnCount;
    QHash<QString, int> m_emojiRowDataIndex; // QHash<emoji, row>
    QStringList m_selectedEmojis;
};

#endif // EMOJILISTMODEL_H
