#ifndef FEEDTYPELISTMODEL_H
#define FEEDTYPELISTMODEL_H

#include "columnlistmodel.h"
#include "atpabstractlistmodel.h"

#include <QAbstractListModel>

struct FeedTypeItem
{
    QString group;
    FeedComponentType type = FeedComponentType::Timeline;
    AtProtocolType::AppBskyFeedDefs::GeneratorView generator;
    AtProtocolType::AppBskyGraphDefs::ListView list;
    bool editable = false;
};

class FeedTypeListModel : public AtpAbstractListModel
{
    Q_OBJECT
public:
    explicit FeedTypeListModel(QObject *parent = nullptr);

    // モデルで提供する項目のルールID的な（QML側へ公開するために大文字で始めること）
    enum FeedTypeListModelRoles {
        ModelData = Qt::UserRole + 1,
        GroupRole,
        FeedTypeRole,
        DisplayNameRole,
        AvatarRole,
        UriRole,
        CreatorDisplayNameRole,
        EditableRole,
        EnabledRole,
    };
    Q_ENUM(FeedTypeListModelRoles)

    int rowCount(const QModelIndex &parent = QModelIndex()) const;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;
    Q_INVOKABLE QVariant item(int row, FeedTypeListModel::FeedTypeListModelRoles role) const;

    virtual Q_INVOKABLE int indexOf(const QString &cid) const;
    virtual Q_INVOKABLE QString getRecordText(const QString &cid);
    virtual Q_INVOKABLE QString getOfficialUrl() const { return QString(); }
    virtual Q_INVOKABLE QString getItemOfficialUrl(int row) const
    {
        Q_UNUSED(row)
        return QString();
    }

    Q_INVOKABLE void clear();
    Q_INVOKABLE bool getLatest();
    Q_INVOKABLE bool getNext();

    Q_INVOKABLE void reloadRealtimeFeedRules();
    Q_INVOKABLE void removeRealtimeFeedRule(int row);

protected:
    QHash<int, QByteArray> roleNames() const;
    virtual bool aggregateQueuedPosts(const QString &cid, const bool next = false)
    {
        Q_UNUSED(cid)
        Q_UNUSED(next)
        return true;
    }
    virtual bool aggregated(const QString &cid) const
    {
        Q_UNUSED(cid)
        return false;
    }
    virtual void finishedDisplayingQueuedPosts() { }
    virtual bool checkVisibility(const QString &cid);

private:
    void getFeedDetails();
    void getLists();

    QList<QString> m_cueUri;
    QList<FeedTypeItem> m_feedTypeItemList;
};

#endif // FEEDTYPELISTMODEL_H
