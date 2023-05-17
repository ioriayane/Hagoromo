#ifndef POSTTHREADLISTMODEL_H
#define POSTTHREADLISTMODEL_H

#include "../atprotocol/lexicons.h"
#include "atpabstractlistmodel.h"
#include <QObject>

class PostThreadListModel : public AtpAbstractListModel
{
    Q_OBJECT

    Q_PROPERTY(QString postThreadUri READ postThreadUri WRITE setPostThreadUri NOTIFY
                       postThreadUriChanged)
public:
    explicit PostThreadListModel(QObject *parent = nullptr);

    // モデルで提供する項目のルールID的な（QML側へ公開するために大文字で始めること）
    enum PostThreadListModelRoles {
        ModelData = Qt::UserRole + 1,
        CidRole,
        UriRole,
        DisplayNameRole,
        HandleRole,
        AvatarRole,
        RecordTextRole,
        ReplyCountRole,
        RepostCountRole,
        LikeCountRole,
        IndexedAtRole,
        EmbedImagesRole,
        EmbedImagesFullRole,

        HasQuoteRecordRole,
        QuoteRecordDisplayNameRole,
        QuoteRecordHandleRole,
        QuoteRecordAvatarRole,
        QuoteRecordRecordTextRole,
        QuoteRecordIndexedAtRole,
        QuoteRecordEmbedImagesRole,
        QuoteRecordEmbedImagesFullRole,

        HasReplyRole,
        ReplyRootCidRole,
        ReplyRootUriRole,
        ReplyParentDisplayNameRole,
        ReplyParentHandleRole,
        IsRepostedByRole,
        RepostedByDisplayNameRole,
        RepostedByHandleRole,
    };
    Q_ENUM(PostThreadListModelRoles)

    int rowCount(const QModelIndex &parent = QModelIndex()) const;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;

    Q_INVOKABLE QVariant item(int row, PostThreadListModel::PostThreadListModelRoles role) const;

    Q_INVOKABLE void getLatest();

    QString postThreadUri() const;
    void setPostThreadUri(const QString &newPostThreadUri);

signals:

    void postThreadUriChanged();

protected:
    QHash<int, QByteArray> roleNames() const;

private:
    QList<QString> m_cidList; // これで取得したポストの順番を管理して実態はm_viewPostHashで管理
    QHash<QString, AtProtocolType::AppBskyFeedDefs::PostView> m_postHash;
    QString m_postThreadUri;

    void copyFrom(const AtProtocolType::AppBskyFeedDefs::ThreadViewPost *thread_view_post);
};

#endif // POSTTHREADLISTMODEL_H
