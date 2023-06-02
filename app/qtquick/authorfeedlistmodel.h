#ifndef AUTHORFEEDLISTMODEL_H
#define AUTHORFEEDLISTMODEL_H

#include "timelinelistmodel.h"

class AuthorFeedListModel : public TimelineListModel
{
    Q_OBJECT

    Q_PROPERTY(QString authorDid READ authorDid WRITE setAuthorDid NOTIFY authorDidChanged)

public:
    AuthorFeedListModel(QObject *parent = nullptr);

    Q_INVOKABLE void getLatest();

    QString authorDid() const;
    void setAuthorDid(const QString &newAuthorDid);

signals:
    void authorDidChanged();

private:
    QString m_authorDid;
};

#endif // AUTHORFEEDLISTMODEL_H
