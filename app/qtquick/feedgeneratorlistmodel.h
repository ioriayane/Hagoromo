#ifndef FEEDGENERATORLISTMODEL_H
#define FEEDGENERATORLISTMODEL_H

#include "atpabstractlistmodel.h"

class FeedGeneratorListModel : public AtpAbstractListModel
{
    Q_OBJECT
public:
    explicit FeedGeneratorListModel(QObject *parent = nullptr);

    // モデルで提供する項目のルールID的な（QML側へ公開するために大文字で始めること）
    enum FeedGeneratorListModelRoles {
        ModelData = Qt::UserRole + 1,
        UriRole,
        CidRole,
        CreatorHandleRole,
        CreatorDisplayNameRole,
        CreatorAvatarRole,
        DisplayNameRole,
        DescriptionRole,
        LikeCountRole,
        AvatarRole,
        SavingRole,
    };
    Q_ENUM(FeedGeneratorListModelRoles)

    int rowCount(const QModelIndex &parent = QModelIndex()) const;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;
    Q_INVOKABLE QVariant item(int row,
                              FeedGeneratorListModel::FeedGeneratorListModelRoles role) const;

    virtual Q_INVOKABLE int indexOf(const QString &cid) const;
    virtual Q_INVOKABLE QString getRecordText(const QString &cid);

    Q_INVOKABLE void clear();
    Q_INVOKABLE void getLatest();

protected:
    QHash<int, QByteArray> roleNames() const;
    virtual void finishedDisplayingQueuedPosts() {};
    void getSavedGenerators();

    QHash<QString, AtProtocolType::AppBskyFeedDefs::GeneratorView> m_generatorViewHash;
    QList<QString> m_savedUriList;
};

#endif // FEEDGENERATORLISTMODEL_H
