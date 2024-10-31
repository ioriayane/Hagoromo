#ifndef EMBEDIMAGELISTMODEL_H
#define EMBEDIMAGELISTMODEL_H

#include <QAbstractListModel>

struct EmbedImageItem
{
    QString uri;
    QString alt;
};

class EmbedImageListModel : public QAbstractListModel
{
    Q_OBJECT

    Q_PROPERTY(int count READ count NOTIFY countChanged FINAL)
    Q_PROPERTY(bool running READ running WRITE setRunning NOTIFY runningChanged FINAL)
public:
    explicit EmbedImageListModel(QObject *parent = nullptr);

    // モデルで提供する項目のルールID的な（QML側へ公開するために大文字で始めること）
    enum EmbedImageListModelRoles {
        ModelData = Qt::UserRole + 1,
        UriRole,
        AltRole,
        NumberRole,
    };
    Q_ENUM(EmbedImageListModelRoles)

    int rowCount(const QModelIndex &parent = QModelIndex()) const;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;

    Q_INVOKABLE QVariant item(int row, EmbedImageListModel::EmbedImageListModelRoles role) const;
    Q_INVOKABLE void update(int row, EmbedImageListModel::EmbedImageListModelRoles role,
                            const QVariant &value);

    Q_INVOKABLE void clear();
    Q_INVOKABLE void remove(const int row);
    Q_INVOKABLE bool append(const QStringList &uris);
    Q_INVOKABLE void updateAlt(const int row, const QString &alt);

    Q_INVOKABLE QStringList uris() const;
    Q_INVOKABLE QStringList alts() const;

    int count() const;
    void setCount(int newCount);

    bool running() const;
    void setRunning(bool newRunning);

protected:
    QHash<int, QByteArray> roleNames() const;

signals:
    void countChanged();

    void runningChanged();

private:
    void appendItem();

    QStringList m_uriCue;
    QList<EmbedImageItem> m_embedImageList;
    int m_count;
    bool m_running;
};

#endif // EMBEDIMAGELISTMODEL_H
