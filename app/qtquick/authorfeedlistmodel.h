#ifndef AUTHORFEEDLISTMODEL_H
#define AUTHORFEEDLISTMODEL_H

#include "timelinelistmodel.h"

class AuthorFeedListModel : public TimelineListModel
{
    Q_OBJECT

    Q_PROPERTY(QString authorDid READ authorDid WRITE setAuthorDid NOTIFY authorDidChanged)
    Q_PROPERTY(
            AuthorFeedListModelFilterType filter READ filter WRITE setFilter NOTIFY filterChanged)

public:
    AuthorFeedListModel(QObject *parent = nullptr);

    enum AuthorFeedListModelFilterType {
        PostsWithReplies,
        PostsNoReplies,
        PostsWithMedia,
    };
    Q_ENUM(AuthorFeedListModelFilterType)

    Q_INVOKABLE void getLatest();

    QString authorDid() const;
    void setAuthorDid(const QString &newAuthorDid);

    AuthorFeedListModelFilterType filter() const;
    void setFilter(AuthorFeedListModelFilterType newFilter);

signals:
    void authorDidChanged();

    void filterChanged();

private:
    QString m_authorDid;
    AuthorFeedListModelFilterType m_filter;
};

#endif // AUTHORFEEDLISTMODEL_H
