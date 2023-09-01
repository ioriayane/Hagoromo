#ifndef FEEDTYPELISTMODEL_H
#define FEEDTYPELISTMODEL_H

#include "columnlistmodel.h"
#include "atpabstractlistmodel.h"

#include <QAbstractListModel>

struct FeedTypeItem
{
    FeedComponentType type = FeedComponentType::Timeline;
    AtProtocolType::AppBskyFeedDefs::GeneratorView generator;
};

class FeedTypeListModel : public AtpAbstractListModel
{
    Q_OBJECT
public:
    explicit FeedTypeListModel(QObject *parent = nullptr);

    // モデルで提供する項目のルールID的な（QML側へ公開するために大文字で始めること）
    enum FeedTypeListModelRoles {
        ModelData = Qt::UserRole + 1,
        FeedTypeRole,
        DisplayNameRole,
        AvatarRole,
        UriRole,
        CreatorDisplayNameRole,
    };
    Q_ENUM(FeedTypeListModelRoles)

    int rowCount(const QModelIndex &parent = QModelIndex()) const;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;
    Q_INVOKABLE QVariant item(int row, FeedTypeListModel::FeedTypeListModelRoles role) const;

    virtual Q_INVOKABLE int indexOf(const QString &cid) const;
    virtual Q_INVOKABLE QString getRecordText(const QString &cid);

    Q_INVOKABLE void clear();
    Q_INVOKABLE void getLatest();
    Q_INVOKABLE void getNext();

protected:
    QHash<int, QByteArray> roleNames() const;
    virtual void finishedDisplayingQueuedPosts() { }
    virtual bool checkVisibility(const QString &cid);

private:
    void getFeedDetails();

    QList<QString> m_cueUri;
    QList<FeedTypeItem> m_feedTypeItemList;
};

#endif // FEEDTYPELISTMODEL_H
