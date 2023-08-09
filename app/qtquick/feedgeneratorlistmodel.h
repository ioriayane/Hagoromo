#ifndef FEEDGENERATORLISTMODEL_H
#define FEEDGENERATORLISTMODEL_H

#include "atpabstractlistmodel.h"

class FeedGeneratorListModel : public AtpAbstractListModel
{
    Q_OBJECT

    Q_PROPERTY(QString query READ query WRITE setQuery NOTIFY queryChanged)
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
    Q_INVOKABLE void saveGenerator(const QString &uri);
    Q_INVOKABLE void removeGenerator(const QString &uri);

    QString query() const;
    void setQuery(const QString &newQuery);

signals:
    void queryChanged();

protected:
    QHash<int, QByteArray> roleNames() const;
    virtual void finishedDisplayingQueuedPosts() { }
    virtual bool checkVisibility(const QString &cid);
    void getSavedGenerators();
    void putPreferences(const QString &json);

    QString appendGeneratorToPreference(const QString &src_json, const QString &uri) const;
    QString removeGeneratorToPreference(const QString &src_json, const QString &uri) const;

    QHash<QString, AtProtocolType::AppBskyFeedDefs::GeneratorView> m_generatorViewHash;
    QList<QString> m_savedUriList;

private:
    QString m_query;
};

#endif // FEEDGENERATORLISTMODEL_H
